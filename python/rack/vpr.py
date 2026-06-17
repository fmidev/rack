""" Rack command line argument generator

Utility for constructing strings that can be executed in shell.

    * property
    * property2

"""
import argparse
import sys
from pathlib import Path
import os
import re
import inspect
import logging

from rack import script
import rack.log
import rack.prog
import rack.cmdline
import rack.gnuplot
import rack.core
from rack.plot_common import handle_infile, handle_style


logger = rack.log.logger.getChild(Path(__file__).stem)

reg = rack.prog.Register


def build_parser():
    """ Creates registry of supported options of this script
    """
    parser = argparse.ArgumentParser(description="Vertical Profile of Reflectivity (VPR) generator")

    parser.add_argument(
        "INFILE",
        nargs='*',
        help="Input files")

    parser.add_argument(
        "--OUTFILE",
        default="",
        help="Output file (basename). See --FORMAT")

    def camel_to_upper_underscore(name: str) -> str:
        s1 = re.sub(r'(.)([A-Z][a-z]+)', r'\1_\2', name)
        s2 = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1)
        return s2.upper()

    rack.prog.Register.expand_options(rack.core.Rack.pVerticalProfile, parser, name_mapper=camel_to_upper_underscore)

    rack.prog.Register.expand_options(rack.core.Rack.select, parser, name_mapper=camel_to_upper_underscore)

    parser.add_argument(
        "--PRODUCT",
        default=None,
        help="Compute a meteorological product and save it as a radar overview image (e.g. 'pCappi,1500'). "
             "The VPR selection area is overlaid using gRadarSector. Implies SVG output.")

    parser.add_argument(
        "--SIZE",
        default="400,800",
        help="Size of the output gnuplot image in pixels, as width,height (e.g. 400,800).")

    parser.add_argument(
        "--PALETTE",
        default='default',
        help="Add colours using a palette. Affects PNG image only.")

    parser.add_argument(
        "--FORMAT",
        default=None,
        help="One or several file formats (mat, h5, png, svg)")

    parser.add_argument(
        "--OUTDIR",
        type=str,
        metavar="<path>|AUTO",
        default=None,
        help="Common path of output files.")

    parser.add_argument(
        "--config",
        help="Path to JSON config file")

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
        help="Generate GnuPlot image (e.g. 'my-vpr.png')")

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
        help="print parsed commands with given line separator (e.g. '\\n' or '\\t')")

    parser.add_argument(
        "--test",
        action='store_true',
        help="run some tests")

    parser.add_argument(
        "--STYLE",
        default=".SECTOR=stroke:white;stroke-width:3",
        help="Adjust CSS styles for the SVG output")

    parser.add_argument(
        "--ALIGN",
        metavar="[TOP|BOTTOM|LEFT|RIGHT]",
        default="",
        help="Position of the Pseudo RHI image wrt. radar image")

    return parser


# Utilities load_config, read_default_args, export_defaults_to_json,
# handle_infile, handle_style imported from rack.plot_common above.


