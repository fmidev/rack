#!/bin/bash


if (( ${#*} < 2 )); then
    echo "Creates andre example: command line and output files"
    echo "Usage"
    echo "  ${0##*/} <nick> <detector1=params1>  <params2> "
    exit
fi

NICK=$1
VOLUME=volume-$NICK.h5
SITE=${SITE:-`rack $VOLUME --format '${NOD}' -o -`}
SITE=${SITE:-'fixxx'}

if [ ! -f $VOLUME ]; then
   echo "Input volume '$VOLUME' not found"
   exit 1
fi

VOLUME_DETECTED=volume-$NICK-detected.h5

# rm -v  $VOLUME_DETECTED

cmddet="rack $VOLUME --keep '/dataset1'  --aBird '' --aEmitter '' --aJamming '' --aShip '' --aSpeckle '' --aNonMet '' "

echo "VOLUME_DETECTED=$VOLUME_DETECTED"
if [ ! -f $VOLUME_DETECTED ]; then
    echo "$cmddet -o $VOLUME_DETECTED" >& ${VOLUME_DETECTED%.*}.cmd
    eval "$cmddet -o $VOLUME_DETECTED"
fi

shift

SUBPANELS=''
while [ ${#*} != 0 ]; do
    OPERATOR=( ${1/=/ } )
    VALUES=${OPERATOR[1]}
    OPERATOR=${OPERATOR[0]}   #  emitter
    aOPERATOR='a'${OPERATOR^} # aEmitter
    #QUANTITY=${OPERATOR^^}    #  EMITTER
    VOLUME_CORRECTED=volume-$NICK-$OPERATOR,$VALUES.h5
    if [ $aOPERATOR == 'aGapFill' ]; then
	pregap="--aRemover 0.6"
    else
	pregap=''
    fi 
    cmd="rack $VOLUME_DETECTED $pregap --$aOPERATOR $VALUES -o $VOLUME_CORRECTED"
    #cmd="$cmddet --select quantity=DBZH --$aOPERATOR $VALUES -o $VOLUME_CORRECTED"
    echo
    echo "# PANEL $cmd"
    eval "$cmd"

    VOLUME_IMG_SUBPANEL=volume-$NICK-$OPERATOR,$VALUES-panel.png
    #if [ ! -f XXX$VOLUME_IMG_SUBPANEL ]; then # QUANTITY=DBZH 
    cmd="./make-panel.sh $NICK $VOLUME_CORRECTED " #$SITE"
    echo "$cmd" 
    eval "$cmd"
    #mv -v $NICK-$SITE-panel.png $VOLUME_IMG_SUBPANEL
    convert $NICK-panel.png -fill black -pointsize 16 -draw "text 400,21 \"$OPERATOR=$VALUES\""  $VOLUME_IMG_SUBPANEL
    #fi
    SUBPANELS="$SUBPANELS $VOLUME_IMG_SUBPANEL"
    shift
done

VOLUME_IMG_PANEL=volume-$NICK-$OPERATOR-panel.png
cmd="convert +append $SUBPANELS -resize 800x640 $VOLUME_IMG_PANEL"
echo "$cmd"
eval "$cmd"

VOLUME_IMG_ANIM=volume-$NICK-$OPERATOR-panel.gif
cmd="convert -delay 60 -loop 0 $SUBPANELS -resize 800x640 $VOLUME_IMG_ANIM"
echo "# For animation:"
echo "$cmd"
#eval "$cmd"
echo

