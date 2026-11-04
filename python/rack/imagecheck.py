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
PNG and TIFF (including GeoTIFF) via Pillow.  GeoTIFF *geographic* metadata
(34736/34737 GeoKey parameters) is *reported* but geographic accuracy is
intentionally left to GDAL CLI tools such as ``gdalinfo`` - that keeps the
dependency list small, and avoids false precision from comparing reprojected
rasters pixel-by-pixel.

GDAL_METADATA (tag 42112) and GDAL_NODATA (42113) are used, though: Rack's
GeoTIFF writer (src/main/fileio-geotiff.cpp) embeds SCALE/OFFSET (gain/offset)
always, and NODATA/UNDETECT as well when RACK_EXTENSIONS is set - "undetect"
has no standard TIFF/GDAL slot, so this is the only place it is recorded.
check_pair() uses this, for single-band images, to convert both sides to
physical units before comparing, and to treat nodata/undetect as symbolic
markers rather than arbitrary byte codes - see its docstring for the
`raw`/`markers`/`marker_mismatch_cost` spec keys.

Parking lot — planned future checks
------------------------------------
SVG / XML   structural check via xmllint; pixel check by rendering to PNG
            and comparing with this module.
HDF5        text-dump-based comparison (separate plan already sketched).
GeoTIFF geo-metadata (listgeo/gdalinfo dumps) compared with numeric tolerance
            - kept outside this module (no GDAL dependency here) and outside
            doxymoron's plain-text .txt diff (which is exact-match only).

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
import re
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

