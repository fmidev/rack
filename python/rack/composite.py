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
import logging

#from types import SimpleNamespace

import rack.log
import rack.cmdline
import rack.config
import rack.core
import rack.maps
import rack.process
import rack.prog
import rack.svg
import subprocess

logger = rack.log.logger.getChild(Path(__file__).stem)
# logger.setLevel(logging.INFO)

reg = rack.prog.Register

# Global
default_tiledir  = 'tiles/'
default_tilename = '${what:date}${what:time}-GEOCONF={GEOCONF}_${NOD}-${what:product}-${what:prodpar}-${what:quantity}.h5'

class scheme:
    TILE  = "TILE"
    TILED = "TILED"
    DEFAULT = ""


def build_parser() -> argparse.ArgumentParser:
    """ Creates registry of supported options of this script
    """
    parser = argparse.ArgumentParser(description="Example app with JSON config support")

    rack.maps.add_basic_arguments(parser)

    add_arguments(parser)

    return parser

def add_arguments(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:

    """
    parser.add_argument("--host", default="localhost", help="Server hostname")
    parser.add_argument("--port", type=int, default=8080, help="Server port")
    parser.add_argument("--debug", action="store_true", help="Enable debug mode")
    parser.add_argument("--timeout", type=float, default=5.0, help="Request timeout (seconds)")
    parser.add_argument("--logfile", default=None, help="Path to log file")
    """

    parser.add_argument(
        "--SCHEME",
        default='',
        metavar="<empty>|TILE|TILED",
        help=f"Compositing scheme. For TILE, default OUTDIR={default_tiledir}, OUTFILE={default_tiledir}") 

    parser.add_argument(
        "--METHOD",
        default="MAXIMUM",
        help="Compositing method. See: rack -h cMethod") 

    parser.add_argument(
        "INFILE",
        nargs='*',
        help="Input files")

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
        
    parser.add_argument(
        "--OUTFILE",
        default="composite.h5",
        help="Output file or basename. See --FORMATS")
    

    parser.add_argument(
        "--PPROD",
        metavar="<pProd>[,<args>]|<quantity>",
        help="Meteorological product. See: rack -h products") 

    """
        Selection
    """
    rack.prog.Register.expand_options(rack.core.Rack.select, parser, name_mapper=True)
    parser.add_argument(
        "--DATASET",
        default='', 
        metavar="<index>[:<index2>]",
        help="Adds path=/dataset<index>:<index2> to --select") 

    """
    Selection
    parser.add_argument(
        "--SELECT",
        default=None,
        help="") 

    parser.add_argument(
        "--QUANTITY",
        default=None,
        #default='DBZH',
        metavar="<code>",
        help="Same as --SELECT quantity=<code> , where code is DBZH, VRAD, HGHT") 


    parser.add_argument(
        "--PRF",
        default=None, 
        metavar="SINGLE|DOUBLE|ANY",
        help="Same as --SELECT prf=<prf>") 
    """

    parser.add_argument(
        "--PALETTE",
        default='default',
        help="Add colours using a palette. Affects PNG image only.")

    # parser.add_argument("--PROCESSES", default='4', help="Apply ") 
    parser.add_argument(
        "--FORMATS",
        default="",
        help="Set formats (h5, png, tif, svg) explicitly") 


    parser.add_argument(
        "--EXTRACT",
        default='DATA,WEIGHT',
        metavar="DATA,WEIGHT",
        help="Comma separated list of fields to copy from compositing array") 




    # SVG related
    rack.svg.add_parameters(parser)

    parser.add_argument(
        "--svgRadarLabel",
        default = "",
        metavar = "string-with-variables",
        help=
            "Construct string of ODIM variables, like ${what:source}, ${NOD} or ${SRC}."
            "Newlines '\\n' and edges '|' accepted."
        )

    parser.add_argument(
        "--svgStyle",
        default = "",
        metavar = "CSS style definition",
        help=
            "Example: text.TEXTBOX=font-size:8px"
        )


    parser.add_argument(
        "--transparency",
        default="nodata=1,undetect=0", 
        metavar="",
        help="Radar image transparency for SVG output.") 

    # Use somehow rack.maps.add_parameters(parser, SELECTOR)
    parser.add_argument(
        "--map",
        default=None, 
        metavar="file.png",
        help="Background map image for SVG output.") 

    parser.add_argument(
        "--mapServer",
        default=None, 
        metavar="default",
        help="Read mapconf/server-<server>.cnf.") 

    parser.add_argument(
        "--mapLayers",
        type=str,
        default="",
        help="Layers to request, comma-separated, e.g. 'OSM-WMS,TOPO-WMS'") 


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
        "--print", "-p",
        metavar="<line_separator>",
        default=None,
        help="print command line with parameter separator, like ' \\n  '")

    parser.add_argument(
        "--rack_script",
        metavar="<filename>",
        default=None,
        help="Save rack command to a shell script file (one arg per line with backslash continuation)")

    parser.add_argument(
        "--test",
        action='store_true',
        help="run some tests")


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
"""
class safe_dict(dict):
    def __missing__(self, key):
        return '{' + key + '}'
"""
        
