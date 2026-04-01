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


    parser.add_argument(
        "--svgRelativePaths",
        action='store_true',
        help="Strip prefix (--OUTDIR) from image links (for WWW usage)")

    parser.add_argument(
        "--svgBackGround",
        default = "",
        metavar = "<file>",
        help="Add map etc. under actual data image layer")

    parser.add_argument(
        "--svgOutputs",
        default = None,
        metavar = "auto|DATA,WEIGHT,...",
        help="Which fields of composite to include (see --EXTRACT)")


 
def handle_parameters(args):
    """Handle  args.debug, args.verbose args.log_level

    """
    pass

    
    #if args.svgRelativePaths:
def handle_outfile(args, cmdBuilder: rack.core.Rack):
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



 
