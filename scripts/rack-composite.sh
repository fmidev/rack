#!/bin/bash
#
# Script using Rack to make radar data composites
#
# Markus.Peura@fmi.fi
#

LC_NUMERIC='en_GB.utf8'
START_TIME=`date +'%s'`

#OPTIONS='--longoptions cBBox:,cProj:,cSize:'

# Parse options. Dumped for 
#if ! options=$(getopt -o 'ho:' --longoptions help,cBBox:,cBBoxTile:,cProj:,cSize:,cTime:,cTimeDecay:,inputPrefix:,outfile:,MODE:,PROCESSES:,PRODUCT:,QUANTITY:,TILEDIR: -n ${0} -- "$@"); then
    # Error, getopt will put out a message for us
#    exit 1
#fi
# echo "$options"
# Redefine options
# set -- $options


# This is a selector for selecting between basically two schemes.
# The schemes are not direct concepts of rack, but control this script.
#
# Direct scheme (default). Read all the raw files and create a composite
SCHEME=
#
# Create a tile for a single radar (and save it)
# SCHEME=tile
#
# Composite from tiles
# SCHEME=tiled

TILEDIR=

# Compositing principle: MAXIMUM, MAXW, AVERAGE or WAVG,p,r,gain,offset
METHOD=${METHOD:-MAXIMUM}
# METHOD=MAXQ
# METHOD=${METHOD:-WAVG,1,1,-32}
# METHOD=${METHOD:-WAVG,3,2,-32}

# Geographical bounding box
# BBOX='-10.434576838640398,31.746215319325056,57.81196475014995,67.62103710275053' # OPERA
# BBOX=${BBOX:-'-10,35,35,70'} # Europe
# BBOX=${BBOX:-'16,57,35,70'}  # Finland
# BBOX=${BBOX:-'8,53,32,70.5'} # Finland and Sweden
# BBOX=${BBOX:-'8,47.5,33,73'}   # Baltic
# BBOX=${BBOX:-'0,47.5,45.74,73'}   # BalticRus
# BBOX=${BBOX:-'-10,35,35,74'}   # EuroRus
# BBOX=${BBOX:-'17.6,57.6,29.7,64.0'}    # Finland/KOR-IKA-VAN
# BBOX=${BBOX:-'17.80505,59.37909,38.37097,69.35400'}  # LAPS
# BBOX=${BBOX:-'-10.4346,31.7462,57.8120,67.6210'}   # OPERA
BBOX=${BBOX:-'6,51.3,49,70.2'} # FMI Scandinavia


# Addititional sub-area cropping, aiming at larger composite
BBOXTILE=


# Geographical projection using PROJ.4 syntax
# EPSG=${EPSG:-"4326"}
# EPSG 4326 +proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs
# PROJ=${PROJ:-'+proj=stere +lon_0=25.0 +lat_0=60.000'}    # Finland
# PROJ=${PROJ:-"+proj=stere +lon_0=$LON0 +lat_0=$LAT0"}  
if [ "$EPSG" != '' ]; then
    PROJ="+init=epsg:$EPSG"
fi
#PROJ=${PROJ:-'+proj=stere +lon_0=20E +lat_0=90N +lat_ts=60 +a=6371288'}
# PROJ=${PROJ:-'+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs'}
# PROJ=${PROJ:-"+init=epsg:$EPSG"}
PROJ=${PROJ:-"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"}  # Same as epsg:4326


# Size of the image
# SIZE=${SIZE:-640,800}
# SIZE=${SIZE:-960,1200}
# SIZE=${SIZE:-800,1000}
SIZE=${SIZE:-800,800}

#CTYPE=${CTYPE:-'C,0.5,-32,0,255'}

ENCODING=
UNDETECT=

