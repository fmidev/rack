
import json
import sys # sys.stderr
import re
import shlex

from pathlib import Path

import logging

import rack
logging.basicConfig(format='%(levelname)s:\t: %(message)s')
logger = logging.getLogger(__name__) 
logger.setLevel(logging.INFO)

def add_parameters(parser, path_prefix=None):
    """ Creates registry of supported options of this script
    parser = argparse.ArgumentParser(description="Example app with JSON config support")
    """

    # Use load_config(args.config)
    parser.add_argument(
        "--config",
        help="Path to JSON config file")
    
    parser.add_argument(
        "--export-config",
        default=None,
        help="Save configuration to file")


# No good (why so?)
def read_defaults(parser):
    """Parse args with precedence:
       CLI > JSON config > defaults
    """
    # First parse known args to see if --config is given
    args, remaining_argv = parser.parse_known_args()

    if args.config:
        config = read(args.config, False)
        args.config = None
        parser.set_defaults(**config)


def read_if_found(filename, formats:list = []) -> dict: # todo path prefix?
    for fmt in formats:
        path = Path(filename).with_suffix(fmt)
        logger.info(f"Checking config file: {path}")
        if path.is_file():
            logger.info(f"Found config file: {path}")
            return read(path, False)
    logger.warning(f"No config file found for {filename} with formats {formats}")
    return {}

def resolve_path(confname: str, confpath_syntax: str = "conf/{key}.json") -> tuple:
    """Resolve the key for this configuration  (a bare KEY or a path like <prefix>-<KEY>.<ext>)
    to (key, geoconf_dict), without touching args/parser state.
    """

    filepath = Path(confname)
    key = filepath.name

    m = re.search('^[^A-Z]*([A-Z]+[A-Z0-9_-]*[A-Z0-9])?[^A-Z]*', key)
    if not m:
        raise ValueError(f'smart CONF: could not extract KEY from argument: {confname}')

    # Todo: check ambiguity (several keys in path, or several files with same key but different extension)
    if key == m.group(1):
        if len(m.groups()) > 1:
            logger.warning(f"Ambiguous key extraction from {confname}: {m.groups()}")
        # Nothing removed - plain key given.
        filepath = Path(confpath_syntax.format(key=key))
        #logger.info(f"Reading conf(s) '{key}' -> {filepath.parent}{filepath.stem}{formats}")
    else:
        # Adopt keyword "reduced" from filepath.
        key = m.group(1)
    
    return key, filepath

def read_smart(confname: str, confpath_syntax: str = "conf/{key}.json") -> tuple:
    """Resolve the key for this configuration  (a bare KEY or a path like <prefix>-<KEY>.<ext>)
    to (key, geoconf_dict), without touching args/parser state.
    """

    # filepath = Path(confname)
    # key = filepath.name

    key, filepath = resolve_path(confname, confpath_syntax)

    if filepath.suffix:
        #ormats = [filepath.suffix]
        logger.info(f"Reading conf(s) '{key}' -> {filepath}")
        conf = rack.config.read(filepath)
    else:
        formats = ['.json', '.cnf']
        logger.info(f"Reading conf(s) '{key}' -> {filepath.parent}{filepath.stem}.{formats}")
        conf = rack.config.read_if_found(filepath, formats)
            
    return key, conf


def read(filename, lenient=False) -> dict: # todo path prefix?
    """Load config file.
    
        Parse the file as either JSON file or simple "key=value" file.
    """

    path = Path(filename)
    
    if not path.is_file():
        msg = f"File not found: {filename}"
        if lenient:
            logger.warning(msg)
            return {}
        else:
            logger.error(msg)
            raise Exception(msg)
        
    with open(path, "r") as f:
        lines = f.readlines()
        return parse(lines)
        #return json.load(f)
    
    logger.error(f"Reading file failed")
    # raise Exception(msg)
    return {}
    
    
PARSE_ERROR  = -1
PARSE_SKIP   =  0
PARSE_APPEND = 1

def parse(lines: list, dst: object=None, handleMissing = PARSE_SKIP) -> dict:
    """ Try to parse text lines primarily as JSON and then with key=value syntax.

        param strict: if True, raise error if
    """

    KEY_VALUE_RE = re.compile(r"^\s*(?:(?P<key>([a-zA-Z][a-zA-Z0-9_]*)))\s*=\s*(?:(?P<value>.+))$")
    conf={}
        
    if lines[0].strip().startswith('{'):
        try:
            logger.debug("Parsing as JSON")
            jsonDecoder = json.JSONDecoder()
            text = "".join(lines)
            conf = jsonDecoder.decode(text)
        except json.JSONDecodeError as e:
            i=1
            for line in lines:
                print(f"{i} {line}")
                i += 1
            logger.error(e)
            exit(-1)
    else:
        logger.debug("Parsing as key=value pairs")
        for line in lines:
            line = line.split("#", 1)[0] # Remove comments
            line = line.strip()
            if not line:
                continue
            m = KEY_VALUE_RE.match(line)
            if m:
                key = m.group('key')
                value = m.group('value')
                #logger.debug(f"{key} = {value}")
                value = shlex.split(value, comments=True)
                #logger.warning(value)
                value = " ".join(value)
                conf[key] = value # todo auto type detect?
            else:
                logger.error(f"could not parse: {line}")

    if dst:

        if not isinstance(dst, dict): # dangerous?
            dst = vars(dst)

        for (k,v) in conf.items():
            if (k in dst) or (handleMissing==PARSE_APPEND):
                dst[k] = v
            elif (handleMissing==PARSE_ERROR):
                logger.error(f"dst = {dst}")
                raise KeyError(f"Key '{k}' not found")
        logger.warning(dst)
    return conf
    #logger.info(f"JSON conf({block.arg}): {conf}")

def write(filename, conf:dict, exclude=[]):

    conf_copy = {}

    for k,v in conf.items():
        
        if k in {'config'}:
            continue

        if k.startswith('export_'): # command
            continue

        if v is None:
            continue
        
        

        conf_copy[k] = v

    if (filename == "-") or (filename.lower() == "stdout"):
        json.dump(conf_copy, sys.stdout, indent=4)
        sys.stdout.write("\n")
    else:
        with open(filename, "w") as f:
            json.dump(conf_copy, f, indent=4)





