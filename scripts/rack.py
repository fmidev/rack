""" Rack command line argument generator

Utility for constructing strings that can be executed in shell.

    * property
    * property2

"""
import argparse
import json
import sys
from pathlib import Path
import os
import re # GEOCONF filename-KEY extraction

from types import SimpleNamespace

import logging
logging.basicConfig(format='%(levelname)s\t %(name)s: %(message)s')
logger = logging.getLogger("rack.py") # change to __NAME__ etc
logger.setLevel(logging.INFO)


# Global
default_tiledir  = 'tiles/'
default_tilename = '${what:date}${what:time}-GEOCONF={GEOCONF}_${NOD}-${what:product}-${what:prodpar}-${what:quantity}.h5'

def build_parser():
    """ Creates registry of supported options of this script
    """
    parser = argparse.ArgumentParser(description="Example app with JSON config support")

    """
    parser.add_argument("--host", default="localhost", help="Server hostname")
    parser.add_argument("--port", type=int, default=8080, help="Server port")
    parser.add_argument("--debug", action="store_true", help="Enable debug mode")
    parser.add_argument("--timeout", type=float, default=5.0, help="Request timeout (seconds)")
    parser.add_argument("--logfile", default=None, help="Path to log file")
    """

    parser.add_argument(
        "INFILE",
        nargs='*',
        help="Input files")

    parser.add_argument(
        "--OUTFILE",
        default="",
        help="Output file (basename). See --FORMAT")
    

    """ Geographical information

    """
    parser.add_argument(
        "--GEOCONF",
        metavar="<KEY>|<filepath>-<KEY>.json>",
        help="Read BBOX, PROJ, SIZE from file, default: geoconf/geoconf-<KEY>.json")  # FMI Scandinavia

    parser.add_argument(
        "--BBOX",
        default='6,51.3,49,70.2',
        metavar="<lonLL,latLL,lonUR,latUR>",
        help="Bounding box [cBBox]")  # FMI Scandinavia

    parser.add_argument(
        "--PROJ",
        default="+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs",
        help="")   # Same as epsg:4326

    parser.add_argument(
        "--SIZE",
        default="800,800",
        help="") 

    parser.add_argument(
        "--METHOD",
        default="MAXIMUM",
        help="Compositing method. See: rack -h cMethod") 

    parser.add_argument(
        "--PPROD",
        metavar="<pProd>[,<args>]|<quantity>",
        help="Meteorological product. See: rack -h products") 

    """
    Selection
    """
    parser.add_argument(
        "--SELECT",
        default=None,
        help="") 

    parser.add_argument(
        "--QUANTITY",
        default='DBZH',
        help="Extracted quantity") 

    parser.add_argument(
        "--DATASET",
        default='',
        help="Extracted quantity") 

    parser.add_argument(
        "--PALETTE",
        default='default',
        help="Add colours using a palette. Affects PNG image only.")

    # parser.add_argument("--PROCESSES", default='4', help="Apply ") 
    parser.add_argument(
        "--FORMAT",
        default='h5',
        help="One or several file formats (h5, png, tif)") 

    parser.add_argument(
        "--SCHEME",
        default='',
        metavar="<empty>|TILE|TILED",
        help="Compositing scheme. For TILE, default OUTDIR={default_tiledir}, OUTFILE={default_tiledir}") 


    #parser.add_argument("--loop", type=str, help="<file>.json Path to JSON config file")
    parser.add_argument(
        "--INDIR",
        type=str,
        metavar="<path>|AUTO",
        default="AUTO",
        help="Common path of input files.")

    parser.add_argument(
        "--OUTDIR",
        type=str,
        metavar="<path>|AUTO",
        default=None,
        help="Common path of output files.")

    """
    parser.add_argument(
        "--tiledir",
        type=str,
        metavar="<dirpath>",
        default="tiles",
        help="Directory for reading and writing tiles.")

    parser.add_argument(
        "--tilename",
        type=str,
        metavar="<filename_syntax>",
        default="${what:date}${what:time}-${NOD}-${what:product}-${what:prodpar}-${what:quantity}.h5",
        help="Directory for reading and writing tiles.")
    """
    
    # Optional config file
    parser.add_argument(
        "--config",
        help="Path to JSON config file")
    
    # CONFFILE=mposite-${CONF:+$CONF}.cnf"
    parser.add_argument(
        "--export-config",
        default=None,
        help="Save configuration to file")

    parser.add_argument(
        "-n", "--newline",
        type=str,
        metavar="<chars>",
        default=" \\\n",
        help="Argument separator for the resulting command string.")
    
    parser.add_argument(
        "-e", "--exec",
        action='store_true',
        help="execute parsed command")

    parser.add_argument(
        "-d", "--debug",
        action='store_true',
        help="same as --log_level DEBUG")

    parser.add_argument(
        "-v", "--verbose",
        action='store_true',
        help="same as --log_level VERBOSE")

    parser.add_argument(
        "-l", "--log_level",
        help="verbosity level for python wrapper and Rack cmd")

    parser.add_argument(
        "-p", "--print",
        action='store_true',
        help="print parsed command")

    parser.add_argument(
        "-T", "--TIMESTAMP",
        type=str,
        metavar="<YYYYmmddHHMM>",
        default="",
        help="loop variable (separate with commas)")

    parser.add_argument(
        "-S", "--SITE",
        type=str,
        default="",
        help="loop variable (separate with commas)")

    return parser

