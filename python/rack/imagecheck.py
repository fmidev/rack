#!/usr/bin/env python3
"""rack.imagecheck — image file validation for doxymoron test runs and standalone use.

Two operating modes
-------------------
Single image
    Assert properties of one file (size, PIL mode, numpy dtype).
    Example spec: {"expected_size": [800, 600], "expected_dtype": "uint8"}

Image pair
    Compare a produced file against a reference file (geometry match,
    pixel-level equality fraction, RMS / stddev of difference).
    Example spec: {"size": true, "dtype": true, "pixels_equal_min": 0.95}

Image format support
--------------------
PNG and TIFF (including GeoTIFF) via Pillow.  GeoTIFF geographic metadata
(GDAL tags 42112 GDAL_METADATA, 42113 GDAL_NODATA, 34736/34737 GeoKey
parameters) is *reported* but geographic accuracy is intentionally left to
GDAL CLI tools such as ``gdalinfo``.  That keeps the dependency list small.

Parking lot — planned future checks
------------------------------------
SVG / XML   structural check via xmllint; pixel check by rendering to PNG
            and comparing with this module.
HDF5        text-dump-based comparison (separate plan already sketched).

Standalone usage
----------------
    python -m rack.imagecheck image.png --info
    python -m rack.imagecheck image.png --check '{"expected_size": [800,600]}'
    python -m rack.imagecheck img1.png img2.png --check '{"size":true,"pixels_equal_min":0.95}'
    python -m rack.imagecheck img1.png img2.png --check spec.json

Integration with rack.doxymoron
---------------------------------
    import rack.imagecheck as ic
    spec = {"size": True, "pixels_equal_min": 0.97, "on_fail": "error"}
    ic.run_checks(["result.png", "reference.png"], spec)
"""

from __future__ import annotations

import argparse
import json
import logging
import math
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple, Union

import numpy as np
from PIL import Image
from PIL.TiffTags import TAGS

logger = logging.getLogger(__name__)

# ---------------------------------------------------------------------------
# GDAL / GeoTIFF tag IDs (subset)
# ---------------------------------------------------------------------------
_GDAL_METADATA  = 42112
_GDAL_NODATA    = 42113
_GEO_KEY_DIR    = 34736  # GeoDoubleParamsTag
_GEO_ASCII      = 34737  # GeoAsciiParamsTag

_GEOTIFF_TAGS = {_GDAL_METADATA, _GDAL_NODATA, _GEO_KEY_DIR, _GEO_ASCII}


# ---------------------------------------------------------------------------
# ImageInfo — lightweight metadata container
# ---------------------------------------------------------------------------

@dataclass
class ImageInfo:
    """Metadata extracted from one image file."""

    path: Path
    size: Tuple[int, int]   # (width, height), PIL convention
    mode: str               # PIL image mode: 'L', 'RGB', 'RGBA', 'I', 'F', …
    dtype: str              # numpy dtype name: 'uint8', 'uint16', 'int32', …
    is_geotiff: bool = False
    gdal_nodata: Optional[str] = None
    gdal_metadata_xml: Optional[str] = None

    @classmethod
    def load(cls, path: Union[str, Path]) -> "ImageInfo":
        """Open *path* and extract metadata without retaining the pixel array."""
        path = Path(path)
        img = Image.open(path)
        # np.array handles 16-bit TIFF (mode 'I;16') correctly in Pillow ≥ 6.
        arr = np.array(img)
        dtype_name = arr.dtype.name

        tiff_tags: Dict[int, Any] = {}
        if hasattr(img, "tag"):
            tiff_tags = dict(img.tag)

        is_geotiff = bool(tiff_tags.keys() & _GEOTIFF_TAGS)
        nodata = None
        meta_xml = None
        if _GDAL_NODATA in tiff_tags:
            raw = tiff_tags[_GDAL_NODATA]
            nodata = raw[0] if isinstance(raw, (list, tuple)) else str(raw)
        if _GDAL_METADATA in tiff_tags:
            raw = tiff_tags[_GDAL_METADATA]
            meta_xml = raw[0] if isinstance(raw, (list, tuple)) else str(raw)

        return cls(
            path=path,
            size=img.size,
            mode=img.mode,
            dtype=dtype_name,
            is_geotiff=is_geotiff,
            gdal_nodata=nodata,
            gdal_metadata_xml=meta_xml,
        )

    def describe(self) -> str:
        parts = [
            f"{self.path.name}",
            f"size={self.size[0]}x{self.size[1]}",
            f"mode={self.mode}",
            f"dtype={self.dtype}",
        ]
        if self.is_geotiff:
            parts.append("GeoTIFF")
        if self.gdal_nodata is not None:
            parts.append(f"nodata={self.gdal_nodata}")
        return "  ".join(parts)


