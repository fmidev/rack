#!/bin/bash
set -o errexit

if (( ${#*} < 2 )); then
    echo "Creates andre example: command line and output files"
    echo "Usage:"
    echo "  ${0##*/} <nick> <detector1>[=<params>] ... --<detectorN>[=<params>]"
    echo "  ...where <nick> yields volume-<nick>.h5 and detector is in lowercase"
    echo "Example:"
    echo "  ${0##*/} anom-sea <detector1>[=<params>] ... --<detectorN>[=<params>]"
    exit
fi


# NICK ~ "CASE"
NICK=$1
VOLUME=volume-$NICK.h5

if [ ! -f $VOLUME ]; then
    echo "# Volume '$VOLUME' not found"
    exit
fi

SITE=${SITE:-`rack $VOLUME --format '${NOD}' -o -`}

# VOLUME_IMG=${VOLUME%%.*}.png
# VOLUME_IMG_PANEL=${NICK}${SITE:+"-$SITE"}-panel.png # GOOD
# VOLUME_IMG_PANEL=${NICK}-${SITE}-panel.png
VOLUME_IMG_PANEL=${NICK}-panel.png # DBZH?

# if [ "$SITE" == '' ]; then
# eval SITE=`rack --verbose 0 $VOLUME --format '${NOD}' --formatOut -`



#cmd="QUANTITY=$QUANTITY ./make-panel.sh $NICK $VOLUME $SITE"
cmd="make $VOLUME_IMG_PANEL"
echo "$cmd" 
if [ -f $VOLUME_IMG_PANEL ]; then
    echo "# $VOLUME_IMG_PANEL exists, ok (not running above cmd)"
else
    eval "$cmd"
    if [ $? != 0 ]; then
	echo "Making '$VOLUME_IMG_PANEL' Failed.."
	exit 1
    fi
fi

shift 
MULTIPLE=''
if [ ${#*} != '1' ]; then
  MULTIPLE='--store 1'
fi
echo $MULTIPLE

CMD="rack $VOLUME \\\\\n  $MULTIPLE\\\\\n"
while [ ${#*} != 0 ]; do

    #echo $1

    DETECTOR=( ${1/=/ } )
    VALUES=${DETECTOR[1]}
    DETLABEL=${DETECTOR[0]}           #  emitter2
    DETECTOR=${DETECTOR[0]//[0-9]/}   #  emitter
    aDETECTOR='a'${DETECTOR^} # aEmitter
    QUANTITY=${DETECTOR^^}    #  EMITTER

    echo "$DETECTOR ($aDETECTOR)"

    # NEw
    DEFAULT_VALUES=( `rack --helpExample $aDETECTOR | head -1` )
    DEFAULT_VALUES=${DEFAULT_VALUES[1]}
    VALUES=${VALUES:-$DEFAULT_VALUES}

    if [ "$MULTIPLE" == '' ]; then

	echo "# Primary test (echoed, for debugging)"
	VALUES=( ${VALUES[*]} )
	VALUES="${VALUES[*]}"
	VALUES=${VALUES// /,}
	#cmd="rack $VOLUME --$aDETECTOR $VALUES -o andre-$DETECTOR.h5"
	#echo "$cmd"
	####  eval "$cmd"
	#if [ $? != '0' ]; then
	#    echo "$aDETECTOR failed"
	#    exit 1
	#fi

	ANOM_BASE=andre-$NICK-$DETLABEL
	ANOM_IMG=$ANOM_BASE.png
	ANOM_IMG_CART=$ANOM_BASE-cart.png
	ANOM_IMG_PANEL=$ANOM_BASE-panel.png

	echo "# Longer test" # -o $VOLUME_IMG
	select="-Q '*${QUANTITY}*'" # +"OP"
	#cmd="rack $VOLUME   --store intermediate=1 --$aDETECTOR $VALUES $select -o $ANOM_IMG $select --cSize 500 --encoding C,0.004,-0.004,0,16  -c -o $ANOM_IMG_CART"
	cmd="rack $VOLUME   --store intermediate=1 --$aDETECTOR $VALUES $select -o $ANOM_IMG $select --cSize 500   -c -o $ANOM_IMG_CART"

	echo "$cmd"
	eval "$cmd"
	convert $ANOM_IMG -resize 500x360! tmp.png
	convert -append -frame 1 $ANOM_IMG_CART tmp.png $ANOM_IMG_PANEL

	# $VOLUME_IMG

	IMG_PANEL="result-$NICK-$DETLABEL.png"
	#convert +append -frame 1 $VOLUME_IMG_PANEL $ANOM_IMG_PANEL   -resize 600x600 $IMG_PANEL
	annotate="-pointsize 10 -draw 'text 10,35 \"$NICK\"'"
	convert +append -frame 1 $VOLUME_IMG_PANEL $ANOM_IMG_PANEL  -resize 600x600 -draw 'text 8,342 "'$NICK'"'  $IMG_PANEL

	#convert $IMG_PANEL -pointsize 8 -draw 'text 10,35 "'$NICK'"' result-$DETLABEL-$NICK.png
	
	#ls -l $IMG_PANEL 
	ls -ltr $VOLUME  $VOLUME_IMG_PANEL $ANOM_IMG $ANOM_IMG_PANEL  $IMG_PANEL

    else

	# Multiple...
	CMD="$CMD  --$aDETECTOR $VALUES \\\\\n" 

    fi

    shift

done



if [ "$MULTIPLE" != '' ]; then

    VOLUME_DET='volume-anom-detected.h5'
    CMD="$CMD   -o $VOLUME_DET \\\\\n"
    CMD="$CMD  --aRemover 0.5 \\\\\n"
    CMD="$CMD   -o volume-anom-removed.h5 \n"

    echo -e "$CMD"    
    echo -e "$CMD" > andre-detection.cmd
    source andre-detection.cmd

    # TODO

    cmd="rack $VOLUME_DET --aRemover 0.5 -o volume-anom-remover.png"
    echo "$cmd"
    eval "$cmd"

    cmd="rack $VOLUME_DET --aGapFill 1500,5 -o volume-anom-gapfill.png"
    echo "$cmd"
    eval "$cmd"

    cmd="rack $VOLUME_DET --aDamper 0.5,0.0 -o volume-anom-damper.png"
    echo "$cmd"
    eval "$cmd"



fi

exit


cmd="rack $VOLUME \
 --store intermediate=1 \
 --aBiomet 0dBZ,1000m,5dBZ,500m \
 --aEmitter 5000m,3.0deg,0.6 \
 --aEmitter 20000m,5.0deg,0.6 \
 --aShip 20dBZ,10dBZ,1500m,3deg \
 --aSpeckle -10dBZ,5 \
  -o volume-detected.h5 \
 --aGapFill 1500,5 --aDamper 0.5,0.0 \
  -o volume-corrected.h5
"

# --aBiomet 0dBZ,1000m,5dBZ,500m \
# --aShip 20dBZ,10dBZ,1500m,3deg \
# --aSpeckle -10dBZ,5 \

