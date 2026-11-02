
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

The new approach: Doxymoron
---------------------------

Currently, a Python module (rack.doxymoron) is being developed and it
detects the following types:


| Test type              | Syntax            | Description                 |
| -----------------------| ------------------| ----------------------------| 
| Code example, inline   | \code{.ext} ...   |                             |
| Code example, included | \include file.ext |                             | 

(Under construction.)

*Output testing*

| File type              | Checked/appended CLI option  | Utility                     |
| -----------------------| -----------------------------| ----------------------------| 
| Hdf5                   | -o file.txt                  | diff                        |
| PNG                    | -o file.png                  | rack.check_image            |
| GeoTIFF                | -o file.tiff                 | gdalinfo, diff              | 


*Configuration examples*


```
tests : {
  "out/file.h5": true,
  "out/file.png": 0.95,
  "out/file.tif": 120000
}
```

- The file must exist (or must not exist)
- The file size is at least x
- The image array has similarity

Notice that 



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
