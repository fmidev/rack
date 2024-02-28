#!/bin/bash

INPUT_VOLUME=${1:-'volume.h5'}
INPUT_VOLUME=${INPUT_VOLUME:-'volume.h5'}
INPUT_BASENAME=${INPUT_VOLUME##*/}
INPUT_BASENAME=${INPUT_BASENAME%.*}
OUTDIR=$PWD
LEGENDS=${LEGENDS:-'true'}
LABEL=${LABEL:-"$INPUT_BASENAME"}
DEMO=${DEMO:-0}

OUTFILE=${OUTFILE:-"$PWD/$LABEL-panel.svg"}
FORMAT=${OUTFILE##*.}
OUTFILE_SVG=${OUTFILE%.*}'.svg'
OUTFILE_PNG=${OUTFILE%.*}'.png'
OUTFILE_CMD=${OUTFILE%.*}'.cmd'

if [ $# == 0 ]; then
    echo '# Utility to create single-radar image panels'
    echo '# Usage: '
    echo "${0} <input-volume.h5> [quantity] [quantity2]"
    
fi

shift

EVEN=$(( $# % 2 == 0 ))
LAYOUT=${LAYOUT:-'orientation=HORZ,direction=INCR'} 
ENDLINE=`echo -e '#%'`

cmd="rack $INPUT_VOLUME --outputConf svg:$LAYOUT $ENDLINE";

while [ $# != 0 ] ; do
    QUANTITY=$1
    cmd="$cmd -Q '$QUANTITY' -c --palette '$QUANTITY' "
    if (( $EVEN )) && (( $# % 2 == 0 )); then
	# Default: legend first, then radar image
	cmd="$cmd --legendOut $OUTDIR/leg-$QUANTITY.svg --outputFile $OUTDIR/$LABEL-$QUANTITY.png $ENDLINE"
    else
	# Image first, then legend (ending the row)
	cmd="$cmd --outputFile $OUTDIR/$LABEL-$QUANTITY.png --legendOut $OUTDIR/leg-$QUANTITY.svg $ENDLINE"
	cmd="$cmd --outputConf 'svg:group=set$#' $ENDLINE";
    fi
    shift
done

cmd="$cmd --outputFile $OUTFILE_SVG $ENDLINE"

echo $cmd
if (( $DEMO > 0 )); then
    echo $cmd | tr '#%' '\\\n' > $OUTFILE_CMD
fi

cmd=`echo $cmd | tr -d '#%'`
eval $cmd
if [ $? != 0 ]; then
    echo "# Something went wrong, error code: $?"
    exit 1
else
    display $OUTFILE
fi


if [ $FORMAT == 'png' ]; then
    echo "# Converting $OUTFILE_-> SVG $OUTFILE_PNG"
    cmd="convert $OUTFILE_SVG $OUTFILE_PNG"
    echo $cmd
fi