# <Item name="KEY" ...>value</Item> entries inside the GDAL_METADATA (42112) XML blob.
# Rack's GeoTIFF writer (src/main/fileio-geotiff.cpp) emits SCALE/OFFSET always, and
# NODATA/UNDETECT when RACK_EXTENSIONS is set - the only place "undetect" is recorded,
# since TIFF/GDAL have no native concept of it.
_GDAL_ITEM_RE = re.compile(r'<Item\s+name="([^"]+)"[^>]*>([^<]*)</Item>')


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
    gdal_items: Dict[str, str] = field(default_factory=dict)

    # Parsed from gdal_items, in raw (unscaled) pixel-code units.
    gain: Optional[float] = None
    offset: Optional[float] = None
    nodata_code: Optional[float] = None
    undetect_code: Optional[float] = None

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

        items = dict(_GDAL_ITEM_RE.findall(meta_xml)) if meta_xml else {}

        def _num(key: str, fallback: Optional[str] = None) -> Optional[float]:
            raw_val = items.get(key, fallback)
            return float(raw_val) if raw_val is not None else None

        return cls(
            path=path,
            size=img.size,
            mode=img.mode,
            dtype=dtype_name,
            is_geotiff=is_geotiff,
            gdal_nodata=nodata,
            gdal_metadata_xml=meta_xml,
            gdal_items=items,
            gain=_num("SCALE"),
            offset=_num("OFFSET"),
            # GDAL_NODATA (tag 42113) is the standard fallback; Rack's own NODATA
            # Item (when present) is the authoritative source.
            nodata_code=_num("NODATA", nodata),
            undetect_code=_num("UNDETECT"),
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


# Pixel categories for marker-aware comparison (single-band images only).
_CAT_DATA = 0
_CAT_NODATA = 1
_CAT_UNDETECT = 2


def _categorize(arr: np.ndarray, nodata: Optional[float], undetect: Optional[float]) -> np.ndarray:
    """Classify each raw pixel value as real data, nodata, or undetect."""
    cat = np.full(arr.shape, _CAT_DATA, dtype=np.int8)
    if undetect is not None:
        cat[arr == undetect] = _CAT_UNDETECT
    if nodata is not None:
        cat[arr == nodata] = _CAT_NODATA  # nodata wins if codes coincide
    return cat


def _resolve_markers(spec: Dict[str, Any], info: ImageInfo, which: str) -> Tuple[Optional[float], Optional[float]]:
    """Resolve (nodata, undetect) raw codes for one image ('produced' or 'reference').

    An explicit spec["markers"] overrides the image's own embedded metadata - either
    shared between both images ({"nodata": ..., "undetect": ...}) or per-image
    ({"produced": {...}, "reference": {...}}), for files without Rack's GDAL_METADATA
    Items (e.g. plain GeoTIFFs, or a different encoding than the file's own).
    """
    markers = spec.get("markers")
    entry = None
    if isinstance(markers, dict):
        if isinstance(markers.get(which), dict):
            entry = markers[which]
        elif "nodata" in markers or "undetect" in markers:
            entry = markers
    nodata = entry.get("nodata") if entry else info.nodata_code
    undetect = entry.get("undetect") if entry else info.undetect_code
    return nodata, undetect


def _diff_stats_marked(
    val1: np.ndarray, val2: np.ndarray,
    cat1: np.ndarray, cat2: np.ndarray,
    marker_mismatch_cost: Optional[float],
) -> Tuple[Dict[str, float], np.ndarray]:
    """Marker-aware difference statistics for single-band arrays (*val1*/*val2* already
    converted to physical units where possible; *cat1*/*cat2* from _categorize()).

    A pixel that is the *same* marker category in both images (e.g. both "nodata") is a
    perfect match regardless of its raw byte value - needed since the same marker can be
    an arbitrary, encoding-specific code. A pixel that is a marker in one image but not
    matched by the same category in the other (marker vs. data, or nodata vs. undetect)
    is always a mismatch; its contribution to rms/diff_std is *marker_mismatch_cost* if
    given, otherwise it is excluded from those continuous stats entirely (the raw byte
    gap, e.g. nodata=65535 vs a real value, is not a meaningful physical distance).
    Either way it always counts against pixels_equal/elements_equal.
    """
    data_mask = (cat1 == _CAT_DATA) & (cat2 == _CAT_DATA)
    marker_match = (cat1 == cat2) & (cat1 != _CAT_DATA)
    mismatch = ~data_mask & ~marker_match

    diff = np.zeros_like(val1)
    diff[data_mask] = val1[data_mask] - val2[data_mask]
    included = np.ones_like(diff, dtype=bool)
    if marker_mismatch_cost is not None:
        diff[mismatch] = marker_mismatch_cost
    else:
        included = ~mismatch

    diff_included = diff[included]
    rms = math.sqrt(float(np.mean(diff_included ** 2))) if diff_included.size else 0.0
    std = float(np.std(diff_included)) if diff_included.size else 0.0

    equal_mask = marker_match.copy()
    equal_mask[data_mask] = (val1[data_mask] == val2[data_mask])

    stats = {
        "rms": rms,
        "diff_std": std,
        "pixels_equal": float(np.mean(equal_mask)),
        "elements_equal": float(np.mean(equal_mask)),
        "marker_mismatches": int(np.count_nonzero(mismatch)),
    }
    return stats, equal_mask


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

    Single-band images only (e.g. GeoTIFF; multi-channel images such as RGB PNG use
    plain array comparison, unaffected by the keys below):

    raw                 : bool  — force raw pixel-code comparison even when both images
                                   carry gain/offset metadata (default False: convert to
                                   physical units automatically when both have it)
    markers             : dict  — override the nodata/undetect codes read from each
                                   image's own GDAL_METADATA (see ImageInfo), either
                                   shared ({"nodata": 65535, "undetect": 0}) or per-image
                                   ({"produced": {...}, "reference": {...}})
    marker_mismatch_cost: float — fixed distance (in the same units as the pixel values
                                   compared - physical, unless raw=True) charged to
                                   rms/diff_std for a pixel that is a marker in one image
                                   without a matching marker category in the other
                                   (marker vs. data, or nodata vs. undetect). Omit to
                                   exclude such pixels from rms/diff_std entirely; they
                                   always count against pixels_equal/elements_equal.
                                   A pixel that is the *same* marker category in both
                                   images is always a perfect match, regardless of its
                                   raw byte value.
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

            if arr1.ndim == 2 and arr2.ndim == 2:
                # Single-band (e.g. GeoTIFF): honour markers (nodata/undetect) and,
                # where available, convert to physical units via each image's own
                # gain/offset before comparing - see _diff_stats_marked().
                raw_mode = bool(spec.get("raw", False))
                physical = (
                    not raw_mode
                    and inf1.gain is not None and inf1.offset is not None
                    and inf2.gain is not None and inf2.offset is not None
                )
                if physical:
                    val1 = arr1.astype(np.float64) * inf1.gain + inf1.offset
                    val2 = arr2.astype(np.float64) * inf2.gain + inf2.offset
                else:
                    val1 = arr1.astype(np.float64)
                    val2 = arr2.astype(np.float64)

                nodata1, undetect1 = _resolve_markers(spec, inf1, "produced")
                nodata2, undetect2 = _resolve_markers(spec, inf2, "reference")
                cat1 = _categorize(arr1, nodata1, undetect1)
                cat2 = _categorize(arr2, nodata2, undetect2)

                stats, _ = _diff_stats_marked(val1, val2, cat1, cat2, spec.get("marker_mismatch_cost"))
                result.note("physical", physical)
                if nodata1 is not None or nodata2 is not None:
                    result.note("nodata", (nodata1, nodata2))
                if undetect1 is not None or undetect2 is not None:
                    result.note("undetect", (undetect1, undetect2))
            else:
                if arr1.dtype != arr2.dtype:
                    # upcast both to float for a meaningful comparison
                    arr1 = arr1.astype(np.float64)
                    arr2 = arr2.astype(np.float64)
                stats = _diff_stats(arr1, arr2)

            for k, v in stats.items():
                result.note(k, round(v, 6) if isinstance(v, float) else v)
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