# ---------------------------------------------------------------------------
# CheckResult
# ---------------------------------------------------------------------------

@dataclass
class CheckResult:
    """Outcome of one or more checks."""

    passed: bool = True
    failures: List[str] = field(default_factory=list)
    warnings: List[str] = field(default_factory=list)
    stats: Dict[str, Any] = field(default_factory=dict)

    def fail(self, message: str) -> None:
        self.passed = False
        self.failures.append(message)

    def warn(self, message: str) -> None:
        self.warnings.append(message)

    def note(self, key: str, value: Any) -> None:
        self.stats[key] = value

    def handle(self, on_fail: str = "error") -> None:
        """Log results and raise / warn / ignore depending on *on_fail*."""
        for w in self.warnings:
            logger.warning("WARN  %s", w)
        if self.passed:
            stats_str = "  ".join(f"{k}={v}" for k, v in self.stats.items())
            logger.info("PASS  %s", stats_str)
            return
        for f in self.failures:
            logger.error("FAIL  %s", f)
        summary = f"{len(self.failures)} check(s) failed: " + "; ".join(self.failures)
        if on_fail == "error":
            raise AssertionError(summary)
        elif on_fail == "warn":
            logger.warning("imagecheck: %s", summary)
        else:  # "ignore"
            logger.info("imagecheck (ignored): %s", summary)


# ---------------------------------------------------------------------------
# Pixel-array helpers
# ---------------------------------------------------------------------------

def _load_array(path: Union[str, Path]) -> np.ndarray:
    """Return a numpy array for *path*, preserving bit-depth via Pillow."""
    return np.array(Image.open(Path(path)))


def _diff_stats(a1: np.ndarray, a2: np.ndarray) -> Dict[str, float]:
    """Compute element-wise difference statistics between two arrays."""
    flat1 = a1.astype(np.float64).ravel()
    flat2 = a2.astype(np.float64).ravel()
    diff = flat1 - flat2
    rms = math.sqrt(float(np.mean(diff ** 2)))
    std = float(np.std(diff))
    # pixel-level equality: all channels of a pixel must match
    if a1.ndim == 2:
        pixels_equal = float(np.mean(a1 == a2))
    else:
        pixels_equal = float(np.mean(np.all(a1 == a2, axis=-1)))
    elements_equal = float(np.mean(a1 == a2))
    return {
        "rms": rms,
        "diff_std": std,
        "pixels_equal": pixels_equal,
        "elements_equal": elements_equal,
    }


# ---------------------------------------------------------------------------
# Single-image check
# ---------------------------------------------------------------------------

def check_single(
    path: Union[str, Path],
    spec: Dict[str, Any],
    on_fail: Optional[str] = None,
) -> CheckResult:
    """Check properties of a single image against *spec*.

    Spec keys recognised
    --------------------
    expected_size   : [width, height]
    expected_mode   : str  e.g. "L", "RGB", "RGBA"
    expected_dtype  : str  e.g. "uint8", "uint16", "float32"
    on_fail         : "error" | "warn" | "ignore"   (default "error")
    """
    result = CheckResult()
    on_fail = on_fail or spec.get("on_fail", "error")
    info = ImageInfo.load(path)
    logger.info("imagecheck single: %s", info.describe())
    result.note("path", str(path))
    result.note("size", info.size)
    result.note("mode", info.mode)
    result.note("dtype", info.dtype)

    if "expected_size" in spec:
        ew, eh = spec["expected_size"]
        if info.size != (ew, eh):
            result.fail(
                f"size mismatch: got {info.size[0]}x{info.size[1]}, expected {ew}x{eh}"
            )

    if "expected_mode" in spec:
        if info.mode != spec["expected_mode"]:
            result.fail(
                f"mode mismatch: got '{info.mode}', expected '{spec['expected_mode']}'"
            )

    if "expected_dtype" in spec:
        if info.dtype != spec["expected_dtype"]:
            result.fail(
                f"dtype mismatch: got '{info.dtype}', expected '{spec['expected_dtype']}'"
            )

    result.handle(on_fail)
    return result


