import sys
import argparse
import pathlib
#from pathlib import Path
import shlex
#import logging
import json
#import rack.base
import rack.log
#from rack.cmdline import logger
#from rack.vpr import logger, logger

#logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)
logger = rack.log.logger.getChild(pathlib.Path(__file__).stem)

# Todo: rack.Formatter?
def args_to_list(parser: argparse.ArgumentParser, args:argparse.Namespace): #, arg_separator=None) -> str :
    """
    Exports arguments to a list of strings, suitable for CLI usage.
    Only includes arguments that differ from their default values.  
    
    parameters:
    - parser: the ArgumentParser that defines the arguments and their defaults
    - args: the Namespace object containing the parsed arguments
    - arg_separator: if provided, will be used to separate multiple values for the same argument (e.g., for lists)
    """

    parts = []

    for action in parser._actions:

         # Skip help and non-stored actions
        if action.dest == argparse.SUPPRESS:
            continue

        if not hasattr(args, action.dest):
            continue

        current_value = getattr(args, action.dest)

        # Free arguments (often input files)
        if not action.option_strings:
            if current_value is None:
                continue
            if isinstance(current_value, list):
            # if isinstance(current_value, (list, set, tuple)):
                for v in current_value:
                    #logger.warning(f"adding {action.dest} + {v}")
                    parts.append(shlex.quote(str(v)))
            else:
                parts.append(shlex.quote(str(current_value)))
            continue


        #dest = action.dest
        #current_value = getattr(args, dest)
        default_value = action.default

        # Skip if value equals default
        if current_value == default_value:
            continue

        option = action.option_strings[-1]  # use long option if available

        # Handle different action types
        # Todo: rack.Formatter?
        if isinstance(action, argparse._StoreTrueAction):
            if current_value:
                parts.append(option)
        elif isinstance(action, argparse._StoreFalseAction):
            if not current_value:
                parts.append(option)
        elif isinstance(action, argparse._StoreAction):
            if isinstance(current_value, (list, set, tuple)):
                # parts.append(option)
                # parts.append(shlex.quote(",".join([str(i) for i in current_value])))
                option += " " + shlex.quote(",".join([str(i) for i in current_value]))     
                #for v in current_value:
                #    logger.warning(f"adding {option} + {v}")
                #    parts.append(option)
                #    parts.append(shlex.quote(str(v)))
            else:
                option += " " + shlex.quote(str(current_value))
                #parts.append(option)
                #parts.append(shlex.quote(str(current_value)))
            parts.append(option)

    return parts

def args_to_cli(parser, args, separator:str=" ") -> str :
    return separator.join(args_to_list(parser, args))


def load_config(filename: str) -> dict:
    """Load a JSON config file; return empty dict if not found."""
    path = pathlib.Path(filename)
    if not path.is_file():
        print(f"File not found: {filename}", file=sys.stderr)
        return {}
    with open(path, "r") as f:
        return json.load(f)


def read_default_args(parser):
    """Apply JSON config file (if --config given) as parser defaults."""
    args, _ = parser.parse_known_args()
    if hasattr(args, 'config') and args.config:
        config = load_config(args.config)
        parser.set_defaults(**config)


def export_defaults_to_json(parser, args, filename: str = "config_template.json"):
    """Write current argument values to a JSON config file."""
    logger.debug(f'Writing config to: {filename}')
    conf = {k: v for k, v in vars(args).items()
            if k != 'export_config' and v is not None}
    with open(filename, "w") as f:
        json.dump(conf, f, indent=4)
    logger.info(f"Config written to: {filename}")
