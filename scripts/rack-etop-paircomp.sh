#!/bin/bash


# Requisites: 

# General: palettes 
for QUANTITY in HGHT CLASS-ETOP; do
    #LEGEND="$OUTDIR/legend-${QUANTITY}-${WEIGHTS}.svg"
    LEGEND_IMAGE="$OUTDIR/legend-${QUANTITY}.svg"
    if [ ! -f $LEGEND_IMAGE ]; then
	cmd="rack --palette ${QUANTITY} --paletteOut $LEGEND_IMAGE"
	echo $cmd
	eval $cmd
    fi
done

# Determine shared BBOXes

# Construct conf name...
BBOX_CONF=''
for i in $*; do
    BBOX_CONF=${BBOX_CONF:+"${BBOX_CONF}-"}
    INFILE=${i%.*}
    INFILE=${INFILE//[0-9._]/}
    BBOX_CONF+=$INFILE
done

# .. and store bounding boxes to pairwise confs.
for i in OVERLAP DATA; do
    BASENAME="conf/${BBOX_CONF}-$i"
    #BBOX_CONF="$BASENAME.json"
    echo -n "$BASENAME[.json]... "
    if [ -f "$BASENAME.json" ]; then
	echo "exists, ok"
    else
	echo "does not exist, creating:"
	PROJ="3067"
	BBOX="-208000,6390000,1072000,7926000"
	SIZE="1280,1536"
	cmd="rack --cProj '$PROJ' --cBBox '$BBOX' --cSize '$SIZE' --cInit --script '-Q DBZH --cAdd' $* --cExtract channels=DATA:bbox=$i  --formatFile conf/conf-template-$i.json -o $BASENAME.json  -o $BASENAME.txt --palette '' -o $BASENAME.png"
	echo $cmd
	eval $cmd
    fi
done



#exit 0

# Consider :
ETOP=${ETOP:-'10'}
Z=${Z:-'-40'}
H=${H:-'18000'}

# Use 0:0:... if only reference point should be used.
# WEIGHTS=${WEIGHTS:-'0:0:.6:.4:.2'}
# WEIGHTS=${WEIGHTS:-'1:0.8:.6:.4:.2'}
weights=${WEIGHTS:+",weights=${WEIGHTS}"}
weightLabel=${WEIGHTS:+"-w${WEIGHTS/:/-}"}

# Hyviä lukemia
# -40 dBZ 30km
# -40 dBZ 60km
# -50 dBZ 70km ?
# -50 dBZ 30km
# WEIGHTS=${WEIGHTS:-'1.0:0.8:0.6:0.4:0.2'}

avgWindow=${AVG_WINDOW:+",avgWindow=$AVG_WINDOW"}
avgLabel=${AVG_WINDOW:+"-avg${AVG_WINDOW/:/x}"}

OUTDIR="./test-etop${ETOP}_ref${Z}dBZ"
mkdir --parents $OUTDIR/sub/
OUTFILE=panel${weightLabel}-ref${Z}dBZ-${H}m${avgLabel}.png


# geoconf="--cMethod LATEST  --cSize 320,493  --cProj 3067 --cBBox '20.7902,57.8923,26.0793,62.4792'"

geoconf="--execFile conf/${BBOX_CONF}-DATA.json"



etop="--pEchoTop2 ${ETOP},reference=${Z}:${H}${weights}${avgWindow}"
etop_basic="--pEchoTop2 ${ETOP}"

# Create class images (dependent on ETOP_THR and WEIGHTS, not on referemce(Z,H)
for INFILE in $*; do
    #echo $NOD
    CLASS_IMAGE=${INFILE##*/}
    CLASS_IMAGE=${CLASS_IMAGE%.*}-${ETOP}-${WEIGHTS}_CLASS-ETOP.png
    # CLASS_IMAGE="$OUTDIR/${CLASS_IMAGE}-${ETOP}-${WEIGHTS}_CLASS-ETOP.png"
    echo $OUTDIR/$CLASS_IMAGE
    if [ ! -f $OUTDIR/$CLASS_IMAGE ]; then
	# $geoconf
	cmd="rack $INFILE --cProj 3067 $etop_basic -Q CLASS-ETOP -c --palette CLASS-ETOP -o $OUTDIR/$CLASS_IMAGE"
	echo $cmd
	eval $cmd
    fi
done

# exit 0
# 

#legend1='--paletteOut legend-HGHT.svg'

# legend2='--paletteOut legend-CLASS-ETOP.svg'
# class_etop="-Q CLASS-ETOP -c --palette CLASS-ETOP -o echoTop-CLASS-ETOP-\${NOD}.png  $legend2"

# rack --odim 2.2 --verbose NOTE \
#     --outputConf svg:absolutePaths=true,title="EchoTop($ETOP) ref:${Z}dBZ-${H}m \${what:date|%Y-%m-%d} \${what:time|%H:%M} UTC" \
#     --outputPrefix $PWD/  \
#     --cMethod LATEST  $geoconf \
#     --script "$etop -Q HGHT -c --palette HGHT $legend1  -o echoTop-HGHT-\${NOD}.png $class_etop --outputConf svg:group=\${NOD}" \
#     $* \
#     -o panel.svg && convert panel.svg $OUTDIR/$OUTFILE  # && display panel.svg

# title='EchoTop($ETOP) ref:${Z}dBZ-${H}m \${what:date|%Y-%m-%d} \${what:time|%H:%M} UTC'

#      --cMethod LATEST  $geoconf \
    
geoconf="--execFile conf/${BBOX_CONF}-OVERLAP.json"

cmd="rack --odim 2.2 --verbose WARNING \
     --outputConf svg:absolutePaths=true,title='$ETOP $weightLabel (${Z}dBZ-${H}m) $avgLabel',group='\${NOD}'  \
     --outputPrefix $PWD/$OUTDIR/ \
     --cMethod AVERAGE  $geoconf \
     --script '$etop -Q HGHT -c --palette HGHT $legend1 -o sub/echoTop-HGHT-\${NOD}.png $class_etop ' \
     $* \
     -o panel.svg"
#  && display panel.svg"

echo $cmd
eval $cmd
convert $OUTDIR/panel.svg $OUTDIR/${OUTFILE%.*}-koe.png



echo $OUTDIR/$OUTFILE

# PAIRWISE (deviation)
#geoconf="--execFile conf/${BBOX_CONF}-OVERLAP.json"

cmd="rack --odim 2.2 --verbose NOTE \
     --outputConf svg:absolutePaths=true,title='ETOP ${ETOP}dBZ $weightLabel (${Z}dBZ:${H}m) $avgLabel' \
     --outputPrefix $PWD/$OUTDIR/ \
     --cMethod LATEST  $geoconf \
     --script '$etop --cAdd' \
     $* \
     --cExtract DATA --palette '' -o echoTop-$ETOP-${Z}:${H}.png --encoding S --cExtract DEVIATION -Q HGHTDEV -o echoTop-$ETOP-${Z}:${H}-DEV.png \
      -o panel.svg  \
     " 

echo $cmd
eval $cmd
convert $OUTDIR/panel.svg $OUTDIR/${OUTFILE%.*}-pair.png