# ---------------------------------------------------------------------------
# Image pair check
# ---------------------------------------------------------------------------

def check_pair(
    produced: Union[str, Path],
    reference: Union[str, Path],
    spec: Dict[str, Any],
    on_fail: Optional[str] = None,
) -> CheckResult:
    """Compare *produced* against *reference* according to *spec*.

    Spec keys recognised
    --------------------
    size              : bool  — sizes must match (always checked before pixel ops)
    mode              : bool  — PIL modes must match
    dtype             : bool  — numpy dtypes must match
    pixels_equal_min  : float — minimum fraction of spatially equal pixels [0..1]
                                  (for multi-channel images: ALL channels at a position)
    elements_equal_min: float — minimum fraction of equal array elements [0..1]
                                  (counts each channel separately)
    rms_max           : float — maximum allowed RMS of pixel difference
    diff_std_max      : float — maximum allowed std of pixel difference
    on_fail           : "error" | "warn" | "ignore"   (default "error")
    """
    result = CheckResult()
    on_fail = on_fail or spec.get("on_fail", "error")

    inf1 = ImageInfo.load(produced)
    inf2 = ImageInfo.load(reference)
    logger.info("imagecheck pair:")
    logger.info("  produced : %s", inf1.describe())
    logger.info("  reference: %s", inf2.describe())

    result.note("produced", str(produced))
    result.note("reference", str(reference))

    # --- geometry ---
    if spec.get("size", False) or any(
        k in spec for k in ("pixels_equal_min", "elements_equal_min", "rms_max", "diff_std_max")
    ):
        if inf1.size != inf2.size:
            result.fail(
                f"size mismatch: produced {inf1.size[0]}x{inf1.size[1]}, "
                f"reference {inf2.size[0]}x{inf2.size[1]}"
            )
        else:
            result.note("size", inf1.size)

    # --- PIL mode ---
    if spec.get("mode", False):
        if inf1.mode != inf2.mode:
            result.fail(f"mode mismatch: produced '{inf1.mode}', reference '{inf2.mode}'")
        else:
            result.note("mode", inf1.mode)

    # --- numpy dtype ---
    if spec.get("dtype", False):
        if inf1.dtype != inf2.dtype:
            result.fail(f"dtype mismatch: produced '{inf1.dtype}', reference '{inf2.dtype}'")
        else:
            result.note("dtype", inf1.dtype)

    # --- pixel / array comparisons (only if geometry is ok so far) ---
    pixel_checks = {
        k for k in ("pixels_equal_min", "elements_equal_min", "rms_max", "diff_std_max")
        if k in spec
    }
    if pixel_checks:
        if not result.passed:
            result.warn(
                "pixel comparison skipped because geometry check already failed"
            )
        elif inf1.size != inf2.size:
            # sizes differ (spec.size was False, but we still can't compare)
            result.warn("pixel comparison skipped: sizes differ")
        else:
            arr1 = _load_array(produced)
            arr2 = _load_array(reference)
            if arr1.dtype != arr2.dtype:
                # upcast both to float for a meaningful comparison
                arr1 = arr1.astype(np.float64)
                arr2 = arr2.astype(np.float64)
            stats = _diff_stats(arr1, arr2)
            for k, v in stats.items():
                result.note(k, round(v, 6))
            logger.info(
                "  rms=%.4f  diff_std=%.4f  pixels_equal=%.4f  elements_equal=%.4f",
                stats["rms"], stats["diff_std"],
                stats["pixels_equal"], stats["elements_equal"],
            )

            if "pixels_equal_min" in spec:
                thr = float(spec["pixels_equal_min"])
                if stats["pixels_equal"] < thr:
                    result.fail(
                        f"pixels_equal {stats['pixels_equal']:.4f} < required {thr:.4f}"
                    )

            if "elements_equal_min" in spec:
                thr = float(spec["elements_equal_min"])
                if stats["elements_equal"] < thr:
                    result.fail(
                        f"elements_equal {stats['elements_equal']:.4f} < required {thr:.4f}"
                    )

            if "rms_max" in spec:
                thr = float(spec["rms_max"])
                if stats["rms"] > thr:
                    result.fail(f"rms {stats['rms']:.4f} > allowed {thr:.4f}")

            if "diff_std_max" in spec:
                thr = float(spec["diff_std_max"])
                if stats["diff_std"] > thr:
                    result.fail(f"diff_std {stats['diff_std']:.4f} > allowed {thr:.4f}")

    result.handle(on_fail)
    return result


