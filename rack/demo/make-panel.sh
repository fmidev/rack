#!/bin/bash

#IMGDIR='../images'
#VENISON='http://localhost:8080/venison/cache'
#WGET='wget --proxy=off --no-clobber'

#if (( ${#*} < 3 )); then
if (( $# < 3 )); then
    echo "Creates a Cartesian colour image on top of an B-scan image"
    echo "Usage"
    echo "  ${0##*/} <nick> <volume> <site>"
    exit
fi
# basename map.png
# Arguments:
#   <volume-h5>
#   <site>           #<map-image>
#   [<panel-image>]
#function make_panel(){

BASENAME=$1
VOLUME=$2
#BASENAME=${VOLUME%%.*}
SITE=$3
MAP=map-$SITE.png
#PANEL=$3
PANEL=${PANEL:-"$BASENAME-$SITE-panel.png"}
QUANTITY=${QUANTITY:-'DBZH'}
select=${QUANTITY:+"--select quantity=$QUANTITY"}

if [ $QUANTITY == 'TH' ]; then
    palette="--palette palette-DBZH.txt"
else
    palette="--palette palette-$QUANTITY.txt"
fi

cmd="rack $VOLUME $select -o $BASENAME.png --cSize 500,500 $select -c $palette --target 'C,0.2,-32,1,100' --imageAlpha -o $BASENAME-cart-rgb.png"
echo "$cmd"
eval "$cmd"

composite -compose Over $BASENAME-cart-rgb.png $MAP $BASENAME-cart-map.png
convert $BASENAME-cart-map.png -filter Point -resize 500x500\! tmp1.tif
convert $BASENAME.png          -filter Point -resize 500x360\! tmp2.tif
convert -append -frame 1 tmp1.tif tmp2.tif  $PANEL

#}
#make_panel $*