"""
Simplifies lists and dictionaries for command line use.
Converts ['A','b',7] to A,b,7 .

Strips chars '[', ']', '{', '}'  
def arg2str(arg, separator=","):
    if (type(arg)==list) or (type(arg)==tuple):
        return separator.join([str(i) for i in arg])
    elif type(arg)==dict:
        return separator.join([f"{k}={v}" for (k,v) in arg.items()])
    else:
        return str(arg) #str(type(arg)) + 
"""

    
def get_defaults(parser):
    return {a.dest: a.default for a in parser._actions if a.dest != 'help'}


def export_defaults_to_json_OLD(parser, args, filename="config_template.json"):
    """Write all parser defaults to a JSON file."""
    logger.debug(f'Writing defaults to a JSON file: {filename}')
    
    defaults = get_defaults(parser)
    conf = {}
    for k,v in vars(args).items():
        if k == 'export_config': # TODO: add set of commands skipped
            continue
        if v is None:
            continue
        conf[k] = v

    
    #{a.dest: a.default for a in parser._actions if a.dest != 'help'}
    with open(filename, "w") as f:
        json.dump(conf, f, indent=4)
        #json.dump(defaults, f, indent=4)
    logger.info(f"✅ Config template written to: {filename}")




def read_default_args(parser):
    """Parse args with precedence:
       CLI > JSON config > defaults
    """
    # First parse known args to see if --config is given
    args, remaining_argv = parser.parse_known_args()

    if args.config:
        config = rack.config.read(args.config)
        parser.set_defaults(**config)

    # Re-parse all args with updated defaults
    # final_args = parser.parse_args()
    # return final_args

GEOCONF_PATH_SYNTAX = "mapconf/geo-{key}"


def apply_geoconf_OLD(args, geoconf: dict, defaults: dict = None):
    """Overlay a geoconf dict onto args, without clobbering values already set away
    from the parser default - i.e. CLI-explicit / caller-supplied values win over geoconf.
    """

    # This is a bit tricky hack, as geoconf does not know which 
    # arguments were set explicitly by the user, and which were 
    # left at their default values.

    if defaults is None:
        defaults = {a.dest: a.default for a in build_parser()._actions}

    for k, v in geoconf.items():
        v2 = getattr(args, k, None)
        if k in defaults and getattr(args, k, defaults[k]) != defaults[k]:
            #logger.info(f"Keeping already-set '{k}'={getattr(args, k)!r}, not overriding with geoconf value {v!r}")
            continue
        setattr(args, k, v)


def read_geoconf(args): #, parser):

    key, filepath = rack.config.resolve_path(args.GEOCONF, GEOCONF_PATH_SYNTAX)
    geoconf = rack.config.read(filepath, formats=['.json', '.cnf'], lenient=False)
    for k, v in geoconf.items():
        if hasattr(args, k):
            if getattr(args, k, None) is None:
                setattr(args, k, v)
        else:
            logger.warning(f"GEOCONF {key} has unknown attribute '{k}' (value {v})")

    args.GEOCONF = key
    # apply_geoconf(args, geoconf)
    return geoconf



