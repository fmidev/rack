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
conf="--odim 2.2 --outputConf svg:absolutePaths=true,title=none --outputPrefix '$PWD/\${NOD}-\${what:quantity}'"


case $LEGEND in
    left)
	legendLeft='--legendOut .svg'
	;;
    right)
	legendRight='--legendOut .svg'
	;;
    ''|none)
	;;
    *)
	echo "Unknown legend: $LEGEND"
	exit 1
	;;
esac


QIND=${AVG_WINDOW:+'/QIND'}
QIND=${QIND:-'QIND'}
SLOPE=${AVG_WINDOW:+'DBZ-SLOPE-SMOOTH'}
SLOPE=${SLOPE:-'DBZ-SLOPE'}

script_basic=""
script=""
#script="-Q CLASS-ETOP $cart --palette 'CLASS-ETOP' -o clsetop.png --legendOut clsetop.svg -Q '$SLOPE' $cart --palette 'DBZ-SLOPE' -o smooth.png --legendOut slope.svg -Q '$QIND' $cart -o qind${avg_label:+-$avg_label}.png  --paletteIn QIND-BW --legendOut qind.svg -Q HGHT $cart --palette 'HGHT' -o .png --legendOut hght.svg "

#script="--pEchoTop $ETOP,avgWindow=${AVG_WINDOW}";
script="--pEchoTop $ETOP,weights=$WEIGHTS,avgWindow=$AVG_WINDOW"

QUANTITIES=${QUANTITIES:-'CLASS-ETOP,DBZ-SLOPE,QIND,HGHT'}

for i in ${QUANTITIES//,/ }; do
    q=${i//\//}
    case $i in
	QIND)
	    script="$script -Q $i $cart --paletteIn QIND-BW  $legendLeft -o .png  $legendRight"
	    # script="$script --paletteIn QIND-BW -Q QIND $legendLeft -Q QIND -c -o .png --paletteIn QIND-BW $legendRight"
	    # -Q '$QIND'    $cart -o qind${avg_label:+-$avg_label}.png  --paletteIn QIND-BW --legendOut .svg
	    ;;
	DBZ-SLOPE)
	    script="$script -Q $SLOPE $cart --palette DBZ-SLOPE $legendLeft -o .png  $legendRight"
	    # 	    -Q '$SLOPE'   $cart --palette 'DBZ-SLOPE'  -o .png --legendOut .svg
	    #script="$script --paletteIn DBZ-SLOPE -Q $SLOPE $legendLeft -Q DBZ-SLOPE -c -o .png --paletteIn QIND-BW $legendRight"
	    ;;
	DBZH|VRAD)
	    script_basic="-Q $i $cart --palette $q $legendLeft -o .png  $legendRight"
	    ;;
	*)
	    script="$script -Q $i $cart --palette $q $legendLeft -o .png  $legendRight"
	    #-Q CLASS-ETOP $cart --palette 'CLASS-ETOP' -o .png --legendOut .svg
	    #-Q HGHT       $cart --palette 'HGHT'       -o .png --legendOut .svg 
	    ;;
    esac
done

#script="-Q CLASS-ETOP $cart --palette 'CLASS-ETOP' -o .png --legendOut .svg -Q '$SLOPE' $cart --palette 'DBZ-SLOPE' -o .png --legendOut .svg -Q '$QIND' $cart -o qind${avg_label:+-$avg_label}.png  --paletteIn QIND-BW --legendOut .svg -Q HGHT $cart --palette 'HGHT' -o .png --legendOut .svg "



case $LAYOUT in
    basic)
	cmd="rack $conf  $1   --pEchoTop2 $ETOP,weights=$WEIGHTS,avgWindow=$AVG_WINDOW  -Q DBZ-SLOPE -c --palette 'DBZ-SLOPE' --legendOut .svg -o orig.png -Q QIND -o orig.png  -Q DBZ-SLOPE-SMOOTH -c --palette 'DBZ-SLOPE' -o smooth.png -Q QIND -o smooth.png  --paletteIn QIND-BW --legendOut .svg -Q HGHT -c --palette 'HGHT' -o .png --legendOut x.svg --outputPrefix '' -o $OUTFILE_BASE.svg"
	;;
    raw)
	cmd="rack $conf $1 --pEchoTop $ETOP,weights=$WEIGHTS  -Q DBZ-SLOPE -c --palette 'DBZ-SLOPE' -o raw.png --legendOut slope.svg  -Q QIND -o raw.png  --paletteIn QIND-BW --legendOut .svg -Q HGHT -c --palette 'HGHT' -o .png --legendOut hght.svg --outputPrefix '' -o $OUTFILE_BASE.svg"
	;;
    avg)
	AVG_WINDOW=${AVG_WINDOW:-'5500:11'}  # Metres x degrees
	cmd="rack $conf $1 --pEchoTop $ETOP,weights=$WEIGHTS,avgWindow=$AVG_WINDOW  -Q DBZ-SLOPE-SMOOTH -c --palette 'DBZ-SLOPE' -o smooth.png --legendOut slope.svg -Q /QIND -o smooth-q.png  --paletteIn QIND-BW --legendOut qind.svg -Q HGHT -c --palette 'HGHT' -o .png --legendOut hght.svg --outputPrefix '' -o $OUTFILE_BASE.svg"
	;;
    four|*)
	#cart='-c'
	# Ylim. cart ennen Q-out
	#cmd="rack $conf $1 --pEchoTop $ETOP,weights=$WEIGHTS,avgWindow=$AVG_WINDOW -Q CLASS-ETOP $cart --palette 'CLASS-ETOP' -o clsetop.png --legendOut clsetop.svg -Q '$SLOPE' $cart --palette 'DBZ-SLOPE' -o smooth.png --legendOut slope.svg -Q '$QIND' $cart -o qind${avg_label:+-$avg_label}.png  --paletteIn QIND-BW --legendOut qind.svg -Q HGHT $cart --palette 'HGHT' -o .png --legendOut hght.svg --outputPrefix '' -o $OUTFILE_BASE.svg"
	#cmd="rack $conf $1 --pEchoTop $ETOP,weights=$WEIGHTS,avgWindow=$AVG_WINDOW $script --outputPrefix '' -o $OUTFILE_BASE.svg"
	cmd="rack $conf $1 $script_basic $script --outputPrefix '' -o $OUTFILE_BASE.svg"
	;;
esac

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

echo "LAYOUT=$LAYOUT"


