"""rack.create_wind_data — synthetic Doppler wind data generator

Generates artificial polar radar Doppler (VRAD) data from a parametric
wind model and writes a raw PNG image together with its ODIM metadata
file.  Optionally builds and executes a rack command that converts the
raw data into an ODIM HDF5 file.

Wind model::

    V(p) = BASEWIND + MATRIX * (p − CENTER)

where p = [x, y] is the position vector in metres.

Usage (standalone)::

    python -m rack.create_wind_data --help
    python -m rack.create_wind_data mywind         # generate files + run rack
    python -m rack.create_wind_data --print mywind # show rack command only
    python -m rack.create_wind_data -e mywind      # explicit exec
"""

import argparse
import math
import pathlib
import subprocess
import sys

import numpy as np
from PIL import Image

import rack.cmdline
import rack.core
import rack.gnuplot as gp
import rack.log
import rack.prog
from rack.command import Literal

logger = rack.log.logger.getChild(pathlib.Path(__file__).stem)


# ── helpers ───────────────────────────────────────────────────────────────────

def _parse_floats(s: str) -> list:
    return [float(v) for v in s.replace(',', ' ').split()]


def _compute_wind(xy: np.ndarray,
                  basewind: np.ndarray,
                  matrix: np.ndarray,
                  center: np.ndarray,
                  random: float) -> np.ndarray:
    """Return wind [u; v] at positions xy (shape 2 × N)."""
    w = basewind + matrix @ (xy - center)
    if random:
        w += random * np.random.rand(*w.shape)
    return w


def _outbase(args) -> pathlib.Path:
    base = pathlib.Path(args.OUTFILE)
    if args.OUTDIR:
        base = pathlib.Path(args.OUTDIR) / base.name
    return base


# ── data generation ───────────────────────────────────────────────────────────

