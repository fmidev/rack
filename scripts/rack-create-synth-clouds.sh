#!/bin/bash

GEOM=${GEOM:-'CART'}

WIDTH=${WIDTH:-500}
HEIGHT=${HEIGHT:-500}
SIZE=${SIZE:-$WIDTH,$HEIGHT}

OUTDIR=${OUTDIR:-"."}
OUTFILE=${OUTFILE:-"rack-synth-clouds.png"}

cmd=( ppmforge -clouds  -seed '1001' -inclination 0  -dimension '2.2' -power '1.0' -hour '12.00' -width "$WIDTH" -height "$HEIGHT" )
cmd2=( convert ppm:-  -type 'GrayScale'  "$OUTDIR/$OUTFILE" )

echo "# ${cmd[*]} | ${cmd2[*]} "
${cmd[@]} | ${cmd2[@]}

if [ $? != 0 ]; then
    echo "# Command failed"
fi