def extract_prefix(paths: list, shortPaths=None) -> str:
    if not list:
        return ""
    if not isinstance(paths, list):
        raise ValueError(f"Argument paths={paths} is not a list")
    if len(paths)==1:
        return ""
    str_paths = [str(p) for p in paths]
    prefix = os.path.commonpath(str_paths)
    if prefix:
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
        
    
    

def handle_tilepath_defaults(dirpath, filepath) -> tuple:
    #print (dirpath, filepath)
    if not filepath:
        filepath = default_tilename
    else:
        if type(filepath) == list:
            if (len(filepath) > 1):
                #raise Exception(f'Multiple outputs not supported by SCHEME=TILE: ', filepath)
                logger.warning(f'Multiple outputs not supported by SCHEME=TILE {filepath}')
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
            
    #print (dirpath, filepath)
    return (str(dirpath).removesuffix('/')+'/', str(filepath))

def handle_geoconf(args, Rack: rack.core.Rack):

    if args.GEOCONF:
        if type(args.GEOCONF) in (str, Path):
            # logger.info("# args.GEOCONF? %s", args)
            #read_geoconf(args.GEOCONF) 
            read_geoconf(args) 
        elif type(args.GEOCONF) == dict:
            vars(args).update(args.GEOCONF)
        else:
            raise Exception(f'Unhandled type for GEOCONF: {type(args.GEOCONF)}')
        

    if not args.SIZE:
        args.SIZE = (512, 512)
    args.SIZE = rack.typical(args.SIZE, [int], ",")
    Rack.cSize(args.SIZE)

    if args.PROJ:
        Rack.cProj(args.PROJ)

    if args.BBOX:
        args.BBOX = rack.typical(args.BBOX, [float], ",")
        Rack.cBBox(args.BBOX)
    else: #raise Exception(f'BBOX not set, cannot continue: {args.BBOX}')
        logger.info("Bounding box not given, will be set automatically")
        return


    if args.map:
        args.map = Path(args.map)
        if args.map.parent == Path('.'):
            if args.OUTDIR:
                args.map = Path(args.OUTDIR, args.map)

        logger.info(f"Getting map background: {args.map}")

        serverConf = {} 
        if args.mapServer:
            
            #key, path = rack.config.resolve_path(args.mapServer, "mapconf/server-{key}.cnf")
            logger.info(f"Reading config for server: {args.mapServer}")
            serverConf = rack.maps.get_server_conf(args.mapServer)
            #server_conf = rack.config.read(path, formats=['.cnf', '.json'], lenient=False)

        # def get(mapCache:str, mapServer:str="mundialis", mapLayers:list=["OSM-WMS"], mapForce=False, mapLink:str=None, **kw_args) -> pathlib.Path:
        

        if args.mapLayers:
            serverConf["layers"] = args.mapLayers

        
        rack.maps.get(
            serverConf, 
            mapLink=args.map, 
            mapCache=rack.maps.MAP_CACHE_PATH_SYNTAX, 
            #mapServer=args.mapServer, 
            #mapLayers=args.mapLayers, 
            mapName=args.map.name, 
            SIZE=args.SIZE,
            EPSG=args.PROJ,
            #WIDTH=args.SIZE[0], 
            #HEIGHT=args.SIZE[1],
            #EPSG=args.PROJ,
            BBOX=args.BBOX,
            #CRS=f"EPSG:{args.PROJ}",
            #FORMATS='image/png',
            #**serverCsonf                    
            #mapForce=args.mapForce
        )

    if args.METHOD:
        Rack.cMethod(args.METHOD)



def handle_prod(args, scriptBuilder: rack.core.Rack):
     
     if (args.PPROD):
        (key,value) = args.PPROD.split(',',1)
        cmd = getattr(scriptBuilder, key)
        if value is None:
            cmd()
        else:
            cmd(value)