""" Utils etc

"""


"""    
def traverse_loop(loop, conf, inputList:list, routineList:list, outputList:list):
    if not loop:
        return
    k,v = loops.popitem()
    if k == 'SITE':
        inputList.append()
"""

class safe_dict(dict):
    def __missing__(self, key):
        return '{' + key + '}'

"""
Simplifies lists and dictionaries for command line use.
Converts ['A','b',7] to A,b,7 .

Strips chars '[', ']', '{', '}'  
"""
def arg2str(arg, separator=","):
    if (type(arg)==list) or (type(arg)==tuple):
        return separator.join([str(i) for i in arg])
    elif type(arg)==dict:
        return separator.join([f"{k}={v}" for (k,v) in arg.items()])
    else:
        return str(arg) #str(type(arg)) + 

    
def get_defaults(parser):
    return {a.dest: a.default for a in parser._actions if a.dest != 'help'}


def export_defaults_to_json(parser, args, filename="config_template.json"):
    """Write all parser defaults to a JSON file."""
    logger.debug(f'Writing defaults to a JSON file: {filename}')
    
    defaults = get_defaults(parser)
    conf = {}
    for k,v in vars(args).items():
        if k == 'export_config':
            continue
        if v is None:
            continue
        conf[k] = v

    
    #{a.dest: a.default for a in parser._actions if a.dest != 'help'}
    with open(filename, "w") as f:
        json.dump(conf, f, indent=4)
        #json.dump(defaults, f, indent=4)
    logger.info(f"✅ Config template written to: {filename}")


def load_config(filename):
    """Load JSON config if it exists."""
    path = Path(filename)
    if not path.is_file():
        print(f"⚠️ File not found: {filename}", file=sys.stderr)
        return {}
    with open(path, "r") as f:
        return json.load(f)


def read_default_args(parser):
    """Parse args with precedence:
       CLI > JSON config > defaults
    """
    # First parse known args to see if --config is given
    args, remaining_argv = parser.parse_known_args()

    if args.config:
        config = load_config(args.config)
        parser.set_defaults(**config)

    # Re-parse all args with updated defaults
    # final_args = parser.parse_args()
    # return final_args

def read_geoconf(args): #, parser):

    # First, assume it is a full path.
    filepath = Path(args.GEOCONF)
    args.GEOCONF = str(filepath.name)
    
    m = re.search('^[^A-Z]*([A-Z]+[A-Z0-9_-]*[A-Z0-9])?[^A-Z]*', args.GEOCONF)
    if m:
        # Nothing removed - plain key given?
        if args.GEOCONF == m.group(1):
            filepath = Path(f'geoconf/geoconf-{args.GEOCONF}.json')
        else:
            # Adopt keyword "reduced" from filepath.
            args.GEOCONF = m.group(1)
    else:
        Exception('--GEOCONF: could not extract KEY from filename: ', key)

    
    geoconf = load_config(filepath)
    vars(args).update(geoconf)
    logger.warn(f"Geoconf: {args.GEOCONF}")
    return geoconf
    
    # parser.set_defaults(**geoconf)
    
    # print ("GEOCONF: ", args.GEOCONF, " = ", filepath)

    

def append_geoconf(cmdList:list, conf:dict):
    geo_dict = {'cSize':'SIZE', 'cProj':'PROJ', 'cBBox':'BBOX', 'cMethod':'CMETHOD'}
    # consider argument-less options: val is None
    for (key,confkey) in geo_dict.items():
        if (confkey in conf):
            cmdList.append("--{key} '{val}'".format(key=key, val=arg2str(conf[confkey])))



