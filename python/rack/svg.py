import pathlib # Path

import rack.core
from rack.style import *

import logging
logging.basicConfig(format='%(levelname)s\t %(name)s: %(message)s')
logger = logging.getLogger(pathlib.Path(__file__).stem) 
logger.setLevel(logging.INFO)

 
def add_parameters(parser, path_prefix=None):

    """ Creates registry of options supported by this script
    parser = argparse.ArgumentParser(description="Example with x support")
    """
    cmdBuilder = rack.core.Rack()

    parser.add_argument(
        "--svgBackGround",
        default = "",
        metavar = "<file>",
        help="Add map etc. under actual data image layer")

    parser.add_argument(
        "--svgGroupTitle",
        default = "",
        metavar = "[RADAR|TIME|DATETIME|DATE|<text>]",
        help="Set titles for vert/horz rows.")

    parser.add_argument(
        "--svgLayout",
        default = "",
        metavar = cmdBuilder.gLayout().get_key_list(),
        help="Direction and orientation of image panels")

    parser.add_argument(
        "--svgOutputs",
        default = None,
        metavar = "auto|DATA,WEIGHT,...",
        help="Which fields of composite to include (see --EXTRACT)")

    parser.add_argument(
        "--svgRelativePaths",
        action='store_true',
        help="Strip prefix (--OUTDIR) from image links (for WWW usage)")

    parser.add_argument(
        "--svgTitle",
        default = "",
        metavar = "<text>",
        help="Set main title")



 

def handle_conf(args, cmdBuilder: rack.core.Rack):

    if args.svgLayout:
        cmdBuilder.gLayout(str(args.svgLayout).split(","))

    if args.svgRelativePaths:
        cmdBuilder.outputConf("svg:paths=PREFIXED")
    else:
        cmdBuilder.outputConf("svg:paths=ABSOLUTE")
    # TODO test (difficult: internal params)

    if args.svgTitle:
        cmdBuilder.gTitle(args.svgTitle)

    if args.svgGroupTitle:
        if args.svgGroupTitle=='RADAR':
            cmdBuilder.gTitle("${PLC} [${NOD}]")
        elif args.svgGroupTitle=='DATE':
            cmdBuilder.gTitle("${what:date|%Y/%m/%d}")
        elif args.svgGroupTitle=='TIME':
            cmdBuilder.gTitle("${what:time|%H:%M} UTC")
        elif args.svgGroupTitle=='DATETIME':
            cmdBuilder.gTitle("${what:date|%Y/%m/%d} ${what:time|%H:%M} UTC")
        else:
            cmdBuilder.gTitle(args.svgGroupTitle)
    
    
    #if args.svgRelativePaths:
def handle_outfiles(args, cmdBuilder: rack.core.Rack):
    """Light adjustments to args.OUTFILE"""

    logStyle = Style(Emoji.DISC, Color.MAGENTA)

    logger.info(logStyle.add(Emoji.SUCCESS).str(f"Handling {args.svgOutputs}"))
        
    if not args.OUTFILE:
        args.OUTFILE = 'out.svg'

    if not cmdBuilder:
        logger.error("No cmdBuilder set")    
        return

    outputs = args.svgOutputs

    if not outputs:
        return
    elif isinstance(outputs, str):
        if outputs == "auto":
            if args.EXTRACT:
                outputs = args.EXTRACT.split(",")
            else:
                outputs = ["svg"]
        else:
            outputs =  outputs.split(",")

    outfile = pathlib.Path(args.OUTFILE)
    basename = outfile.stem

    for i in outputs:
        if i == "DATA":
            #if select...:
            # cmdBuilder.select    
            cmdBuilder.paletteDefault()
            cmdBuilder.imageTransp()
        elif i == "WEIGHT":
            cmdBuilder.select(quantity="QIND")
        filename = f"{basename}_{i}.png"
        logger.info(logStyle.emojis(Emoji.DISC).str(f"Saving {filename}"))
        cmdBuilder.outputFile(filename)
    
    if outfile.suffix != ".svg":
        filename = f"{basename}.svg"
        logger.info(logStyle.emojis(Emoji.DISC).str(f"Saving {filename}"))
        cmdBuilder.outputFile(filename)



 
