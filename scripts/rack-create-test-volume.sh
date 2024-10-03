#!/bin/bash
#
# Generates an artificial radar volume (chessboard data).
#
# Part of Rack distribution
#
# Markus.Peura@fmi.fi

PATH=${PATH}':/fmi/dev/bin'

export DATE TIME QUANTITY OBJECT TYPE GAIN OFFSET NODATA UNDETECT NOD WMO PLC

TIMESTAMP=${TIMESTAMP:-`date --utc +'%Y%m%d%H%M%H'`}
DATE=${TIMESTAMP:0:8}
TIME=${TIMESTAMP:8:4}'00'

QUANTITY=${QUANTITY:-'DBZH'}
OBJECT=${OBJECT:-'SCAN'}
NOD=${NOD:-'xx123'}
SOURCE=${SOURCE:-"NOD:$NOD"}

#ENCODING=${ENCODING:-$TYPE,$GAIN,$OFFSET,$UNDETECT,$NODATA}
ENCODING=( ${ENCODING//,/ } )
TYPE=${TYPE:-${ENCODING[0]}}
TYPE=${TYPE:-'C'}

# echo $ENCODING ... ${ENCODING[*]}
GAIN=${GAIN:-${ENCODING[1]}}
OFFSET=${OFFSET:-${ENCODING[2]}}
UNDETECT=${UNDETECT:-${ENCODING[3]}}
NODATA=${NODATA:-${ENCODING[4]}}

ENCODING=( $TYPE $GAIN $OFFSET $UNDETECT $NODATA quantity=$QUANTITY )
ENCODING="${ENCODING[*]}"
ENCODING=${ENCODING// /,}

SIZE=${SIZE:-"500,360"}
SIZE=( ${SIZE/,/ } )
WIDTH=${WIDTH:-${SIZE[0]}}   
HEIGHT=${HEIGHT:-${SIZE[1]}} 
HEIGHT=${HEIGHT:-$WIDTH} 
if (( WIDTH * HEIGHT > 40000000 )); then
    echo "# Size check failed (invoked by: $0)"
    echo "416 Resulting array too large ($WIDTH x $HEIGHT) = $(( WIDTH * HEIGHT / 1000 )) kB"
    exit 1
fi
SIZE="$WIDTH,$HEIGHT"

VALUES=${VALUES:-$(( RANDOM & 127 )),$(( RANDOM & 127 + 128 ))}

if [ "$OUTFILE" == '' ]; then
    echo "Create radar volume files containing artificial data"
    echo "Usage:"
    echo "  VALUES=... ENCODING=... OUTPUT_FILE=...  $0 "
    echo "Examples:"
    echo "  OUTPUT_FILE=out.h5 $0 "
    echo "  VALUES=$VALUES  ENCODING=$ENCODING $0 "
    exit 0
fi


# PALETTE=default
if [ "$PALETTE" == 'default' ]; then
    PALETTE=$QUANTITY
fi
#palette=${PALETTE:+"--palette palette/palette-$PALETTE.txt"}
palette=${PALETTE:+"--palette $PALETTE"}

OUTDIR=${OUTDIR:-"."}
OUTFILE=${OUTFILE:-"${TIMESTAMP}_OBJECT=${OBJECT}_QUANTITY=${QUANTITY}.h5"}

# Base "data": a 2x2 grid replicated over the image
#
VALUES=${VALUES/:/,}
VALUES=( ${VALUES/,/ } )
DARK=${VALUES[0]} 
LIGHT=${VALUES[1]} 
# DARK=$((  RANDOM & 127 ))
# LIGHT=$(( RANDOM & 127 + 128 ))
DATA=${DATA:-"$DARK,$LIGHT,$LIGHT,$DARK"}
#DATA=${DATA:-"$DARK,$UNDETECT,$NODATA,$LIGHT"}

# Step 3: add ODIM variables, convert to HDF5
#rack ${OUTDIR}/${IMGFILE} \

BASENAME=${OUTFILE%.*}
METADATA=${BASENAME}.txt
cat > $OUTDIR/$METADATA <<EOF
what:object="$OBJECT" 
what:date="$DATE"
what:time="$TIME"
what:source="$NOD"
how:simulated="True" 
dataset1/data1/data
EOF

if [ $OBJECT == 'SCAN' ] || [ $OBJECT == 'VOL' ]; then
    TILESIZE=$(( WIDTH / 5 ))x$(( HEIGHT / 20 ))
    RSCALE=${RSCALE:-'500'}
    if [ "$SITE" != '' ]; then
	POS=`fgrep ${SITE}= share/sites.cnf | head -1 | cut -d= -f2`
	POS=( ${POS//,/ } )
	LON=${POS[0]} 
	LAT=${POS[1]} 
    fi
    LON=${LON:-'25.0'}
    LAT=${LAT:-'60.0'}
    ELANGLE=${ELANGLE:-0.5}
    
    cat >> $OUTDIR/$METADATA <<EOF
where:lon=$LON 
where:lat=$LAT 
dataset1/where:rscale=$RSCALE 
dataset1/where:elangle=$ELANGLE 
EOF
#dataset1/where:nbins=$NBINS 
#dataset1/where:nrays=$NRAYS 
else
    TILESIZE=$(( WIDTH / 10 ))x$(( HEIGHT / 10 ))
    #PROJ=${PROJ:-"+proj=aeqd +lon_0=25 +lat_0=60 +ellps=WGS84"}
    PROJ=${PROJ:-"+proj=laea +lat_0=55.0 +lon_0=10.0 +x_0=1950000.0 +y_0=-2100000.0 +units=m +ellps=WGS84"}
    # +proj=laea +lat_0=55.0 +lon_0=10.0 +x_0=1950000.0 +y_0=-2100000.0 +units=m +ellps=WGS84
    BBOX=${BBOX:-"20,60,30,70"}
    BBOX=( ${BBOX//,/ } )
    cat >> $OUTDIR/$METADATA <<EOF
where:LL_lon=${BBOX[0]}
where:LL_lat=${BBOX[1]}
where:UR_lon=${BBOX[2]}
where:UR_lat=${BBOX[3]}
where:projdef="$PROJ"
EOF
fi



IMGFILE=${BASENAME}-raw.png
#TILEFILE=${BASENAME}-tile.png
#TILESIZE=$(( WIDTH / 5 ))x$(( HEIGHT / 20 ))
TILEFILE=tile-${DATA}-${TILESIZE}.png


#
if [ "$COORD" == 'c' ]; then
    cart='-c'
else
    cart=''
fi


if [ ! -s "${OUTDIR}/${TILEFILE}" ]; then
    echo "# Step 0: Creating 'data' pattern (checkerboard), TILESIZE=$TILESIZE, file: ${OUTDIR}/${TILEFILE}"
    echo "P2 2 2 255  ${DATA//,/ }" | convert pgm:- -filter Point -resize ${TILESIZE}\! ${OUTDIR}/${TILEFILE}
    
    if [ ! -s "${OUTDIR}/${TILEFILE}" ]; then
	echo "502 Could not generate tile file: ${OUTDIR}/${TILEFILE}" 
	exit 1
    fi
fi

#if [ "$FORMAT" == 'sh' ]; then
#   OUTFILE=${BASENAME}.h5
#fi

# TODO: rack: read txt, instead of params

# Step 2: expand to a gridded png image
# convert -size ${NBINS}x${NRAYS} tile:$OUTDIR/${TILEFILE} -type GrayScale -depth 8 ${OUTDIR}/${IMGFILE}

cmd="convert -size ${WIDTH}x${HEIGHT} tile:$OUTDIR/${TILEFILE} -type GrayScale -depth 8 ${OUTDIR}/${IMGFILE}"
echo $cmd
eval $cmd

if [ $? != 0 ] || [ ! -s ${OUTDIR}/${IMGFILE} ]; then
    echo "500 Error: command failed"
    exit 1
fi


# rm -v $OUTDIR/${IMGFILE} 
# rm -v $OUTDIR/${TILEFILE}

cmd="rack $OUTDIR/$METADATA ${OUTDIR}/${IMGFILE} --encoding '$ENCODING' --completeODIM ${cart} ${palette} -o ${OUTDIR}/${OUTFILE}"
#cmd="rack  --/what:object=$OBJECT --/what:date=$DATE --/what:time=$TIME --encoding '$ENCODING' ${OUTDIR}/${IMGFILE} --completeODIM ${cart} ${palette} -o ${OUTDIR}/${OUTFILE}"
echo $cmd




if [ "$FORMAT" == 'sh' ]; then
    # That's it
    echo ${cmd%.*}.h5 >  ${OUTDIR}/${OUTFILE}
    exit 0
fi

eval $cmd

#source $OUTDIR/$METADATA

if [ $? != 0 ] || [ ! -s ${OUTDIR}/${OUTFILE} ]; then
    echo "Error:"
    cat $OUTDIR/$METADATA
    echo "<a href=\"$METADATA\">$OUTDIR/$METADATA</a>"
    echo "500 Error: command script failed"
    exit 1
fi




