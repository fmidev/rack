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

echo "Utility for installing Rack dependencies locally using MiniConda"

if  [ $# == 0 ]; then
    echo "Usage: $0 <pkg> <pkg2> <pkg...>"
    echo "Note: consider installing only missing packages"
    echo "Example:"
    echo "$0 tiff geotiff png proj hdf5"
    exit 0;
fi

# source $(conda info --base)/etc/profile.d/conda.sh
#conda init bash

# conda activate "$VENV"
conda activate "$VENV"
if  [ $? != 0 ]; then
    echo "Looks like VENV=$VENV does not exist"
    read -e  -i "yes" -p "  Create one? " CREATE
    #echo X=$CREATE
    if  [ "$CREATE" == 'yes' ]; then
	conda create -n "$VENV"
	conda activate "$VENV"
	if  [ $? != 0 ]; then
	    echo "Failed in activating VENV=$VENV"
	    exit 1
	fi
    fi
fi

cmd_full=''
if  [ ! -d $VENV_DIR ]; then
    echo "Using VENV_DIR=$VENV_DIR"
else
    echo "Directory not found VENV_DIR=$VENV_DIR"
    echo "Consider explicit setting: VENV_DIR=<dir> $0"
    cmd_full="conda create -n '$VENV' --yes"
    #exit 2
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
    if [ $pkg == 'png' ] || [ $pkg == 'tiff' ]; then
	pkg="lib$pkg"
    fi

    


    if [ "$cmd_full" != '' ]; then
	cmd_full="$cmd_full   $pkg${version:+=$version}"
    else
	cmd="conda install -y $pkg${version:+=$version}"
	echo $cmd
    fi

    if [ $? == 0 ]; then	
	echo >> $CONF_FILE
	echo "# Automatically added by $0" >> $CONF_FILE
	echo "${DIR_INC}=$VENV_DIR/include" >> $CONF_FILE
	echo "${DIR_LIB}=$VENV_DIR/lib"  >> $CONF_FILE
	if [ $pkg == 'geotiff' ]; then
	    echo
	fi
    else
	echo "install cmd failed: $cmd"
	exit 0
    fi
	
    echo 
done

echo "Alternative single-line cmd: $cmd_full"

echo "Next, continue with: "
echo "VENV_DIR=$VENV_DIR ./configure.sh"



