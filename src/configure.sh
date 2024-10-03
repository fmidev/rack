#!/bin/bash
# Markus.Peura@fmi.fi

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



# Todo: add Conda "detection", ie support for finding libs in conda path

# Given a file (like 'geotiff.h'), returns its directory.
# Among several paths, selects the last one.
#
# ARGS: (include|lib)  <variable-name> <file>
# TYPE: 
#
function guess_dir(){

    local TYPE=$1  # include or lib
    local KEY=$2   # name of the variable
    local FILE=$3
    local P=''
    local i

    #if [ "$VENV_DIR" != '' ]; then
    #fi
    echo "Determining '$TYPE' dir for '$KEY', FILE=$FILE

    
    # Step 1: try standard
    for i in ${VENV_DIR:+"$VENV_DIR/${TYPE}"} /{usr,var}{,/local}/${TYPE}{,64,/${FILE%.*},/lib${FILE%.*},/x86_64-linux-gnu} ; do
	echo -n  "  - Checking: $i/$FILE"
	if [ -f $i/$FILE ]; then
	    P=$i/$FILE
	    echo " [OK] "
	    break
	else
	    echo "..."
	fi
    done
	     
    if [ "$P" != '' ]; then
	echo "# $KEY: found standard path: $P"
    else
	local cmd="locate --regex '$FILE$'" # [1-9]?
	echo "# $KEY: trying: $cmd"
	eval $cmd
	P=`locate --regex ${FILE}$ | fgrep -v /doc | tail -1`
    fi

    # Strip filename 
    P=${P%/*}
    local P_old
    eval  P_old=\$$KEY
    #if [ "$P" != "$P_old" ]; then
    echo "# Previous value: $KEY=$P_old"
    #echo $KEY="$P"
    #fi
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

function unset_if_unfound(){
  if [ ! -d "$1" ]; then
      echo "Warning: $1 not found"
      unset $1
  fi
}

# Todo recode
echo 'Automagically detecting CCFLAGS and LDFLAGS'

prefix=${prefix:-'/var/opt'} # or '.'?
echo


echo "# Checking if 'pkg-config' utility is available"
PKGC=''
if pkg-config --version > /dev/null ; then
    echo -e "# Calling 'pkg-config' for setting CCFLAGS and LDFLAGS"
    # pkg-config warnings here can be discarded
    PKGC="pkg-config --silence-errors"
fi

CCFLAGS='-std=gnu++11 -fopenmp' # ${GEOTIFF_INCLUDE:+"-I$GEOTIFF_INCLUDE"}
LDFLAGS='-std=gnu++11 -fopenmp'


for i in hdf5 proj png tiff geotiff; do

    if [ "$PKGC" != '' ]; then

	# Attempt #1 ...
	CCFLAGS="$CCFLAGS `$PKGC --cflags $i`"    && LDFLAGS="$LDFLAGS `$PKGC --libs $i`"    && continue

	# Attempt #2 ...
	CCFLAGS="$CCFLAGS `$PKGC --cflags lib$i`" && LDFLAGS="$LDFLAGS `$PKGC --libs lib$i`" && continue

	echo -e "# $i:\t Not found with 'pkg-config' "
    fi

    
    key=${i^^}_INCLUDE
    guess_dir 'include' ${key} $i.h
    ask_variable ${key} "Include dir for $i"
    eval value=\$${key}
    if [ ! -e "$value" ]; then
	echo -e "# $i:\t warning: not found"
	continue
    fi

    #VALUE=${value:+"-I$value"}
    CCFLAGS="$CCFLAGS -I$value"

    i=${i%_*} # proj_api => proj
    key=${i^^}_LIB
    guess_dir 'lib' ${key} lib$i.so
    ask_variable ${key} "Library dir for $i"
    eval value=\$${key}
    if [ -e "$value" ]; then
	LDFLAGS="$LDFLAGS -L$value -l$i"
	#LDLIBS="$LDLIBS -l$i"
    else
	echo -e "# $i:\t warning: not found"
    fi

done



echo "Final values: "

ask_variable CCFLAGS "Include paths"
ask_variable LDFLAGS "Library paths"
#ask_variable LDLIBS  "Libraries"

ask_variable prefix 'Directory prefix for binary executable: ${prefix}/bin/'
warn_if_unfound $prefix

echo "# GeoTiff support (optional)" >> $CONF_FILE
USE_GEOTIFF='NO'
USE_GEOTIFF=${GEOTIFF_LIB:+"YES"}
echo "USE_GEOTIFF=${USE_GEOTIFF}" >> $CONF_FILE

echo 
echo "Created $CONF_FILE with contents:"
echo
cat  $CONF_FILE

echo "Updated '$CONF_FILE'"
echo 
echo "Continue with ./build.sh"

