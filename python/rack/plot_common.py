"""Shared utilities for rack profile plot modules (pseudorhi, vpr, etc.).

Modules conforming to rack.cmdline.RackModule — having module-level
build_parser() and compose_command() — can import these helpers and use
rack.cmdline.run_module() as their main().
"""
import json
import sys
from pathlib import Path

import rack.log
import rack.core

logger = rack.log.logger.getChild(Path(__file__).stem)


def load_config(filename: str) -> dict:
    """Load a JSON config file; return empty dict if not found."""
    path = Path(filename)
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


def handle_style(args, cmdBuilder: rack.core.Rack):
    """Apply CSS style settings to the rack command sequence."""
    cmdBuilder.gStyle(".IMAGE_BORDER=stroke:gray")
    if getattr(args, 'STYLE', None):
        for style in args.STYLE.strip().split('|'):
            cmdBuilder.gStyle(style.strip())
