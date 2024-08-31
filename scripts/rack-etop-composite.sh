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
    echo "Generate composites using EchoTop2 "
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
conf="--odim 2.2 --outputConf svg:absolutePaths=true,title='$TITLE' --outputPrefix '$PWD/'" # \${NOD}-\${what:quantity}-

#for cmethod in ${CMETHOD//:/ }; do

cmd="rack $conf --cMethod '$CMETHOD' --cSize '$SIZE' --cProj '$PROJ' --cBBox '$BBOX' --cInit --script '--pEchoTop2 '$ETOP,weights=$WEIGHTS' --cAdd' ${INPUTS[*]} --cExtract dw$crop  -Q HGHT --palette '' -o HGHT.png -Q QIND -o QIND.png --outputPrefix '' -o $OUTFILE_BASE.svg"
echo $cmd
eval $cmd &> cmd.log

if [ "$OUTFILE_EXT" != 'svg' ]; then 
    cmd2="convert -verbose $OUTFILE_BASE.{svg,$OUTFILE_EXT}"
    echo >> cmd.log
    echo $cmd2
    eval $cmd2 &>> cmd.log
    echo $OUTFILE_BASE.{svg,$OUTFILE_EXT}
fi

#done