#  Meteorological product to be composited. Leave empty if first sweep only (PPI).
#  PRODUCT=${PRODUCT:-'pCappi,500'} # altitude (metres)
product=${PRODUCT:+'--'${PRODUCT/,/ }}
#  product=${QUANTITY:+"-Q '$QUANTITY' $product"}
#  If PRODUCT is unset, use lowest sweep (PPI) DBZH.
QUANTITY=${QUANTITY:-'DBZH'}
QUANTITY=${QUANTITY//\*/}
# if [ "$PRODUCT" == '' ]; then
#   delete='--delete dataset1?[02-9]'
# fi
# product=${product:-"--select quantity=$QUANTITY"}
product=${product:-"-Q '$QUANTITY'"}


PALETTE=${PALETTE:-$QUANTITY}

ctime=${TIMESTAMP:+"--cTime $TIMESTAMP"}
# 
decay=${DECAY:+"$ctime --cTimeDecay $DECAY"}
# DECAY=${DECAY:-'1'}

# Temporary directory in which the tiled are written.
TILEDIR=${TILEDIR:-"tiles"}

# In using TILE scheme, the maximum number of parallel processes. 
PROCESSES=${PROCESSES:-'4'}


# Filename of the output product
OUTFILE=

# These are for debugging and demos
FORMAT=${FORMAT:-'h5'}



while [ $# -gt 0 ]
do
    # Consume next (1st) argument
    key=$1
    unset value
    KEY=${1##*-} # trim leading hyphens
    # echo $KEY
    KEY=${KEY^^} # uppercase
    KEY=${KEY//[- ]/}
    if [ "$KEY" == 'PATH' ] || [ "${KEY:1}" == 'PATH' ]; then
       echo "$0: illegal variable name: $1 -> $KEY "
       exit 0
    fi
    case $key in
	-h|--help)
	    set -- 
	    break
	    ;;
	# Script-level options
	(--[A-Z]*)
	    value=$2
	    shift
	    ;;
	--debug)
	    DEBUG=1
	    ;;
	# Rack options: polar products and Cartesian (trim leading C or P)
	--[cp][A-Z][A-Za-z]*)
	    KEY=${KEY:1}
	    value=$2
	    shift
	    ;;
	# Other Rack options
	--[a-z]*) 
	    value=$2
	    shift
	    ;;
#	(--) 
#	    shift
#	    break;;
	(-*) 
	    echo "$0: error - unrecognized option key $key ($KEY)" 1>&2;
	    exit 1
	    ;;
	(*) 
	    break
	    ;;
    esac
    echo "# Fetch next argument as 1st"
    if [ -v "$KEY" ]; then
	eval "$KEY"=$value
    else
	echo "$0: error: unrecognized option $key ($KEY)" 1>&2
	exit 1
    fi
    shift
done

echo "BBOX=$BBOX"
echo "SIZE=$SIZE"
echo "$@"



# Binary executable to use
RACK=${RACK:-'rack'}


#exit 0




if [ "$MODE" != '' ]; then
    echo "500 Use 'SCHEME' instead of 'MODE'"
    exit -1
fi


