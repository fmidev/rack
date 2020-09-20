#!/bin/bash

# Installation script for 'Drain' and 'Rack'
#
# markus peura  fmi.fi



# Exit on fail
set -o errexit

# Current dir './' must be explicit
CONF_FILE="./install-rack.cnf"  

if [ ! -f $CONF_FILE ]; then
  ./configure.sh
fi

source $CONF_FILE

#USE_GEOTIFF=${GEOTIFF:+'YES'}
#USE_GEOTIFF=${USE_GEOTIFF:-'NO'}
echo "Using GeoTIFF: ${USE_GEOTIFF}"

#pushd rack
if [ $# != 0 ]; then
    make $*
    exit
fi


#make drainroot=$PWD  USE_GEOTIFF=${USE_GEOTIFF} CCFLAGS="$CCFLAGS" LDFLAGS="$LDFLAGS" release
make RACK_DIR=$PWD  USE_GEOTIFF=${USE_GEOTIFF} CCFLAGS="$CCFLAGS" LDFLAGS="$LDFLAGS" release
if [ $? != 0 ]; then
    echo "ERROR: Compiling rack failed"
    exit 1
fi


make prefix=$prefix install
if [ $? != 0 ]; then
    echo "ERROR: Installing rack failed. Consider: $0 clean"
    exit 1
fi

#popd


echo "Success."
#echo "Consider creating documentation by calling 'make doc' in ./rack ."
echo
echo "Consider setting:"
echo "export LD_LIBRARY_PATH=$HDFROOT/lib:$PROJROOT/lib:$LD_LIBRARY_PATH"
echo "export PATH=$PATH:$RACKROOT/bin"


exit 0


