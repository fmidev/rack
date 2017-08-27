#!/bin/bash

for i in widespread/2009*.h5; do
    #base=${i##*/}
    short=${i#*.}
    echo $short
   # rack $i --delete dataset1?[2-90] --delete data[124-9] --/dataset1/data3/what:{gain=0.0596929,offset=-7.64069} -o widespread/$short
done;


for Q in AMVU AMVV; do
    
rack --cMethod MAX \
     --cProj '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs' \
     --cBBox 17,57.7,33,69.5 \
     --cSize 800,1000 \
     --cInit \
     --verbose 5 \
     --script "--pDopplerDeAlias 1500,95 -Q $Q --cCreate --cAdd"  \
     widespread/polar.*.h5 \
     --cExtract dwsc \
-o composite-$Q.h5 

done

# --sample '30,30,skipVoid=1' --format '{LON} {LAT}\t{AMVU} {AMVV} {QIND}' -o 

rack  composite-AMVU.h5 composite-AMVV.h5  --sample '30,30,skipVoid=1' --format '{LON} {LAT}\t{AMVU} {AMVV} {QIND}' -o foo.dat
