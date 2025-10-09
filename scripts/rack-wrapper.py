import argparse
import json
import sys
from pathlib import Path
import os

def build_parser():
    parser = argparse.ArgumentParser(description="Example app with JSON config support")

    """
    parser.add_argument("--host", default="localhost", help="Server hostname")
    parser.add_argument("--port", type=int, default=8080, help="Server port")
    parser.add_argument("--debug", action="store_true", help="Enable debug mode")
    parser.add_argument("--timeout", type=float, default=5.0, help="Request timeout (seconds)")
    parser.add_argument("--logfile", default=None, help="Path to log file")
    """

    parser.add_argument("inputFiles", nargs='*', help="Input files")
    
    
    # RACK=${RACK:-'rack'}
    parser.add_argument("--BBOX", default='6,51.3,49,70.2', metavar="<lonLL,latLL,lonUR,latUR>", help="Bounding box [cBBox]")  # FMI Scandinavia
    parser.add_argument("--PROJ", default="+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs", help="")   # Same as epsg:4326
    parser.add_argument("--SIZE", default="800,800", help="") 
    parser.add_argument("--METHOD", default="MAXIMUM", help="Compositing method. See: rack -h cMethod") 
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
        help="Compositing scheme (h5, png, tif)") 

    # Optional config file
    parser.add_argument("--config", help="Path to JSON config file")
    # CONFFILE=mposite-${CONF:+$CONF}.cnf"
    parser.add_argument("--export-config", default=None, help="Save configuration to file")

    #parser.add_argument("--loop", type=str, help="<file>.json Path to JSON config file")
    parser.add_argument(
        "--inputPrefix",
        type=str,
        metavar="<path>|AUTO",
        default="AUTO",
        help="Common path of input files.")

    parser.add_argument(
        "--outputPrefix",
        type=str,
        metavar="<path>|AUTO",
        default="AUTO",
        help="Common path of output files.")

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

    parser.add_argument(
        "--newline",
        type=str,
        metavar="<chars>",
        default=" \\\n",
        help="Argument separator for the resulting command string.")
    
    parser.add_argument(
        "-e", "--exec", action='store_true',
        help="execute parsed command")

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
    print(f"✅ Config template written to: {filename}")


def load_config(filename):
    """Load JSON config if it exists."""
    path = Path(filename)
    if not path.is_file():
        print(f"⚠️ File not found: {filename}", file=sys.stderr)
        return {}
    with open(path, "r") as f:
        return json.load(f)


def merge_args(parser):
    """Parse args with precedence:
       CLI > JSON config > defaults
    """
    # 1️⃣ First parse known args to see if --config is given
    args, remaining_argv = parser.parse_known_args()

    config = {}
    if args.config:
        config = load_config(args.config)
        parser.set_defaults(**config)

    # 2️⃣ Re-parse all args with updated defaults
    final_args = parser.parse_args()

    return final_args

def append_geoconf(cmdList:list, conf:dict):
    geo_dict = {'cSize':'SIZE', 'cProj':'PROJ', 'cBBox':'BBOX', 'cMethod':'CMETHOD'}
    # consider argument-less options: val is None
    for (key,confkey) in geo_dict.items():
        if (confkey in conf):
            cmdList.append("--{key} '{val}'".format(key=key, val=conf[confkey]))
    # cmdList.extend("--cSize {SIZE}\\--cProj {PROJ}\\--cBBox {BBOX}\\--cMethod {CMETHOD}".format(**args).split('\\'))

def traverse_loop(loop, conf, inputList:list, routineList:list, outputList:list):
    if not loop:
        return
    k,v = loops.popitem()
    if k == 'SITE':
        inputList.append()

def extract_prefix(paths: list, shortPaths=None):
    str_paths = [str(p) for p in paths]
    prefix = os.path.commonpath(str_paths)
    if (prefix):
        prefix += '/'
        #if not (shortPaths is None):
        #    shortPaths.extend([str(p).removeprefix(prefix) for p in paths])
    shortPaths.extend([str(p).removeprefix(prefix) for p in paths])
    return prefix

    
def main():
    parser = build_parser()

    # Export template if user requests it
    # args = parser.parse_args()
    args = merge_args(parser)

    # Needs parser for arg definitions, args for current values
    if (args.export_config):
        export_defaults_to_json(parser, args, args.export_config)
        #sys.exit(0)

    # if (args.loop):
    #    args.loop = load_config(args.loop)

    # args = merge_args(parser)

    cmdList = ['rack']
    cmdRoutine = []
    # Outputs

    # Settings
    append_geoconf(cmdList, vars(args))
    print (cmdList)
    
    print("🔧 Final configuration:")
    print(vars(args))
    print("🔧 Final configuration:")
    for key, value in vars(args).items():
        print(f"  {key}: {value}")


    #print (f"=== Full cmd for {timestamp}-{site} ===")
    if (args.PPROD):
        (key,value) = args.PPROD.split(',',1)
        if value is None:
            cmdRoutine.append(f"--{key}")
        else:
            cmdRoutine.append(f"--{key} '{value}'")

    if (args.SCHEME == 'TILE'):
        args.tiledir.removesuffix('/')
        cmdRoutine.append(f"--cCreateTile -o '{args.tiledir}/{args.tilename}'")
    #elif (args.SCHEME == 'TILED'):
    else:
        cmdRoutine.append("--cAdd")
            
    if (args.inputFiles):
        if len(args.inputFiles)==1 :
            cmdList.extend(args.inputFiles) # list of 1 elem
            cmdList.extend(cmdRoutine)
        else:
            shortPaths=[]
            if (args.inputPrefix == 'AUTO'):
                args.inputPrefix = extract_prefix(args.inputFiles, shortPaths)
            if (args.inputPrefix):
                cmdList.append(f'--inputPrefix "{args.inputPrefix}"')
            cmdRoutine = " ".join(cmdRoutine).replace("'",'"')
            cmdList.append(f"--script '{cmdRoutine}'")
            cmdList.extend(shortPaths)
            #print (shortPaths)

    print (args.newline.join(cmdList))                
    print ("\n\n")
    
if __name__ == "__main__":
    main()
