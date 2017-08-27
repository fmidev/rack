#!/bin/bash
# Generates an artificial radar volume (chessboard data).

NBINS=${NBINS:-'500'}
NRAYS=${NRAYS:-'360'}
RSCALE=${RSCALE:-'500'}

LON=${LON:-'25.0'}
LAT=${LAT:-'60.0'}

QUANTITY=${QUANTITY:-'DBZH'}
GAIN=${GAIN:-'0.5'}
OFFSET=${OFFSET:-'-32'}
UNDETECT=${UNDETECT:-'0'}
NODATA=${NODATA:-'255'}

# Base "data": a 2x2 grid replicated over the image
DARK=$((  RANDOM & 127 ))
LIGHT=$(( RANDOM & 127 + 128 ))
DATA=${DATA:-"$DARK,$LIGHT,$LIGHT,$DARK"}

OUTDIR=${OUTDIR:-"."}
OUTFILE=${OUTFILE:-"testdata-$QUANTITY.h5"}
IMGFILE=${OUTFILE%%.*}.png
TILEFILE=${OUTFILE%%.*}-tile.png

# Step 1: create png image file
echo $(( NBINS / 20 ))x$(( NRAYS / 20 ))
echo "P2 2 2 255  ${DATA//,/ }" | convert pgm:- -filter Point -resize $(( NBINS / 5 ))x$(( NRAYS / 20 ))\! $OUTDIR/${TILEFILE}

convert -size ${NBINS}x${NRAYS} tile:$OUTDIR/${TILEFILE} -type GrayScale -depth 8 $OUTDIR/${IMGFILE}

rack ${OUTDIR}/${IMGFILE} \
--/what:object=PVOL \
--/where:lon=$LON \
--/where:lat=$LAT \
--/dataset1/data1/where:nbins=$NBINS \
--/dataset1/data1/where:nrays=$NRAYS \
--/dataset1/data1/where:rscale=$RSCALE \
--/dataset1/data1/what:quantity=$QUANTITY \
--/dataset1/data1/what:gain=$GAIN   \
--/dataset1/data1/what:offset=$OFFSET \
-o ${OUTDIR}/${OUTFILE}

rm $OUTDIR/${IMGFILE} 
rm $OUTDIR/${TILEFILE}