# Optional: use conf files, named composite-${CONF}.cnf
# Set CONF=<conf> to apply.
for i in ${CONF//,/ } ; do   # ORDER?
    CONFFILE="composite-${CONF:+$CONF}.cnf"
    if [ -f $CONFFILE ]; then
	echo "Applying conf file '$CONFFILE'"
	source $CONFFILE
    else
	echo "Conf file '$CONFFILE' not found."
	exit 1
    fi 
done





inprefix=${INPREFIX:+"--inputPrefix $INPREFIX"} # inputPrefix
outprefix=${OUTPREFIX:+"--outputPrefix $OUTPREFIX"} # inputPrefix

#OUTDIR=${OUTDIR:-'.'}


# Filename of the output product
OUTFILE=${OUTFILE:-"composite"${ANDRE:+'-a'$ANDRE}${CONF:+'-c'$CONF}${METHOD:+"-$METHOD"}".$FORMAT"}

BASENAME=${OUTFILE%.*}


if [ $# == 0 ]; then
    echo "Script using Rack to make radar data composites."
    echo "Usage:"
    echo "  $0 <rack-args> *.h5"
    echo "Variables can be overriden in shell. Example:"
    echo "  BBOX=$BBOX SIZE=$SIZE METHOD=$METHOD PROJ='$PROJ' make-composite.sh *.h5"
    echo "Default values:"
    for i in BBOX SIZE METHOD PROJ SCHEME TILEDIR; do
      eval echo "'  '$i=\'\$$i\'"
    done 
    exit
fi

# VERBOSE=${VERBOSE:-5}
verbose=${VERBOSE:+"--verbose $VERBOSE"}

# Approximate center point for radar-centered projections (esp. azimuthal equidistant)
BBOX2=( ${BBOX//,/ } )
LON_LL=`printf '%.0f' ${BBOX2[0]}`
LAT_LL=`printf '%.0f' ${BBOX2[1]}`
LON_UR=`printf '%.0f' ${BBOX2[2]}`
LAT_UR=`printf '%.0f' ${BBOX2[3]}`
LON0=$(( ( LON_LL + LON_UR ) / 2 ))
LAT0=$(( LAT_LL ))



ANDRECONF="andre-$ANDRE.cnf" 
if [ -f $ANDRECONF ]; then
  ANDRE=( `cut -d\# -f1 $ANDRECONF` )
else
  if [ "$ANDRE" != '' ]; then
      echo "Andre conf file '$ANDRECONF' not found"
      exit 1
  fi
fi




# Overall weight. In this script, this is for demonstration only.
# The value can be varied for each radar, depending on its overall signal
# quality etc. 
# RANDOMWEIGHT=TRUE

# Weight applied to old composite data loaded as a warm start.
# CWEIGHT=1.0  # default ${CWEIGHT:-'0.9'}

# Debugging
debug=${DEBUG:+'--debug'}

# This is the "physical" value (dBZ) used when 'undetect' is read.
# This is NOT the undetect code (marker) to be used; see --encoding below.
# UNDETECT=${UNDETECT:-'-40'}
X=( ${UNDETECT/,/ } )
UNDETECT_VALUE=${UNDETECT_VALUE:-${X[0]}}
UNDETECT_WEIGHT=${UNDETECT_WEIGHT:-${X[1]}}
undetect=${UNDETECT_VALUE:+"--cUndetect $UNDETECT_VALUE"}' '${UNDETECT_WEIGHT:+"--undetectWeight $UNDETECT_WEIGHT"}

# ENCODING=${ENCODING:-'C,0.5,-32,0,255'}
encoding=${ENCODING:+"--encoding $ENCODING"}

bboxtile=${BBOXTILE:+"--cBBoxTile $BBOXTILE"} 

if [ "$SCHEME" == 'TILE' ]; then
  mkdir --verbose --parents "$TILEDIR"
  rm -v $TILEDIR/*-tile.h5
  #if [ -s $TILEDIR/ ]; then
  #    REPORT_TILES="Tile directory '$TILEDIR' was not empty, consider 'rm -vf $TILEDIR/*.h5'"
      #exit -1
  #fi
  decay=''
fi

# EPSG code ok
#if [ ${PROJ//0-9/} == ''  ]; then
#    
#fi 


NEWLINE='\
'

if [ "$INFILE" != '' ]; then
    CWEIGHT=${CWEIGHT:-'0.9'}
    INIT="        $INFILE $time --cAddWeighted $CWEIGHT "
    #INIT="--cLoad $INFILE $time --cAddWeighted $CWEIGHT "
else
    INIT="$NEWLINE --cProj '$PROJ' $NEWLINE --cSize $SIZE --cBBox $BBOX $bboxtile  $encoding --cInit"
fi

# PART 1: Initial part    # --encoding $CTYPE
command="$RACK $debug $decay --cMethod $METHOD $undetect  $INIT $NEWLINE $inprefix $outprefix $verbose" 


# Routine applied to each input volume (in default and TILE schemes).
routine="$delete ${ANDRE[*]} $product $encoding "  #--cCreate"

weighed=${CWEIGHT:+-"Weighted $CWEIGHT"}
if [ "$SCHEME" == '' ]; then
    command="$command $NEWLINE --script '$routine --cAdd${weighted}' "  
fi 

if [ "$SCHEME" == 'TILED' ]; then
    command="$command --script '--cAdd${weighted}' "  
fi 




# TODO simplify
#encoding='--encoding C,0.5,-32,0,255'

# Traverse arguments. 
# If an argument is a directory, expand as *.h5 files.
# If an argument is a file named <file>.lst, expand as --execFile <name>.lst 
INPUT=''
for i in $*; do
    if [ -d $i ]; then 
	INPUT=( ${INPUT[*]} $i/*.h5 )
    else
	if [ ${i##*.} == 'lst' ]; then 
	    echo ${x#*.}
	    INPUT=( ${INPUT[*]} --execFile $i )
	else
	    INPUT=( ${INPUT[*]} $i )
	fi
    fi
done

# PART 2: Input the given radars
NPROC=0
for i in ${INPUT[*]}; do
    # echo $i
    # Quality based weighting illustrated with random numbers.
    #W=${WEIGHT:+"Weighted 0.$RANDOM"}
    case "$SCHEME" in
    TILE)
       basename=${i##*/}
       command2="$command $i $routine --cCreateTile -o $TILEDIR/${basename%.*}-tile.$FORMAT"
       echo $NPROC # $command2
       #echo -e "$command2" > ${basename%.*}-tile-proc$NPROC.cmd
       echo -e "$command2" > $TILEDIR/proc$NPROC-last.cmd
       ln -sf $TILEDIR/proc$NPROC-last.cmd ${BASENAME}-TILE.cmd
       #echo -e "$command" > $BASENAME-cmd${SCHEME:+"-$SCHEME"}.txt
       if [ "$DEMO" != 0 ]; then
	   eval "$command2"  &
       fi
       NPROC=$(( NPROC + 1 ))
       if [ $NPROC -ge $PROCESSES ]; then
        wait # 
        NPROC=0
       fi
    ;;
    TILED)
	#ommand="$command $NEWLINE  --cLoad $i --cAdd " #'\\\n'
	#command="$command $NEWLINE          $i --cAdd " #'\\\n'
	command="$command $NEWLINE          $i " #'\\\n'
    ;;
    DUMMY)
       command="$command $NEWLINE  $i" #'\\\n'
    ;;
    *)
       command="$command $NEWLINE  $i " # --autoExec!
    ;;
    esac    

