#!/bin/bash


# VOLUME=volume-widespread.h5
VOLUME=${VOLUME:-'201304181130_fivan_doppler.h5'}

WINDOW=${WINDOW:-'2500,90'}

BASENAME='doppler-demo'
SAMPLES="$BASENAME.dat"
SAMPLES_2="$BASENAME-2.dat"
SAMPLES_CART="$BASENAME-cart.dat"

# Comment if not desired
rm $SAMPLES

IMAGE_BG=$BASENAME-bkg.png

echo "Demonstrates doppler dealiasing"
#echo "Example:"

echo "Creating darker background image"

WIDTH=${WIDTH:-'500'}
HEIGHT=${HEIGHT:-$WIDTH}


BBOX='{where:LL_lon} {where:LL_lat}\n{where:UR_lon} {where:UR_lat}\n'
PROJ='{where:projdef}\n'

SCALE=${SCALE:-'0.001'}

verbose=${VERBOSE:+"--verbose $VERBOSE"}

echo "Detect LAT,LON bounding box and projection."
echo "Dump data in LAT,LON positions"
echo "Convert data and the bbox to metrix coords with desired scale ($SCALE)"


#rack 201304181130_fivan_doppler.h5 --pDopplerDeAlias 2500,90 -o  foo.h5   --append data -Q AMVU -c -Q AMVV -c -o out-dealiased-cart.h5


FORMAT='{LON} {LAT}\t{AMVU} {AMVV} {QIND}'
#FORMAT='{i} {j2} {AMVU} {AMVV} {QIND}'

# --cSize $WIDTH,$HEIGHT 

if [ ! -f $SAMPLES ]; then 
    cmd="rack $verbose $VOLUME  -Q DBZH -t gain=1.0 -c -o $IMAGE_BG "
    cmd="$cmd  -t d  --pDopplerDeAlias $WINDOW"
    #cmd="$cmd -o out-dealiased.h5  --sample 30,30,skipVoid=1 --format '{LON} {LAT}\t{AMVU} {AMVV} {QIND}' -o $SAMPLES  "
    cmd="$cmd --append data  -Q AMVU -c -Q AMVV -c  -o out-dealiased-cart.h5 --format '$FORMAT' -o $SAMPLES_CART "
    cmd="$cmd --formatFile vector-field.gnu --formatOut doppler.gnu"
    echo $cmd
    eval $cmd
fi

if [ -f $SAMPLES ]; then
    head $SAMPLES
fi 

export HEADSIZE=0.05,4,8

cmd="OUTFILE=$BASENAME.png TEMPLATE_GNP=doppler.gnu  gnuplot-vectors.sh $SAMPLES_CART $IMAGE_BG"
echo $cmd
eval $cmd
