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

from rack import args, script
import rack.log
import rack.prog
#import rack.cmdline
import rack.gnuplot
import rack.core
#from 
import rack.vertical
#from rack.vertical import ensure_arguments 


logger = rack.log.logger.getChild(Path(__file__).stem)

reg = rack.prog.Register


def build_parser():
    """ Creates registry of supported options of this script
    """
    parser = argparse.ArgumentParser(description="Vertical Profile of Reflectivity (VPR) generator")


    
    def camel_to_upper_underscore(name: str) -> str:
        s1 = re.sub(r'(.)([A-Z][a-z]+)', r'\1_\2', name)
        s2 = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1)
        return s2.upper()

    rack.prog.Register.expand_options(rack.core.Rack.pVerticalProfile, parser, name_mapper=camel_to_upper_underscore)

    parser.add_argument(
        # Uppercase option but lowercase member, for compatibility with rack.pseudorhi, 
        # where this parameter is native. 
        "--SIZE", # "--size",
        dest = "size",
        default="400,800",
        help="Size of the output gnuplot image in pixels, as width,height (e.g. 400,800).")

    rack.prog.Register.expand_options(rack.core.Rack.select, parser, name_mapper=camel_to_upper_underscore)

    rack.vertical.complete_arg_parser(parser)

    parser.add_argument(
        "--STYLE",
        default=".SECTOR=stroke:white;stroke-width:3",
        help="Adjust CSS styles for the SVG output")

    #parser.add_argument(
    #    "--test",
    #    action='store_true',
    #    help="run some tests")


    return parser



def handle_graphics(args, progBuilder: rack.core.Rack):
    progBuilder.gRadarRay(radius=args.range.replace(',', ':'), azimuth=args.az_angle)

def handle_vert_product(args, progBuilder: rack.core.Rack):

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
        SIZE = str(args.size).replace(':', ',').split(',')
        cmdBuilder.cSize(SIZE[0], SIZE[1])
        cmdBuilder.gLinkImage(f"{args.OUTDIR}{args.gnuplot}")

    # optional: PNG, if az_slots?

    if 'h5' in args.FORMAT:
        cmdBuilder.outputFile(f"{args.basename}.h5")
        args.FORMAT.remove('h5')

    if 'mat' in args.FORMAT:
        args.FORMAT.remove('mat')
    else:
        logger.warning("No output format '.mat' specified – current version forces it.")
        # for profile data. Use --FORMAT mat or --FORMAT h5 to save the profile data.")
        # exit(1)
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

    confCmdReg.terminal(rack.gnuplot.Terminal(terminal), size=args.size)
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
    if args.print != None:
        if args.print == '':
            args.print = ' \\n  '  # default separator 
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

    rack.vertical.initialize_rack(args, rackCmdReg)

    # VPR specific commands:
    handle_vert_product(args, rackCmdReg)
    rackCmdReg.handle_published_cmd_args(args, rack.core.Rack.select)
    rackCmdReg.handle_published_cmd_args(args, rack.core.Rack.pVerticalProfile, True)
    handle_outfiles_vpr(args, rackCmdReg)
    handle_gnuplot(args, rackCmdReg)

    rack.vertical.finalize_svg_output(args, rackCmdReg)



    if args.FORMAT:
        raise Exception('Unhandled formats:', args.FORMAT)

    return rackProg


def main():
    rack.vertical.run_module(sys.modules[__name__])


if __name__ == "__main__":
    main()
