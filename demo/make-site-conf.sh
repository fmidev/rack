#!/bin/bash

if (( !$# )); then
  echo 'Rack utility script'
  echo 'Given input volumes from different sites, dumps site coordinates.'
  echo 'Usage:'
  echo $0 '<files*.h5>'
  exit
fi

#if [[ -f composite.cnf ]]; then
#    source composite.cnf
#fi


for i in $*; do
   echo "# $i" 
   rack --verbose 0 $i  --format '${what:sourceCMT}=${where:lon},${where:lat}\n' --formatOut -
   echo 
done

echo 


#COMP="--cProj '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs' --cBBox 0,0,89,89  --cSize 500,500 --cInit"
# --cBBox 0,0,89,89  --cSize 500,500 --cInit
for i in $*; do
   echo "# $i" 
   rack --verbose 0 --cProj '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs' --cSize 15000,15000 $i -c --format 'BBox=${where:LL_lon},${where:LL_lat},${where:UR_lon},${where:UR_lat}\n' --formatOut - -o out.png
   echo 
done


