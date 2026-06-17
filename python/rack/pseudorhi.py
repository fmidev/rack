""" Rack command line argument generator

Utility for constructing strings that can be executed in shell.

    * property
    * property2

"""
import argparse
from ctypes.wintypes import SIZE
from html import parser
import json
import subprocess
import sys
from pathlib import Path
import os
import re # GEOCONF filename-KEY extraction
import inspect
import logging

from types import SimpleNamespace

from matplotlib import lines, scale
from numpy import size
from sympy import arg

from rack import script
#from rack.composite import handle_prod
import rack.log
import rack.prog
#import rack.select
import rack.cmdline
import rack.gnuplot
import rack.core
import rack.args  


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
    rack.prog.Register.expand_options(rack.core.Rack.pPseudoRhi, parser, name_mapper=camel_to_upper_underscore)

    rack.prog.Register.expand_options(rack.core.Rack.select, parser, name_mapper=camel_to_upper_underscore)     

    parser.add_argument(
        "--PRODUCT",
        default=None,
        help="Compute also a meteorogical product and save it in Cartesian coordinates.")

    parser.add_argument(
        "--PALETTE",
        default='default',
        help="Add colours using a palette. Affects PNG image only.")

    # parser.add_argument("--PROCESSES", default='4', help="Apply ") 
    parser.add_argument(
        "--FORMAT",
        #default='h5',
        default=None,
        help="One or several file formats (h5, png, tif, svg)") 

    """
    parser.add_argument(
        "--SIZE",
        default="800,600",
        help="Size of the output image in pixels, as width,height (e.g. 800,600). Only applies to raster formats (e.g. PNG).") 
    """

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
        metavar="<filename>",
        default=None,
        help="Generate GnuPlot image (e.g. 'png')")

    parser.add_argument(
        "--gnuplot_script",   
        metavar="<filename>",
        default=None,
        help="Explicit name for GnuPlot script (e.g. 'plot.gnu')")

    parser.add_argument(
        "--title",   
        metavar="<string>",
        default=None,
        help="Set title for GnuPlot output.")

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

    parser.add_argument(
        "--ALIGN",
        metavar="[TOP|BOTTOM|LEFT|RIGHT]",
        default="",
        help="Position of the Pseudo RHI image wrt. radar image")

    parser.add_argument(
        "--STYLE",
        default=".RAY=stroke:white;stroke-endcap:round",
        help="Adjust CSS styles for the output")


    return parser

# Utils etc

def get_defaults(parser):
    return {a.dest: a.default for a in parser._actions if a.dest != 'help'}

# General utility? -> move
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



"""
def expand_string(inputSet, key, values):
    
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
 """       

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



def ensure_arguments(args, cmdBuilder: rack.core.Rack) -> str:
    """ Ensure required arguments are present, and set defaults for optional arguments.
        Also, perform any necessary transformations on the arguments, 
        e.g. parsing a comma-separated string into a list.

        Example: args.SIZE: "800,600" -> (800, 600)

        Also, adds "hidden" arguments to the args namespace, e.g. args.basename,
        which are derived from the provided arguments and used later in the command generation.        
    """
    

    v = vars(args)

    if not args.OUTFILE:
        args.OUTFILE = 'pseudo-rhi.svg'

    p = Path(args.OUTFILE)

    v["basename"] = p.stem  # Adds "hidden" argument to the args namespace

    if not args.OUTDIR:
        args.OUTDIR = p.parent
        args.OUTFILE = f"{p.stem}.{p.suffix}"

    args.OUTDIR = str(args.OUTDIR)

    if args.OUTDIR:
        # ensure 1 trailing slash for later prefixing
        args.OUTDIR = args.OUTDIR.rstrip('/') + '/' 


    if args.FORMAT:
        args.FORMAT = set(args.FORMAT.strip().split(','))
    else:
        args.FORMAT = set()
    args.FORMAT.add(p.suffix.strip('.'))

    if args.PRODUCT:
        logger.debug(f"An auxiliary image with range indicator is requested")
        args.FORMAT.add('svg')

    if 'svg' in args.FORMAT:
        if not args.gnuplot:
            args.gnuplot = f"{p.stem}-gnuplot.png"

    if args.ALIGN:
        align = args.ALIGN.upper()
        if align == 'TOP':
            cmdBuilder.gLayout("VERT", "DOWN", "LEFT")
        elif align == 'BOTTOM':
            cmdBuilder.gLayout("VERT", "DOWN", "LEFT")
        elif align == 'LEFT':
            cmdBuilder.gLayout("HORZ", "UP", "RIGHT")
        elif align == 'RIGHT':
            cmdBuilder.gLayout("HORZ", "DOWN", "RIGHT")
        else:
            logger.warning(f"Unsupported ALIGN value: {args.ALIGN}. Ignoring.") 
        

    cmdBuilder.outputPrefix(args.OUTDIR)

    logger.debug(f"args.OUTDIR={args.OUTDIR}")
    logger.debug(f"args.OUTFILE={args.OUTFILE}")
    logger.debug(f"args.FORMAT={args.FORMAT}")
    logger.debug(f"args.basename={args.basename}")
    logger.debug(f"args.size={args.size}")


