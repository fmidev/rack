#!/bin/bash
#
# markus peura  fmi.fi

echo "# Building Rack..."
echo

USE_GEOTIFF=${USE_GEOTIFF:-'YES'}
echo "Using GeoTIFF: ${USE_GEOTIFF}"

export OBJ_DIR=Build

# Excutable to be created. Must contain path or be callable using $PATH.
export TARGET=./rack

#if [ ! -f $TARGET ] || [ "$1" == 'clean' ]; then
# echo "# Target '$TARGET' exists."
./make.sh $* -DUSE_GEOTIFF_${USE_GEOTIFF}  -g2 -O2 -Wall -fmessage-length=0
#fi

if [ $? != 0 ]; then
    exit 1
fi


#VERSION=$( $TARGET --version | head -1 )
VERSION=$( fgrep __RACK_VERSION__ main/rack.h | cut -d' ' -f 3 | tr -d '"' )
RACK="rack-${VERSION}"
echo "# Installing '$RACK' "

CONF='install-rack.cnf'
if [ -f $CONF ]; then
    source $CONF
else
    echo "# Config file $CONF missing?"
fi

prefix=${prefix:-"/usr/local"}
echo "# Using prefix=$prefix"


mkdir -v --parents ${prefix}/bin
cp -vi $TARGET ${prefix}/bin/${RACK} && pushd ${prefix}/bin; ln -s ${RACK} rack || ln -si ${RACK} rack; popd

if [ $? != 0 ]; then
    exit 1
fi

echo "Success."
# echo "Consider creating documentation by calling 'make doc' in ./rack ."
echo
echo "In library linking problems, consider adjusting env variables, esp. :"
echo "export LD_LIBRARY_PATH=$HDFROOT/lib:$PROJROOT/lib:$LD_LIBRARY_PATH"
echo "export PATH=$PATH:$RACKROOT/bin"

exit 0


