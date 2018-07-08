#!/bin/bash
set -o errexit

if (( ${#*} < 2 )); then
    echo "Creates andre example: command line and output files"
    echo "Usage"
    echo "  ${0##*/} <nick> --<detector1>[=<params>] ... --<detectorN>[=<params>]"
    exit
fi



NICK=$1
VOLUME=volume-$NICK.h5
VOLUME_IMG=${VOLUME%%.*}.png
#VOLUME_IMG_PANEL=${VOLUME%%.*}-panel.png
VOLUME_IMG_PANEL=${NICK}-$SITE-panel.png

if [ ! -f $VOLUME ]; then
    echo "Volume '$VOLUME' not found"
    exit
fi

if [ -f $VOLUME_IMG_PANEL ]; then
  echo "$VOLUME_IMG_PANEL exists, ok"
else
  cmd="QUANTITY=$QUANTITY ./make-panel.sh $NICK $VOLUME $SITE"
  echo "$cmd" 
  eval "$cmd"
fi

shift 
MULTIPLE=''
if [ ${#*} != '1' ]; then
  MULTIPLE='--store intemediate=1'
fi
echo $MULTIPLE

CMD="rack $VOLUME \\\\\n  $MULTIPLE\\\\\n"
while [ ${#*} != 0 ]; do

    echo $1

    DETECTOR=( ${1/=/ } )
    VALUES=${DETECTOR[1]}
    DETECTOR=${DETECTOR[0]}   #  emitter
    aDETECTOR='a'${DETECTOR^} # aEmitter
    QUANTITY=${DETECTOR^^}    #  EMITTER

    if [ $aDETECTOR == 'aRhohvlow' ]; then
	aDETECTOR='aRhoHVLow'
    fi

    echo "$DETECTOR ($aDETECTOR)"
    # OPTIONS=`rack --help $aDETECTOR | tail -1`  
    OPTIONS=`rack --help $aDETECTOR | fgrep '#' | fgrep -v 'Parameter' | tr -d '# '`  
    if [ $? != '0' ]; then
	echo "$aDETECTOR not found"
	exit -1
    fi
    
    echo $OPTIONS
#    done
#    exit

    rack --helpExample $aDETECTOR | fgrep '#' > tmp.cmd

    # OLD
    #DEFAULT_VALUES=( `rack --helpExample $aDETECTOR | fgrep 'Default values' | cut -d ':' -f2-` )
    #VALUES=${VALUES:-$DEFAULT_VALUES}

    # NEw
    DEFAULT_VALUES=( `rack --helpExample $aDETECTOR | head -1` )
    DEFAULT_VALUES=${DEFAULT_VALUES[1]}
    VALUES=${VALUES:-$DEFAULT_VALUES}

    if [ "$MULTIPLE" == '' ]; then

    # Primary test
	#EXAMPLE=( `rack --helpExample $aDETECTOR | head -1` )
	VALUES=( ${VALUES[*]} )
	VALUES="${VALUES[*]}"
	VALUES=${VALUES// /,}
	cmd="rack $VOLUME --$aDETECTOR $VALUES -o andre-$DETECTOR.h5"
	echo "$cmd"
	eval "$cmd"
	if [ $? != '0' ]; then
	    echo "$aDETECTOR failed"
	    exit 1
	fi
	cmd="rack volume.h5  --$aDETECTOR '$VALUES'  -o andre-$DETECTOR.h5"
	echo "$cmd" > $DETECTOR.cmd
	echo >> $DETECTOR.cmd
	cat tmp.cmd >> $DETECTOR.cmd
	rack --help $aDETECTOR | fgrep '#' >> $DETECTOR.cmd

	ANOM_IMG=andre-$DETECTOR.png
	ANOM_IMG_CART=andre-$DETECTOR-cart.png
	ANOM_IMG_PANEL=andre-$DETECTOR-panel.png

	cmd="rack $VOLUME -o $VOLUME_IMG  --store intermediate=1 --$aDETECTOR $VALUES --select 'quantity=$QUANTITY' -o $ANOM_IMG --select 'quantity=$QUANTITY' --cSize 500,500 --encoding C,0.004,-0.004,0,16  -c -o $ANOM_IMG_CART"
        # quantity=$QUANTITY -o $ANOM_IMG"
	echo "$cmd"
	eval "$cmd"
	convert $ANOM_IMG -resize 500x360! tmp.png
	convert -append -frame 1 $ANOM_IMG_CART tmp.png $ANOM_IMG_PANEL
	#fi
	
	ls -l $VOLUME $VOLUME_IMG $VOLUME_IMG_PANEL $ANOM_IMG $ANOM_IMG_PANEL 

	IMG_PANEL="result-$DETECTOR.png"
	convert +append -frame 1 $VOLUME_IMG_PANEL $ANOM_IMG_PANEL   -resize 600x600 $IMG_PANEL

	ls -l $IMG_PANEL 

    else

    # Multiple...
	CMD="$CMD  --$aDETECTOR $VALUES \\\\\n" 
	


    fi

#if [ ! -f $VOLUME_IMG ]; then
#    SAVE_IMG="-o $VOLUME_IMG"
#else 
#    SAVE_IMG=''
#fi

    shift
done



if [ "$MULTIPLE" != '' ]; then

    VOLUME_DET='volume-anom-detected.h5'
    CMD="$CMD   -o $VOLUME_DET \\\\\n"
    CMD="$CMD  --aRemover 0.5 \\\\\n"
    CMD="$CMD   -o volume-anom-removed.h5 \n"

    echo -e "$CMD"    
    echo -e "$CMD" > andre-detection.cmd
    source andre-detection.cmd

    # TODO

    cmd="rack $VOLUME_DET --aRemover 0.5 -o volume-anom-remover.png"
    echo "$cmd"
    eval "$cmd"

    cmd="rack $VOLUME_DET --aGapFill 1500,5 -o volume-anom-gapfill.png"
    echo "$cmd"
    eval "$cmd"

    cmd="rack $VOLUME_DET --aDamper 0.5,0.0 -o volume-anom-damper.png"
    echo "$cmd"
    eval "$cmd"



fi

exit


cmd="rack $VOLUME \
 --store intermediate=1 \
 --aBiomet 0dBZ,1000m,5dBZ,500m \
 --aEmitter 5000m,3.0deg,0.6 \
 --aEmitter 20000m,5.0deg,0.6 \
 --aShip 20dBZ,10dBZ,1500m,3deg \
 --aSpeckle -10dBZ,5 \
  -o volume-detected.h5 \
 --aGapFill 1500,5 --aDamper 0.5,0.0 \
  -o volume-corrected.h5
"

# --aBiomet 0dBZ,1000m,5dBZ,500m \
# --aShip 20dBZ,10dBZ,1500m,3deg \
# --aSpeckle -10dBZ,5 \