def handle_style(args, cmdBuilder: rack.core.Rack) -> str:
    cmdBuilder.gStyle(".IMAGE_BORDER=stroke:gray")
    if args.STYLE:
        for i in args.STYLE.strip().split('|'):
            cmdBuilder.gStyle(i.strip())
        
def handle_outfiles_prhi(args, cmdBuilder: rack.core.Rack) -> str:

    if 'h5' in args.FORMAT:
        cmdBuilder.outputFile(f"{args.basename}.h5")
        args.FORMAT.remove('h5')

    cmdBuilder.paletteDefault()

    # print(args)
    # anticipated gnuplot-produced image:
    SIZE = str(args.size).replace(':', ',').split(',')
    cmdBuilder.cSize(SIZE[0],SIZE[1])
    # Linking wants full path.
    cmdBuilder.gLinkImage(f"{args.OUTDIR}{args.gnuplot}")
    
    if args.background  != "":  # color
        cmdBuilder.select(quantity='QIND')
        cmdBuilder.encoding(type="C", gain=0.004, offset=0) # 
        cmdBuilder.imageAlpha()
        cmdBuilder.imageFlatten(args.background)

    # This is for gnuplot script to use as background
    cmdBuilder.gInclude("SKIP")
    cmdBuilder.outputFile(f"{args.basename}-background.png")
    




def handle_product(args, progBuilder: rack.core.Rack):
    """ Compute an auxiliary product (e.g. CAPPI) and save it in Cartesian coordinates.
        Add an indicator line with the same range and height as the Pseudo-RHI. 
        
        This allows to illustrate the radar beams in the Pseudo-RHI image, and 
        also to use the Cartesian product as background for the Pseudo-RHI in GnuPlot.
    """

    if args.prf:
        progBuilder.select(prf=args.prf)

    if args.PRODUCT:
        cmd,params = args.PRODUCT.split(',', 1)
        cmd = cmd.strip()
        params = params.strip()
        if hasattr(rack.core.Rack, cmd):
            rackCmd = getattr(rack.core.Rack, cmd)
            # logger.warning(type(rackCmd))
            logger.warning(f"Adding cmd: {cmd}, params: {params}")
            rackCmd(progBuilder, *params.split(','))
            
            SIZE = str(args.size).replace(':', ',').split(',')
            if args.ALIGN in ['TOP', 'BOTTOM']:
                progBuilder.cSize(SIZE[0],SIZE[0]) 
            else:
                progBuilder.cSize(SIZE[1],SIZE[1]) 
            progBuilder.cCreate() 
            progBuilder.paletteDefault()
            safe_params = params.replace('/', '-').replace(':', '-').replace(' ', '_')
            progBuilder.outputFile(f"{args.basename}-{cmd}{safe_params}.png")
            progBuilder.gRadarRay(radius=args.range.replace(',', ':'), azimuth=args.az_angle)
        else:
            logger.warning(f"Unsupported product: {args.PRODUCT}. Skipping.")


