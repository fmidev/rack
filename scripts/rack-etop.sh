#!/bin/bash


dbzh=${CLASS:+"-Q DBZH -c --palette '' -o png"}
hght=" -Q HGHT -c --palette 'HGHT' --legendOut -koe.svg -o png "
#class=${CLASS:+"-Q /CLASS-ETOP -c --palette 'CLASS-ETOP' -o png"}
class="-Q /CLASS-ETOP -c --palette 'CLASS-ETOP' -o png --legendOut svg"
qind="-Q QIND -c -o png"

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


cmd="rack --odim 2.2 --outputConf svg:absolutePaths=true --outputPrefix '$PWD/\${NOD}-\${what:quantity}.' $1   --pEchoTop2 20,avgWindow=15000:7 $hght $class $qind --outputPrefix '' -o out.svg"
echo $cmd
eval $cmd


# rack --odim 2.2 --outputConf svg:absolutePaths=true --outputPrefix $PWD/ 202408100400_radar.polar.fivih.h5 -Q DBZH -c --palette '' -o DBZH.png  --pEchoTop2 20,avgWindow=15000:7  -Q CLASS-ETOP -c --palette '' -o CLASS.png  -Q HGHT -c --palette '' -o HGHT.png -Q QIND -o QIND.png -o koe.svg

# rack --odim 2.2 --outputConf svg:absolutePaths=true --outputPrefix $PWD/ 202408100400_radar.polar.fivih.h5 -Q DBZH -c --palette '' -o DBZH.png  --pEchoTop2 20,avgWindow=15000:7  -Q HGHT -c --palette 'HGHT' -o HGHT.png -o out.svg && display out.svg
