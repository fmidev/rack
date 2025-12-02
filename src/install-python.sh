#!/bin/bash


#VERSION=$( fgrep RACK_VERSION main/rack.h | tr '(),' '\t\t.' | cut -f2 )
#RACK_LATEST="rack-${VERSION}"

echo "# Installing Python scripts supported by '$RACK'"

CONF='install-rack.cnf'
if [ -f $CONF ]; then
    source $CONF
else
    echo "# Config file $CONF missing?"
fi


if [ "$PYTHON_DIR" == '' ]; then
    echo "Python dir \$PYTHON_DIR not set. Run: ./configure.sh"
    exit 1
else
    mkdir -v --parents ${PYTHON_DIR}/rack/
    # including __init__.py
    for i in ../python/rack/*.py; do
	TARGET=${i##*/}
	cp -vu $i ${PYTHON_DIR}/rack/
    done
    PYTHONPATH_STRIP=${PYTHONPATH//"${PYTHON_DIR}"/}
    if [ "$PYTHONPATH_STRIP" == "$PYTHONPATH" ]; then
	echo "Consider setting:"
	echo "export PYTHONPATH='${PYTHONPATH:+$PYTHONPATH:}${PYTHON_DIR}'"
    fi
    #$PYTHONPATH
fi
