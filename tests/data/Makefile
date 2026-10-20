
# Convert get-data-X-Y-Z target name to cache/X/Y/Z directory path
DATA_DIR = cache/$(subst -,/,$(patsubst get-data-%,%,$@))

# General help
help:
	@ cat Makefile | grep '^[a-z][a-z\-]*' -B 1 | cut -d ':' -f1

# Remove sample data from cache
clean-data-cache:
# Warning: rm -vrf.
# For safety, path starts with a literal identifier ("cache" etc.)
# ... and start deletion from subdirectories. Note: cache/Cache.md 
	rm -vrf cache/*/

# Ensure link to cache dir
cache:
	@ mkdir -v --parents ../cache
	@ ln -vs ../cache

# Link scripts of the distribution
scripts:
	@ ln -vs ../scripts

# Retrieve open data of the Finnish Meteorological Institute
get-data-all: get-data-synth get-data-fmi get-data-europe
	@ echo 'Ok'

# Retrieve open data of the Finnish Meteorological Institute
get-data-fmi: cache
# TODO: link retrieved to fmi short cut dir?
#	mkdir --parents $(DATA_DIR)
# TODO:	scripts/rack-get-data-fmi.sh all
	scripts/rack-get-data-fmi.sh 

# Retrieve open data of the EUMETNET OPERA community
get-data-europe: cache
# TODO: link retrieved to fmi short cut dir?
#	mkdir --parents $(DATA_DIR)
	scripts/rack-get-data-europe.sh all

# Create synthetic test data (geometric and cloud-like)
get-data-synth: get-data-synth-geom get-data-synth-precip
	echo 'Ok'

# Create rasters containing geometric patterns (checkerboard etc.)
get-data-synth-geom: get-data-synth-geom-basic get-data-synth-geom-doppler
	echo 'Ok'

# Create rasters containing geometric patterns (checkerboard etc.)
get-data-synth-geom-basic: cache
	mkdir --parents $(DATA_DIR)
	OUTDIR=$(DATA_DIR) OUTFILE=volume.h5 scripts/rack-create-test-volume.sh

# Create fractal clouds
get-data-synth-precip: cache
	mkdir --parents $(DATA_DIR)
	./get-data-synth-precip.sh

