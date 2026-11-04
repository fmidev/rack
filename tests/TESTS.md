
============
Testing Rack
============

C++ unit tests
==============

Under construction.
Implemented as separate executables, each loosely corresponding to a class.


Command line use cases
======================

These are mainly based on code documentation applying Doxygen syntax.
The documentation system detects keywords added in C++ comments.
The keywords affect graphic formatting, include images and display examples of code usage.

Goals
-----

**Every example in the manual should be a tested one.** This goal is compatible with the
logic as any manual consists of relevant usage examples, the examples should be
checked before each release.

**The examples appearing in the manual should be kept simple.** For example, long file paths and names should be avoided.
Also, if testing requires additional commands or options, they should be hidden.

**As many tests as possible should be automatic.** Currently, the test set consists of around 150
tests. For effectivity, human intervention – for example visual inspection of result images – should be minimized.

How these goals are accomplished is explained next.


The new approach: Doxymoron
---------------------------

As a part of **Rack** distribution Python module `rack.doxymoron` has been designed exclusively
to support testing the usage of the product.

In the documentation of **Rack**, formatted using Doxygen conventions and keywords, `Doxymoron` detects and
keeps track of the following types:

| Function               | Syntax                     | Comment                     |
| -----------------------| -------------------------- | ----------------------------| 
| Code example, inline   | \code{.ext} ... \endcode   | Doxyfile native             |
| Code example, included | \include file.ext          | Doxyfile native             | 
| Include an image       | \image file.ext            | Doxyfile native             |
| Configure test         | \~conf{.ext} ... \~        | Doxymoron extension         |
| Run code, hidden       | \~exec{.ext} ... \~        | Doxymoron extension         |
| Link code example      | \linkfile{file.txt}        | Doxymoron extension         |

*Default testing scheme*

Each test is run in a dedicated working directory, which is a subdirectory under `rack/tests/`, prefixed with `env-`.
These subdirectories can be thought as "virtual environments" for tests. A working directory can be shared
among several tests. Some tests can use their own directories. The default directory is `/env-default` it this can be overridden
in configuration section (`\conf`)  with `workdir` variable. 

A directory typically contains following entries:

* Input directory, typically named './data' .
* One or several input files with simple names, for example `volume.h5` or `volume-ground-clutter.h5` .
* A directory for background maps, typically named `./maps` and/or map image files, typically named `map-*.py`
* Output directory `./out`
* A directory, name `./reference`, containing files against which output is validated.

Actually, the above directories and files are often soft links, which helps in keeping the code examples simple.
Output directory `./out` is typically a link to a shared directory used by Doxygen for input.
Input data and map links often point to systematically named paths and/or filenames describing the original source – information that
can be actually used for *retrieving* those files (For example, see Python module `rack.maps` ).

**Output file testing**

Output testing can be applied to both program code (Python only) and command line code of `rack` executable or a python module (`rack.*`) .
The code can be examples written in the manual as well as excplict test code embedded as `\~exec` blocks which do not appear in the compiled
manual.

A basic test simply checks that an output file is generated. Further, file size and contents can be checked.
What and how the file is validated depends on the file type.


