#!/bin/bash


# Etop
ETOP=${ETOP:-'20'}
WEIGHTS=${WEIGHTS:-'1:.8:.6:.4:.2'}

# FINRAD
# PROJ: 3067
# BBOX: -208000,6390000,1072000,7926000
#   1280,1536 pix (1000 m/pix)
#   2560,3072 pix  (500 m/pix)
#   5120,6144 pix  (250 m/pix)
	
# interpolation;
# interpolation_dry;  
# extrapolation_up; 
# extrapolation_down;
# clear;

PROJ=${PROJ:-'3067'}
BBOX=${BBOX:-'15,57,34,65'}
SIZE=${SIZE:-'680,505'} 
# CMETHOD=${CMETHOD:-'AVERAGE:MAXIMUM:MAXW:WAVG,2,1'}
CMETHOD=${CMETHOD:-'MAXW'}




INPUTS=( $* )

if [ ${#INPUTS} == 0 ]; then
    echo "Generate composites using EchoTop (2) "
    echo "# Markus.Peura@fmi.fi"
    echo "$0 <volumes>.h5"
    echo "Example: CMETHOD=$CMETHOD WEIGHTS=$WEIGHTS  $0 201708121600_radar.polar.fi{kor,ika,van}.h5"
    exit 1
fi

OUTFILE=${OUTFILE:-"out${avg_label:+-$avg_label}.svg"} # ${LAYOUT:+-$LAYOUT}
OUTFILE_BASE=${OUTFILE%.*}
OUTFILE_EXT=${OUTFILE##*.}

# CROP=INPUT
crop=${CROP:+":bbox=$CROP"}

# NO avg support yet

#TITLE='\${what:date|%Y-%m-%d}T\${what:time|%H:%M}Z  – $CMETHOD – \${how:nodes}'
TITLE=none


case $LEGEND in
    left)
	legendLeft="--legendOut 'legend-\${what:quantity}.svg'"
	;;
    right)
	legendRight="--legendOut 'legend-\${what:quantity}.svg'"
	;;
    ''|none)
	;;
    *)
	echo "Unknown legend: $LEGEND"
	exit 1
	;;
esac


# --quantityConf HGHT:zero=MIKA --undetectWeight 0.1
# orientation=VERT
conf="--odim 2.2 --outputConf svg:absolutePaths=true,title='$TITLE'${SVGCONF:+,$SVGCONF} --outputPrefix '$PWD/'" # \${NOD}-\${what:quantity}-

#for cmethod in ${CMETHOD//:/ }; do

cmd="rack $conf --cMethod '$CMETHOD' --cSize '$SIZE' --cProj '$PROJ' --cBBox '$BBOX' --cInit --script '--pEchoTop '$ETOP,weights=$WEIGHTS' --cAdd' ${INPUTS[*]} --cExtract dw$crop  -Q HGHT --palette '' $legendLeft -o HGHT.png $legendRight  --paletteIn QIND-BW $legendLeft -Q QIND -o QIND.png  --paletteIn QIND-BW $legendRight --outputPrefix '' -o $OUTFILE_BASE.svg"
echo $cmd
eval $cmd &> cmd.log


if [ $? != 0 ]; then
   cat cmd.log
   exit 1
fi


case $OUTFILE_EXT in
    svg)
	exit $?
	;;
    xxpng)
	cmd2="inkscape $OUTFILE_BASE.svg -o $OUTFILE"
	;;
    pdf)
	cmd2="inkscape $OUTFILE_BASE.svg -o $OUTFILE"
	;;
    *)
	cmd2="convert $OUTFILE_BASE.svg $OUTFILE"
	;;
esac

echo $cmd2
eval $cmd2 &>> cmd.log
echo


