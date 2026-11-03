
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
| Code example, inline   | \code{.ext} ... \endcode   |                             |
| Code example, included | \include file.ext          |                             | 
| Include an image       | \image file.ext            |                             |
| Configure test         | \~conf{.ext} ... \~        | Doxymoron extension         |
| Run code, hidden       | \~exec{.ext} ... \~        | Doxymoron extension         |
| Link code example      | \~linkfile file.txt ... \~ | Doxymoron extension         |

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
* A directory, name `./check` containg files against which output validation is done.

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


| File type              | CLI option                   | Utility                     |
| -----------------------| -----------------------------| ----------------------------| 
| Hdf5                   | -o file.txt                  | diff                        |
| PNG                    | -o file.png                  | rack.check_image            |
| GeoTIFF                | -o file.tif                  | gdalinfo, diff              |
| XML (HTML, SVG)        | -o file.html / -o file.svg   | xmllint (or Python native?) |
| SVG                    | -o file.svg                  | as converted to PNG         |

The basic test (checking that the file exists) is simply "launched" [TODO] by a validation file: for example,
`valid/$FILENAME` checks that respective `out/$FILENAME` is generated.
Also, some basic tests are automatically run, following the table above. As to content of image files, graylevel or RGB data similarity
is required to be currently at least 95%. More complex tests must be configured, explained below.

*Configuration*

The tests can be optionally specified or adjusted in configuration sections ( `\conf{.py}` or `\conf{.py}` ) preceeding the example code.

In JSON, the entry has a key which equals the name of a checked file, $FILENAME (plain filename, without directory part).
Then, the entry may have the following elements:
| Key            | Type                | Requirement                         | 
| ---------------| --------------------|-------------------------------------|  
| `"exists"`     | `boolean`           | File must exist (or must not exist) | 
| `"filename"`   | `string`            | Validation file, if not "valid/$FILENAME" |
| `"size"`       | `integer`           | File size, exact                    | 
| `"size"`       | `[integer,integer]` | File size, accepted range           | 
| `"similarity"` | `float` (max. 1.0)  | Image array similarity              | 

Example:

```
tests : {
  "file-xyz.png": true,
  "file.h5":  true,
}
```
or simply
```
tests : ["out/file.png", "out/file.h5"]
}
```
Note: if output files exists prior to testing, they are deleted.

A more profound testing checks file properties.

```
tests : {
  "out/file.h5": {
    "check": "valid/file-test.h5",
    "size": 120000,
    "similarity": 0.9,
    "size": 120000
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




Python
======


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