def generate_wind_data(args) -> dict:
    """Generate raw wind data files.

    Returns a dict with keys:
    ``raw_png``, ``meta_txt``, ``vec_txt``, ``gnu_file``, ``plot_png``.
    """

    bins, beams = [int(v) for v in args.SIZE.split(',')]
    basewind    = np.array(_parse_floats(args.BASEWIND), float).reshape(2, 1)
    center      = np.array(_parse_floats(args.CENTER),   float).reshape(2, 1) * 1000.0  # km → m
    mat_flat    = np.array(_parse_floats(args.MATRIX),   float)
    matrix      = mat_flat.reshape(2, 2) * 1e-4  # unit: m/s per 10 km → per m
    rscale      = float(args.RSCALE)
    vNyquist    = float(args.VNYQUIST)
    random      = float(args.RANDOM)
    dtype_str   = args.TYPE

    outbase   = _outbase(args)
    raw_png   = outbase.with_suffix('.png')
    meta_txt  = outbase.with_suffix('.txt')
    vec_txt   = pathlib.Path(str(outbase) + '.TXT')
    gnu_file  = outbase.with_suffix('.gnu')
    plot_png  = pathlib.Path(str(outbase) + '.PNG')

    # ── Cartesian wind vectors for gnuplot visualisation ─────────────────────
    step    = 50
    r_cart  = rscale * np.arange(-bins, bins, step)
    x, y    = np.meshgrid(r_cart, r_cart)
    xy_cart = np.vstack([x.ravel(), y.ravel()])
    w_cart  = _compute_wind(xy_cart, basewind, matrix, center, 0.0)

    with open(vec_txt, 'w') as fh:
        fh.write('% x[m]\ty[m]\tu[m/s]\tv[m/s]\n')
        for col in np.vstack([xy_cart, w_cart]).T:
            fh.write(f'{col[0]:.0f}\t{col[1]:.0f}\t{col[2]:.1f}\t{col[3]:.1f}\n')

    sc = args.ARROWSCALE
    using_expr = f'($1-{sc}*$3/2.0):($2-{sc}*$4/2.0):({sc}*$3):({sc}*$4)'
    gnuScript = rack.prog.CommandSequence()
    gnuScript.fmt = gp.GnuPlotFormatter(param_separator=',\n  ')
    gnuScript.fmt.CMD_SEPARATOR = '\n'
    gnuReg = gp.Registry(gnuScript)
    gnuReg.terminal(gp.Terminal.PNG, size=(640, 640))
    gnuReg.output(str(plot_png))
    gnuReg.title('Wind field')
    gnuReg.plot(gp.Registry.plot_entry(
        filename=str(vec_txt),
        using=using_expr,
        style=gp.Style.VECTORS,
        title='Wind'))
    with open(gnu_file, 'w') as fh:
        fh.write(gnuScript.to_string() + '\n')

    # ── Polar (azimuth × range) VRAD data ────────────────────────────────────
    r   = rscale * np.arange(bins, dtype=float)                         # (bins,)
    azm = np.arange(beams, dtype=float) * (2.0 * math.pi / beams)      # (beams,)

    # Radar convention: clockwise from North → swap sin/cos vs. unit circle
    sin_azm = np.sin(azm)   # (beams,)
    cos_azm = np.cos(azm)

    # Cartesian positions for every (beam, bin)
    x_pol = np.outer(sin_azm, r)   # (beams, bins)
    y_pol = np.outer(cos_azm, r)
    xy_pol = np.vstack([x_pol.ravel(), y_pol.ravel()])   # (2, beams*bins)

    w_pol = _compute_wind(xy_pol, basewind, matrix, center, random)

    # Radial unit vector repeated for each bin
    radial = np.vstack([np.repeat(sin_azm, bins),
                        np.repeat(cos_azm, bins)])       # (2, beams*bins)

    w_proj    = np.sum(w_pol * radial, axis=0)           # (beams*bins,)
    w_aliased = (w_proj + vNyquist) % (2.0 * vNyquist) - vNyquist

    # Encode to integer storage type
    t    = np.iinfo(dtype_str)
    mean = float(t.min + 1 + t.max - 1) / 2.0
    gain = vNyquist / float(t.max - 1 - mean)
    bias = -mean * gain

    encoded = ((w_aliased - bias) / gain).reshape(beams, bins).astype(dtype_str)
    Image.fromarray(encoded).save(raw_png)

    with open(meta_txt, 'w') as fh:
        prefix = '--/dataset1/data1/'
        fh.write(f'{prefix}what:quantity=VRAD\n')
        fh.write(f'{prefix}what:gain={gain}\n')
        fh.write(f'{prefix}what:offset={bias}\n')
        fh.write(f'{prefix}where:rscale={rscale}\n')
        fh.write(f'{prefix}how:NI={vNyquist}\n')

    logger.info(f"gain={gain:.6f}  bias={bias:.6f}  vNyquist={vNyquist}")
    logger.info(f"Generated: {raw_png}, {meta_txt}, {vec_txt}, {gnu_file}")

    return dict(raw_png=raw_png, meta_txt=meta_txt,
                vec_txt=vec_txt, gnu_file=gnu_file, plot_png=plot_png)


# ── rack command builder ──────────────────────────────────────────────────────

def compose_command(args,
                    raw_png: pathlib.Path = None,
                    meta_txt: pathlib.Path = None) -> rack.prog.CommandSequence:
    """Build the rack command that converts raw PNG + metadata → ODIM HDF5.

    ``raw_png`` and ``meta_txt`` default to paths inferred from *args*.
    """

    outbase = _outbase(args)
    if raw_png is None:
        raw_png  = outbase.with_suffix('.png')
    if meta_txt is None:
        meta_txt = outbase.with_suffix('.txt')

    prog = rack.prog.CommandSequence(programName='rack', quote="'")
    r    = rack.core.Rack(prog)

    r.inputFile(str(raw_png))
    r.execFile(str(meta_txt))
    prog.add(Literal('--/how:simulated=True'))
    r.outputFile(str(outbase.with_suffix('.h5')))

    return prog


# ── argument parser ───────────────────────────────────────────────────────────

