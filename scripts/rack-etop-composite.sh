#!/bin/bash

CMETHOD=${CMETHOD:-'AVERAGE:MAXIMUM:MAXW:WAVG,2,1'}

# interpolation;
# interpolation_dry;  
# extrapolation_up; 
# extrapolation_down;
# clear;
	
WEIGHTS=${WEIGHTS:-'1:.8:.6:.4:.2'}

INPUTS=( $* )

if [ ${#INPUTS} == 0 ]; then
    echo "Generate composites using EchoTop2 "
    echo "$0 <volumes>.h5"
    echo "Example: CMETHOD=$CMETHOD WEIGHTS=$WEIGHTS  $0 201708121600_radar.polar.fi{kor,ika,van}.h5"
    exit 1
fi





for cmethod in ${CMETHOD//:/ }; do

    cmd="rack --outputConf svg:absolutePaths=true,title='\${what:date|%Y-%m-%d}T\${what:time|%H:%M}Z  – $cmethod – \${how:nodes}' --outputPrefix $PWD/ --odim 2.2 --cMethod '$cmethod' --cSize 680,505 --cProj 3067 --cBBox '15,57,34,65' --cInit --script '--pEchoTop2 '10,weights=$WEIGHTS' --cAdd' ${INPUTS[*]} --cExtract dw:bbox=INPUT -Q HGHT --palette '' -o HGHT.png -Q QIND -o QIND.png -o out.svg"
    echo $cmd
    eval $cmd

    cmd2="convert out.svg etop-composite-$cmethod.png"
    echo $cmd2
    eval $cmd2

done
