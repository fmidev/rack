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
TITLE=${TITLE:-''}
#ABS_PATHS=

# ${FILE##*/}

OUTFILE=${OUTFILE:-"$PWD/$LABEL-panel.png"} # .svg"}
#echo $OUTFILE

OUTDIR=${OUTDIR:-${OUTFILE%/*}}
OUTFILE=${OUTFILE##*/}
#echo $OUTFILE

if [ "$OUTDIR" == "$OUTFILE" ]; then
    OUTDIR=$PWD
fi

FORMAT=${OUTFILE##*.}
OUTFILE_SVG=${OUTFILE%.*}'.svg'
OUTFILE_PNG=${OUTFILE%.*}'.png'
OUTFILE_CMD=${OUTFILE%.*}'.cmd'
CMD_ARGS="$*" # save for cmd file

function rack_panel_help(){
    echo '# Utility to create single-radar image panels'
    echo '# Usage: '
    #echo "${0} <volume.h5> [product] <quantity> [quantity2]"
    echo "${0} <args> "
    echo "Arguments can be:"
    echo "  - input files, like: volume.h5"
    echo "  - ODIM quantities, like: DBZH"
    echo "  - Rack product cmds, like: --pCappi 500"
    echo "Also env variables are supported, like "
}
    

if [ $# == 0 ]; then
    rack_panel_help
    exit 0
fi

echo $OUTFILE_SVG
# exit 1
# shift

#EVEN=$(( $# % 2 == 0 ))
ORIENTATION=${ORIENTATION:-'HORZ'}
DIRECTION=${DIRECTION:-'INCR'}
# LAYOUT=${LAYOUT:-'orientation=HORZ,direction=INCR'}
MAX=${MAX:-'0'}
QUANTITY='DBZH'
ENDLINE=`echo -e '#%'`



# MAIN

cmd="rack $ENDLINE";
cmd="$cmd --outputPrefix $OUTDIR/ $ENDLINE";
cmd="$cmd --outputConf svg:title='$TITLE',absolutePaths=true,orientation=$ORIENTATION,direction=$DIRECTION $ENDLINE"; # $LAYOUT

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
	QIND)
	    cmd="$cmd -Q QIND --outputFile $LABEL-$QUANTITY-QIND-$COUNTER.png $ENDLINE"
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
   
    cmd="$cmd -Q '$QUANTITY' --cCreate "
    #if [ "$QUANTITY" != 'QIND' ]; then
    PALETTE=$QUANTITY
    #else
    #PALETTE="QIND-BW"
    #fi
    cmd="$cmd --palette '$PALETTE' "
    
    if (( MAX > 0 )) && (( COUNTER % MAX == 0 )); then
	# Image first, then legend (ending the row)
	cmd="$cmd --outputFile $LABEL-$QUANTITY-$COUNTER.png"
	#if [ "QUANTITY" != 'QIND' ]; then
	cmd="$cmd --legendOut leg-$QUANTITY.svg "
	#fi
	cmd="$cmd $ENDLINE" # 
	# Start new line / column
	cmd="$cmd --outputConf 'svg:group=set$COUNTER' $ENDLINE";
    else
	# Default: legend first, then radar image
	#if [ "QUANTITY" != 'QIND' ]; then
	cmd="$cmd --legendOut leg-$QUANTITY.svg " # $ENDLINE"
	#fi
	cmd="$cmd --outputFile $LABEL-$QUANTITY-$COUNTER.png $ENDLINE"
    fi

    echo "# COUNTER $COUNTER / $MAX"
    
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
    #display $OUTFILE
    inkscape $OUTFILE_SVG -o $OUTFILE
fi


if [ $FORMAT == 'png' ]; then
    echo "# Converting $OUTFILE_-> SVG $OUTFILE_PNG"
    #cmd="convert $OUTDIR/$OUTFILE_SVG $OUTDIR/$OUTFILE_PNG"
    cmd="inkscape $OUTDIR/$OUTFILE_SVG -o $OUTDIR/$OUTFILE_PNG"
    echo $cmd
fi