| File type              | Validator filename | Utility user for validation              |
| -----------------------| --------------------------------------------------------| 
| Hdf5                   | $BASENAME.txt      | `diff` - compared as text, -o $BASENAME.txt |
| PNG                    | original           | `rack.imagecheck`             |
| TIFF / GeoTIFF         | original           | `rack.imagecheck` - pixel content, gain/offset and nodata/undetect markers (see below) |
| GeoTIFF (geo metadata) | $BASENAME.gdal     | `diff`, text extracted with `gdalinfo`/`listgeo` - geographic metadata only (projection, extent); kept separate from `rack.imagecheck` to avoid a GDAL dependency there, and not yet implemented (plain text diff is too strict for `gdalinfo`'s numeric rounding - needs a tolerance-aware comparison, parked for later) |
| XML (HTML, SVG)        | original           | `xmllint` (or Python native?) |
| SVG                    | original           | Compared like PNG, conversion via `convert` or `inkscape` |

The basic test (checking that the file exists) is simply "launched" [TODO] by a validation file: for example, if 
`reference/$FILENAME` exists it means that respective `out/$FILENAME` should be generated.
Also, some basic tests are automatically run, following the table above. As to content of image files,
validated by rack.imagecheck, 
graylevel or RGB data similarity is required to be currently at least 95%.
I more complex tests are needed, they must be configured as explained below.

*GeoTIFF: markers and physical units*

Rack's GeoTIFF writer (`src/main/fileio-geotiff.cpp`) embeds `SCALE`/`OFFSET` (gain/offset)
always, and `NODATA`/`UNDETECT` as well (when `RACK_EXTENSIONS` is set) as `<Item>` entries
in the standard `GDAL_METADATA` TIFF tag - "undetect" has no native TIFF/GDAL concept, so
this is the only place it is recorded. `rack.imagecheck` reads this automatically: for
single-band images (not RGB/PNG), it converts both the produced and reference image to
physical units via each file's own gain/offset before comparing, and treats pixels marked
`nodata`/`undetect` in both images as a match regardless of their raw byte codes (which may
differ between encodings) - a marker matched against a *different* marker category, or
against a real data value, is a genuine error, but its cost is not computed from the (often
arbitrary) raw byte difference; see `marker_mismatch_cost` below. This means GeoTIFFs stored
with different types (e.g. 8-bit vs. 16-bit) can be compared meaningfully. This machinery is
intentionally *not* applied to geographic metadata (projection, extent) - see the table above.

*Configuration*

The tests can be optionally specified or adjusted in configuration sections
( `\conf{.sh}` or `\conf{.py}` ) preceeding the example code.

In JSON, the entry has a key which equals the name of a checked file, $FILENAME (plain filename, without directory part).
Its value is a single spec (below), or a **list of specs** to check the same file several ways (e.g. against two
different reference images) - a JSON object cannot repeat a key, so a list is how one output file gets more than
one check:
```
tests : {
  "out/composite-fi.tif": [
    {"similarity": 0.95},
    {"filename": "reference/composite-fi-AEQD.tif", "similarity": 0.95, "on_fail": "warn"}
  ]
}
```
(The second entry above is a *meta test*: it deliberately checks against a reference rendered with a different,
incompatible map projection, to confirm the similarity check actually detects a real mismatch. `on_fail: "warn"`
keeps it from failing the whole test run - see below.)

Each spec may have the following elements:
| Key                     | Type                     | Requirement                         |
| ------------------------| --------------------------|-------------------------------------|
| `"exists"`              | `boolean`                 | File must exist (or must not exist) |
| `"filename"`            | `string`                  | Reference file, if not "reference/$FILENAME" |
| `"size"`                | `integer`                 | File size, exact                    |
| `"size"`                | `[integer,integer]`       | File size, accepted range           |
| `"similarity"`          | `float` (max. 1.0)        | Image array similarity (PNG/TIFF)   |
| `"on_fail"`             | `"error"` \| `"warn"` \| `"ignore"` | What a failed check does: abort the run (default), log a warning, or just log |
| `"markers"`             | `dict`                    | TIFF only: override the nodata/undetect codes read from GDAL_METADATA - shared (`{"nodata":65535,"undetect":0}`) or per-image (`{"produced":{...},"reference":{...}}`) |
| `"marker_mismatch_cost"`| `float`                   | TIFF only: fixed distance (physical units) charged for a marker mismatched against a different category or real data, instead of excluding it from rms/diff_std |
| `"raw"`                 | `boolean`                 | TIFF only: compare raw pixel codes even when both images have gain/offset metadata (default: convert to physical units automatically) |

Example:

```
tests : {
  "composite-fi.png": {"size": [64158, 244750]},
  "composite-fi.tif": {"size": [200000, 300000]},
  "composite-fi.h5":  {"size": [200000, 300000]}
}
```
or simply, expecting only existence (creation) of files:
```
tests : ["out/file.png", "out/file.h5"]
}
```
Note: if output files *declared for testing* (declared in configuration and/or exist in `./reference` directory)
they are deleted [TODO] before executing the code excerpt. 

A more profound testing checks file properties.

```
tests : {
  "out/myfile.h5": {
    "check": "reference/file-test.png",
    "size": [120000,130000],
    "similarity": 0.9
  }
}
```

```
tests : {
  "out/file.h5": true,
  "out/file.png": 0.95,
  "out/file.tif": 120000
}
```


Notice that zero (0) is not a supported value in any of these (to avoid ambiguity with false, file size and similarity).




Python tests
============

(Documentation under construction.)


Located in:
- [python/rack/tests](https://github.com/fmidev/rack/tree/main/python/rack/tests)

Some examples:

```
# General
python3 -m unittest discover -s rack/tests
python3 -m unittest discover -s rack/tests -v

# Module specific
# Example: `rack.prog` - `test_prog*.py`
python3 -m unittest rack/tests/test_prog.py
python3 -m unittest rack/tests/test_prog_rack.py -v
python3 -m unittest rack/tests/test_prog_gnuplot.py -v

# More examples
python3 -m unittest -v rack.tests.test_typical
python3 -m unittest -v rack.tests.test_maps

# Direct path vs $PYTHON_PATH (module)
python3 -m unittest    rack/tests/test_composer.py 
python3 -m unittest -v rack.tests.test_composer

# GnuPlot tests
python3  -m unittest -v rack.tests.test_prog_gnuplot
python3  -m unittest -v rack.tests.test_prog_gnuplot.TestGnuPlot.test_plot
python3  -m unittest -v rack.tests.test_prog_gnuplot.TestGnuPlot.test_plot_entries
python3  -m unittest -v rack.tests.test_prog_gnuplot.TestGnuPlot.test_plot_expr_and_datafile_fail
```
