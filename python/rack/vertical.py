"""Shared utilities for rack profile plot modules (pseudorhi, vpr, etc.).

Modules conforming to rack.cmdline.RackModule — having module-level
build_parser() and compose_command() — can import these helpers and use
rack.cmdline.run_module() as their main().
"""
import argparse
import sys
from pathlib import Path

from rack.args import export_defaults_to_json, load_config
from rack.cmdline import RackFormatter, logger
import rack.log
import rack.core
from rack.vpr import logger

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
    
    parser.add_argument(
        "-e", "--exec",
        action='store_true',
        help="execute parsed command")


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

    #parser.add_argument(
    #    "--STYLE",
    #    default=".SECTOR=stroke:white;stroke-width:3",
    #    help="Adjust CSS styles for the SVG output")

    parser.add_argument(
        "--ALIGN",
        #metavar="[TOP|BOTTOM|LEFT|RIGHT]",
        metavar="[BOTTOM|RIGHT]",
        default="",
        help="Position of the Pseudo RHI image wrt. radar image")


def initialize_rack(args, rackCmdReg: rack.core.Rack):

    # Set Python logging verbosity, and also rack verbosity with verbosityKey
    # Handle --log_level <level>, --debug, --verbose
    # verbosityKey = rack.log.handle_parameters(args)
    rackCmdReg.verbose(level=rack.log.handle_parameters(args))

    rack.vertical.ensure_arguments(args, rackCmdReg)

    rack.vertical.handle_infile(args, rackCmdReg)

    # Optional Cartesian overview with sector indicator
    rack.vertical.handle_horz_product(args, rackCmdReg)


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

    v["basename"] = p.stem

    if not args.OUTDIR:
        args.OUTDIR = p.parent
        args.OUTFILE = f"{p.stem}.{p.suffix}"

    args.OUTDIR = str(args.OUTDIR)

    if args.OUTDIR:
        args.OUTDIR = args.OUTDIR.rstrip('/') + '/'

    #if args.size:
    #    args.size = rack.typical(args.size, tuple,',')

    if args.FORMAT:
        args.FORMAT = set(args.FORMAT.strip().split(','))
    else:
        args.FORMAT = set()
    args.FORMAT.add(p.suffix.strip('.'))

    if args.PRODUCT:
        logger.debug("An auxiliary radar overview image with sector indicator is requested")
        args.FORMAT.add('svg')

    cmdBuilder.gTitle('${what:date|%Y-%m-%d} ${what:time|%H:%M} ${NOD}-${PLC}')
    # spoils vertical layout...
    #cmdBuilder.gGroupTitle('${what:product} ${what:prodpar} ${what:quantity}')

    if args.ALIGN:
        align = args.ALIGN.upper()
        if align == 'TOP':
            cmdBuilder.gLayout("VERT", "DOWN", "LEFT")
        elif align == 'BOTTOM':
            cmdBuilder.gLayout("VERT", "DOWN", "LEFT")
        elif align == 'LEFT':
            cmdBuilder.gGroupTitle('${what:product} ${what:prodpar} ${what:quantity}')
            cmdBuilder.gLayout("HORZ", "UP", "RIGHT")
        elif align == 'RIGHT':
            cmdBuilder.gGroupTitle('${what:product} ${what:prodpar} ${what:quantity}')
            cmdBuilder.gLayout("HORZ", "DOWN", "RIGHT")
        else:
            logger.error(f"Unsupported ALIGN value: {args.ALIGN}. Ignoring.")
            exit(2)


    if 'svg' in args.FORMAT:
        if not args.gnuplot:
            args.gnuplot = f"{p.stem}-gnuplot.png"

    cmdBuilder.outputPrefix(args.OUTDIR)

    logger.debug(f"args.OUTDIR={args.OUTDIR}")
    logger.debug(f"args.OUTFILE={args.OUTFILE}")
    logger.debug(f"args.FORMAT={args.FORMAT}")
    logger.debug(f"args.basename={args.basename}")

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
            params = product[0].strip()
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
    if args.ALIGN in ['TOP', 'BOTTOM']:
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

    prog = module.compose_command(args)


    if args.print == '':
        args.print = r' \\n  '  # default separator 
    
    if args.print:
        logger.info("# Rack cmd:")
        sep = args.print.replace(r'\t', '\t').replace(r'\n', '\n')
        fmt = RackFormatter(params_format="'{params}'", cmd_separator=sep)
        print(prog.to_string(fmt))

    if getattr(args, 'exec', False):
        logger.info("# Executing Rack...")
        fmt = RackFormatter(params_format="'{params}'")
        logger.debug(prog.to_string(fmt))
        fmt = RackFormatter()
        cmd = prog.to_token_list(fmt)
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if result.returncode != 0:
            if result.stdout:
                logger.info(f"stdout:\n{result.stdout.rstrip()}")
            if result.stderr:
                logger.warning(f"stderr:\n{result.stderr.rstrip()}")
            logger.warning("Failed: \n"+prog.to_string(fmt))
            logger.error(f"Command exited with code {result.returncode}")
            exit(result.returncode)

        if getattr(args, 'gnuplot_script', None):
            gnuplot_cmd = "gnuplot"
            cmd = [gnuplot_cmd, args.gnuplot_script]
            logger.info(f"# Executing GnuPlot script: {gnuplot_cmd} {args.gnuplot_script}")
            subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    line = rack.args.args_to_cli(parser, args)
    logger.warning(f"Python command line args: {line}")


