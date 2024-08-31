#!/bin/bash


dbzh=${CLASS:+"-Q DBZH -c --palette 'DBZH' -o -z.png"}
hght=" -Q HGHT -c --palette 'HGHT' -o png  --legendOut -h.svg "
#class=${CLASS:+"-Q /CLASS-ETOP -c --palette 'CLASS-ETOP' -o png"}
class="-Q /CLASS-ETOP -c --palette 'CLASS-ETOP' -o png --legendOut -c.svg"
qind="-Q QIND -c -o -q.png --paletteIn QIND-BW --legendOut -q.svg"

# interpolation;
# interpolation_dry;  
# extrapolation_up; 
# extrapolation_down;
# clear;
	
WEIGHTS=${WEIGHTS:-'1:.8:.6:.4:.2'}

INPUTS=( $* )

OUTFILE=${OUTFILE:-"out${LAYOUT:+-$LAYOUT}${avgLabel:+-$avgLabel}.svg"}
OUTFILE_BASE=${OUTFILE%.*}
OUTFILE_EXT=${OUTFILE##*.}

if [ ${#INPUTS} == 0 ]; then
    echo "Generate composites using EchoTop2 "
    echo "$0 <volumes>.h5"
    echo "Example: CMETHOD=$CMETHOD WEIGHTS=$WEIGHTS  $0 201708121600_radar.polar.fi{kor,ika,van}.h5"
    exit 1
fi


cmd="rack --odim 2.2 --outputPrefix '$PWD/\${NOD}-\${what:quantity}.' --outputConf svg:group='\${NOD},absolutePaths=true' --script '--pEchoTop2 20,avgWindow=15000:7 $hght $class $qind ' $* --outputPrefix '' -o ${OUTFILE_BASE}.svg"
echo $cmd
eval $cmd &> cmd.log

if [ "$OUTFILE_EXT" != 'svg' ]; then 
    cmd2="convert $OUTFILE_BASE.{svg,$OUTFILE_EXT}"
    echo $cmd2
    eval $cmd2 &>> cmd.log
fi



# rack --odim 2.2 --outputConf svg:absolutePaths=true --outputPrefix $PWD/ 202408100400_radar.polar.fivih.h5 -Q DBZH -c --palette '' -o DBZH.png  --pEchoTop2 20,avgWindow=15000:7  -Q CLASS-ETOP -c --palette '' -o CLASS.png  -Q HGHT -c --palette '' -o HGHT.png -Q QIND -o QIND.png -o koe.svg

# rack --odim 2.2 --outputConf svg:absolutePaths=true --outputPrefix $PWD/ 202408100400_radar.polar.fivih.h5 -Q DBZH -c --palette '' -o DBZH.png  --pEchoTop2 20,avgWindow=15000:7  -Q HGHT -c --palette 'HGHT' -o HGHT.png -o out.svg && display out.svg
