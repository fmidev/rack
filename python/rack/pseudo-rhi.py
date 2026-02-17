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

from matplotlib import lines, scale
from numpy import size

from rack import script
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
    
    
    # Rhi specific parameters

    def camel_to_upper_underscore(name: str) -> str:
        # Insert underscore between lowercase-to-uppercase transitions
        s1 = re.sub(r'(.)([A-Z][a-z]+)', r'\1_\2', name)
        # Insert underscore between lower/number-to-uppercase transitions
        s2 = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1)
        return s2.upper()

    # Rhi specific parameters
    # rack.prog.Register.export_func(rack.core.Rack.pPseudoRhi, parser, key="RHI", name_mapper=camel_to_upper_underscore)
    rack.prog.Register.publish_func(rack.core.Rack.pPseudoRhi, parser, name_mapper=camel_to_upper_underscore)

    rack.prog.Register.publish_func(rack.core.Rack.select, parser, name_mapper=camel_to_upper_underscore)     


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
        "--background",   
        metavar="<intensity>|<red,green,blue>",
        default="0", # black
        help="Illustrate radar beams with given background color.")

        
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

    if not args.INFILE:
        logger.error("No input file specified. Use --INFILE or check your config file.")
        sys.exit(1) 

    if type(args.INFILE) == str:
        args.INFILE = [ args.INFILE ]

    # probably Pseudo-RHI should support multiple input files, but for now just take the first one
    for i in args.INFILE:
        progBuilder.inputFile(i)


            
def get_background_filename(args, prefixed=False):
    p = Path(args.gnuplot)
    if prefixed and args.OUTDIR:
        return f"{args.OUTDIR}{p.stem}-background.png"  
    else:
        return f"{p.parent}/{p.stem}-background.png"

def handle_outfiles(args, cmdBuilder: rack.core.Rack) -> str:
    # Assumes prefix has been handled

    if not (args.OUTFILE or args.gnuplot):
        args.OUTFILE = 'out.h5'


    if args.OUTFILE:
        formats = set(args.FORMAT.strip().split(','))
        formats.add(args.OUTFILE.split('.').pop())
    #else:
    #    args.FORMAT = args.gnuplot.split('.').pop()
    #    args.OUTFILE = args.gnuplot.removesuffix(f".{args.FORMAT}") + f"-bkg.{args.FORMAT}"
    #    formats = {args.FORMAT} 

    #args.OUTFILE
    
    
    p = Path(args.OUTFILE)
    
    if (not args.OUTDIR) and len(formats) > 1:
        if p.parent and p.parent != Path('.'):
            args.OUTDIR = f"{p.parent}/"
            cmdBuilder.outputPrefix(args.OUTDIR)
    
    if args.OUTDIR:
        cmdBuilder.outputPrefix(args.OUTDIR)
        output_basename = p.stem
    else:
        output_basename = f"{p.parent}/{p.stem}"
        
    
    # output_basename = output_basename.removesuffix(f".{formats}")
    # OUTPUT_PREFIX not handled, but could be derived from OUTFILE if needed, e.g. for gnuplot output files
    # parent = Path(args.OUTFILE).parent
    # if parent and parent != Path('.'):  
    #    cmdBuilder.outputPrefix(f"{parent}/")
    
    logger.debug(f"formats: {formats}")
        
    if 'h5' in formats:
        cmdBuilder.outputFile(f"{output_basename}.h5")
        formats.remove('h5')

    if 'tif' in formats:
        cmdBuilder.outputConf("tif:tile=512")
        cmdBuilder.outputFile(f"{output_basename}.tif")
        formats.remove('tif')

    if ('png' in formats) or args.gnuplot:

        cmdBuilder.paletteDefault()
        
        if args.background  != "":
            cmdBuilder.select(quantity='QIND')
            cmdBuilder.encoding(type="C", gain=0.004, offset=0) # 
            cmdBuilder.imageAlpha()
            cmdBuilder.imageFlatten(args.background)
        # transparency?
        if (args.gnuplot):
            # cmdBuilder.outputFile(f"{Path(args.gnuplot).stem}-background.png")
            cmdBuilder.outputFile(get_background_filename(args, prefixed=False))
        
        if ('png' in formats):
            cmdBuilder.outputFile(f"{output_basename}.png")
            formats.remove('png')

    if 'svg' in formats:
        cmdBuilder.outputFile(f"{output_basename}.svg")
        formats.remove('svg')

    if (formats):
        raise Exception('Unhandled formats:', formats)
        
"""
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
"""


