

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


# ====== Misc ======

# Save help lines of a command
%.hlp:
	ls out/$@ || rack --help $* > out/$@

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

clean-map-cache:
	rm -vrf cache/mapcache/

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
get-data-synth-geom: get-data-synth-geom-basic
#get-data-synth-geom-doppler
	echo 'Ok'

# Create rasters containing geometric patterns (checkerboard etc.)
get-data-synth-geom-basic: cache
	@ mkdir --parents $(DATA_DIR)
	@ OUTDIR=$(DATA_DIR) OUTFILE=volume.h5 scripts/rack-create-test-volume.sh

# Synthetic precipitation output files (defined once; reused as dependencies below)
SYNTH_PRECIP_DIR  = cache/data-synth-precip
SYNTH_PRECIP_CART = $(SYNTH_PRECIP_DIR)/synth-precip.h5
SYNTH_PRECIP_POL  = $(SYNTH_PRECIP_DIR)/synth-precip-polar.h5

# Create fractal clouds (Cartesian image) — real file target; make skips if up to date
$(SYNTH_PRECIP_CART): cache
	mkdir --parents $(SYNTH_PRECIP_DIR)
	DIMENSION=2.2 POWER=1 OUTDIR=$(SYNTH_PRECIP_DIR) OUTFILE=$(notdir $@) scripts/rack-create-synth-clouds.sh

# Phony shortcut
get-data-synth-precip: $(SYNTH_PRECIP_CART)

# Create fractal clouds (Polar coordinates) — $< is the Cartesian file above; $@ is the output
$(SYNTH_PRECIP_POL): $(SYNTH_PRECIP_CART)
	rack $< --cPolar rscale=540,lon=21.64,lat=60.12 -o $@

# Phony shortcut
get-data-synth-precip-polar: $(SYNTH_PRECIP_POL)


mapcache:
	mkdir --parents cache/mapcache
	ln -sf cache/mapcache/ .

# Maps
get-maps-all: mapcache/map-gulf.png mapcache/map-gulf-gray.png mapcache/map-kiira.png
	@ echo "Ok"


mapcache/map-gulf-gray.png: mapcache
	python3 -m rack.maps --BBOX='-15000,6508000,625000,6892000' --mapServer OPENFMI --mapLayers 'Basemaps:naturalearthgray' --SIZE 640,384 --PROJ 3067 --mapLink $@

mapcache/map-gulf.png: mapcache
	python3 -m rack.maps --BBOX='-15000,6508000,625000,6892000' --mapServer default --SIZE 640,384 --PROJ 3067 --mapLink $@

mapcache/map-kiira.png: mapcache
	python3 -m rack.maps --BBOX='-47793,6427699,631909,7104304' --mapServer default  --SIZE 1000,1000 --PROJ 3067 --mapLink $@
