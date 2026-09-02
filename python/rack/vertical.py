"""Shared utilities for plot modules rack.pseudorhi and rack.vpr .

Modules conforming to rack.cmdline.RackModule — having module-level
build_parser() and compose_command() — can import these helpers and use
rack.cmdline.run_module() as their main().

Typical computation flow:

Rack
- Reads radar data input
- A meteorological product (like CAPPI 500) is computed and saved as PNG file
- Data for GnuPlot is produced (text dump for VPR, colour image for pseudo-RHI)
- An SVG image is created, displaying GnuPlot image together with the product image.
"""

import argparse
import sys
from pathlib import Path

from rack.args import export_defaults_to_json, load_config
from rack.cmdline import RackFormatter
import rack.cmdline
import rack.log
import rack.core
import rack.prog
import rack.gnuplot

logger = rack.log.logger.getChild(Path(__file__).stem)

def complete_arg_parser(parser: argparse.ArgumentParser):
    """ Creates registry of supported options of this script
    """
    parser.add_argument(
        "INFILE",
        nargs='*',
        help="Input file(s)")

    parser.add_argument(
        "--OUTDIR",
        type=str,
        metavar="<path>|AUTO",
        default=None,
        help="Common path of output files.")

    parser.add_argument(
        "--OUTFILE",
        default="",
        help="Output file (basename). See --FORMAT")
    
    parser.add_argument(
        "--PRODUCT",
        default=None,
        help="Compute a meteorological product and save it as a radar overview image (e.g. 'pCappi,1500'). "
             "The VPR selection area is overlaid using gRadarSector. Implies SVG output.")

    parser.add_argument(
        "--PALETTE",
        default='default',
        help="Add colours using a palette. Affects PNG image only.")

    parser.add_argument(
        "--FORMAT",
        default=None,
        help="One or several (auxiliary) file formats (mat, h5, png, svg)")


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
    
    rack.cmdline.add_parameters(parser)

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

    #parser.add_argument(
    #    "--STYLE",
    #    default=".SECTOR=stroke:white;stroke-width:3",
    #    help="Adjust CSS styles for the SVG output")

    # TODO: pick selected SVG commands and handling.
    parser.add_argument(
            "--svgRelativePaths",
            action='store_true',
            help="Strip prefix (--OUTDIR) from image links (for WWW usage)")

    parser.add_argument(
        "--svgAlign",
        #metavar="[TOP|BOTTOM|LEFT|RIGHT]",
        metavar="[TOP|BOTTOM|RIGHT]",
        # metavar="[HORZ|VERT]",
        default="",
        help="Position of the plot image wrt. radar image")


def initialize_rack(args, rackCmdReg: rack.core.Rack):

    # Set Python logging verbosity, and also rack verbosity with verbosityKey
    # Handle --log_level <level>, --debug, --verbose
    # verbosityKey = rack.log.handle_parameters(args)
    rackCmdReg.verbose(level=rack.log.handle_parameters(args))

    ensure_arguments(args, rackCmdReg)

    handle_infile(args, rackCmdReg)

    # Optional Cartesian overview with sector indicator
    handle_horz_product(args, rackCmdReg)

def get_full_path(prefix, filename) -> Path:
    if prefix:
        return Path(prefix, filename)
    else:
        return Path(filename)

