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
import logging

from types import SimpleNamespace

from numpy import size

from rack.composer import handle_prod
import rack.log
import rack.prog
import rack.select
import rack.gnuplot
import rack.core

logger = rack.log.logger.getChild(Path(__file__).stem)
# logger.setLevel(logging.INFO)

reg = rack.prog.Register



def build_parser():
    """ Creates registry of supported options of this script
    """
    parser = argparse.ArgumentParser(description="Pseudo-RHI generator")


    parser.add_argument(
        "INFILE",
        nargs='*',
        help="Input files")

    parser.add_argument(
        "--OUTFILE",
        default="",
        help="Output file (basename). See --FORMAT")
    


    """
    az_angle default: 0
    size default: [500, 250]
    range default: [0, 0]
    height default: [0, 10000]
    beamWidth default: 0.25
    beamPowerThreshold default: [0.005, 0.01]
    """
    
    # Rhi specific parameters
    parser.add_argument(
        "--rhi",
        metavar="KEY=VALUE[,KEY2=VALUE2]",
        default=None,
        help="RHI parameters collectively, e.g. --rhi RANGE=1000,AZIMUTH=45 or --rhi RANGE=500:1500,AZIMUTH=30")

    def camel_to_upper_underscore(name: str) -> str:
        # Insert underscore between lowercase-to-uppercase transitions
        s1 = re.sub(r'(.)([A-Z][a-z]+)', r'\1_\2', name)
        # Insert underscore between lower/number-to-uppercase transitions
        s2 = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1)
        return s2.upper()

    rack.prog.Register.explode_args(rack.core.Rack.pPseudoRhi, parser, name_mapper=camel_to_upper_underscore)

    #rack.core.Rack.pPseudoRhi.__

    # Rhi specific parameters
    """
    parser.add_argument(
        "--AZIMUTH",
        default=0.0,
        metavar="<angle>",  
        help="RHI Azimuthal direction [degrees]")

    # Rhi specific parameters
    parser.add_argument(
        "--RANGE",
        default=0.0,
        metavar="<max>|<min>:<max>",  
        help="RHI Radial range [metres] or relative [0...1]")

    # Rhi specific parameters
    parser.add_argument(
        "--BEAM_WIDTH",
        default=1.0,
        metavar="<width>",  
        help="RHI Beam width [degrees]")

    parser.add_argument(
        "--WEIGHT_THRESHOLD",
        default=0.1,
        metavar="<value>",  
        help="RHI Weight threshold [0-1] for masking output")

    parser.add_argument(
        "--SIZE",
        default="800,400",
        metavar="<width>,<height>",
        help="") 

    """
        
    """
    Selection: todo: move to separate function, and also support for multiple SELECT-like parameters (e.g. quantity and dataset)
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
        "--DATASET",
        default='', 
        metavar="<index>[:<index2>]",
        help="Same as --SELECT path=/dataset<index>") 

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
        "--FORMAT",
        default='h5',
        help="One or several file formats (h5, png, tif, svg)") 


    #parser.add_argument("--loop", type=str, help="<file>.json Path to JSON config file")
    """
    parser.add_argument(
        "--INDIR",
        type=str,
        metavar="<path>|AUTO",
        default="AUTO",
        help="Common path of input files.")
    """

    parser.add_argument(
        "--OUTDIR",
        type=str,
        metavar="<path>|AUTO",
        default=None,
        help="Common path of output files.")




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
        "--gnuplot",   
        metavar="<format>",
        default=None,
        help="Generate GnuPlot script with given format (e.g. 'png')")

    parser.add_argument(
        "--print", "-p",   
        metavar="<line_separator>",
        default=None,
        #action='store_true',
        help="print parsed commands with given line separator (e.g. '\\n' or '\\t')")

    parser.add_argument(
        "--test",
        action='store_true',
        help="run some tests")



    return parser

""" Utils etc

