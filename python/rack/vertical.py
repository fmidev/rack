"""Shared utilities for rack profile plot modules (pseudorhi, vpr, etc.).

Modules conforming to rack.cmdline.RackModule — having module-level
build_parser() and compose_command() — can import these helpers and use
rack.cmdline.run_module() as their main().
"""
import json
import sys
from pathlib import Path

from rack.cmdline import RackFormatter, logger
import rack.log
import rack.core
from rack.vpr import logger

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

def handle_horz_product(args, progBuilder: rack.core.Rack):
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
    from rack.vertical import load_config, export_defaults_to_json

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

    if getattr(args, 'print', None):
        sep = args.print.replace(r'\t', '\t').replace(r'\n', '\n')
        logger.info("# Rack cmd line:")
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
            logger.error(f"Command exited with code {result.returncode}")

        if getattr(args, 'gnuplot_script', None):
            gnuplot_cmd = "gnuplot"
            cmd = [gnuplot_cmd, args.gnuplot_script]
            logger.info(f"# Executing GnuPlot script: {gnuplot_cmd} {args.gnuplot_script}")
            subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    line = rack.args.args_to_cli(parser, args)
    logger.debug(f"Python command line args: {line}")


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