def handle_dataset(args):
    if args.DATASET:
        # logger.info(f"Adding dataset selection: {args.DATASET}")
        args.path = f"/dataset{args.DATASET}"
        # ogger.info(args)
        #scriptBuilder.select(f"path=/dataset{args.DATASET}")



def handle_infile(args, progBuilder: rack.core.Rack):
    
    # TODO: glob handling/bypassing support 
    logger.debug(f"INFILE {args.INFILE}")
        
    if type(args.INFILE) == str:
        args.INFILE = [ args.INFILE ]
    
    if not args.INFILE:
        logger.warning("No input files?")
        return

    shortPaths=[]
    if (args.INDIR == 'AUTO'):
        args.INDIR = extract_prefix(args.INFILE, shortPaths)
    
    #print("check! "+args.INDIR)
    if args.INDIR:
        if not args.INDIR.endswith('/'):
            args.INDIR+'/'
            logger.info("added '/' to INDIR")
        progBuilder.inputPrefix(args.INDIR)

    if shortPaths:
        args.INFILE = shortPaths

    for i in args.INFILE:
        progBuilder.inputFile(i)


def handle_outfiles(args, cmdBuilder: rack.core.Rack):
    
    if args.OUTDIR:
        logger.warning(f"Adding OUTDIR: {args.OUTDIR}")
        if not args.OUTDIR.endswith('/'):
            args.OUTDIR += '/'
        cmdBuilder.outputPrefix(args.OUTDIR)
    #else:
    #    logger.warning(f"No OUTDIR: {args.OUTDIR}")

    
    # Assumes prefix has been handled
    outfile = Path(args.OUTFILE)
    
    #output_basename = args.OUTFILE
    #fmt = args.OUTFILE.split('.').pop()
    #output_basename = output_basename.removesuffix(f".{fmt}")
    output_basename = outfile.stem
    #fmt = outfile.suffix

    if args.FORMATS:
        formats = rack.typical(args.FORMATS, {str}, ",")
        # set(args.FORMATS.strip().split(','))
    else:
        #formats = set([outfile.suffix[1:]]) # drop leading dot
        formats = {outfile.suffix[1:]} # drop leading dot

    logger.debug(f"formats: {formats}")

    if 'h5' in formats:
        cmdBuilder.outputFile(f"{output_basename}.h5")
        formats.remove('h5')

    if 'tif' in formats:
        cmdBuilder.outputConf("tif:tile=512")
        cmdBuilder.outputFile(f"{output_basename}.tif")
        formats.remove('tif')

    if args.svgOutputs:
        formats.add('svg')

    if 'svg' in formats:
        if not args.svgOutputs:
            args.svgOutputs = "auto"
 
    if args.svgOutputs:
        if args.svgOutputs == "auto":
            args.svgOutputs = args.EXTRACT
        rack.svg.handle_conf(args, cmdBuilder)
        rack.svg.handle_outfiles(args, cmdBuilder)    
        cmdBuilder.outputFile(f"{output_basename}.svg")
        formats.remove('svg')

    if 'png' in formats:
        cmdBuilder.paletteDefault()
        # transparency?
        cmdBuilder.outputFile(f"{output_basename}.png")
        formats.remove('png')

    #if 'svg' in formats:
        # cmdBuilder.paletteDefault()
        # transparency?
    #    cmdBuilder.outputFile(f"{output_basename}.svg")
    #    formats.remove('svg')


    if (formats):
        raise Exception('Unhandled formats:', formats)
        
def create_script(args) -> rack.prog.CommandSequence:
    """ 
        Creates and initializes a script to be extended.  
    """

    #script = rack.prog.CommandSequence(quote=rack.prog.get_secondary_quote())
    script = rack.prog.CommandSequence(quote="'") # check! Or shlex?
    script.fmt = rack.cmdline.RackFormatter(params_format='"{params}"')   

    # This part is common for both TILE and default SCHEME.
    scriptBuilder = rack.core.Rack(script)
    handle_dataset(args)
    scriptBuilder.add_cmd_with_expanded_args(rack.core.Rack.select, args)
    #cmd = handle_select(args, scriptBuilder)
    #logger.warning(f"scriptBuilder-select: {cmd}")
    handle_prod(args, scriptBuilder)
    
    return scriptBuilder
    # And then:
    # scriptBuilder.cCreateTile()
    # scriptBuilder.cAdd() or scriptBuilder.cAddWeighted()




