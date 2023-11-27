#!/bin/bash

#IMGDIR='../images'
#VENISON='http://localhost:8080/venison/cache'
#WGET='wget --proxy=off --no-clobber'

#if (( ${#*} < 3 )); then
if (( $# < 1 )); then
    echo "Creates a Cartesian colour image on top of an B-scan image, like 'anom-sea-fikor-panel.png' ."
    echo "Detects volume from /what:source:NOD ."
    echo "Usage:"
    echo "  ${0##*/} <nick> [ <volume> <site> ]"
    echo "Example:"
    echo "  ./make-panel.sh anom-noise"
    exit
fi
# basename map.png
# Arguments:
#   <volume-h5>
#   <site>           #<map-image>
#   [<panel-image>]
#function make_panel(){

BASENAME=$1
VOLUME=${2:-"volume-$BASENAME.h5"}
#SITE=`rack $VOLUME --format '${NOD}' -o -`
TIME=( `rack $VOLUME --format '${what:date}${what:time}' -o -` )
TIMESTAMP="${TIME:0:4}/${TIME:4:2}/${TIME:6:2} ${TIME:8:2}:${TIME:10:2}"


# If explicit quantity given
#

SITE=${SITE:-$3}
SITE=${SITE:-`rack $VOLUME --format '${NOD}' -o -`}
if [ "$SITE" == '' ]; then
    echo "# ERROR: SITE not given, and could not be derived from data. Exiting..."
    echo 
    exit -1
fi

#BASENAME=$BASENAME${SITE:+"-$SITE"}${QUANTITY:+"-$QUANTITY"}
BASENAME=$BASENAME${QUANTITY:+"-$QUANTITY"}

MAP=map-$SITE.png
make $MAP


#PANEL=$3
PANEL=${PANEL:-"$BASENAME-panel.png"}
QUANTITY=${QUANTITY:-'DBZH'}
#select=${QUANTITY:+"--select quantity=$QUANTITY"}
select="-Q $QUANTITY"

if [ $QUANTITY == 'TH' ]; then
    palette="--palette DBZH"
else
    palette="--palette ''"
fi

#cmd="rack $VOLUME $select -o $BASENAME.png --cSize 500,500 $select  -c $palette --target 'C,0.2,-32,1,100' --imageAlpha -o $BASENAME-cart-rgb.png"
cmd="rack $VOLUME $select --encoding C,0.4 -o $BASENAME.png --cSize 500 $select -c $palette --iPhysical false --imageTransp 0.1:0.3 -o $BASENAME-cart-rgb.png"
echo "$cmd"
eval "$cmd"


echo $TIMESTAMP $SITE $MAP


#if [ -s $BASENAME-cart-map.png ]; then
if [ -f $MAP ]; then
    #RGB1=tmp1.tif
    TMP2=tmp2.tif
    composite -compose Over $BASENAME-cart-rgb.png $MAP $BASENAME-cart-map.png
    #convert $BASENAME-cart-map.png -filter Point -resize 500x500\! -fill gray -pointsize 12 -draw "text 10,16 \"$TIMESTAMP\" text 10,30 \"$SITE\"" $RGB1
    convert $BASENAME.png -filter Point -resize 500x360\!  $TMP2
    #convert -append -frame 1  $RGB1 $RGB2  $PANEL
    convert -append -frame 1 $BASENAME-cart-map.png $TMP2 -fill black -pointsize 16 -draw "text 10,21 \"$TIMESTAMP\""  -fill gray -draw "text 10,40 \"$SITE\"" $PANEL 
    echo -e "Created: $PANEL \n"
    #convert  -append -frame 1 $BASENAME-cart-map.png $BASENAME.png mika.jpg
else
    echo "Empty map image: $MAP, PWD=$PWD "
    pwd 
    ls -ltr $MAP
    #rm -vf $MAP
    exit -1
fi
    






