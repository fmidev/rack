

help:
#	@echo make
	@ cat Makefile | grep '^[a-z.-]\+:' | cut -d':' -f1 | tr ':' '|'
#@ X="`cat Makefile | grep '^[a-z.-]\+:' | cut -d':' -f1 | tr ':' '|' `" && echo make \[ $$X \]
	@ echo Examples:
	@ echo make test_prog_gnuplot.py

test-python: python/rack/
	cd python && python3 -m unittest discover -s rack/tests -v

test_prog_%.py:
	cd python && python3 -m unittest rack/tests/$@
	@ echo other tests:
	@ echo python3 -m unittest -v rack.tests.test_prog_gnuplot.TestGnuPlot.test_plot

test-svg:
	cd ../demo && ./test-svg.sh
