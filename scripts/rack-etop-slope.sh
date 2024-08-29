#!/bin/bash


ETOP=${ETOP:-'20'}
# interpolation;
# interpolation_dry;  
# extrapolation_up; 
# extrapolation_down;
# clear;
WEIGHTS=${WEIGHTS:-'1:.8:.6:.4:.2'}

smooth_output=${AVG_WINDOW:+''}


AVG_WINDOW=${AVG_WINDOW:-'5500:11'}  # Metres x degrees
LAYOUT=${LAYOUT:-default}

INPUTS=( $* )

TITLE=${TITLE:-"Echo Top ${ETOP} dBZ   \${what:date|%Y-%m-%d}T\${what:time|%H:%M}Z   \${PLC} (\${NOD})"}

echo ${#INPUTS[*]}
if [ ${#INPUTS[*]} != 1 ]; then
    echo "Generate composites using EchoTop2 "
    echo "$0 <volume>.h5"
    echo "Example: AVG_WINDOW=$AVG_WINDOW WEIGHTS=$WEIGHTS  $0 201708121600_radar.polar.fi{kor,ika,van}.h5"
    exit 1
fi

conf="--odim 2.2 --outputConf svg:absolutePaths=true,title='$TITLE' --outputPrefix '$PWD/\${NOD}-\${what:quantity}-'"

case $LAYOUT in
    default)
	cmd="rack $conf  $1   --pEchoTop2 $ETOP,weights=$WEIGHTS,avgWindow=$AVG_WINDOW  -Q DBZ-SLOPE -c --palette 'DBZ-SLOPE' --legendOut .svg -o orig.png -Q QIND -o orig.png  -Q DBZ-SLOPE-SMOOTH -c --palette 'DBZ-SLOPE' -o smooth.png -Q QIND -o smooth.png  --paletteIn QIND-BW --legendOut .svg -Q HGHT -c --palette 'HGHT' -o .png --legendOut x.svg --outputPrefix '' -o out.svg"
	;;
    raw)
	cmd="rack $conf $1 --pEchoTop2 $ETOP,weights=$WEIGHTS  -Q DBZ-SLOPE -c --palette 'DBZ-SLOPE' -o raw.png --legendOut slope.svg  -Q QIND -o raw.png  --paletteIn QIND-BW --legendOut .svg -Q HGHT -c --palette 'HGHT' -o .png --legendOut hght.svg --outputPrefix '' -o out.svg"
	;;
    avg)
	cmd="rack $conf $1 --pEchoTop2 $ETOP,weights=$WEIGHTS,avgWindow=$AVG_WINDOW  -Q DBZ-SLOPE-SMOOTH -c --palette 'DBZ-SLOPE' -o smooth.png --legendOut slope.svg -Q /QIND -o smooth-q.png  --paletteIn QIND-BW --legendOut qind.svg -Q HGHT -c --palette 'HGHT' -o .png --legendOut hght.svg --outputPrefix '' -o out.svg"
	;;
    *)
esac

echo $cmd
eval $cmd &> cmd.log

cmd2="convert out.svg out.png"
echo $cmd2
eval $cmd2 
