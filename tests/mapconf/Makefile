# Map retrieval designed for Rack tests by rack.doxymoron


.PRECIOUS: *.png 


EPSG=3035
BBOX=2379694,1227288,6179967,5627586
SIZE=640,768

include geo.cnf


all: map-TOPO-WMS.png map-world-map-combo-en.png
	@echo "Retrieved $@ $*"

map-%.png:
	-python3 -m rack.maps --mapLink $@ ./mapcache/EPSG:${EPSG}/BBOX=${BBOX}_LAYERS=$*_SIZE=${SIZE}_STYLES=.png && ln -svi $@ map.png

