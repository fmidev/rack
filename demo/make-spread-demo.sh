#!/bin/bash

# Inputs:
#INFILE=shapes1-motion-q.h5
INFILE=${INFILE:-motion-demo.h5}


#ORIG_BASE=shapes1-motion-vis
#if [ ! -f ${ORIG_BASE}.png ]; then
#    BACKGROUND=shapes1.png OUTFILE=${ORIG_BASE}.png ../demo/draw-vectors.sh $INFILE
#    convert -frame 1 +append ${ORIG_BASE}.png shapes1-q.png ${ORIG_BASE}-panel.png
#fi
   


# for i in Average-50 GaussianAverage-100 FlowAverage-50 DistanceTransformFill-50 DistanceTransformFillExp-50; do make spread-${i} METHOD=${i%-*} W=${i#*-}; done

#W=${W:-10}
if (( W > 0 )); then
    echo $0 Average-$W GaussianAverage-$(( 2 * W )) FlowAverage-$W  Blender-$W  DistanceTransformFill-$W DistanceTransformFillExp-$W
    exit 0
fi

BACKGROUND=${BACKGROUND-'shapes1.png'}

OUTFILE1=$1
OUTFILE1=${OUTFILE1:-"Average-50.h5"}
shift

CODE=code.tmp
echo > $CODE

for OUTFILE in ${OUTFILE1} $*; do
    
    #OUTFILE=${OUTFILE:-"Average-50.h5"}
    OUTBASE=${OUTFILE%.*}
    METHOD=${OUTBASE%-*}
    W=${OUTBASE#*-}
    echo "METHOD=$METHOD WIDTH=$W"
    
    rack ${INFILE} --i${METHOD} ${W},${W} -o $OUTBASE.h5
    BACKGROUND=$BACKGROUND STEP=25 COLORS=pink,blue,green:0 SCALE=0.75 OUTFILE_GNP=$METHOD-gnu.png OUTFILE=$METHOD-combo.png ./draw-vectors.sh $OUTBASE.h5
    convert $METHOD-combo.png -pointsize 20 -fill white -draw "text 20,30 \"${METHOD} ${W}x${W}\"" $METHOD-combo.png 
    convert -frame 1 +append $METHOD-combo.png  $METHOD-gnu.png  spread-$METHOD-panel.png

    echo -e "\\\code\n rack motion.h5 --i${METHOD} ${W},${W}  motion-${METHOD}.h5\n\\\endcode" >> $CODE
    echo >> $CODE 
    echo "\\image html  spread-$METHOD-panel.png \"Vector field spread with $METHOD, window size $W x $W.\"" >> $CODE
    echo "\\image latex spread-$METHOD-panel.png \"Vector field spread with $METHOD, window size $W x $W.\" width=0.5\textwidth" >> $CODE
    echo >> $CODE 

done