def handle_gnuplot(args, progBuilder: rack.core.Rack):

    # from rack.gnuplot import Terminal, Datafile, Format, PlotSequence, ConfSequence, Registry
    
    if not args.gnuplot:
        return

    # ensure png file is generated by rack, and use that as input for gnuplot (background image)
    
    script = []
    script.append("# GnuPlot script\n")
    
    terminal = args.gnuplot.split('.').pop()
    if terminal not in ['png', 'svg', 'tif']:
        logger.warning(f"Unsupported gnuplot terminal format: {terminal}, defaulting to png")
        terminal = 'png'


    gpl_conf = rack.gnuplot.ConfSequence()
    gpl_plot = rack.gnuplot.PlotSequence()
 
    confCmdReg = rack.gnuplot.Registry(gpl_conf)
    plotCmdReg = rack.gnuplot.Registry(gpl_plot)
    

    # conf.comment("Generated by rack with --gnuplot option")
    #conf.comment("General settings")
    confCmdReg.terminal(rack.gnuplot.Terminal(terminal), size=args.size) # Hard-coded. See also handle_prhi for size handling.
    confCmdReg.output(args.gnuplot)

    confCmdReg.multiplot()    # (rows=1, cols=1)
    confCmdReg.title(f"Pseudo-RHI [{args.select}] fikor {args.az_angle} deg  2017/08/12 16:00")


    confCmdReg.unset("tics")
    confCmdReg.grid(rack.gnuplot.Tics.XTICS, rack.gnuplot.Tics.YTICS)
    confCmdReg.unset("border")

    # consider single command with multiple parameters for margins, e.g. margin left 10, right 20, top 30, bottom 40
    confCmdReg.comment("lmargin at screen 0.22")
    confCmdReg.comment("rmargin at screen 0.9")
    confCmdReg.comment("bmargin at screen 0.18")
    confCmdReg.comment("tmargin at screen 0.9")

    # left, right, bottom, top in character units. See also margin command for more flexible margins in screen units.
    confCmdReg.set("margins 12, 4, 4, 3"  ) 

    #print("# GnuPlot script\n")
    script.extend(gpl_conf.to_list())
    #print(gpl_conf.to_string())  # ";\n"
    gpl_conf.clear() # Clear conf sequence to separate it from plot commands in the output

    #confCmdReg.comment("General settings")
    
    plotCmdReg.comment("Plotting the background image (radar beams)")
    plotCmdReg.plot(filename=get_background_filename(args, prefixed=True), 
                    style=rack.gnuplot.Style.RGBIMAGE) # linecolor='rgb "gray"', linewidth=1)
    script.extend(gpl_plot.to_list())
    #print(gpl_plot.to_string())  # ";\n
    gpl_plot.clear() # Clear plot sequence to separate it from background image plot in the output

    xrange = (-220000, +240000)
    yrange = (100, 8000)

    confCmdReg.xrange(*xrange)
    confCmdReg.yrange(yrange)
    confCmdReg.set("border") # see above unset border, to remove default border and tics, and then add custom border back with margins
    #confCmdReg
    confCmdReg.set("tics out nomirror scale 2")
    confCmdReg.set("mxtics 5")
    confCmdReg.key(rack.gnuplot.Key.LEFT, outside=True)
    confCmdReg.xlabel(f'Range {xrange}')
    confCmdReg.ylabel(f'Altitude {yrange}')
    script.extend(gpl_conf.to_list())


    plotCmdReg.comment("Plotting a dummy line (to ensure gnuplot output is not empty)")
    plotCmdReg.plot('x*0', style=rack.gnuplot.Style.LINES) # linecolor='rgb "gray"', linewidth=1)
    script.extend(gpl_plot.to_list())
    #print(gpl_plot.to_string())  # ";\n
    gpl_plot.clear()

    script = "\n".join(script)
    print(script)
    script_filename = f"{args.gnuplot}.gnu"
    with open(script_filename, "w") as f:
        f.write(script)
    logger.info(f"GnuPlot script written to: {script_filename}")


def compose_command(args):
    """Main library entry point.

    Args:
        args (argparse.Namespace): Parsed arguments from `build_parser()`
    """

    global logger

    if isinstance(args, dict):
        args = argparse.Namespace(**args)

    #logger.info(f"Parsed args: {args}")

    # Rack command sequence, the "program" to be executed
    rackProg = rack.prog.CommandSequence(programName='rack', quote="'")

    # consider also PythonCommandSequence 

    # Command registry, "factory" for adding command to the program sequence.
    rackCmdReg = rack.core.Rack(rackProg)

    """
    script = rack.prog.CommandSequence(quote=prog.get_secondary_quote())
    scriptBuilder = rack.core.Rack(script)
    scriptFmt = rack.prog.RackFormatter(params_format='"{params}"')
    """

    # Set Python logging verbosity, and also rack verbosity with verbosityKey
    # Handle --log_level <level>, --debug, --verbose
    verbosityKey = rack.log.handle_parameters(args)
    rackCmdReg.verbose(level=verbosityKey)

    # RHI specific
    handle_infile(args, rackCmdReg)

    # RHI specific here
    # handle_outfile(args, rackCmdReg)

    # removed for debugging
    rackCmdReg.handle_published_cmd_args(args, rack.core.Rack.select)

    rackCmdReg.handle_published_cmd_args(args, rack.core.Rack.pPseudoRhi)

    #handle_prhi(args, progBuilder)

    # Note: progBuilder is passed to handle_gnuplot, but it doesn't actually modify the progBuilder, just generates a separate GnuPlot script.
    # This is a bit hacky, but allows us to keep all command handling in one place for now.
    handle_gnuplot(args, rackCmdReg)

    # Rack outfiles (only)
    handle_outfiles(args, rackCmdReg)
        
    return rackProg



  
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
    # rack.select.build_parser(parser)

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