def compose_command(args) -> rack.prog.CommandSequence:
    """Main library entry point.

    Args:
        args (argparse.Namespace): Parsed arguments from `build_parser()`
    """

    global logger

    if isinstance(args, dict):
        args = argparse.Namespace(**args)

    if not args.select and not args.quantity:
        args.quantity = 'DBZH'
    
    # Rack command sequence, the "program" to be executed
    prog = rack.prog.CommandSequence(programName='rack', quote="'")

    # Command registry, "factory" for adding command to the program sequence.
    progBuilder = rack.core.Rack(prog)

    #progBuilder.handle_expanded_cmd_args(args, rack.core.Rack.select)
    

    # Set Python logging verbosity, and also rack verbosity with verbosityKey
    verbosityKey = rack.log.handle_parameters(args)
    if verbosityKey != "INFO": # "logging.INFO:
        logger.info(f"Adding explicit verbosity level: {logging.INFO}")
        progBuilder.verbose(level=verbosityKey)

    # perhaps not set yet
    #rack.svg.handle_conf(args, progBuilder)

    #logger.info("# args %s", args)
    if isinstance(args.INFILE, str):
        # Todo: if args.GLOB -> expand
        args.INFILE = [args.INFILE]

    logger.info("compose_command # args %s", args)
    # Init
    handle_geoconf(args, progBuilder)

    # Handle svg_conf?
    if args.svgStyle:
        progBuilder.gStyle(args.svgStyle)


    if (args.SCHEME == 'TILE'):

        if len(args.INFILE) > 1:
            logger.warning("Several inputs, check that outputs have syntax")

        scriptBuilder = create_script(args)

        # todo: svg routine (in script or not)
        if (args.svgRadarLabel):
            scriptBuilder.gRadarLabel(args.svgRadarLabel)

        scriptBuilder.cCreateTile()
        # or svgRadarLabel here?

        (dirpath,filepath) = handle_tilepath_defaults(args.OUTDIR, args.OUTFILE)
        #logger.debug(dirpath)
        #if dirpath:
        #    progBuilder.outputPrefix(dirpath)
        if dirpath:# NEw 2026/04
            if args.OUTDIR and (dirpath != args.OUTDIR):
                logger.warning(f"replacing output prefix OUTDIR: {args.OUTDIR} -> {dirpath}")
            progBuilder.outputPrefix(dirpath)
            #args.OUTDIR = dirpath # NEw 2026/04
        args.OUTDIR = ""

        #logger.debug(filepath)
        args.OUTFILE = filepath.replace('{GEOCONF}', str(args.GEOCONF))
        
        #scriptBuilder.outputFile(filepath)
        handle_outfiles(args, scriptBuilder)
        #progBuilder.script(script.to_string(scriptFmt))
        progBuilder.script(scriptBuilder.getCmdSequence().to_string())
        # prog   <- (inputPrefix) input(s)
        handle_infile(args, progBuilder)
    else: #if (args.SCHEME == ''):

        if args.BBOX and args.PROJ and args.SIZE:
            progBuilder.cInit()

        if len(args.INFILE) > -99: # > 1 !
            logger.info("Several inputs, hence defining a script")
            
            scriptBuilder = create_script(args)
            #script = rack.prog.CommandSequence(quote=prog.get_secondary_quote())
            #scriptBuilder = rack.core.Rack(script)
            #scriptFmt = rack.cmdline.RackFormatter(params_format='"{params}"')
            #handle_select(args, scriptBuilder)
            #handle_prod(args, scriptBuilder)

            
            scriptBuilder.cAdd()
            # todo: svg routine (in script or not)
            if (args.svgRadarLabel):
                scriptBuilder.gRadarLabel(args.svgRadarLabel)
            
            #progBuilder.script(script.to_string(scriptFmt))
            progBuilder.script(scriptBuilder.getCmdSequence().to_string()) # script.to_string())
            # force quotes?

            handle_infile(args, progBuilder)
        else:
            logger.info("Single input - not using script")
            # Hence, here input first.
            handle_infile(args, progBuilder)
            # Then, product generation and mapping to Cartesian.
            # handle_select(args, progBuilder)
            handle_dataset(args)
            progBuilder.add_cmd_with_expanded_args(rack.core.Rack.select, args)
            handle_prod(args, progBuilder)
            progBuilder.cAdd()
            if (args.svgRadarLabel):
                progBuilder.gRadarLabel(args.svgRadarLabel)

        progBuilder.cExtract(args.EXTRACT)
        handle_outfiles(args, progBuilder)
    
    
        
    return prog