def append_outputs(cmdList:list, output_basename:str, formats: list, output_prefix=None):

    
    if len(formats) > 1:
        if not output_prefix:
            output_prefix = str(Path(output_basename).parent)
            #print ("prefix: ", type(output_prefix))
            if (output_prefix == '.'):
                output_prefix = None

        if output_prefix:
            #output_prefix = str(output_prefix) + '/'
            output_prefix += '/'
            cmdList.append(f"--outputPrefix {output_prefix}")
            output_basename = output_basename.removeprefix(output_prefix)
            

    #cmdList.append(f"--echo {formats}")
    if not formats:
        fmt = output_basename.split('.').pop()
        formats = [ fmt ]
        output_basename.removesuffix(fmt)
    
    #file_suffix = 
    fmts = [];
    fmts.extend(formats)
    if 'h5' in fmts:
        fmts.remove('h5')
        cmdList.append(f"--outputFile {output_basename}.h5")

    if 'tif' in fmts:
        fmts.remove('tif')
        cmdList.append(f"--outputConf tif:tile=512 -o {output_basename}.tif")

    if 'png' in fmts:
        fmts.remove('png')
        cmdList.append(f"--palette default -o {output_basename}.png")

    
    if (fmts):
        raise Exception('Unhandled formats:', fmts)
        
    return cmdList
            


def extract_prefix(paths: list, shortPaths=None):
    str_paths = [str(p) for p in paths]
    prefix = os.path.commonpath(str_paths)
    if (prefix):
        prefix += '/'
        #if not (shortPaths is None):
        #    shortPaths.extend([str(p).removeprefix(prefix) for p in paths])
    shortPaths.extend([str(p).removeprefix(prefix) for p in paths])
    return prefix

def expand_string(inputSet, key, values):
    """ Given
    """
    if type(inputSet) is not set:
        inputSet = set(inputSet) # also converts a list to a set

    key = '{'+key+'}'
        
    if type(values) is str:
        values = values.split(',')

    result = set()
    for x in inputSet:
        x = str(x)
        for v in values:
            result.add(x.replace(key, v))
            
    return result
        
    
    

def handle_tilepath_defaults(dirpath, filepath) -> (str, str):
    if not filepath:
        filepath = default_tilename
    else:
        if type(filepath) == list:
            if (len(filepath) > 1):
                raise Exception('Multiple outputs not supported by SCHEME=TILE: ', filepath)
            filepath = filepath[0]
        filepath = Path(filepath)
        tiledir = str(filepath.parent)
        if (tiledir == '.'):
            if dirpath:
                dirpath = Path(dirpath)
        else:
            if dirpath:
                # Append
                dirpath = Path(dirpath, tiledir)
            # Strip
            filepath = str(filepath.name)

    if not dirpath:
        dirpath = default_tiledir
            
    return (str(dirpath).removesuffix('/')+'/', filepath)