def handle_gnuplot(args, progBuilder: rack.core.Rack): #, **kw_args): #range_m:tuple=None, height_m:tuple=None):
    
    if not args.gnuplot:
        return
    
    terminal = args.gnuplot.split('.').pop()
    if terminal not in ['png', 'svg', 'tif']:
        logger.warning(f"Unsupported gnuplot terminal format: {terminal}, defaulting to png")
        terminal = 'png'

    gpl_plot = rack.prog.CommandSequence()
    gpl_plot.fmt = rack.gnuplot.GnuPlotFormatter(param_separator=',\n  ')
    gpl_plot.fmt.cmd_separator='\n'
    gpl_plot.fmt.CMD_SEPARATOR='\n'

    confCmdReg = rack.gnuplot.Registry(gpl_plot)
    plotCmdReg = rack.gnuplot.Registry(gpl_plot)
    
    # conf.comment("Generated by rack with --gnuplot option")
    # conf.comment("General settings")
    confCmdReg.terminal(rack.gnuplot.Terminal(terminal), size=args.size) # Hard-coded. See also handle_prhi for size handling.
    confCmdReg.output(args.gnuplot)

    confCmdReg.multiplot()    # (rows=1, cols=1)
    if args.title:
        confCmdReg.title(args.title)
    elif args.select:
        confCmdReg.title(f"Pseudo-RHI [{args.select}] {args.az_angle} deg")
    elif args.title is None:
        confCmdReg.title(f"Pseudo-RHI {args.az_angle} deg")


    confCmdReg.unset("tics")
    confCmdReg.grid(rack.gnuplot.Tics.XTICS, rack.gnuplot.Tics.YTICS)
    confCmdReg.unset("border")

    # consider single command with multiple parameters for margins, e.g. margin left 10, right 20, top 30, bottom 40
    """
    confCmdReg.comment("lmargin at screen 0.22")
    confCmdReg.comment("rmargin at screen 0.9")
    confCmdReg.comment("bmargin at screen 0.18")
    confCmdReg.comment("tmargin at screen 0.9")
    """
    
    # left, right, bottom, top in character units. See also margin command for more flexible margins in screen units.
    confCmdReg.set("margins 12, 4, 4, 3"  ) 

    # confCmdReg.comment("General settings")
    
    plotCmdReg.comment("Plotting the background image (radar beams)")
    #plotCmdReg.plot(filename=get_background_filename(args, prefixed=True), filetype="png", 
    plotCmdReg.plot(filename=f"{args.basename}-background.png", filetype="png",     
                    style=rack.gnuplot.Style.RGBIMAGE) # linecolor='rgb "gray"', linewidth=1)
    
    range_args = {
        "xrange": args.range,
        "yrange": args.height
    }
    for (k,v) in range_args.items():
        if type(v) == list:
            v = tuple(v)
        elif type(v) == str:
            v = tuple(int(i) for i in str(v).strip().split(':'))
        range_args[k] = v

    
    xrange = tuple(range_args.get('xrange', (-250000, 250000)))
    yrange = tuple(range_args.get('yrange', (0, 8000)))

    confCmdReg.xrange(xrange)
    confCmdReg.yrange(yrange)

    # See above unset border, to remove default border and tics,
    # and then add custom border back with margins
    confCmdReg.set("border") 
    confCmdReg.set("tics out nomirror scale 2")
    confCmdReg.set("mxtics 5")
    # confCmdReg.key(rack.gnuplot.Key.LEFT, outside=True)
    confCmdReg.xlabel(f'Range {xrange[0]} – {xrange[1]} m')
    confCmdReg.ylabel(f'Altitude {yrange[0]} – {yrange[1]} m')
    # script.extend(gpl_conf.to_list())
    confCmdReg.unset("key") # legend 

    plotCmdReg.comment("Plotting a dummy line (to ensure gnuplot output is not empty)")
    plotCmdReg.plot('x*0', style=rack.gnuplot.Style.LINES) # linecolor='rgb "gray"', linewidth=1)
    
    script = gpl_plot.to_string()  #"\n".join(script)
    if args.print:
        logger.info("# GnuPlot script:")
        print(script)
    
    # Always write the GnuPlot script to a file, even if not executing it,
    # for debugging and user reference. The filename is derived from the --gnuplot argument.
    if not args.gnuplot_script:
        args.gnuplot_script = f"{args.gnuplot}.gnu" 
    #args.gnuplot_script = get_background_filename(args, prefixed=False).removesuffix(f".{terminal}") + ".gnu"
    #script_filename = f"{args.gnuplot}.gnu" # TODO replace .png -> -png.gnu, .svg -> -svg.gnu, etc.
    with open(args.gnuplot_script, "w") as f:
        f.write(script)
        logger.info(f"GnuPlot script written to: {args.gnuplot_script}")
    
    return script


