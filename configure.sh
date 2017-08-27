#!/bin/bash
# Markus.Peura@fmi.fi

CONF_FILE='install-rack.cnf'
echo "Configuring Rack installation (creating/updating '$CONF_FILE')"

if [ -f "$CONF_FILE" ]; then
  cp -v $CONF_FILE $CONF_FILE.bak
  source $CONF_FILE
fi

echo
echo -n "# Conf by $USER, " > $CONF_FILE
date >> $CONF_FILE
echo >> $CONF_FILE


# Given a file (like 'geotiff.h'), returns its directory.
# If several paths found, gives the last one.
function guess_path(){
    #echo "(Found $1)"
    dirname `locate $1 | tail -1`
    #DIRS=(` locate /$1 `)
    #echo ${DIRS[*]%/*} # all
}


# Utility to change default variables (above)
# ARGS: <variable-name> <prompt-text>
function ask_variable(){
  local x=$1
  local X
  eval X="\$$1"
  shift
  echo $*
  read -e  -i "$X" -p "  $x=" $x
  eval X=\$$x
  echo "# $*" >> $CONF_FILE
  echo "$x='$X'" >> $CONF_FILE
  echo >> $CONF_FILE
  echo
}

function warn_if_unfound(){
  if [ ! -d "$1" ]; then
      echo "Warning: $1 not found"
  fi
}

# Todo recode
echo 'Searching for includes...'
echo
locate hdf5.h
HDFROOT=${HDFROOT:-`guess_path hdf5.h`}
#echo -n '...'
echo
locate proj_api.h
PROJROOT=${PROJROOT:-`guess_path proj_api.h`}
#echo -n '...'
echo
locate geotiff.h
GEOTIFF=${GEOTIFF:-`guess_path geotiff.h`}
#echo -n '...'
prefix=${prefix:-'/var/opt'} # or '.'?
#echo -ne '\r'
echo



echo "Give PATH PREFIXES"
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
    echo "# -using 'pkg-config' for setting CPPFLAGS and LDFLAGS"    echo 
    CCFLAGS=${GEOTIFF:+"-I$GEOTIFF"}
    LDFLAGS=''
    for i in hdf5 proj png ${GEOTIFF:+'tiff'} ${GEOTIFF:+'geotiff'}; do
	CCFLAGS="$CCFLAGS `pkg-config --cflags $i`" && LDFLAGS="$LDFLAGS `pkg-config --libs $i`" || LDFLAGS="$LDFLAGS -l$i"
    done
fi
echo

ask_variable CCFLAGS "General include options"
ask_variable LDFLAGS "General library options"

echo 
echo "Created $CONF_FILE with contents:"
echo
cat  $CONF_FILE

echo 
echo "Continue with ./build.sh"

