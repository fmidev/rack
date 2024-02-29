#!/bin/bash

#INPUT_VOLUME=${1:-'volume.h5'}
#INPUT_VOLUME=${INPUT_VOLUME:-'volume.h5'}
#INPUT_BASENAME=${INPUT_VOLUME##*/}
#INPUT_BASENAME=${INPUT_BASENAME%.*}
#LABEL=${LABEL:-"$INPUT_BASENAME"}
LABEL=${LABEL:-'rack-panel'}
OUTDIR=$PWD
LEGENDS=${LEGENDS:-'true'}
DEMO=${DEMO:-0}

OUTFILE=${OUTFILE:-"$PWD/$LABEL-panel.svg"}
FORMAT=${OUTFILE##*.}
OUTFILE_SVG=${OUTFILE%.*}'.svg'
OUTFILE_PNG=${OUTFILE%.*}'.png'
OUTFILE_CMD=${OUTFILE%.*}'.cmd'
CMD_ARGS="$*" # save for cmd file

function rack_panel_help(){
    echo '# Utility to create single-radar image panels'
    echo '# Usage: '
    echo "${0} <volume.h5> [product] [quantity] [quantity2]"
}
    

if [ $# == 0 ]; then
    rack_panel_help
fi

# shift

#EVEN=$(( $# % 2 == 0 ))
LAYOUT=${LAYOUT:-'orientation=HORZ,direction=INCR'}
MAX=10
QUANTITY='DBZH'
ENDLINE=`echo -e '#%'`



# MAIN

cmd="rack --outputConf svg:$LAYOUT $ENDLINE";

COUNTER=0
while [ $# != 0 ] ; do

    ARG=$1
    shift
    
    case $ARG in
	--max)
	    MAX=$1
	    echo "# Setting MAX=$MAX"
	    shift
    	    continue
	    ;;
	[A-Z]*)
	    echo $ARG quantity
	    QUANTITY="$ARG"
	    ;;
	*.h5 | *.hdf)
	    echo $ARG file
	    INPUT_VOLUME=$ARG
	    cmd="$cmd '$ARG' $ENDLINE"
	    continue
	    ;;
	--p*)
	    echo $ARG $1 command
	    cmd="$cmd '$ARG' $1 $ENDLINE"
	    shift
	    continue
	    ;;
	*)
	    echo "# could not read $ARG"
	    continue
    esac

    if [ ! -v INPUT_VOLUME ]; then
	echo "# Input not yet given, ok"
	continue
	#rack_panel_help
	#exit 1
    fi
    
    cmd="$cmd -Q '$QUANTITY' -c --palette '$QUANTITY' "
    if (( MAX > 0 )) && (( COUNTER % MAX == 0 )); then
	# Image first, then legend (ending the row)
	cmd="$cmd --outputFile $OUTDIR/$LABEL-$QUANTITY-$COUNTER.png --legendOut $OUTDIR/leg-$QUANTITY.svg $ENDLINE"
	# Start new line / column
	cmd="$cmd --outputConf 'svg:group=set$COUNTER' $ENDLINE";
    else
	# Default: legend first, then radar image
	cmd="$cmd --legendOut $OUTDIR/leg-$QUANTITY.svg --outputFile $OUTDIR/$LABEL-$QUANTITY-$COUNTER.png $ENDLINE"
    fi

    COUNTER=$(( ++COUNTER ))
done

cmd="$cmd --outputFile $OUTFILE_SVG "



if (( $DEMO == 0 )); then
    echo $cmd | tr '#%' '\\\n'
    echo
else
    echo "# $0 $CMD_ARGS" > $OUTFILE_CMD
    echo >> $OUTFILE_CMD
    echo $cmd | tr '#%' '\\\n' >> $OUTFILE_CMD
    cat $OUTFILE_CMD
    echo
fi

cmd=`echo $cmd | tr -d '#%'`
eval $cmd
if [ $? != 0 ]; then
    echo "# Something went wrong, error code: $?"
    exit 1
else
    echo "# Created $OUTFILE_SVG"
    display $OUTFILE
fi


if [ $FORMAT == 'png' ]; then
    echo "# Converting $OUTFILE_-> SVG $OUTFILE_PNG"
    cmd="convert $OUTFILE_SVG $OUTFILE_PNG"
    echo $cmd
fi


