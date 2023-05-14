#!/bin/bash

if [ $# == 0 ]; then
    echo "Script for generating a pseudo-RHI with Rack (range-height indicator)"
    echo "Additionally, uses gnuplot to plot the data."
    echo "Usage:"
    echo " ${0##*/} <volume.h5>"
    exit 
fi

rack pvol_filuo_20110628T1600*.h5  --pPseudoRhi 300,400,200,10,250,0,10000 -O pseudoRhi.png --palette palette/palette-DBZH.txt --select quantity=QIND --imageAlpha '0.0039,0' --imageFlatten 0 -o pseudo-rhi-raw.png  
#--image C -o pseudo-rhi-beams.png

# --image C,-0.01,1

#composite -compose Over pseudo-rhi-raw.png white.png  koe.png
#composite -compose Over pseudo-rhi-raw.png pseudo-rhi-beams.png  pseudo-rhi-over.png

#composite -compose Multiply pseudo-rhi-raw.png pseudo-rhi-beams.png  pseudo-rhi-multiply.png

#rack pvol_filuo_20110628T1600*.h5  --pPseudoRhi 300,400,200,10,250,0,10000 --palette palette/palette-DBZH.txt --select quantity=QIND --imageAlpha '0.0015,0'  -o pseudo-rhi-raw.png --image C,-0.005,-255 -o beams.png; display beams.png

WIDTH=${WIDTH:-'640'}
HEIGHT=${HEIGHT:-'400'}
export GNUPLOT_TERMINAL="png truecolor enhanced size $WIDTH,$HEIGHT"
gnuplot make-pseudo-rhi.gnp; 
#display world_population.png 


