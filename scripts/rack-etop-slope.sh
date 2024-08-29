#!/bin/bash


# interpolation;
# interpolation_dry;  
# extrapolation_up; 
# extrapolation_down;
# clear;
	
WEIGHTS=${WEIGHTS:-'1:.8:.6:.4:.2'}
AVG_WINDOW=${AVG_WINDOW:-'5500:11'}  # Metres x degrees

INPUTS=( $* )

echo ${#INPUTS[*]}
if [ ${#INPUTS[*]} != 1 ]; then
    echo "Generate composites using EchoTop2 "
    echo "$0 <volume>.h5"
    echo "Example: AVG_WINDOW=$AVG_WINDOW WEIGHTS=$WEIGHTS  $0 201708121600_radar.polar.fi{kor,ika,van}.h5"
    exit 1
fi


cmd="rack --odim 2.2 --outputConf svg:absolutePaths=true,title='\${what:date|%Y-%m-%d}T\${what:time|%H:%M}Z \${NOD} - \${PLC}' --outputPrefix '$PWD/\${NOD}-\${what:quantity}-' $1   --pEchoTop2 20,weights=$WEIGHTS,avgWindow=$AVG_WINDOW  -Q DBZ-SLOPE -c --palette 'DBZ-SLOPE' --legendOut .svg -o orig.png -Q QIND -o orig.png  -Q DBZ-SLOPE-SMOOTH -c --palette 'DBZ-SLOPE' -o smooth.png -Q QIND -o smooth.png  --outputPrefix '' -o out.svg"
echo $cmd
eval $cmd

cmd2="convert out.svg out.png"
echo $cmd2
eval $cmd2
