#!/bin/bash


DST_DIR=${DST_DIR:-'./data'}
mkdir -v --parents $DST_DIR

#DATA_DIR=${DATA_DIR:-'./data'}

INFO_DIR=${INFO_DIR:-"cache/volume-info"}
mkdir -v --parents $INFO_DIR


echo "# Retrieve"

VOLUMES=()

# Step 1: collect files having no info file 
for i in $*; do
    # Strip extension
    INFO_FILE=${i%.*}.txt
    # Change dir path
    INFO_FILE=$INFO_DIR/${INFO_FILE##*/}
    echo "# INFO_FILE=$INFO_FILE"
    if [ ! -f $INFO_FILE ]; then
	echo "# Added"
	VOLUMES+=($i)
    fi
done

# Step 2: write missing info files
rack --format metadata --script "-o $INFO_DIR/"'${inputFileBasename}.txt'  ${VOLUMES[*]}

# Step 3: revisit all the files and link to a name based on extracted file info 
export TIMESTAMP SRC  
#for i in ${VOLUMES[*]}; do
for i in $*; do
    # Strip extension
    INFO_FILE=${i%.*}.txt
    # Change dir path
    INFO_FILE=$INFO_DIR/${INFO_FILE##*/}
    if [ -f $INFO_FILE ]; then
	source $INFO_FILE
	DST_SUBDIR=${DST_DIR}/${TIMESTAMP:0:4}-${TIMESTAMP:4:2}-${TIMESTAMP:6:2}
	mkdir -v --parents $DST_SUBDIR
	DST_FILE=${DST_SUBDIR}/${TIMESTAMP:0:12}_radar.polar.${SRC}.h5
	if [ ! -e $DST_FILE ]; then
	    SRC_FILE=`realpath $i`
	    ln -sv ${SRC_FILE} ${DST_FILE}
	else
	    echo "# Exists: ${DST_FILE}"
	fi
    else
	echo "# Info file missing for: $i"
	echo "# Info file: $INFO_FILE"
    fi
done
