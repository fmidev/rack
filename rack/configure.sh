#!/bin/bash
# Markus.Peura@fmi.fi

CONF_FILE='install-rack.cnf'
echo "Creating/updating Rack config file '$CONF_FILE')"
echo "(Optionally edited later.)"

if [ -f "$CONF_FILE" ]; then
  cp -v $CONF_FILE $CONF_FILE.bak
  source $CONF_FILE
fi

echo
echo -n "# Conf by $USER, " > $CONF_FILE
date >> $CONF_FILE
echo >> $CONF_FILE


# Given a file (like 'geotiff.h'), returns its directory.
# Among several paths, selects the last one.
#
# ARGS: <variable-name> <file>
#
function guess_include_dir(){

    local KEY=$1
    local P=''
    
    # Step 1: try standard
    for i in /{usr,var}{,/local}/include ; do
	if [ -f $i/$2 ]; then
	    P=$i/$2
	    break
	fi
    done
	     
    if [ "$P" != '' ]; then
	echo "# $KEY: found standard path: $P"
    else
	echo "# $KEY: trying 'locate'"
	locate --regex "$2$"
	local P=`locate --regex $2$ | tail -1`
	#echo " -> $KEY="$P""
    fi

    # Strip filename 
    P=${P%/*}
    local P_old
    eval  P_old=\$$KEY
    if [ "$P" != "$P_old" ]; then
	echo "# Was: $KEY=$P_old"
	echo $KEY="$P"
    fi
    eval $KEY="$P"
    echo 
}


# Utility to change default variables (above)
# ARGS: <variable-name> <prompt-text>
function ask_variable(){
  local key=$1
  local X
  eval X="\$$key"
  shift
  echo $*
  read -e  -i "$X" -p "  $key=" $key
  eval X=\$$key
  echo "# $*" >> $CONF_FILE
  echo "$key='$X'" >> $CONF_FILE
  echo >> $CONF_FILE
  echo
}

function warn_if_unfound(){
  if [ ! -d "$1" ]; then
      echo "Warning: $1 not found"
  fi
}

# Todo recode
echo 'Automagically detecting for directories...'
guess_include_dir  HDFROOT  hdf5.h
guess_include_dir  PROJROOT proj_api.h
guess_include_dir  GEOTIFF  geotiff.h
prefix=${prefix:-'/var/opt'} # or '.'?
echo



echo "Check the directories detected above:"
echo 
ask_variable HDFROOT  "Hierarchical Data Format (HDF5) library"
warn_if_unfound $HDFROOT

ask_variable PROJROOT "PROJ.4 projection library"
warn_if_unfound $PROJROOT

ask_variable GEOTIFF  "GeoTIFF include dir (leave empty if GeoTIFF not needed)"

ask_variable prefix "For installing Rack executable in \$prefix/bin/"
warn_if_unfound $PROJROOT

echo
echo "# General compilation setup"


if pkg-config --version > /dev/null ; then
    echo "# -using 'pkg-config' for setting CPPFLAGS and LDFLAGS"
    echo 
    CCFLAGS=${GEOTIFF:+"-I$GEOTIFF"}
    LDFLAGS=''
    for i in hdf5 proj png ${GEOTIFF:+'tiff'} ${GEOTIFF:+'geotiff'}; do

	# Try pkg-config
	CCFLAGS="$CCFLAGS `pkg-config --silence-errors --cflags $i`" && LDFLAGS="$LDFLAGS `pkg-config --silence-errors --libs $i`" && continue

	# ... Failed, so retry pkg-config with prefix 'lib'
	CCFLAGS="$CCFLAGS `pkg-config --silence-errors --cflags lib$i`" && LDFLAGS="$LDFLAGS `pkg-config --silence-errors --libs lib$i`" && continue

	#echo "Note: pkg-config warnings here can be discarded!"
	#echo 

	# ... Failed, so simply just add it
	LDFLAGS="$LDFLAGS -l$i"
	
    done

fi
echo

ask_variable CCFLAGS "General include options"
ask_variable LDFLAGS "General library options"

echo 
echo "Created $CONF_FILE with contents:"
echo
cat  $CONF_FILE

echo "Updated '$CONF_FILE'"
echo 
echo "Continue with ./build.sh"

