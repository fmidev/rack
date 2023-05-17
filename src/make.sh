#!/bin/bash 
#
# Markus Peura fmi.fi
#
# Replacement for Makefile. Compiles files *.cpp to object files and then links them to an executable. 
#

OBJ_DIR=${OBJ_DIR:-'Build'}
CC=g++
TARGET=${TARGET:-'./rack'}

if [ "$1" == 'clean' ]; then
    CLEAN=YES
    shift
fi

# -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP  
OPTS=${OPTS:-$*}
OPTS=${OPTS:-'-g2 -O2 -Wall -fmessage-length=0 -fopenmp'}
# -O0 -g3 -Wall -c -fmessage-length=0
# -MMD -MP -MF"src/andre/AndreOp.d" -MT"src/andre/AndreOp.o" -o "src/andre/AndreOp.o" "../src/andre/AndreOp.cpp"

# g++ man page:
#
# -glevel Request debugging information and also use level to specify how much information.  The default level is 2.
#
# -O2 Optimize even more.  GCC performs nearly all supported optimizations that do not involve a space-speed tradeoff.
# As compared to -O, this option increases both compilation time and the performance of the generated code.

CONF='install-rack.cnf'
if [ -f $CONF ]; then
    source $CONF
else
   echo "# Config file $CONF missing, run:"
   echo "#   ./configure.sh"
   exit 1
fi


CCFLAGS=${CCFLAGS:-"-std=c++11 -I. -I/usr/include/geotiff -I/usr/include/hdf5/serial -I/usr/include/libpng12"}
LDFLAGS=${LDFLAGS:-"-std=c++11 -lproj -lpng12 -ltiff -lgeotiff -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5"}

echo "# CCFLAGS=$CCFLAGS"
echo "# LDFLAGS=$LDFLAGS"

OBJS=()

# if [ ! -f sources.lst ]; then
#    echo {andre,data,drain/{image,imageops,util,prog},hi5,main,product,radar}/*.cpp | tr ' ' '\n' > sources.lst
# fi


for SRC in {andre,data,drain/{image,imageops,util,prog},hi5,main,product,radar}/*.cpp ; do

    SRC_FILE=${SRC##*/}
    SRC_DIR=${SRC%/*}
    DST_DIR=$OBJ_DIR/${SRC_DIR}
    # echo $SRC_DIR $DST_DIR
    mkdir --parents -v $DST_DIR/
    
    OBJ=$DST_DIR/${SRC_FILE%.*}.o
    
    if [ -v CLEAN ] || [ ! -f $OBJ ] || [ $SRC -nt $OBJ ] ; then
	echo "# Building file: $SRC -> $OBJ"
	cmd="$CC $OPTS $CCFLAGS -I. -c -o $OBJ $SRC"
	echo "$cmd"
	eval "$cmd"
	RESULT=$?
	if [ $RESULT != 0 ]; then
	    echo "Build failed (result code $RESULT)"
	    exit $RESULT
	fi
	echo 
    fi

    OBJS+=( $OBJ )
    
done

# echo ${OBJS[*]}

echo "# ${#OBJS[*]} object files"
echo


echo "# Building target: $TARGET"
cmd="$CC $OPTS -o '${TARGET}' ${OBJS[*]} ${LDFLAGS}"
echo "$cmd"
eval "$cmd"

RESULT=$?
if [ $RESULT != 0 ]; then
    echo "Build failed (result code $RESULT)"
    exit $RESULT
fi

