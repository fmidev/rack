
OUTFILE=${OUTFILE:-'volume-tmp.h5'}
TXTFILE=${OUTFILE%.*}'.txt'

function HELP(){
    local cmd=$1
    rack --help "$cmd" > help-$cmd.inc
}
# Markus.Peura@fmi.fi


# mGiven text data, creates a shortened excerpt with extension '.inc'
# $1 - infile
# $2 - [outfile]
function EXCERPT(){

    if [ $# == 0 ]; then
	echo "Error: no arguments"
	exit  1
    fi

    local INFILE="$1"
    local BASENAME=${INFILE%.*}
    local OUTFILE="$2"
    OUTFILE=${OUTFILE:-${BASENAME}.inc}
    
    local TMPFILE=${OUTFILE%.*}.tmp
    
    # Pick header (all of it)
    head -20 $INFILE | fgrep '#' > $TMPFILE

    # Pick 5 first lines of data
    fgrep -v '#' $INFILE | head -5 >> $TMPFILE

    # Add clip-clip marker
    echo -e  '...\n\t...\n\t\t...' >> $TMPFILE

    # Pick 5 last lines of data
    tail  -5 $INFILE >> $TMPFILE

    mv -v $TMPFILE $OUTFILE
    
    echo "# Created $OUTFILE"
}


function START(){
    SCRIPT=$1
    SCRIPT=${SCRIPT:-'script.inc'}
    echo -n > $SCRIPT
}

function TITLE(){
    echo
    echo "# $*"
    echo "# $*" >> $SCRIPT
}

function TEST(){

    echo

    #cmd="rack $INFILE $* -o $OUTFILE"
    #echo ${cmd/$INFILE/volume.h5} >> $SCRIPT
    echo "rack volume.h5 $* -o $OUTFILE" >> $SCRIPT
    echo >> $SCRIPT

    # Actually...
    # Note: clean what:source
    # weird chars cause binary mode, and premature end of (f)grep scanning
    cmd="rack $INFILE --/what:source= $* " #-o $OUTFILE"
    echo "# $cmd"
    
    # Save also txt file, pruning nosave-marked '~' objects. Note: source string causes binary mode?
    eval "$cmd -o - " > $TXTFILE.raw
    fgrep '~' -v $TXTFILE.raw > $TXTFILE
    #eval "$cmd -o - " | fgrep -v '~' > $TXTFILE
    if [ $? != 0 ]; then
	echo "Command failed..."
	exit  1
    fi
    
}

function REQUIRE(){
    for i in $*; do
	#rack --verbose 4 $OUTFILE -o - | grep ^$i > /dev/null
 	cmd="grep '^$i' $TXTFILE"
	echo $cmd
	eval $cmd > /dev/null
	if [ $? != 0 ]; then
	    echo "Required but missing: " $i
	    exit  1
	else
	    echo "OK - contains: " $i
	fi
    done
}

function EXCLUDE(){
    for i in $*; do
	#rack --verbose 4 $OUTFILE -o - | grep ^$i > /dev/null
	cmd="grep ^$i $TXTFILE"
	echo $cmd
	eval $cmd > /dev/null
	if [ $? == 0 ]; then
	    echo "Excluded but still present: " $i
	    exit  1
	else
	    echo "OK - deleted: " $i
	fi
    done
}

