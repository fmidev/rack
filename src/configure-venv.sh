#!/bin/bash
# Markus.Peura@fmi.fi

VENV=${VENV:-'rack-install'}

VENV_DIR=${VENV_DIR:-"~/.conda/envs/$VENV"}

CONF_FILE='install-rack.cnf'
echo "Creating/updating Rack config file '$CONF_FILE'"
echo "(Optionally edited later.)"

if [ -f "$CONF_FILE" ]; then
  cp -vi $CONF_FILE $CONF_FILE.bak
  source $CONF_FILE
fi

echo
echo -n "# Conf by $USER, " > $CONF_FILE
date >> $CONF_FILE
echo >> $CONF_FILE

if  [ $# == 0 ]; then
    echo "Example: $0"
    echo "$0 hdf5 png geotiff proj"
    exit 0;
fi

conda init
#conda activate "$VENV"
conda activate "$VENV"
if  [ $? == "xx0" ]; then
    echo "Looks like VENV=$VENV does not exist"
    read -e  -i "yes" -p "  Create one? " CREATE
    #echo X=$CREATE
    if  [ "$CREATE" == 'yes' ]; then
	conda create -n "$VENV"
    fi
    conda activate "$VENV"
    if  [ $? != 0 ]; then
	echo "Failed in activating VENV=$VENV"
	exit 1
    fi
fi

if  [ ! -d $VENV_DIR ]; then
    echo "Using VENV_DIR=$VENV_DIR"
else
    echo "Directory not found VENV_DIR=$VENV_DIR"
    echo "Consider explicit setting: VENV_DIR=<dir> $0"
    exit 2
fi

for i in $*; do
    #echo $i
    # pkg=${i%=*} 
    # version=${i##*=}
    pkg=( ${i/=/ } ) 
    version=${pkg[1]}

    echo "# $pkg version: $version"
    DIR_INC=${pkg^^}_INCLUDE
    DIR_LIB=${pkg^^}_LIB
    if [ $pkg == 'png' ]; then
	pkg=libpng
    fi

    
    cmd="conda install $pkg${version:+=$version}"
    echo $cmd

    if [ $? == 0 ]; then	
	echo >> $CONF_FILE
	echo "# Automatically added by $0" >> $CONF_FILE
	echo "${DIR_INC}=$VENV_DIR/include" >> $CONF_FILE
	echo "${DIR_LIB}=$VENV_DIR/lib"  >> $CONF_FILE
    else
	echo "install cmd failed: $cmd"
	exit 0
    fi
	
    echo 
done



