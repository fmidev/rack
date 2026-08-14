""" Rack command line argument generator

Utility for constructing strings that can be executed in shell.

    * property
    * property2

"""
import argparse
import sys
from pathlib import Path

import rack.log
import rack.prog
import rack.gnuplot
import rack.core
import rack.vertical


logger = rack.log.logger.getChild(Path(__file__).stem)



def build_parser():
    """ Creates registry of supported options of this script
    """
    parser = argparse.ArgumentParser(description="Vertical Profile of Reflectivity (VPR) generator")

    rack.prog.Register.expand_options(rack.core.Rack.pVerticalProfile, parser, name_mapper=True)

    parser.add_argument(
        # Uppercase option but lowercase member, for compatibility with rack.pseudorhi, 
        # where this parameter is native. 
        "--SIZE", # "--size",
        dest = "size",
        default="400,800",
        help="Size of the output gnuplot image in pixels, as width,height (e.g. 400,800).")

    parser.add_argument(
        "--XRANGE",
        default=None,
        help="Plot range, x axis")

    parser.add_argument(
        "--YRANGE",
        default=None,
        help="Plot range, y axis")

    rack.prog.Register.expand_options(rack.core.Rack.select, parser, name_mapper=True)

    rack.vertical.complete_arg_parser(parser)

    parser.add_argument(
        "--STYLE",
        default=".SECTOR=stroke:white;stroke-width:3",
        help="Adjust CSS styles for the SVG output")

    parser.add_argument(
        "--MASK",
        default="false",
        help="Masking of radar sector in SVG output. Options: 'true','false'")

    return parser





def write_output_vpr(args, cmdBuilder: rack.core.Rack):
    """ Link the gnuplot image (for SVG embedding) and write the profile data file. """

    if args.gnuplot:
        SIZE = str(args.size).replace(':', ',').split(',')
        cmdBuilder.cSize(SIZE[0], SIZE[1])
        p = rack.vertical.get_full_path(args.OUTDIR, args.gnuplot)
        if (p.is_absolute()):
            cmdBuilder.gLinkImage(p)
        else: 
            cmdBuilder.gLinkImage(f"{p.stem}{p.suffix}")
        #cmdBuilder.gLinkImage(f"{args.OUTDIR}{args.gnuplot}")

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
    # cmdBuilder.select(quantity=r'^COUNT$|^DBZH$|HGHT')
    # cmdBuilder.select(quantity=r'^COUNT$|^DBZH$|HGHT')
    # cmdBuilder.outputFile(f"{args.basename}.mat")
    cmdBuilder.outputFile(args.datafilename)

def create_gnuplot_script(args, progBuilder: rack.core.Rack):

    if not args.gnuplot:
        return

    terminal = rack.vertical.gnuplot_terminal(args)
    plotScript, plotBuilder = rack.vertical.gnuplot_new_script()

    
    plotBuilder.comment(f"GnuPlot script created by {__name__}")

    plotBuilder.terminal(rack.gnuplot.Terminal(terminal), size=args.size)
    plotBuilder.output(args.gnuplot)

   # Parse parameters used for title and axis ranges
    height_tuple = rack.typical(args.height, [int], "[:,]")
    
    if args.title:
        plotBuilder.title(args.title)
    else:
        range_tuple  = rack.typical(args.range, [int], "[:,]")
        azm_tuple    = rack.typical(args.azm,   [int], "[:,]")
        if azm_tuple != (0,0):
            plotBuilder.title(f"Vertical profile {range_tuple[0]/1000}–{range_tuple[1]/1000} km, {azm_tuple[0]}–{azm_tuple[1]}deg")
        else:
            plotBuilder.title(f"Vertical profile {range_tuple}m")

    plotBuilder.set("grid")

    plotBuilder.ylabel("HGHT (m)")

    # x1 (bottom): DBZH reflectivity  [TODO: add --XRANGE option]
    #confCmdReg.xlabel("DBZH")
    quantities = rack.typical(args.quantity, list, "[:,]")

    plotBuilder.xlabel(",".join(quantities))

    plotBuilder.comment(f"If needed, tune x range with --XRANGE")
    if args.XRANGE:
        plotBuilder.xrange(*rack.typical(args.XRANGE, [int], "[:,]"))
    
    #confCmdReg.xrange((-32, 60))

    # x2 (top): COUNT, log scale  [TODO: add --X2RANGE option]
    plotBuilder.x2label("count")  # (f"count({args.quantity})")
    plotBuilder.set("log x2")
    plotBuilder.x2range((1, 10000))
    plotBuilder.set("x2tics")

    # y: altitude
    plotBuilder.yrange(height_tuple)

    mat_file = str(rack.vertical.get_full_path(args.OUTDIR, args.datafilename))
    # args.datafilename
    column_index = 1 # HGHT, to be skipped
    plotBuilder.comment(f"Quantities: HGHT and {quantities}")

    """
    Column order
    # 1 HGHT (fixed)
    # 2 DBZH  (or other user-selected)
    # 3 COUNT (of that DBZH)
    Optionally:
    # 4 ZDR   (or other ...)
    # 5 COUNT (of quantity above)
    etc
    """

    plots = []
    for quantity in quantities:

        logger.debug(f"plotting quantity: {quantity}")

        column_index += 1
        plots.append(plotBuilder.plot_entry(
            filename=mat_file, using=f"{column_index}:1",
            style=rack.gnuplot.Style.LINES, title=quantity)
        )
        
        column_index += 1
        plots.append(plotBuilder.plot_entry(
            filename=mat_file, using=f"{column_index}:1",
            style=rack.gnuplot.Style.LINES, title=f"count({quantity})", axes="x2y1")
        )

    plotBuilder.plot(*plots)

    script_text = plotScript.to_string() + '\n'
    if args.print is not None:
        logger.info("# GnuPlot script:")
        print(script_text)
    rack.vertical.gnuplot_write_script(args, script_text)

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

    progBuilder = rack.core.Rack(rackProg)

    rack.vertical.initialize_rack(args, progBuilder)

    # Access to add "hidden" options.
    v = vars(args)

    # Hidden option: filename for output data.
    
    # VPR specific commands:
    # Visualize selection
    progBuilder.gRadarSector(
        radius  = rack.typical(args.range, [int], r'[,:]'), 
        azimuth = rack.typical(args.azm,   [int], r'[,:]'), 
        MASK=args.MASK)  #
        #MASK="true")
    progBuilder.handle_expanded_cmd_args(args, rack.core.Rack.select) # works bad!
    progBuilder.handle_expanded_cmd_args(args, rack.core.Rack.pVerticalProfile, True)

    v["datafilename"] = f'{args.basename}.mat'
    #rack.vertical.get_full_path(args.OUTDIR, f'{args.basename}.mat')
    #f'{args.basename}.mat'
    write_output_vpr(args, progBuilder)
    create_gnuplot_script(args, progBuilder)

    rack.vertical.finalize_svg_output(args, progBuilder)

    if args.FORMAT:
        raise Exception('Unhandled formats:', args.FORMAT)

    return rackProg


def main():
    rack.vertical.run_module(sys.modules[__name__])


if __name__ == "__main__":
    main()