def compose_command(args) -> rack.prog.CommandSequence:
    """ Standard entry point.

    For example, test programs expect compose_command() as the interface for generating the command sequence to be tested,
    and exec_command() is the interface for executing the generated command sequence. 

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

    ensure_arguments(args, rackCmdReg)

    handle_infile(args, rackCmdReg)

    # RHI specific
    # Note: resolves args.size used by cartesian PRODUCT also.

    # Optional Cartesian
    handle_product(args, rackCmdReg)

    rackCmdReg.handle_published_cmd_args(args, rack.core.Rack.select)
    rackCmdReg.handle_published_cmd_args(args, rack.core.Rack.pPseudoRhi, True)

    handle_outfiles_prhi(args, rackCmdReg)

    handle_gnuplot(args, rackCmdReg)

    handle_style(args, rackCmdReg)

    if 'svg' in args.FORMAT:
        rackCmdReg.outputFile(f"{args.basename}.svg")
        args.FORMAT.remove('svg')

    if 'tif' in args.FORMAT:
        #cmdBuilder.outputConf("tif:tile=512")
        #cmdBuilder.outputFile(f"{args.basename}.tif")
        logger.warning("TIFF output is not currently supported, skipping.")
        args.FORMAT.remove('tif')

    if args.FORMAT:
        raise Exception('Unhandled formats:', args.FORMAT)
    
    return rackProg



"""
def exec_command(args):
    cmdList = compose_command(args)
    os.system(cmdList)  # subprocess!

def test():
    cmds = {
        "INPUTS" 
    }
    compose_command("")
""" 
    
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
        #test()
        sys.exit(0)

    prog = compose_command(args)

    if args.print:
        args.print = args.print.replace(r'\t','\t')
        args.print = args.print.replace(r'\n','\n')
        logger.info("# Rack cmd line:")
        #fmt = rack.prog.RackFormatter(params_format="'{params}'", cmd_separator=" \\\n\t")
        fmt = rack.cmdline.RackFormatter(params_format="'{params}'", cmd_separator=args.print)
        print(prog.to_string(fmt))
        #logger.info("# Python snippet:")
        #print(prog.to_python(prefix="Rack."))
        
    if args.exec:
        logger.info("# Executing Rack...")
        fmt = rack.cmdline.RackFormatter(params_format="'{params}'")
        #fmt = rack.cmdline.RackFormatter(params_format="'{params}'", cmd_separator=args.print)
        logger.debug(prog.to_string(fmt))
        #os.system(prog.to_string(fmt))
        
        fmt = rack.cmdline.RackFormatter()
        cmd = prog.to_token_list(fmt)
        #logger.debug(f"Executing command: {args}")
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if result.returncode != 0:
            if result.stdout:
                logger.info(f"stdout:\n{result.stdout.rstrip()}")
            if result.stderr:
                logger.warning(f"stderr:\n{result.stderr.rstrip()}")        
            logger.error(f"Command exited with code {result.returncode}")
        # os.system(prog.to_string(fmt))  # subprocess! 
        
        if args.gnuplot_script:
            cmd = ["gnuplot", args.gnuplot_script]
            logger.info(f"Executing GnuPlot script: {cmd}")
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            #, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
        #logger.info("# Executing GnuPlot script...")
        #os.system(f"gnuplot {script_filename}") 

    # "Rectified" command line
    line = rack.args.args_to_cli(parser, args)
    logger.debug(f"Python command line args: {line}")


if __name__ == "__main__":
    main()
