#!/bin/bash

# OpenLayers 3857 4326

for EPSG in 3035 3067  3857 3844 4326; do
    fgrep \<$EPSG\> /usr/share/proj/epsg ;
    rack --verbose 4 volume.h5 --cProj +init=epsg:$EPSG -c --verbose 5 -o epsg-$EPSG.tif ;
done