# ---------------------------------------------------------------------------
# run_checks — convenience entry point for doxymoron
# ---------------------------------------------------------------------------

def run_checks(
    files: List[Union[str, Path]],
    spec: Dict[str, Any],
    on_fail: Optional[str] = None,
) -> CheckResult:
    """Dispatch to the right check function based on the number of files.

    *files* with one path  → check_single
    *files* with two paths → check_pair (first=produced, second=reference)

    *on_fail* overrides the spec's ``on_fail`` key when provided.
    """
    if len(files) == 1:
        return check_single(files[0], spec, on_fail=on_fail)
    elif len(files) == 2:
        return check_pair(files[0], files[1], spec, on_fail=on_fail)
    else:
        raise ValueError(f"run_checks expects 1 or 2 files, got {len(files)}")


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def _parse_spec(raw: str) -> Dict[str, Any]:
    """Accept inline JSON string or a path to a JSON file."""
    candidate = Path(raw)
    if candidate.exists() and candidate.suffix == ".json":
        return json.loads(candidate.read_text(encoding="utf-8"))
    return json.loads(raw)


def main(argv: Optional[List[str]] = None) -> int:
    ap = argparse.ArgumentParser(
        description=(
            "Check image file properties (single) or compare two images (pair). "
            "Exit 0 on pass, non-zero on failure."
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s result.png --info
  %(prog)s result.png --check '{"expected_size": [800, 600], "expected_dtype": "uint8"}'
  %(prog)s produced.tif reference.tif --check '{"size": true, "pixels_equal_min": 0.95}'
  %(prog)s produced.png reference.png --check spec.json
""",
    )
    ap.add_argument(
        "images",
        nargs="+",
        metavar="IMAGE",
        help="One image (single check) or two images (pair comparison: produced reference)",
    )
    ap.add_argument(
        "--check",
        metavar="JSON|FILE",
        help="Check spec as inline JSON or path to a .json file",
    )
    ap.add_argument(
        "--info",
        action="store_true",
        help="Print image metadata and exit (no checks performed)",
    )
    ap.add_argument(
        "--on-fail",
        choices=["error", "warn", "ignore"],
        default=None,
        help="Override the spec's on_fail behaviour",
    )
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args(argv)

    logging.basicConfig(
        format="%(levelname)s: %(message)s",
        level=logging.DEBUG if args.verbose else logging.INFO,
    )

    if args.info:
        for p in args.images:
            info = ImageInfo.load(p)
            print(info.describe())
            if info.gdal_metadata_xml:
                print("  GDAL metadata (truncated):", info.gdal_metadata_xml[:120])
        return 0

    if not args.check:
        ap.error("Provide --check SPEC or --info")

    spec = _parse_spec(args.check)
    try:
        result = run_checks(args.images, spec, on_fail=args.on_fail)
    except AssertionError as exc:
        logger.error("%s", exc)
        return 1
    except Exception as exc:
        logger.error("Unexpected error: %s", exc)
        return 2

    if not result.passed:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
