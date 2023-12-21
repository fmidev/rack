#!/bin/bash


# VERSION=$( $TARGET --version | head -1 )
VERSION=$( fgrep __RACK_VERSION__ main/rack.h | cut -d' ' -f 3 | tr -d '"' )
RACK_LATEST="rack-${VERSION}"

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
cp -vi ./rack ${prefix}/bin/${RACK_LATEST} && pushd ${prefix}/bin; ln -s ${RACK_LATEST} rack || ln -si ${RACK_LATEST} rack; popd

if [ $? != 0 ]; then
    exit 1
fi

