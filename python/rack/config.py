
import json
import sys # sys.stderr
from pathlib import Path



def add_parameters(parser, path_prefix=None):
    """ Creates registry of supported options of this script
    parser = argparse.ArgumentParser(description="Example app with JSON config support")
    """

    # Use load_config(args.config)
    parser.add_argument(
        "--config",
        help="Path to JSON config file")
    
    parser.add_argument(
        "--export-config",
        default=None,
        help="Save configuration to file")


# No good
def read_defaults(parser):
    """Parse args with precedence:
       CLI > JSON config > defaults
    """
    # First parse known args to see if --config is given
    args, remaining_argv = parser.parse_known_args()

    if args.config:
        config = read(args.config, False)
        args.config = None
        parser.set_defaults(**config)

        

def read(filename, lenient=False): # todo path prefix?
    """Load JSON config if it exists."""
    path = Path(filename)
    if not path.is_file():
        msg = f"⚠️  File not found: {filename}"
        if lenient:
            print(msg, file=sys.stderr)
            return {}
        else:
            raise Exception(msg)
    with open(path, "r") as f:
        return json.load(f)


def write(filename, conf:dict, exclude=[]):

    conf_copy = {}

    for k,v in conf.items():
        
        if k in {'config'}:
            continue

        if k.startswith('export_'): # command
            continue

        if v is None:
            continue
        
        

        conf_copy[k] = v

    if (filename == "-") or (filename.lower() == "stdout"):
        json.dump(conf_copy, sys.stdout, indent=4)
        sys.stdout.write("\n")
    else:
        with open(filename, "w") as f:
            json.dump(conf_copy, f, indent=4)