def compose_command(args):
    """Main library entry point.

    Args:
        args (argparse.Namespace): Parsed arguments from `build_parser()`
    """

    if isinstance(args, dict):
        args = argparse.Namespace(**args)


    cmdList = ['rack']

    if (args.debug):
        args.log_level = logging.DEBUG
    elif (args.verbose):
        args.log_level = logging.VERBOSE
    
    if (args.log_level):
        if hasattr(logging, args.log_level):
            logger.setLevel(getattr(logging, args.log_level))
        else:
            logger.setLevel(int(args.log_level))
        cmdList.append(f"--verbose '{args.log_level}'")
   
    # Example usage
    #if args.debug:
    #    print("Debug mode enabled")

    #assert hasattr(args, "rack") and hasattr(args, "version"), "Missing required arguments"
    #print(f"Using {args.rack}:{args.version}")

    # Settings
    if args.GEOCONF:
        read_geoconf(args) #, parser)
        # Save reduced value and refresh
        #geoconf = args.GEOCONF
        #args = parser.parse_args()
        #args.GEOCONF = geoconf
        
    arg_vars = vars(args)

    # "MAIN"
    logger.warning(f"Geoconf: {args.GEOCONF}")

    cmdRoutine = []
    # Outputs

    append_geoconf(cmdList, arg_vars)
    # print (cmdList)
    
    #print("🔧 Final configuration:")
    print(vars(args))
    logger.debug("🔧 Final configuration:")
    for key, value in vars(args).items():
        logger.debug(f"  {key}: {value}")


    #print (f"=== Full cmd for {timestamp}-{site} ===")
    if (args.PPROD):
        (key,value) = args.PPROD.split(',',1)
        if value is None:
            cmdRoutine.append(f"--{key}")
        else:
            cmdRoutine.append(f"--{key} '{value}'")

    """ Compositing
    """
    if args.SCHEME in ['TILE','TILED']:
        if not args.GEOCONF:
            # raise Exception('Compositing SCHEME=[TILE|TILED] requires GEOCONF for labelling tile files')
            # print('Using --GEOCONF recommended if compositing SCHEME=[TILE|TILED]', file=sys.stderr)
            logger.note('Using --GEOCONF recommended if compositing SCHEME=[TILE|TILED]')
    
    if (args.SCHEME == 'TILE'):
        (dirpath,filepath) = handle_tilepath_defaults(args.OUTDIR, args.OUTFILE)
        print(dirpath,filepath)
        args.OUTDIR  = dirpath # .removesuffix('/')
        #args.OUTFILE = filepath.replace('{GEOCONF}', str(args.GEOCONF))
        cmdList.append(f"--outputPrefix '{args.OUTDIR}'")
        cmdRoutine.append(f"--cCreateTile -o '{args.OUTFILE}'")
    elif (args.SCHEME == 'TILED'):

        (dirpath,filepath) = handle_tilepath_defaults(args.INDIR, args.INFILE)
        if not args.INDIR:
            args.INDIR = dirpath # default_tiledir
        args.INDIR = args.INDIR.removesuffix('/')
        if not args.INFILE:
            args.INFILE = filepath #default_tilename.replace('{GEOCONF}', args.GEOCONF)
    else:
        cmdRoutine.append("--cAdd")

    if not args.OUTFILE:
        args.OUTFILE = 'out.h5'

    print ("OUTFILE", type(args.OUTFILE), args.OUTFILE)
    
    if (args.INFILE):

        print ("INFILE", type(args.INFILE), args.INFILE)
        if type(args.INFILE) == str:
            args.INFILE = [ args.INFILE ]
        #args.OUTFILE = filepath.replace('{GEOCONF}', str(args.GEOCONF))

        if (args.GEOCONF):
            # Note: should not contain comma...?
            args.INFILE  = expand_string(args.INFILE, "GEOCONF", args.GEOCONF)
            print ("INFILE", args.INFILE)

        if (args.SITE):
            args.INFILE  = expand_string(args.INFILE, "SITE", args.SITE)
            print ("INFILE", args.INFILE)

        # Probably makes little sense in oper. use
        if (args.TIMESTAMP):
            args.INFILE  = expand_string(args.INFILE, "TIMESTAMP", args.TIMESTAMP)

        
        if len(args.INFILE)==1 :
            cmdList.extend(args.INFILE) # list of 1 elem
            cmdList.extend(cmdRoutine)
        else:
            shortPaths=[]
            if (args.INDIR == 'AUTO'):
                args.INDIR = extract_prefix(args.INFILE, shortPaths)
            if (args.INDIR):
                cmdList.append(f'--inputPrefix "{args.INDIR}"')
            cmdRoutine = " ".join(cmdRoutine).replace("'",'"')
            cmdList.append(f"--script '{cmdRoutine}'")
            cmdList.extend(shortPaths)
            #print (shortPaths)

    if (args.OUTFILE):

        if type(args.OUTFILE) == str:
            args.OUTFILE = [ args.OUTFILE ]
            #print ("OUTFILE", type(args.OUTFILE), args.OUTFILE)
            
        if (args.SITE):
            args.OUTFILE  = expand_string(args.OUTFILE, "SITE", args.SITE)
            #print ("OUTFILE", type(args.OUTFILE), args.OUTFILE)

        # Probably makes little sense in oper. use
        if (args.TIMESTAMP):
            args.OUTFILE  = expand_string(args.OUTFILE, "TIMESTAMP", args.TIMESTAMP)

        #if (args.SCHEME == 'TILE') and (len(args.OUTFILE) > 1):
        if len(args.OUTFILE) > 1:
            #print(args.OUTFILE, file=sys.stderr)
            raise Exception(f'Multiple outputs not supported : {args.OUTFILE}') # by SCHEME=TILE

        args.OUTFILE = args.OUTFILE.pop() # set
            
        if (args.SCHEME != 'TILE'):
            cmdList.append("--cExtract DATA,WEIGHT")
            append_outputs(cmdList, args.OUTFILE, args.FORMAT.split(','), args.OUTDIR) # None)

    return args.newline.join(cmdList)
    #print (cmd)
    #print ("\n\n")

def exec_command(args):
    cmd = compose_cmd(args)
    os.system(cmd)
    
def main():

    parser = build_parser()

    read_default_args(parser)

    args = parser.parse_args()

    # Selected commands only for direct command line use

    # Needs parser for arg definitions, args for current values

    # Export template if user requests it
    if args.export_config:
        export_defaults_to_json(parser, args, args.export_config)
        sys.exit(0)

    cmd = compose_command(args)

    if args.print:
        print(cmd)

    if (args.exec):
        os.system(cmd)

    

if __name__ == "__main__":
    main()
