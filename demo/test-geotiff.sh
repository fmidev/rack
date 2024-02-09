#!/bin/bash

# Functions START, TITLE, TEST, REQUIRE, EXCLUDE
# source content-utils.sh

function TEST_TIFF(){

    local projdef=$1
    local outfile=$2
    local epsg=$3
    local filebase=${outfile%.*}
    local file_gdal=${filebase}.gdal
    local cmd

    echo "# RACK"
    cmd="rack $INFILE --cProj '$projdef' -c --outputConf tif:compliancy=EPSG:STRICT -o $outfile"
    echo $cmd
    echo $cmd > ${filebase}.cmd
    eval $cmd
    if [ $? == 0 ]; then
	echo $projdef >> rack-geotiff-supported.inc
    else
	echo "# LibGTIF fails with (plain) Proj.4 definition string:" >> rack-geotiff-supported.inc
	# (corresponding to EPSG:$epsg)
	echo "# $projdef" >> rack-geotiff-supported.inc
	echo $projdef >> rack-geotiff-unsupported.inc
	# echo "ERROR"
	# exit 1
	# continue
	return
    fi
    echo 
    
    echo "# GDALINFO"
    cmd="gdalinfo $outfile > $filebase.tmp"
    echo $cmd
    eval $cmd
    if [ $? != 0 ]; then
	echo "GDAL error with projdef='$projdef' ($outfile)"
    fi

    # Test gain, offset - should be always with radar data

    for RE in '^\s\+NoData\sValue=[0-9]\+\s*$' '^\s\+Offset:.*\sScale:.*\s*$' ; do
	grep "$RE" $filebase.tmp
	if [ $? != 0 ]; then
	    echo "Metadata test failed for regExp $RE ($filebase.tmp)"
	    exit 1
	fi
    done

    
    cat $filebase.tmp | fgrep -v 'Files:' > $file_gdal


    
    
    if [ -v LAST_INFFILE ]; then
	echo
	echo "# DIFF check for gdalinfo"
	cmd="diff $LAST_INFFILE $file_gdal"
	echo $cmd
	eval $cmd 
	if [ $? != 0 ]; then
	    echo "# Note: mapping EPSG:$epsg to PROJ.4 string not reversible " >> rack-geotiff-unsupported.inc
	    #echo "# Note: diff " >> rack-geotiff-supported.inc
	    echo
	    # echo
	    # exit 1
	fi
    fi
    
    LAST_INFFILE=$file_gdal

    if [ "$epsg" != '' ]; then

	# WKT_VERSION='WKT2:2015'
	WKT_VERSION='WKT2:2019'
	# WKT_VERSION='WKT1:GDAL'
	echo "# WKT metadata test ($WKT_VERSION) for EPSG:$epsg"

	#WKT_FILE=$epsg.wkt
	WKT_FILE=$epsg.${WKT_VERSION/:/-}
	if [ ! -f $WKT_FILE ]; then
	    projinfo "epsg:$epsg" -o $WKT_VERSION | tail +2 > $WKT_FILE
	fi
	grep '\(\[\|\]\)' test-init-$epsg.gdal > test-init-$epsg.wkt

	diff --text  $WKT_FILE test-init-$epsg.wkt
	if [ $? != 0 ]; then
	    #echo "# Note: $WKT_VERSION test failed for:  $EPSG $PROJDEF"
	    echo "NOTE: diff2 failed..."
	    echo "# Note: only partially conformant with $WKT_VERSION" >> rack-geotiff-supported.inc
	    #exit 1
	fi

	echo
    fi
    
}



INFILE='volume.h5'
EPSGFILE='/opt/venison/products/maps/wms/epsg'
ARGS=( $* )
ARGS=${ARGS:-4326  2393 3067 5125   3844 3035 3995 }


rm -vf rack-geotiff-supported.inc rack-geotiff-unsupported.inc



OUTFILES=( )
for EPSG in ${ARGS[*]}; do

    # PROJDEF=`cat $EPSGFILE | fgrep "<$EPSG>" | tr -d '<' | cut -d'>' -f2`
    PROJDEF=`projinfo "epsg:$EPSG" -o PROJ | tail -1`
    
    echo $EPSG $PROJDEF

    
    unset LAST_INFFILE
    
    TEST_TIFF $EPSG              rack-$EPSG.tif

    TEST_TIFF "+init=epsg:$EPSG" rack-init-$EPSG.tif      $EPSG

    TEST_TIFF "$PROJDEF"         rack-projdef-$EPSG.tif   $EPSG

    echo "# LISTGEO check"
    gdal_translate -co TILED=YES -co BLOCKXSIZE=512 -co BLOCKYSIZE=512 -co COMPRESS=LZW rack-$EPSG.tif  rack-gdal-$EPSG.tif
    listgeo rack-$EPSG.tif      > orig-$EPSG.lgeo
    listgeo rack-gdal-$EPSG.tif > gdal-$EPSG.lgeo
    diff {orig,gdal}-$EPSG.lgeo > diff-$EPSG.lgeo
    
    echo >> rack-geotiff-supported.inc
    echo >> rack-geotiff-unsupported.inc
    
done


echo

#if [ ${#ARGS[*]} == 0 ]; then
if [ $# == 0 ]; then

    echo "# Generate example image with tiling failure"

    # Not multiples of 128, 256, 512...
    SIZE=300,500
    DEMO=1 ENCODING=C            OUTFILE=composite-fi-C.h5 SIZE=$SIZE  BBOX=17,57.7,33,69.5  ./make-composite.sh  data-20140525/201405251200_radar.polar.fi*.h5
    #convert composite-fi-C.tif composite-fi-C.png

    DEMO=1 ENCODING=S            OUTFILE=composite-fi-S.h5 SIZE=$SIZE  BBOX=17,57.7,33,69.5  ./make-composite.sh  data-20140525/201405251200_radar.polar.fi*.h5
    # DEMO=1 ENCODING=S,0.002,-1100 OUTFILE=composite-fi-S.h5 SIZE=$SIZE  BBOX=17,57.7,33,69.5  ./make-composite.sh  data-20140525/201405251200_radar.polar.fi*.h5
    # convert composite-fi-S.tif composite-fi-S.png
    
    convert +append -frame 3 -fill green  composite-fi-C.tif  composite-fi-S.tif composite-geotiff-error.png

fi

ls -ltr diff-????.lgeo

