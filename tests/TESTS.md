
Python
======

Under construction...

Located in:
- [python/rack/tests](https://github.com/fmidev/rack/tree/main/python/rack/tests)

Loose notes (examples):

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
