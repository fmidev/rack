#!/bin/bash

#if (( ${#*} < 2 )); then
    echo "Creates met. product example(s): command line and output files"
    echo "Usage"
    echo "  INPUT=volume.h5 PROD=pCappi,1500 ${0##*/} "
#    exit
#fi

# 
# PROD=cappi,500
# QUANTITY=DBZH

# Input is volume.h5 by default
INPUT=${INPUT:-'volume.h5'}

# If product not given, use PPI or so.
# WAS:Product is cappi by default
# PROD=${PROD:-'pCappi,500'}

# Split by 1st comma
OPERATOR=( ${PROD/,/ } )

#product=${PROD:+"--${OPERATOR[*]}"}

VALUES=${OPERATOR[1]}
OPERATOR=${OPERATOR[0]}


OPERATOR=${OPERATOR:-'ppi'}



# Displayed quantity is DBZH by default
QUANTITY=${QUANTITY:-'DBZH'}

echo "OPERATOR=$OPERATOR"

#PROD_BRIEF=${OPERATOR:1}
#PROD_BRIEF=${PROD_BRIEF,,}

echo $PROD_BRIEF

if [ "$PROD" != '' ]; then

    # Default values include all the parameters, which is not
    # informative in examples.
    #DEFAULT_VALUES=( `rack --helpExample $OPERATOR | fgrep 'Default values' | cut -d ':' -f2-` )
    DEFAULT_CMD=( `rack --helpExample $OPERATOR | head -1` )
    DEFAULT_VALUES=${DEFAULT_CMD[1]}
    echo "DEFAULT_VALUES: $DEFAULT_VALUES"

    NAME=${OPERATOR:1}
    NAME=${NAME,} # lowercase first

    VALUES=${VALUES:-$DEFAULT_VALUES}
    echo "rack volume.h5 --$OPERATOR $VALUES -o $NAME.h5" > $NAME-cmd.inc

    
    #echo >> $NAME.cmd
    rack --helpExample $OPERATOR | fgrep '#' | tr -d '#' > $NAME-params.inc
    head $NAME-*.inc

fi


OUTFILE_GRAY=$OPERATOR-$QUANTITY.png
OUTFILE_RGB=$OPERATOR-$QUANTITY-rgb.png
OUTFILE_QUALITY=$OPERATOR-QIND.png
OUTFILE_LEGEND=legend-$QUANTITY.png
OUTFILE_PROD=$OPERATOR-$QUANTITY-rgb-leg.png

OUTFILE_FINAL=$OPERATOR-$QUANTITY-${NICK:+"$NICK-"}panel.png

product=${PROD:+"${ENCODING:+--encoding $ENCODING} --$OPERATOR $VALUES"}

cmd="rack $INPUT $product --cSize 400,400 --select quantity=$QUANTITY -c  -o $OUTFILE_GRAY  --palette palette-${QUANTITY/_/-}.txt -o $OUTFILE_RGB --legendOut tmp.svg"
echo $cmd
eval $cmd

#if [ ! -f $OUTFILE_LEGEND ]; then
echo "# Creating legend..."
#cmd="convert tmp.svg  $OUTFILE_LEGEND"
cmd="inkscape -z --export-png $OUTFILE_LEGEND tmp.svg"
echo $cmd
eval $cmd
#fi

cmd="convert +append $OUTFILE_RGB $OUTFILE_LEGEND $OUTFILE_PROD"
echo $cmd
eval $cmd

if [ "$PROD" != '' ]; then 
  echo $OPERATOR

  cmd="rack $INPUT $product --cSize 400,400 -c --select quantity=QIND -o $OUTFILE_QUALITY"
  echo $cmd
  eval $cmd
  
  if [ -f $OUTFILE_QUALITY ]; then
      convert -frame 3 +append $OUTFILE_PROD $OUTFILE_QUALITY $OUTFILE_FINAL
  else
      echo "# No quality field (QIND), ok..."
      mv -v $OUTFILE_PROD  $OUTFILE_FINAL
  fi
else
  mv $OUTFILE_PROD  $OUTFILE_FINAL
  #convert -frame 3 +append $OPERATOR-$QUANTITY-image.png $OPERATOR-image.png 
fi
echo "Created: $OUTFILE_FINAL"