def ensure_arguments(args, cmdBuilder: rack.core.Rack):
    """ Ensure required arguments are present and set defaults for optional ones.

        Adds 'hidden' arguments to the args namespace derived from provided arguments,
        e.g. args.basename, used later in command generation.
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

    if args.FORMAT:
        args.FORMAT = set(args.FORMAT.strip().split(','))
    else:
        args.FORMAT = set()
    args.FORMAT.add(p.suffix.strip('.'))

    if args.PRODUCT:
        logger.debug("An auxiliary radar overview image with sector indicator is requested")
        args.FORMAT.add('svg')

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
            

    if 'svg' in args.FORMAT:
        if not args.gnuplot:
            args.gnuplot = f"{p.stem}-gnuplot.png"

    cmdBuilder.outputPrefix(args.OUTDIR)

    logger.debug(f"args.OUTDIR={args.OUTDIR}")
    logger.debug(f"args.OUTFILE={args.OUTFILE}")
    logger.debug(f"args.FORMAT={args.FORMAT}")
    logger.debug(f"args.basename={args.basename}")


def handle_product(args, progBuilder: rack.core.Rack):
    """ Compute an auxiliary product (e.g. CAPPI) as a radar overview image.
        Overlay gRadarSector to show the VPR selection area (range + azimuth sector).
    """

    if not args.PRODUCT:
        return

    cmd, params = args.PRODUCT.split(',', 1)
    cmd = cmd.strip()
    params = params.strip()

    if not hasattr(rack.core.Rack, cmd):
        logger.warning(f"Unsupported product: {args.PRODUCT}. Skipping.")
        return

    rackCmd = getattr(rack.core.Rack, cmd)
    logger.info(f"Adding product cmd: {cmd}, params: {params}")
    rackCmd(progBuilder, *params.split(','))

    SIZE = str(args.SIZE).replace(':', ',').split(',')
    #side = SIZE[1]  # use height dimension for the square radar overview
    #SIZE = str(args.size).replace(':', ',').split(',')
    if args.ALIGN in ['TOP', 'BOTTOM']:
        progBuilder.cSize(SIZE[0],SIZE[0]) 
    else:
        progBuilder.cSize(SIZE[1],SIZE[1]) 
    #progBuilder.cSize(side, side)
    progBuilder.cCreate()
    progBuilder.paletteDefault()

    safe_params = params.replace('/', '-').replace(':', '-').replace(' ', '_')
    progBuilder.outputFile(f"{args.basename}-{cmd}{safe_params}.png")

    # range is already in metres; pass directly to gRadarSector
    range_raw = str(args.range) if hasattr(args, 'range') and args.range is not None else "10:100"
    azm_raw = str(args.azm) if hasattr(args, 'azm') and args.azm is not None else "0:359.99"

    progBuilder.gRadarSector(
        radius=range_raw.replace(',', ':'),
        azimuth=azm_raw.replace(',', ':'),
        MASK=True)


def handle_outfiles_vpr(args, cmdBuilder: rack.core.Rack):
    """ Link the gnuplot image (for SVG embedding) and write the profile data file. """

    if args.gnuplot:
        SIZE = str(args.SIZE).replace(':', ',').split(',')
        cmdBuilder.cSize(SIZE[0], SIZE[1])
        cmdBuilder.gLinkImage(f"{args.OUTDIR}{args.gnuplot}")

    cmdBuilder.select(quantity=r'^COUNT$|^DBZH$|HGHT')
    cmdBuilder.outputFile(f"{args.basename}.mat")


def handle_gnuplot(args, progBuilder: rack.core.Rack):

    if not args.gnuplot:
        return

    terminal = args.gnuplot.split('.').pop()
    if terminal not in ['png', 'svg', 'tif']:
        logger.warning(f"Unsupported gnuplot terminal format: {terminal}, defaulting to png")
        terminal = 'png'

    gpl_plot = rack.prog.CommandSequence()
    gpl_plot.fmt = rack.gnuplot.GnuPlotFormatter(param_separator=',\n  ')
    gpl_plot.fmt.cmd_separator = '\n'
    gpl_plot.fmt.CMD_SEPARATOR = '\n'

    confCmdReg = rack.gnuplot.Registry(gpl_plot)
    plotCmdReg = rack.gnuplot.Registry(gpl_plot)

    confCmdReg.terminal(rack.gnuplot.Terminal(terminal), size=args.SIZE)
    confCmdReg.output(args.gnuplot)

    # Parse parameters used for title and axis ranges
    range_raw = str(args.range).replace(',', ':') if hasattr(args, 'range') and args.range else "?"
    azm_raw   = str(args.azm).replace(',', ':')   if hasattr(args, 'azm')   and args.azm   else "?"

    height_raw = args.height if hasattr(args, 'height') and args.height is not None else "0:12000"
    if isinstance(height_raw, list):
        height_tuple = tuple(height_raw)
    elif isinstance(height_raw, str):
        height_tuple = tuple(int(i) for i in height_raw.replace(',', ':').split(':'))
    else:
        height_tuple = (0, 12000)

    if args.title:
        confCmdReg.title(args.title)
    elif azm_raw != "?":
        confCmdReg.title(f"Vertical profile ({range_raw}m, {azm_raw}deg)")
    else:
        confCmdReg.title(f"Vertical profile ({range_raw}m)")

    confCmdReg.set("grid")

    confCmdReg.ylabel("HGHT (m)")

    # x1 (bottom): DBZH reflectivity  [TODO: add --XRANGE option]
    confCmdReg.xlabel("DBZH")
    confCmdReg.xrange((-32, 60))

    # x2 (top): COUNT, log scale  [TODO: add --X2RANGE option]
    confCmdReg.x2label("count(DBZH)")
    confCmdReg.set("log x2")
    confCmdReg.x2range((1, 10000))
    confCmdReg.set("x2tics")

    # y: altitude
    confCmdReg.yrange(height_tuple)

    # .mat text file columns: 1=HGHT, 2=DBZH, 3=COUNT
    # quantities = ["DBZH", "COUNT"]  # TODO: make configurable via --QUANTITIES
    mat_file = f"{args.basename}.mat"
    e_dbzh  = plotCmdReg.plot_entry(
        filename=mat_file, using="2:1",
        style=rack.gnuplot.Style.LINES, title="DBZH")
    e_count = plotCmdReg.plot_entry(
        filename=mat_file, using="3:1",
        style=rack.gnuplot.Style.LINES, title="count(DBZH)", axes="x2y1")
    plotCmdReg.plot(e_dbzh, e_count)

    script_text = gpl_plot.to_string()
    if args.print:
        logger.info("# GnuPlot script:")
        print(script_text)

    if not args.gnuplot_script:
        args.gnuplot_script = f"{args.gnuplot}.gnu"
    with open(args.gnuplot_script, "w") as f:
        f.write(script_text)
        logger.info(f"GnuPlot script written to: {args.gnuplot_script}")

    return script_text


def compose_command(args) -> rack.prog.CommandSequence:
    """ Standard entry point.

    For example, test programs expect compose_command() as the interface for generating
    the command sequence to be tested, and exec_command() is the interface for executing it.

    Args:
        args (argparse.Namespace): Parsed arguments from `build_parser()`
    """

    global logger

    if isinstance(args, dict):
        args = argparse.Namespace(**args)

    rackProg = rack.prog.CommandSequence(programName='rack', quote="'")

    rackCmdReg = rack.core.Rack(rackProg)

    verbosityKey = rack.log.handle_parameters(args)
    rackCmdReg.verbose(level=verbosityKey)

    ensure_arguments(args, rackCmdReg)

    handle_infile(args, rackCmdReg)

    # Optional Cartesian overview with sector indicator
    handle_product(args, rackCmdReg)

    rackCmdReg.handle_published_cmd_args(args, rack.core.Rack.select)
    rackCmdReg.handle_published_cmd_args(args, rack.core.Rack.pVerticalProfile, True)

    handle_outfiles_vpr(args, rackCmdReg)

    handle_gnuplot(args, rackCmdReg)

    handle_style(args, rackCmdReg)

    if 'svg' in args.FORMAT:
        rackCmdReg.outputFile(f"{args.basename}.svg")
        args.FORMAT.remove('svg')

    if 'mat' in args.FORMAT:
        args.FORMAT.remove('mat')

    if args.FORMAT:
        raise Exception('Unhandled formats:', args.FORMAT)

    return rackProg


def main():
    rack.cmdline.run_module(sys.modules[__name__])


if __name__ == "__main__":
    main()