"""



    
def get_defaults(parser):
    return {a.dest: a.default for a in parser._actions if a.dest != 'help'}


def export_defaults_to_json(parser, args, filename="config_template.json"):
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
    logger.info(f"Config template written to: {filename}")


def load_config(filename):
    """Load JSON config if it exists."""
    path = Path(filename)
    if not path.is_file():
        print(f"File not found: {filename}", file=sys.stderr)
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
        
    
    



def handle_infile(args, progBuilder: rack.core.Rack):
    
    logger.debug(f"INFILE {args.INFILE}")

    if type(args.INFILE) == str:
        args.INFILE = [ args.INFILE ]

    # probably Pseudo-RHI should support multiple input files, but for now just take the first one
    for i in args.INFILE:
        progBuilder.inputFile(i)

    """
    shortPaths=[]
    if (args.INDIR == 'AUTO'):
        args.INDIR = extract_prefix(args.INFILE, shortPaths)
    
    if args.INDIR:
        progBuilder.inputPrefix(args.INDIR)

    args.INFILE = shortPaths
    for i in args.INFILE:
        progBuilder.inputFile(i)
    """

def handle_outfile(args, cmdBuilder: rack.core.Rack = None):
    """Light adjustments to args.OUTFILE"""

    if args.SCHEME == 'TILE':
        args.OUTFILE = None
    else:
        if not args.OUTFILE:
            args.OUTFILE = 'out.h5'
        if cmdBuilder:
            cmdBuilder.outputFile(args.OUTFILE)
        else:
            logger.error("No cmdBuilder set")

def handle_outfiles(args, cmdBuilder: rack.core.Rack) -> str:
    # Assumes prefix has been handled
    
    output_basename = args.OUTFILE
    fmt = args.OUTFILE.split('.').pop()
    #logger.warning(f"format: {fmt}")
    output_basename = output_basename.removesuffix(f".{fmt}")

    if args.FORMAT:
        formats = args.FORMAT.strip().split(',')
    else:
        formats = {fmt}

    logger.debug(f"formats: {formats}")
        
    if 'h5' in formats:
        cmdBuilder.outputFile(f"{output_basename}.h5")
        formats.remove('h5')

    if 'tif' in formats:
        cmdBuilder.outputConf("tif:tile=512")
        cmdBuilder.outputFile(f"{output_basename}.tif")
        formats.remove('tif')

    if 'png' in formats:
        cmdBuilder.paletteDefault()
        # transparency?
        cmdBuilder.outputFile(f"{output_basename}.png")
        formats.remove('png')

    if 'svg' in formats:
        cmdBuilder.outputFile(f"{output_basename}.svg")
        formats.remove('svg')

    if (formats):
        raise Exception('Unhandled formats:', formats)
        
import inspect
def handle_prhi(args, progBuilder: rack.core.Rack):
 

    if args.rhi:
        progBuilder.pPseudoRhi(args.rhi)
        return
    
    #value = []
    #svalue.append(args.AZIMUTH)
    sig = inspect.signature(progBuilder.pPseudoRhi)
    params = list(sig.parameters.values())
    # Remove 'self' if it exists in the source signature
    #    if params and params[0].name == "self":
    #params = params[1:]

    var_args = vars(args)
    dict_args = {}
    for v in params:
        name = v.name
        if name in var_args and var_args[name] is not None:
            value = var_args[name]
            #value.append(f"{name}={value}")
            dict_args[v.name] = value

    logger.warning(f"Calling pPseudoRhi with args: {dict_args}")

    progBuilder.pPseudoRhi(**dict_args)

    return

    if args.SIZE:
        # args.SIZE = str(args.SIZE).replace(',', ':')
        args.SIZE = [int(x) for x in str(args.SIZE).split(',')]
        if len(args.SIZE) == 1:
            args.SIZE.append(int(args.SIZE[0]/2)) 
        #value.append(f"size={args.SIZE}")

    if args.RANGE:
        args.RANGE = str(args.RANGE).replace(',', ':')
        #value.append(f"range={args.RANGE}")
    
    #if args.BEAM_WIDTH:
    #    value.append(f"beam_width={args.BEAM_WIDTH}")
    
    #if args.WEIGHT_THRESHOLD:
    #    value.append(f"weight_threshold={args.WEIGHT_THRESHOLD}")

    progBuilder.pPseudoRhi(az_angle=args.AZIMUTH, range=args.RANGE, size=args.SIZE, 
                           beamWidth=args.BEAM_WIDTH, beamPowerThreshold=args.WEIGHT_THRESHOLD)
            

def handle_gnuplot(args, progBuilder: rack.core.Rack):

    # from rack.gnuplot import Terminal, Datafile, Format, PlotSequence, ConfSequence, Registry
    
    if not args.gnuplot:
        return

    # ensure png file is generated by rack, and use that as input for gnuplot (background image)
    outpath = Path(args.OUTFILE)
    basename = outpath.stem
    outfile_rack = f'{outpath.stem}-background.png'
    progBuilder.outputFile(outfile_rack)

    terminal = outpath.suffix[1:] 
    if terminal not in ['png', 'svg', 'tif']:
        logger.warning(f"Unsupported gnuplot terminal format: {terminal}, defaulting to png")
        terminal = 'png'


    gnuplot_conf = rack.gnuplot.ConfSequence()

    conf = rack.gnuplot.Registry(gnuplot_conf)
    conf.terminal(rack.gnuplot.Terminal(terminal), size=args.size) # Hard-coded. See also handle_prhi for size handling.
    conf.output(args.OUTFILE)
    #reg.set("terminal", GnuPlot.Terminal.PNG, size=(800, 600))

    # conf.datafile(rack.gnuplot.Datafile.SEPARATOR, rack.gnuplot.Datafile.WHITESPACE)
    #reg.datafile(Datafile.SEPARATOR, char=Datafile.WHITESPACE)
    conf.xdata(rack.gnuplot.Data.TIME)
    conf.timefmt("%s")
    # reg.format_x(Format.X, "%H:%M")
    conf.format_x("%H:%M")
    conf.grid()
    conf.title("statistics 00min 20140525-1200")
    conf.xlabel("TIME START REL")
    conf.ylabel("ELANGLE")

    prog_plot = rack.gnuplot.PlotSequence()
    plot = rack.gnuplot.Registry(prog_plot)
    plot.plot("sin(x)")
    plot.plot("sin(x) with lines")

    print("# Generated GnuPlot script:\n")
    print(gnuplot_conf.to_string())  # ";\n"
    #fmt.PARAM_SEPARATOR = ",\\\n\t"
    print(prog_plot.to_string())  # ";\n
    pass

def compose_command(args):
    """Main library entry point.

    Args:
        args (argparse.Namespace): Parsed arguments from `build_parser()`
    """

    global logger

    if isinstance(args, dict):
        args = argparse.Namespace(**args)

    # Rack command sequence, the "program" to be executed
    prog = rack.prog.CommandSequence(programName='rack', quote="'")

    # Command registry, "factory" for adding command to the program sequence.
    progBuilder = rack.core.Rack(prog)

    """
    script = rack.prog.CommandSequence(quote=prog.get_secondary_quote())
    scriptBuilder = rack.core.Rack(script)
    scriptFmt = rack.prog.RackFormatter(params_format='"{params}"')
    """

    # Set Python logging verbosity, and also rack verbosity with verbosityKey
    # Handle --log_level <level>, --debug, --verbose
    verbosityKey = rack.log.handle_parameters(args)
    progBuilder.verbose(level=verbosityKey)

    handle_infile(args, progBuilder)

    rack.select.handle_select(args, progBuilder)

    handle_prhi(args, progBuilder)

    # Note: progBuilder is passed to handle_gnuplot, but it doesn't actually modify the progBuilder, just generates a separate GnuPlot script.
    # This is a bit hacky, but allows us to keep all command handling in one place for now.
    handle_gnuplot(args, progBuilder)

    # Rack outfiles (only)
    handle_outfiles(args, progBuilder)

    # logger.debug(dirpath)
    # if dirpath:
    #    progBuilder.outputPrefix(dirpath)
        #logger.debug(filepath)
        
    #handle_outfiles(args, scriptBuilder)
    #    progBuilder.script(script.to_string(scriptFmt))
        # prog   <- (inputPrefix) input(s)
    
        
    return prog



  
def exec_command(args):
    cmdList = compose_command(args)
    os.system(cmdList)  # subprocess!

def test():
    cmds = {
        "INPUTS" 
    }
    compose_command("")

def main():

    parser = build_parser()

    rack.log.add_parameters(parser)
    rack.select.build_parser(parser)

    read_default_args(parser)

    args = parser.parse_args()

    # Handle --log_level <level>, --debug, --verbose
    # rack.log.handle_parameters(args)
    
    # Selected commands only for direct command line use

    # Needs parser for arg definitions, args for current values

    # Export template if user requests it
    if args.export_config:
        export_defaults_to_json(parser, args, args.export_config)
        sys.exit(0)

    if args.test:
        logger.info("Running tests..")
        test()
        sys.exit(0)

    prog = compose_command(args)

    if args.print:
        args.print = args.print.replace(r'\t','\t')
        args.print = args.print.replace(r'\n','\n')
        logger.info("# Command line:")
        #fmt = rack.prog.RackFormatter(params_format="'{params}'", cmd_separator=" \\\n\t")
        fmt = rack.prog.RackFormatter(params_format="'{params}'", cmd_separator=args.print)
        print(prog.to_string(fmt))
        # print(cmdList.to_string(" \\\n"))

    if args.exec:
        logger.info("# Executing...")
        fmt = rack.prog.RackFormatter(params_format="'{params}'")
        print(prog.to_string(fmt))
        os.system(prog.to_string(fmt))

    

if __name__ == "__main__":
    main()