# USAGE: rack/composer-test.py:    print(prog.to_string(rack.composite.get_fmt()))
#def get_fmt(cmd_separator=" ", **kwargs):
#    return rack.cmdline.RackFormatter(params_format="'{params}'", cmd_separator=cmd_separator, **kwargs)
    

  
def exec_command(args):
    cmdList = compose_command(args)
    os.system(cmdList)  # subprocess!

"""
def test():
    cmds = {
        "INPUTS" 
    }
    compose_command("")
"""

def main():

    parser = build_parser()

    # read_default_args(parser)
    # More readable:
    # Apply --config and --GEOCONF as new parser defaults *before* the real
    # parse, so that CLI-given values (which always beat argparse defaults)
    # still win over them, and they still win over the built-in defaults.
    known_args, unknown_args = parser.parse_known_args()
    if known_args.config:
        config = rack.config.read(known_args.config)
        parser.set_defaults(**config)

    geoconf_key = None
    if known_args.GEOCONF:
        geoconf_key, geoconf_path = rack.config.resolve_path(known_args.GEOCONF, GEOCONF_PATH_SYNTAX)
        geoconf = rack.config.read(geoconf_path)
        parser.set_defaults(**geoconf)

    args = parser.parse_args()

    if geoconf_key:
        args.GEOCONF = geoconf_key

    # check if no args given, then print help and exit
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)

    # Selected commands only for direct command line use
    # Needs parser for arg definitions, args for current values

    # Export template if user requests it
    if args.export_config:
        rack.args.export_defaults_to_json(parser, args, args.export_config)
        sys.exit(0)


    #logger.info("main() # args %s", type(args))
    prog = compose_command(args)

    if args.exec:
        desc = "Executing command sequence with subprocess.run()"
        logger.info(desc)
        last_line = rack.process.run(prog, description=desc, logger=logger)
        logger.info(f"Process executed, last line:\n {last_line}")
        # fmt = rack.cmdline.RackFormatter(params_format="'{params}'")
        # print(prog.to_string(fmt))
        # result = subprocess.run(prog.to_token_list(fmt), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        # rack.process.handle_result(result, description=desc, logger=logger) # prog.to_string(f

    # Useful in this order: execute first (with perhaps verbous logging)
    # ... and then print what was done:
    if args.print == '':
        args.print = r' \\n  '  # default separator

    if args.print:
        args.print = args.print.replace(r'\t','\t')
        args.print = args.print.replace(r'\n','\n')
        logger.info("# Command line:")
        fmt = rack.cmdline.RackFormatter(params_format="'{params}'", cmd_separator=args.print)
        print(prog.to_string(fmt))
        # print(cmdList.to_string(" \\\n"))

    if getattr(args, 'rack_script', None):
        fmt = rack.cmdline.RackFormatter(params_format="'{params}'", cmd_separator=" \\\n  ")
        #fmt = rack.cmdline.RackFormatter(params_format='"{params}"', cmd_separator=" \\\n  ")
        script_text = prog.to_string(fmt) + '\n'
        with open(args.rack_script, "w") as f:
            f.write(script_text)
        logger.info(f"Rack script written to: {args.rack_script}")


    

if __name__ == "__main__":
    main()