def ensure_arguments(args, cmdBuilder: rack.core.Rack):
    """ Ensure required arguments are present and set defaults for optional ones.

        Also, perform any necessary transformations on the arguments, 
        e.g. parsing a comma-separated string into a list.

        Example: args.SIZE: "800,600" -> (800, 600)

        Also, adds "hidden" arguments to the args namespace, e.g. args.basename,
        which are derived from the provided arguments and used later in the command generation.        
    """
    

    v = vars(args)

    if not args.OUTFILE:
        args.OUTFILE = 'profile.mat'

    p = Path(args.OUTFILE)

    if not args.OUTDIR and (p.parent != Path('.')):
        # Take common prefix of input files, if any, else current directory
        args.OUTDIR = p.parent
        args.OUTFILE = f"{p.stem}{p.suffix}"
        p = Path(args.OUTFILE)
        logger.info(f"args.OUTDIR={args.OUTDIR}")
        

    # Hidden argument: basename of the output file (without path or extension)
    if p.parent != Path('.'):
        v["basename"] = str(Path(p.parent, p.stem))
    else:
        v["basename"] = p.stem
    # else:
    #    args.OUTDIR = Path(args.OUTDIR,p.parent)
    # args.OUTFILE = f"{p.stem}{p.suffix}"

    if args.OUTDIR:
        args.OUTDIR = str(args.OUTDIR)
        # Ensure trailing slash for OUTDIR, so that it can be used as a prefix for output files
        args.OUTDIR = args.OUTDIR.rstrip('/') + '/'

    logger.info(f"args.OUTDIR={args.OUTDIR}")
    logger.info(f"args.OUTFILE={args.OUTFILE}")
    logger.info(f"args.basename={args.basename}")

    #if args.size:
    #    args.size = rack.typical(args.size, tuple,',')

    if args.FORMAT:
        args.FORMAT = set(args.FORMAT.strip().split(','))
    else:
        args.FORMAT = set()
    args.FORMAT.add(p.suffix.strip('.'))

    if args.PRODUCT:
        logger.info("Generating optional radar image with sector indicator")
        if not ('svg' in args.FORMAT):
            logger.info("--FORMAT {args.FORMAT} : adding 'svg', to show both plot and radar images")
            args.FORMAT.add('svg')

    # logger.warning(f"Output formats: {args.FORMAT}") 

    cmdBuilder.gTitle('${what:date|%Y-%m-%d} ${what:time|%H:%M} ${NOD}-${PLC}')
    # spoils vertical layout...
    #cmdBuilder.gGroupTitle('${what:product} ${what:prodpar} ${what:quantity}')
    
    if args.svgRelativePaths:
        cmdBuilder.outputConf("svg:paths=RELATIVE")
    else:
        cmdBuilder.outputConf("svg:paths=ABSOLUTE")

    if args.svgAlign:
        align = args.svgAlign.upper()
        if align == 'RIGHT':
            cmdBuilder.gLayout("HORZ", directionHorz="RIGHT")
        elif align == 'LEFT':
            logger.warning("LEFT alignment is not supported yet. Using RIGHT instead.")
            cmdBuilder.gLayout("HORZ", directionHorz="RIGHT")
            #cmdBuilder.gLayout("VERT", "DOWN", "LEFT")
        elif align == 'TOP':
            cmdBuilder.gGroupTitle('${what:product} ${what:prodpar} ${what:quantity}')
            cmdBuilder.gLayout("HORZ", "UP", "RIGHT")
        elif align == 'BOTTOM':
            cmdBuilder.gGroupTitle('${what:product} ${what:prodpar} ${what:quantity}')
            cmdBuilder.gLayout("HORZ", "DOWN", "RIGHT")
        else:
            logger.error(f"Unsupported svgAlign value: {args.svgAlign}. Ignoring.")
            exit(2)


    if 'svg' in args.FORMAT:
        if not args.exec:
            logger.info("--exec implicitly set (to generate PNG images for the SVG file)")
            args.exec = True  # force execution to generate the SVG output
        if not args.gnuplot:
            args.gnuplot = f"{p.stem}-gnuplot.png"
            if (args.OUTDIR):
                args.gnuplot = Path(args.OUTDIR, args.gnuplot)    

    if (args.OUTDIR):
        cmdBuilder.outputPrefix(args.OUTDIR)

    logger.warning(f"args.FORMAT={args.FORMAT}")

def handle_infile(args, progBuilder: rack.core.Rack):
    """Add input file(s) to the rack command sequence."""
    logger.debug(f"INFILE {args.INFILE}")
    if not args.INFILE:
        logger.error("No input file specified.")
        sys.exit(1)
    if isinstance(args.INFILE, str):
        args.INFILE = [args.INFILE]
    for f in args.INFILE:
        progBuilder.inputFile(f)


def handle_horz_product(args, progBuilder: rack.core.Rack):
    """ Compute an auxiliary product (e.g. CAPPI) as a radar overview image.
        Overlay gRadarSector to show the VPR selection area (range + azimuth sector).
    """

    if not args.PRODUCT:
        return
    
    logger.warning(args.select)

    safe_id = args.PRODUCT 
    if args.PRODUCT == "sweep":
        if args.prf:
            progBuilder.select(quantity=args.quantity, prf=args.prf)
        else:
            progBuilder.select(quantity=args.quantity)
    elif args.PRODUCT == args.PRODUCT.upper():
        # Sweep (select based on QUANTITY)
        if args.prf:
            progBuilder.select(quantity=args.PRODUCT, prf=args.prf)
        else:
            progBuilder.select(quantity=args.PRODUCT)
        # args.PRODUCT = ''
    else:
        if args.prf:
            progBuilder.select(prf=args.prf)
        # Todo: consider other selection criteria.
    
        product = args.PRODUCT.split(',', 1)
        params = None
        cmd = product[0].strip()
        if len(product) == 2:
            params = product[1].strip()
        #cmd, params = args.PRODUCT.split(',', 1)
        #cmd = cmd.strip()
        #params = params.strip()

        if not hasattr(rack.core.Rack, cmd):
            logger.warning(f"Unsupported product: {args.PRODUCT}. Skipping.")
            return

        rackCmd = getattr(rack.core.Rack, cmd)
        logger.info(f"Adding product cmd: {cmd}, params: {params}")
        if params:
            rackCmd(progBuilder, *params.split(','))
        else:
            rackCmd(progBuilder)

        safe_id = cmd + params.replace('/', '-').replace(':', '-').replace(' ', '_')
        # SIZE = str(args.SIZE).replace(':', ',').split(',')
        # side = SIZE[1]  # use height dimension for the square radar overview
        #SIZE = str(args.size).replace(':', ',').split(',')


    SIZE = rack.typical(args.size, [int], ',')

    logger.warning(f"Parsed size: {SIZE}")
    if args.svgAlign in ['TOP', 'BOTTOM']:
        progBuilder.cSize(SIZE[0],SIZE[0]) 
    else:
        progBuilder.cSize(SIZE[1],SIZE[1]) 
    #progBuilder.cSize(side, side)
    progBuilder.cCreate()
    progBuilder.paletteDefault()

    #safe_params = params.replace('/', '-').replace(':', '-').replace(' ', '_')
    #progBuilder.outputFile(f"{args.basename}-{cmd}{safe_params}.png")
    progBuilder.outputFile(f"{args.basename}-{safe_id}.png")
        

