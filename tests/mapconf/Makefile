# Map retrieval designed for Rack tests by rack.doxymoron


.PRECIOUS: *.png 

PYTHON=python3.12
EPSG=3035
BBOX=2379694,1227288,6179967,5627586
SIZE=640,768
BASENAME := $(notdir $(CURDIR))


help-maps:
	@ echo "Usage: make [target]"
	@ X="`cat Makefile | grep -i '^[a-z][a-z\.\-]\+:' | cut -d' ' -f1 | tr -d ':'`"; echo make \[ $$X ... \]

default-map: out/map-${BASENAME}.png

out/map-${BASENAME}.png: all-maps
	@ cp -vi map-${BASENAME}.png out/ 

# map-${BASENAME}.png: all-maps

all-maps: map-TOPO-WMS.png map-world-map-combo-en.png 
	@ echo "Retrieved $@ $*"

include geo.cnf



map-%.png:
	-${PYTHON} -m rack.maps --mapLink $@ ./mapcache/EPSG:${EPSG}/BBOX=${BBOX}_LAYERS=$*_SIZE=${SIZE}_STYLES=.png && ln -sv $@ map-${BASENAME}.png 

clean:
	@ rm -vi map-*.png
