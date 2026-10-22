#!/bin/bash

# Markus Peura fmi 2012-2026

WIDTH=${WIDTH:-500}
HEIGHT=${HEIGHT:-500}
SIZE=${SIZE:-$WIDTH,$HEIGHT}
SIZE2=( ${SIZE//,/ } )
WIDTH=${WIDTH:-$SIZE2[0]}
HEIGHT=${HEIGHT:-$SIZE2[1]}


DIMENSION=${DIMENSION:-'3.3'}
POWER=${POWER:-'1'}
SEED=${SEED:-'1'}

OUTDIR=${OUTDIR:-"."}
OUTFILE=${OUTFILE:-"rack-synth-clouds_DIMENSION=${DIMENSION}_POWER=${POWER}_SEED=${SEED}_SIZE=${SIZE}.png"}

# Appears in filename only if explicitly set
GEOM=${GEOM:-'CART'}

OUTFILE_BASE=${OUTFILE%.*}
OUTFILE_EXT=${OUTFILE##*.}

# echo "# Created $OUTFILE_BASE $OUTFILE_EXT" 


OUTFILE_PNG=${OUTFILE_BASE}'.png'
echo "# Creating Cartesian fractal image:  $OUTDIR/$OUTFILE_PNG"
# -inclination 0 -hour '12.00' 
cmd=( ppmforge -clouds   -seed "$SEED" -dimension "$DIMENSION" -power "$POWER" -width "$WIDTH" -height "$HEIGHT" )
cmd2=( convert ppm:-  -type 'GrayScale'  "$OUTDIR/$OUTFILE_PNG" )

echo "# ${cmd[*]} | ${cmd2[*]} "
${cmd[@]} | ${cmd2[@]}

if [ $? != 0 ]; then
    #    echo "# Created $OUTDIR/$OUTFILE" 
    #else
    echo "# Command failed"
    exit 1
fi


# TODO: full make chain...

OUTFILE_TXT=${OUTFILE_BASE}'.txt'

ENCODING='C,0.5,-32'

if [ $OUTFILE_EXT == 'h5' ]; then


    OBJECT="COMP"
    #    TILESIZE=$(( WIDTH / 10 ))x$(( HEIGHT / 10 ))
    PROJ=${PROJ:-"+proj=laea +lat_0=55.0 +lon_0=10.0 +x_0=1950000.0 +y_0=-2100000.0 +units=m +ellps=WGS84"}
    # TODO epsg
    BBOX=${BBOX:-"20,60,30,70"}
    BBOX=( ${BBOX//,/ } )
    
    cat > $OUTDIR/$OUTFILE_TXT <<EOF
what:object="$OBJECT" 
what:date="$DATE"
what:time="$TIME"
what:source="$NOD"
how:simulated="True" 
dataset1/data1/data
where:LL_lon=${BBOX[0]}
where:LL_lat=${BBOX[1]}
where:UR_lon=${BBOX[2]}
where:UR_lat=${BBOX[3]}
where:projdef="$PROJ"
EOF

    # rack composite.h5 --cPolar rscale=540,lon=21.64,lat=60.12 -o polar.h5 -o koe.png && xv koe.png
    echo "# Step 3: Creating HDF5 structure from metadata text and data array"
    #echo "#   file: $OUTFILE"
    cmd="rack $OUTDIR/$OUTFILE_TXT ${OUTDIR}/${OUTFILE_PNG} --encoding '$ENCODING' --completeODIM -o ${OUTDIR}/${OUTFILE}"
    echo "# $cmd"
    eval $cmd
    
fi
