
import json
import sys # sys.stderr
from pathlib import Path



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



def read(filename): # todo path prefix?
    """Load JSON config if it exists."""
    path = Path(filename)
    if not path.is_file():
        print(f"⚠️  File not found: {filename}", file=sys.stderr)
        return {}
    with open(path, "r") as f:
        return json.load(f)

def write(filename, conf:dict, exclude=[]):

    conf_copy = {}

    for k,v in conf.items():
        if k in {'config', 'export_config'}:
            continue
        if v is None:
            continue
        conf_copy[k] = v

    
    with open(filename, "w") as f:
        json.dump(conf_copy, f, indent=4)
        #json.dump(defaults, f, indent=4)
        #logger.info(f"✅ Config template written to: {filename}")




