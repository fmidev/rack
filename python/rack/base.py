

def test():
    pass

import pathlib # Path
import logging
logging.basicConfig(format='%(levelname)s\t %(name)s: %(message)s')
# logger = logging.getLogger(pathlib.Path(__file__).stem)
logger = logging.getLogger("rack") 
logger.setLevel(logging.INFO)

#import sys # sys.stderr
#from pathlib import Path


"""
dir (logging)
 'CRITICAL',
 'DEBUG',
 'ERROR',
 'FATAL',
 'INFO',
 'WARN',
 'WARNING',
"""

# re-consider rack.log.py...
def add_parameters_logging_OLD(parser, path_prefix=None):
    """ Creates registry of supported options of this script
    parser = argparse.ArgumentParser(description="Example app with JSON config support")
    """
    
    parser.add_argument(
        "-l", "--log_level",
        help="verbosity level for python wrapper and Rack cmd")

    parser.add_argument(
        "-d", "--debug",
        action='store_true',
        help="same as --log_level DEBUG")

    parser.add_argument(
        "-v", "--verbose",
        action='store_true',
        help="same as --log_level VERBOSE")


# re-consider rack.log.py...    
def handle_parameters_logging_OLD(args):
    """Handle  args.debug, args.verbose args.log_level

    """

    global logging
    
    if (args.debug):
        args.log_level = str(logging.DEBUG)
    elif (args.verbose):
        args.log_level = str(logging.INFO)
    
    if (args.log_level):
        level = logging.INFO
        if hasattr(logging, args.log_level):
            level = getattr(logging, args.log_level)
            #logger.setLevel(getattr(logging, args.log_level))
        else:
            level = int(args.log_level)

        logger.setLevel(level)
        fmt = '%(levelname)s\t %(message)s'
        if (level < logging.DEBUG):
            fmt = '%(levelname)s\t %(name)s: %(message)s'
            #logging.basicConfig(format='%(levelname)s\t %(name)s: %(message)s')
            
        logger.warning(f"fmt={fmt}")
        logging.basicConfig(format=fmt)
        #logging.basicConfig(format='%(levelname)s\t %(message)s')
        # handle, but otherwise, cmdList.append(f"--verbose '{args.log_level}'")



        
def copy_values(conf:dict, keys:list) -> list:
    """Pick values from a dict, not removing them
    """
    return [conf[i] for i in keys]

def extract_values(conf:dict, keys:list) -> list:
    """Pick values from a dict, removing them
    """
    return [conf.pop(i) for i in keys]



