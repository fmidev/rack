#!/bin/bash

INPUTS=( $* )

ETOP=${ETOP:-'20'}
# interpolation;
# interpolation_dry;  
# extrapolation_up; 
# extrapolation_down;
# clear;
WEIGHTS=${WEIGHTS:-'1:.8:.6:.4:.2'}

# AVG_WINDOW=${AVG_WINDOW:-'5500:11'}  # Metres x degrees

smooth_output=${AVG_WINDOW:+''}

avg_label=${AVG_WINDOW:+"${AVG_WINDOW/:/x}"}
# LAYOUT=${LAYOUT:-default}

TITLE=${TITLE:-"Echo Top ${ETOP} dBZ   \${what:date|%Y-%m-%d}T\${what:time|%H:%M}Z   \${PLC} (\${NOD})"}

OUTFILE=${OUTFILE:-"out${LAYOUT:+-$LAYOUT}${avg_label:+-$avg_label}.svg"}
OUTFILE_BASE=${OUTFILE%.*}
OUTFILE_EXT=${OUTFILE##*.}
# echo ${OUTFILE_BASE} ${OUTFILE_EXT}
# exit 0
cart='-c'

#cart=${POLAR:+'-c'}
if [ "$POLAR" != '' ]; then
    cart=''
fi


echo ${#INPUTS[*]}
if [ ${#INPUTS[*]} != 1 ]; then
    echo "Draw a SVG panel of image data generated in Echo Top computation"
    echo "# Markus.Peura@fmi.fi"
    #echo "Generate composites using EchoTop2 "
    echo "$0 <volume>.h5"
    echo "Example: $0 201708121600_radar.polar.fi{kor,ika,van}.h5"
    echo "Example: AVG_WINDOW=$AVG_WINDOW WEIGHTS=$WEIGHTS  $0 201708121600_radar.polar.fi{kor,ika,van}.h5"
    echo "Example: LAYOUT=raw WEIGHTS=$WEIGHTS  $0 201708121600_radar.polar.fi{kor,ika,van}.h5"
    exit 1
fi

# conf="--odim 2.2 --outputConf svg:absolutePaths=true,title='$TITLE' --outputPrefix '$PWD/\${NOD}-\${what:quantity}-'"
conf="--odim 2.2 --outputConf svg:absolutePaths=true,title=none --outputPrefix '$PWD/\${NOD}-\${what:quantity}-'"

case $LAYOUT in
    basic)
	cmd="rack $conf  $1   --pEchoTop2 $ETOP,weights=$WEIGHTS,avgWindow=$AVG_WINDOW  -Q DBZ-SLOPE -c --palette 'DBZ-SLOPE' --legendOut .svg -o orig.png -Q QIND -o orig.png  -Q DBZ-SLOPE-SMOOTH -c --palette 'DBZ-SLOPE' -o smooth.png -Q QIND -o smooth.png  --paletteIn QIND-BW --legendOut .svg -Q HGHT -c --palette 'HGHT' -o .png --legendOut x.svg --outputPrefix '' -o $OUTFILE_BASE.svg"
	;;
    raw)
	cmd="rack $conf $1 --pEchoTop2 $ETOP,weights=$WEIGHTS  -Q DBZ-SLOPE -c --palette 'DBZ-SLOPE' -o raw.png --legendOut slope.svg  -Q QIND -o raw.png  --paletteIn QIND-BW --legendOut .svg -Q HGHT -c --palette 'HGHT' -o .png --legendOut hght.svg --outputPrefix '' -o $OUTFILE_BASE.svg"
	;;
    avg)
	AVG_WINDOW=${AVG_WINDOW:-'5500:11'}  # Metres x degrees
	cmd="rack $conf $1 --pEchoTop2 $ETOP,weights=$WEIGHTS,avgWindow=$AVG_WINDOW  -Q DBZ-SLOPE-SMOOTH -c --palette 'DBZ-SLOPE' -o smooth.png --legendOut slope.svg -Q /QIND -o smooth-q.png  --paletteIn QIND-BW --legendOut qind.svg -Q HGHT -c --palette 'HGHT' -o .png --legendOut hght.svg --outputPrefix '' -o $OUTFILE_BASE.svg"
	;;
    four|*)
	QIND=${AVG_WINDOW:+'/QIND'}
	QIND=${QIND:-'QIND'}
	SLOPE=${AVG_WINDOW:+'DBZ-SLOPE-SMOOTH'}
	SLOPE=${SLOPE:-'DBZ-SLOPE'}
	#cart='-c'
	cmd="rack $conf $1 --pEchoTop2 $ETOP,weights=$WEIGHTS,avgWindow=$AVG_WINDOW -Q /CLASS-ETOP $cart --palette 'CLASS-ETOP' -o clsetop.png --legendOut clsetop.svg -Q '$SLOPE' $cart --palette 'DBZ-SLOPE' -o smooth.png --legendOut slope.svg -Q '$QIND' -o qind${avg_label:+-$avg_label}.png  --paletteIn QIND-BW --legendOut qind.svg -Q HGHT $cart --palette 'HGHT' -o .png --legendOut hght.svg --outputPrefix '' -o $OUTFILE_BASE.svg"
	;;
esac

echo $cmd
eval $cmd &> cmd.log

if [ "$OUTFILE_EXT" != 'svg' ]; then 
    cmd2="convert $OUTFILE_BASE.{svg,$OUTFILE_EXT}"
    echo $cmd2
    eval $cmd2 &>> cmd.log
fi