def finalize_svg_output(args, cmdBuilder: rack.core.Rack):
    """Apply CSS style settings to the rack command sequence."""
    cmdBuilder.gStyle(".IMAGE_BORDER=stroke:gray")
    if getattr(args, 'STYLE', None):
        for style in args.STYLE.strip().split('|'):
            cmdBuilder.gStyle(style.strip())

    if 'svg' in args.FORMAT:
        cmdBuilder.outputFile(f"{args.basename}.svg")
        args.FORMAT.remove('svg')



def gnuplot_terminal(args: argparse.Namespace) -> str:
    """Extract and validate the gnuplot terminal type from the output filename."""
    terminal = str(args.gnuplot).rsplit('.', 1)[-1]
    if terminal not in ['png', 'svg', 'tif']:
        logger.warning(f"Unsupported gnuplot terminal format: {terminal}, defaulting to png")
        terminal = 'png'
    return terminal


def gnuplot_new_script() -> tuple:
    """Create a fresh GnuPlot CommandSequence and a Registry builder for it.

    Returns (plotScript, plotBuilder) ready for adding commands.
    """
    plotScript = rack.prog.CommandSequence()
    plotScript.fmt = rack.gnuplot.GnuPlotFormatter(param_separator=',\n  ')
    plotScript.fmt.CMD_SEPARATOR = '\n'
    plotBuilder = rack.gnuplot.Registry(plotScript)
    return plotScript, plotBuilder


def gnuplot_write_script(args, script_text: str):
    """Write *script_text* to file and record the filename in args.gnuplot_script."""
    if not args.gnuplot_script:
        args.gnuplot_script = f"{args.gnuplot}.gnu"
    with open(args.gnuplot_script, "w") as f:
        f.write(script_text)
    logger.info(f"GnuPlot script written to: {args.gnuplot_script}")


def run_module(module):
    """Drive a RackModule as a standalone program.

    Handles the common main() flow shared by all profile plot modules:
    parse args, compose the rack command sequence, print and/or execute it,
    then run gnuplot if a script was generated.

    Usage in any conforming module's main():
        def main():
            rack.vertical.run_module(sys.modules[__name__])
    """
    import subprocess
    import sys
    import rack.log
    import rack.args
    from rack.args import load_config

    def handle_result(result: subprocess.CompletedProcess, info:str):
        if result.returncode != 0:
            if result.stdout:
                logger.info(f"stdout:\n{result.stdout.rstrip()}")
            if result.stderr:
                logger.warning(f"stderr:\n{result.stderr.rstrip()}")
            logger.warning(f"Failed: \n{info}")
            logger.error(f"Command exited with code {result.returncode}")
            exit(result.returncode)

    parser = module.build_parser()
    rack.log.add_parameters(parser)

    # Apply JSON config if --config was given (before full parse)
    known_args, _ = parser.parse_known_args()
    if getattr(known_args, 'config', None):
        parser.set_defaults(**load_config(known_args.config))

    args = parser.parse_args()

    if getattr(args, 'export_config', None):
        export_defaults_to_json(parser, args, args.export_config)
        sys.exit(0)

    if getattr(args, 'test', False):
        logger.info("Running tests..")
        sys.exit(0)

    if not args.INFILE:
        parser.print_help()
        sys.exit(0)

    prog = module.compose_command(args)

    # Shared: apply --exec default, handle --print and --rack_script.
    rack.cmdline.handle_parameters(prog, args, logger)

    # Specialized: this module chains a gnuplot run after a successful exec.
    if getattr(args, 'exec', False):
        logger.info("# Executing Rack...")
        fmt = RackFormatter(params_format="'{params}'")
        logger.debug(prog.to_string(fmt))
        fmt = RackFormatter()
        cmd = prog.to_token_list(fmt)
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        handle_result(result, cmd) # prog.to_string(fmt))
        
        if getattr(args, 'gnuplot_script', None):
            gnuplot_cmd = "gnuplot"
            cmd = [gnuplot_cmd, args.gnuplot_script]
            logger.info(f"# Executing GnuPlot script: {gnuplot_cmd} {args.gnuplot_script}")
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            handle_result(result, cmd)

    line = rack.args.args_to_cli(parser, args)
    logger.warning(f"Python command line args: {line}")


