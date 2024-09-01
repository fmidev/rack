#!/bin/bash


# interpolation;
# interpolation_dry;  
# extrapolation_up; 
# extrapolation_down;
# clear;
	
WEIGHTS=${WEIGHTS:-'1:.8:.6:.4:.2'}

avgWindow=${AVG_WINDOW:+",avgWindow=$AVG_WINDOW"}
avgLabel=${AVG_WINDOW:+"-avg${AVG_WINDOW/:/x}"}

# For SVG images
PREFIX=${PREFIX:-'svg'}

GROUP=${GROUP:-'${NOD}'}
ORIENT=${ORIENT:-'HORZ'}
TITLE=${TITLE:-'none'}

#legend=${LEGEND:+'--legendOut .svg'}

case $LEGEND in
    left)
	legendLeft='--legendOut .svg'
	;;
    right)
	legendRight='--legendOut .svg'
	;;
    *)
	echo "Unknown legend: $LEGEND"
	exit 1
	;;
esac

echo "$LEGEND  $legendLeft $legendRight "

script="";

QUANTITIES=${QUANTITIES:-'HGHT,/CLASS-ETOP,QIND'}

for i in ${QUANTITIES//,/ }; do
    q=${i//\//}
    case $i in
	QIND)
	    script="$script --paletteIn QIND-BW -Q QIND $legendLeft -Q QIND -c -o .png --paletteIn QIND-BW $legendRight"
	;;
	*)
	    script="$script -Q $i -c --palette $q $legendLeft -o .png  $legendRight"
	;;
    esac
done



#dbzh=${CLASS:+"-Q DBZH -c --palette 'DBZH' -o -z.png"}
hght=" -Q HGHT -c --palette 'HGHT' -o .png  --legendOut .svg "
#class=${CLASS:+"-Q /CLASS-ETOP -c --palette 'CLASS-ETOP' -o png"}
class="-Q /CLASS-ETOP -c --palette 'CLASS-ETOP' -o .png --legendOut .svg"
qind="-Q QIND -c -o .png --paletteIn QIND-BW --legendOut .svg"
#script="$hght $class $qind"

INPUTS=( $* )

OUTFILE=${OUTFILE:-"out${LAYOUT:+-$LAYOUT}${avgLabel:+-$avgLabel}.svg"}
OUTFILE_BASE=${OUTFILE%.*}
OUTFILE_EXT=${OUTFILE##*.}

if [ ${#INPUTS} == 0 ]; then
    echo "Generate illustrations using EchoTop (2) "
    echo "$0 <volumes>.h5"
    echo "Example: CMETHOD=$CMETHOD WEIGHTS=$WEIGHTS  $0 201708121600_radar.polar.fi{kor,ika,van}.h5"
    exit 1
fi

#orient=${ORIENT:+",orientation=$ORIENT"}

cmd="rack --odim 2.2 --outputPrefix '$PWD/${PREFIX:+$PREFIX-}\${NOD}-\${what:quantity}' --outputConf svg:group='${GROUP},absolutePaths=true,orientation=$ORIENT,title=$TITLE' --script '--pEchoTop 20,avgWindow=${AVG_WINDOW} $script --cReset' $* --outputPrefix '' -o ${OUTFILE_BASE}.svg"
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
    pdf|png)
	cmd2="inkscape $OUTFILE_BASE.svg -o $OUTFILE"
	;;
    *)
	cmd2="convert $OUTFILE_BASE.{svg,$OUTFILE_EXT}"
	;;
esac

echo $cmd2
eval $cmd2 &>> cmd.log


# rack --odim 2.2 --outputConf svg:absolutePaths=true --outputPrefix $PWD/ 202408100400_radar.polar.fivih.h5 -Q DBZH -c --palette '' -o DBZH.png  --pEchoTop2 20,avgWindow=15000:7  -Q CLASS-ETOP -c --palette '' -o CLASS.png  -Q HGHT -c --palette '' -o HGHT.png -Q QIND -o QIND.png -o koe.svg

# rack --odim 2.2 --outputConf svg:absolutePaths=true --outputPrefix $PWD/ 202408100400_radar.polar.fivih.h5 -Q DBZH -c --palette '' -o DBZH.png  --pEchoTop2 20,avgWindow=15000:7  -Q HGHT -c --palette 'HGHT' -o HGHT.png -o out.svg && display out.svg
