#!/bin/bash

# Creates an image with 2 x 2 panels illustrating compositing

METHOD=${METHOD:-'WAVG,1,4'}

#NICK="-andre-$METHOD"
POLARPRODUCT=pCappi,500
NICK=${NICK:-"-raw-${POLARPRODUCT}-${METHOD}"}
#NICK="-andre-$POLARPRODUCT,$METHOD"


# POLARPRODUCT=cappi,500
DEMO=YES WEIGHT=1.0 OUTFILE=composite$NICK.png BBOX=8,48,33,70 POLARPRODUCT=$POLARPRODUCT ./make-composite.sh data4/
#data/pvol*.h5 

convert  -frame 1 +append composite${NICK}{001,-rgb}.png -resize 50% c-data.png 
convert  -frame 1 +append composite${NICK}{004,003}.png  -resize 50% -gamma 4 c-stat.png

convert -append c-data.png  c-stat.png   composite${NICK}.png