#tile=${TILE:+" -o ${basename%.*}$TILE"}
#  -o ${i%.*}-tile.h5
#  -o ${i%.*}-tile.png ${x#*.}
done

if [ "$SCHEME" == 'TILE' ]; then 
    wait
    END_TIME=`date +'%s'`
    echo "# $PROCESSES procs, $(( END_TIME - START_TIME )) seconds"
    echo $REPORT_TILES
    echo "exit"
    exit 0
fi

# PART 3: Extract image products from the composite





# If $DEMO is set, write some additional image files and 
# metadata as a text file.

grid=${GRID:+"--cGrid $GRID"}

# -O $BASENAME-.p
#  -o $BASENAME.tif
#DEMOFILES=${DEMO:+"$NEWLINE -o $BASENAME.png -o $BASENAME.tif -Q QIND -o $BASENAME-QIND.png -Q COUNT -o $BASENAME-COUNT.png -Q '*DEV' -o $BASENAME-STDEV.png $NEWLINE -Q '${QUANTITY}' --palette palette-${PALETTE}.txt $grid -o $BASENAME-rgb.png $NEWLINE --encoding 'C,0.2,-32,1,100' --imageAlpha -o $BASENAME-rgba.png $NEWLINE -o $BASENAME-h5.txt"}

#DEMOFILES=${DEMO:+"$NEWLINE -o $BASENAME.png -o $BASENAME.tif -Q QIND -o $BASENAME-QIND.png -Q COUNT -o $BASENAME-COUNT.png -Q '*DEV' -o $BASENAME-STDEV.png $NEWLINE -Q '${QUANTITY}' --palette palette-${PALETTE}.txt $grid -o $BASENAME-rgb.png $NEWLINE --imageTransp 0.1:0.2,0.3,0.7 -o $BASENAME-rgba.png $NEWLINE -o $BASENAME-h5.txt"}
DEMOFILES=${DEMO:+"$NEWLINE -o $BASENAME.png -o $BASENAME.tif -Q QIND -o $BASENAME-QIND.png -Q COUNT -o $BASENAME-COUNT.png -Q '*DEV' -o $BASENAME-STDEV.png $NEWLINE -Q '${QUANTITY}' --palette '${PALETTE}' $grid -o $BASENAME-rgb.png $NEWLINE --imageTransp 0.1:0.2,0.3,0.7 -o $BASENAME-rgba.png $NEWLINE -o $BASENAME-h5.txt"}

# if not TILED encoding='' ?
encoding='' 
command="$command $NEWLINE$encoding --cExtract dwsc $NEWLINE -o $OUTFILE $DEMOFILES"

#fi 0,47,36,73
echo

#command2=`echo -e "$command" | tr -d '\\\' `
command2=`echo "$command" | tr -d '\\\' `
echo -e "$command" > $BASENAME${SCHEME:+"-$SCHEME"}.cmd
#echo $command2
#eval $command2
result=0
if [ "$DEMO" != 0 ]; then
    eval $command2
    result=$?
fi
echo "Processed ${#INPUT[*]} input rows (cmds or files)."

#echo -e "\n\n# map: maps.wms_CONF=kap_BBOX=${BBOX}_PROJ=${EPSG}_SIZE=${SIZE}_LAYERS=europe.country.wgs84,scandinavia.major.lakes.wgs84_MODULATE=50,40,130_BGCOLOR=0xd0f0f0.png" >> $BASENAME-cmd${SCHEME:+"-$SCHEME"}.txt

END_TIME=`date +'%s'`
echo "# $(( END_TIME - START_TIME )) seconds"



exit $result
