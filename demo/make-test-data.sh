#!/bin/bash
#
# Generates an artificial radar volume (chessboard data).
#
# Part of Rack distribution
#
# Markus.Peura@fmi.fi

PATH=${PATH}':/fmi/dev/bin'

QUANTITY=${QUANTITY:-'DBZH'}
OUTDIR=${OUTDIR:-"."}
OUTFILE=${OUTFILE:-"testdata-$QUANTITY.h5"}

NBINS=${NBINS:-'500'}
NRAYS=${NRAYS:-'360'}
RSCALE=${RSCALE:-'500'}

LON=${LON:-'25.0'}
LAT=${LAT:-'60.0'}


GAIN=${GAIN:-'0.5'}
OFFSET=${OFFSET:-'-32'}
UNDETECT=${UNDETECT:-'0'}
NODATA=${NODATA:-'255'}

# Base "data": a 2x2 grid replicated over the image
VALUES=${VALUES:-$(( RANDOM & 127 )),$(( RANDOM & 127 + 128 ))}
VALUES=${VALUES/:/,}
#VALUES=${VALUES:-"64:192"}
VALUES=( ${VALUES/,/ } )
DARK=${VALUES[0]} 
LIGHT=${VALUES[1]} 
#DARK=$((  RANDOM & 127 ))
#LIGHT=$(( RANDOM & 127 + 128 ))
DATA=${DATA:-"$DARK,$LIGHT,$LIGHT,$DARK"}

BASENAME=${OUTFILE%.*}
IMGFILE=${BASENAME}-raw.png
TILEFILE=${BASENAME}-tile.png
CMDFILE=${BASENAME}.sh

#
if [ "$COORD" == 'c' ]; then
    cart='-c'
else
    cart=''
fi

# PALETTE=default
if [ "$PALETTE" == 'default' ]; then
    PALETTE=$QUANTITY
fi
palette=${PALETTE:+"--palette palette/palette-$PALETTE.txt"}


# Step 1: create png image file
echo $(( NBINS / 20 ))x$(( NRAYS / 20 ))
echo "P2 2 2 255  ${DATA//,/ }" | convert pgm:- -filter Point -resize $(( NBINS / 5 ))x$(( NRAYS / 20 ))\! ${OUTDIR}/${TILEFILE}

if [ ! -s "${OUTDIR}/${TILEFILE}" ]; then
    echo "502 Could not generate tile file: ${OUTDIR}/${TILEFILE}" 
    exit 1
fi

if [ "$FORMAT" == 'sh' ]; then
   OUTFILE=${BASENAME}.h5
fi

   

cat > $OUTDIR/$CMDFILE <<EOF
# Step 2: expand to a gridded png image
convert -size ${NBINS}x${NRAYS} tile:$OUTDIR/${TILEFILE} -type GrayScale -depth 8 ${OUTDIR}/${IMGFILE}

# Step 3: add ODIM variables, convert to HDF5
rack ${OUTDIR}/${IMGFILE} \
  --/what:object=PVOL \
  --/where:lon=$LON \
  --/where:lat=$LAT \
  --/how:simulated=True \
  --/dataset1/where:nbins=$NBINS \
  --/dataset1/where:nrays=$NRAYS \
  --/dataset1/where:rscale=$RSCALE \
  --/dataset1/data1/what:quantity=$QUANTITY \
  --/dataset1/data1/what:gain=$GAIN   \
  --/dataset1/data1/what:offset=$OFFSET \
  --completeODIM \
  ${cart} ${palette} \
  -o ${OUTDIR}/${OUTFILE}

rm -v $OUTDIR/${IMGFILE} 
rm -v $OUTDIR/${TILEFILE}
EOF

if [ "$FORMAT" != 'sh' ]; then
   source $OUTDIR/$CMDFILE
fi

# That's it
