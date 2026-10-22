

# General help
help:
	@ cat Makefile | grep '^[a-z][a-z\-]*' -B 1 | cut -d ':' -f1
#	@ cat Makefile | grep '^[a-z.-]\+:' | cut -d':' -f1 | tr ':' '|'


# Run all the Python tests
test-python: python/rack/
	cd python && python3 -m unittest discover -s rack/tests -v

# Run a specific Python test
test_prog_%.py:
	cd python && python3 -m unittest rack/tests/$@
	@ echo Examples of other tests:
	@ echo python3 -m unittest -v rack.tests.test_prog_gnuplot.TestGnuPlot.test_plot

# Run SVG tests (deprecating)
test-svg:
	cd ../demo && ./test-svg.sh


# get-data:
#	@ echo "Arguments:"
#	@ cd data/ && make help | grep '^get-data'
#	@ echo "Move to subdir data and issue 'make help' for more info"

# ====== Data retrieval ======


# Convert get-data-X-Y-Z target name to cache/X/Y/Z directory path
#DATA_DIR = cache/$(subst -,/,$(patsubst get-data-%,%,$@))
#DATA_DIR = cache/$(subst -,/,$(patsubst get-%,%,$@))
DATA_DIR = cache/$(patsubst get-%,%,$@)



# Ensure cache dir
cache:
	@ mkdir -v --parents ./cache

# Ensure link to cache dir
data/cache: cache
	@ ln -vsf ../cache data/cache

# Remove sample data from cache
clean-data-cache:
# Warning: rm -vrf.
# For safety, path starts with a literal identifier ("cache" etc.)
# ... and start deletion from subdirectories. Note: cache/Cache.md 
	rm -vrf cache/data-*/

# Link scripts of the distribution
#scripts:
#	@ ln -vs ../scripts

# Retrieve open data of the Finnish Meteorological Institute
get-data-all: get-data-synth get-data-fmi get-data-europe
	@ echo 'Ok'

ENV=*
# Retrieve open data of the Finnish Meteorological Institute
get-data-fmi: cache
# TODO: link retrieved to fmi short cut dir?
	@ mkdir --parents $(DATA_DIR)
# TODO:	scripts/rack-get-data-fmi.sh all
	DATA_DIR=${DATA_DIR} scripts/rack-get-data-fmi.sh env/${ENV}/data/20*radar.polar.fi???.h5

# Retrieve open data of the EUMETNET OPERA community
get-data-europe: cache
	DATA_DIR=${DATA_DIR} scripts/rack-get-data-opera.sh 

# Create synthetic test data (geometric and cloud-like)
get-data-synth: get-data-synth-geom get-data-synth-precip
	echo 'Ok'

# Create rasters containing geometric patterns (checkerboard etc.)
get-data-synth-geom: get-data-synth-geom-basic get-data-synth-geom-doppler
	echo 'Ok'

# Create rasters containing geometric patterns (checkerboard etc.)
get-data-synth-geom-basic: cache
	@ mkdir --parents $(DATA_DIR)
	@ OUTDIR=$(DATA_DIR) OUTFILE=volume.h5 scripts/rack-create-test-volume.sh

# Create fractal clouds
get-data-synth-precip: cache
	@ mkdir --parents $(DATA_DIR)
	@ DIMENSION=2.2 POWER=1 OUTDIR=$(DATA_DIR) OUTFILE="synth-precip.h5" scripts/rack-create-synth-clouds.sh

