

help:
#	@echo make
	@ cat Makefile | grep '^[a-z.-]\+:' | cut -d':' -f1 | tr ':' '|'
#@ X="`cat Makefile | grep '^[a-z.-]\+:' | cut -d':' -f1 | tr ':' '|' `" && echo make \[ $$X \]

test-python: python/rack/
	cd python && python3 -m unittest discover -s rack/tests -v

test-svg:
	cd ../demo && ./test-svg.sh
