#!/bin/bash

# Markus.Peura@fmi.fi

INPUT=${INPUT:-$1}
INPUT=${INPUT:-'volume.h5'}

#DWINDOW='2500,95,VVP=1'
DWINDOW='7500,135,VVP=1'


# Intermediate 
VVP_DATA='vvp.dat'

VVP_IMG='vvp.png'
#VVP_IMG='vvp.eps'
BASENAME=${VVP_IMG%.*}
FORMAT=${VVP_IMG##*.}
#HEADSIZE=${HEADSIZE:-'5,10,40'}
headsize=${HEADSIZE:+"head size $HEADSIZE"}
l
cmd="rack $INPUT --pDopplerDeAlias '$DWINDOW'  -Q 'VVP|HGHT' --sample '12,1,jEnd=1,skipVoid=1'  --format '0 {HGHT} {VVPU} {VVPV} {VVPQ}'  -o $VVP_DATA"

echo $cmd
eval $cmd

HGHT=${HGHT:-6000} # meters
YSCALE=15

#http://www.gnuplotting.org/tag/grid/

gnuplot <<EOF

set terminal png truecolor enhanced size 300,900
#set terminal $FORMAT enhanced size 200,600
set output '$VVP_IMG'

#set palette defined (  0 'red', 1 'blue', 2 'green' )
set palette defined (  0 'white', 1 'black' )

set xrange [-20:+20]
set yrange [0:$HGHT]

#unset tics
unset xtics

unset colorbox
#set lmargin at screen 0.0
#set rmargin at screen 1.0
#set bmargin at screen 0.0
#set tmargin at screen 1.0


#unset border

plot '$VVP_DATA' u (\$1):(1000*\$2):(\$3):($YSCALE*\$4):(\$5)   with vectors $headsize filled lc palette title ''

#plot '$VVP_DATA' u (\$1-\$3/2.0):(1000*\$2 - $YSCALE*\$4/2.0):(\$3):($YSCALE*\$4):(\$5)   with vectors  filled lc palette title ''
EOF

# End
