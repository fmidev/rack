#!/bin/bash

#if (( ${#*} < 2 )); then
echo "Creates met. product example(s): command line and output files"
echo "Usage"
echo "  INPUT=volume.h5 PROD=pCappi,1500 ${0##*/} "
echo "  INPUT=volume.h5 SELECT=/dataset1:3 PROD=pCappi,1500 ${0##*/} "
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
# echo $PROD_BRIEF

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

    make $OPERATOR.hlp
    make $OPERATOR.exm
    
fi


#OUTFILE_GRAY=$OPERATOR-$QUANTITY.png
#OUTFILE_RGB=$OPERATOR-$QUANTITY-rgb.png
#OUTFILE_QUALITY=$OPERATOR-QIND.png
#OUTFILE_LEGEND=legend-$QUANTITY.png
#OUTFILE_PROD=$OPERATOR-$QUANTITY-rgb-leg.png
ID='tmp-${what:product}-${what:quantity}'

OUTFILE=${OUTFILE:-$OPERATOR-$QUANTITY-${NICK:+"$NICK-"}panel.png}


quality=${PROD:+"-Q QIND -o '$ID-QIND.png'"}

select=${SELECT:+"--select $SELECT"} 
product=${PROD:+"${ENCODING:+--encoding $ENCODING} --$OPERATOR $VALUES"}
# Add line only if either is set
prodsel="$select$product"
prodsel=${prodsel:+"$select $product"}

TMP_DIR="$PWD/out"
TMP_FILE_SVG="panel-$NICK.svg"
mkdir --parents $TMP_DIR

style="--gStyle .BORDER='stroke:darkslateblue;stroke-width:1px' --gStyle .IMAGE_BORDER='stroke:slateblue;stroke-width:1px' \\"

cmd="rack $INPUT \\ --outputPrefix $TMP_DIR/ \\ $style $prodsel --cSize 500,500 --select quantity=$QUANTITY -c \\ --paletteDefault -o '$ID-rgb.png' \\$quality --paletteOut '$ID-legend.svg'\\ -o $TMP_FILE_SVG"


echo ${cmd//\\/ }
eval ${cmd//\\/ }

echo "# Converting SVG -> PNG..."
#cmd="convert tmp.svg  $OUTFILE_LEGEND"
#cmd="inkscape -z --export-png $OUTFILE_LEGEND tmp.svg"
cmd2="convert  $TMP_DIR/$TMP_FILE_SVG $OUTFILE"  # -filter point -resize 400 
echo $cmd2
eval $cmd2

CMD_FILE=out/${OUTFILE%.*}.cmd
cmd=${cmd//$ID-/}
echo "# Writing $CMD_FILE ..."
echo -e ${cmd//\\/'\\\n    '} > $CMD_FILE
#display $OUTFILE_FINAL
#read -e  -i "Y" -p "Accept?" ACCEPT
#read -e  -p "Ok?" ACCEP
#if  [ "$ACCEPT" != '' ]
#    exit -1
#fi
