#!/bin/bash

INFILE=${INFILE:-$1}
INFILE=${INFILE:-'measurements.dat'}
QUANTITY=${QUANTITY:-'RATE'}
GAIN=${GAIN:-'0.02'}
ENCODING=${ENCODING:-"C,gain=${GAIN},offset=-${GAIN},undetect=254,nodata=0,quantity=${QUANTITY}"}
# Finland
BBOX=${BBOX:-'17,57.75,32.75,70'}
SIZE=${SIZE:-'500,750'}

palette=`ls palette-${QUANTITY}.{json,txt}` &> /dev/null
cmd="rack  --cProj '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs' --cBBox $BBOX  --cSize $SIZE  \
  --encoding ${ENCODING} \
  --cPlotFile ${INFILE} \
  --cExtract dw \
  -o plot.png \
  --iDistanceTransformFill 7 \
  -o plot-spread1.png  \
  --palette $palette --paletteRefine 64  -o plot-spread-color.png  "


#  --palette palette-RATE.txt --paletteRefine 64 --imageTransp 0,0.5,0.2  -o plot-spread-color.png  
#  --palette palette-RATE.txt --paletteRefine 64 --imageTransp 0:2,0.5,0.2 -o plot-spread-color.png  
#   --encoding C,gain=${GAIN},offset=-${GAIN},undetect=254,nodata=0,quantity=${QUANTITY} \

echo $cmd
eval $cmd

#  -o plot-spread2.h5  \
#  --iGaussianAverage 15 \
#  -o plot-spread2.png  \
#  --iGaussianAverage 15 \
#  --iAverage 15 \