def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog='python -m rack.create_wind_data',
        description=(
            'Generate synthetic polar Doppler (VRAD) radar data from a\n'
            'parametric wind model.\n\n'
            'Wind model:  V(p) = BASEWIND + MATRIX * (p − CENTER)\n'
            '  BASEWIND  [m/s]\n'
            '  MATRIX    [m/s per 10 km], flattened row-major (m11,m12,m21,m22)\n'
            '  CENTER    [km]\n\n'
            'Outputs (base name = OUTFILE):\n'
            '  OUTFILE.png   raw encoded VRAD image (rack input)\n'
            '  OUTFILE.txt   ODIM metadata (rack --execFile argument)\n'
            '  OUTFILE.TXT   gnuplot wind-vector data\n'
            '  OUTFILE.gnu   gnuplot script\n'
            '  OUTFILE.h5    ODIM HDF5 (requires --exec or is the default)\n'
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    parser.add_argument(
        'OUTFILE',
        nargs='?',
        default=None,
        metavar='<outfile-base>',
        help='Base name of output files (no extension).')

    parser.add_argument(
        '--OUTDIR',
        default=None,
        metavar='<path>',
        help='Output directory (default: current directory).')

    parser.add_argument(
        '--SIZE',
        default='500,360',
        metavar='<bins>,<rays>',
        help='Dimensions of the radar data array  (default: 500,360).')

    parser.add_argument(
        '--BASEWIND',
        default='15.0,10.0',
        metavar='<u>,<v>',
        help='Constant base wind [m/s]  (default: 15.0,10.0).')

    parser.add_argument(
        '--MATRIX',
        default='0.0,0.0,0.0,0.0',
        metavar='<m11>,<m12>,<m21>,<m22>',
        help='Wind-gradient matrix [m/s per 10 km]  (default: 0,0,0,0).')

    parser.add_argument(
        '--CENTER',
        default='0.0,0.0',
        metavar='<x>,<y>',
        help='Centre of wind gradient [km]  (default: 0,0).')

    parser.add_argument(
        '--VNYQUIST',
        type=float,
        default=7.59525,
        metavar='<m/s>',
        help='Maximum unambiguous (Nyquist) velocity [m/s]  (default: 7.59525).')

    parser.add_argument(
        '--RSCALE',
        type=float,
        default=250.0,
        metavar='<m>',
        help='Radar bin length [m]  (default: 250).')

    parser.add_argument(
        '--RANDOM',
        type=float,
        default=0.0,
        metavar='<m/s>',
        help='Amplitude of random wind component [m/s]  (default: 0).')

    parser.add_argument(
        '--TYPE',
        default='uint8',
        choices=['uint8', 'uint16'],
        metavar='<uint8|uint16>',
        help='Encoding data type  (default: uint8).')

    parser.add_argument(
        '--ARROWSCALE',
        type=float,
        default=1000.0,
        metavar='<scale>',
        help='Arrow scale for the gnuplot visualisation  (default: 1000).')

    rack.cmdline.add_parameters(parser)

    return parser


# ── entry point ───────────────────────────────────────────────────────────────

def main():
    parser = build_parser()
    rack.log.add_parameters(parser)

    args = parser.parse_args()

    if not args.OUTFILE:
        parser.print_help()
        sys.exit(0)

    # Step 1: generate raw PNG + metadata (always)
    files = generate_wind_data(args)

    # Step 2: build rack command
    prog = compose_command(args,
                           raw_png=files['raw_png'],
                           meta_txt=files['meta_txt'])

    # Step 3: handle --exec / --print / --rack_cmd_file (defaulting to --exec)
    rack.cmdline.handle_parameters(prog, args, logger)

    if getattr(args, 'exec', False):
        from rack.cmdline import RackFormatter
        fmt = RackFormatter()
        cmd = prog.to_token_list(fmt)
        logger.info(f"Executing: {' '.join(str(t) for t in cmd)}")
        result = subprocess.run(cmd,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                text=True)
        if result.stdout:
            logger.debug(f"rack stdout:\n{result.stdout.rstrip()}")
        if result.stderr:
            (logger.warning if result.returncode else logger.info)(
                f"rack stderr:\n{result.stderr.rstrip()}")
        if result.returncode:
            logger.error(f"rack exited with code {result.returncode}")
            sys.exit(result.returncode)
        logger.info("rack completed successfully.")


if __name__ == '__main__':
    main()
