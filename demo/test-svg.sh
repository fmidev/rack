#!/bin/bash

DOCFILE=~/eclipse-workspace/rack/src/main/graphics.inc
TMPDIR=out
RACK="rack --outputPrefix \$PWD/$TMPDIR/ --outputConf svg:absolutePaths=true "

OUTFILES=()

mkdir -v --parents $TMPDIR

echo > $DOCFILE

function WRITE_DOC(){
    
    if [ $# == 0 ]; then
	# Read stdin
	# WARNING: may hang?
	cat >> $DOCFILE
    else
	# Write arguments
	echo $* >> $DOCFILE
    fi
    # echo >> $DOCFILE
}

NEWLINE="
"

function RUN_TEST(){

    local cmd="$RACK $*"
    local args=( $* )
    local argc=${#args[*]}

    # Assume last arg is an output file
    local OUTFILE=${args[$(( argc - 1 ))]}
    local BASENAME=${OUTFILE%.*}
    local FORMAT=${OUTFILE##*.}

    # With original newlines, "\\\n"
    # echo -e ${cmd//\/$TMPDIR/} > $TMPDIR/$BASENAME.cmd
    # echo "# CMD: $TMPDIR/$BASENAME.cmd"
    # rack_cmd=${cmd//\/$TMPDIR/}
    rack_cmd=${cmd//\\/\\$NEWLINE  }
    echo "${rack_cmd}" > $TMPDIR/$BASENAME.cmd
    cmd=(`echo -e $cmd | tr -d '\\'`)
    #cmd=${cmd//\\/}
    cmd="${cmd[*]}"
    echo "# $cmd"
    eval "${cmd}"
    if [ $? != 0 ]; then
	echo "# $cmd"
	echo "Rack command failed"
	exit 1
    fi

    echo "# Format $FORMAT "

    if [ "$FORMAT" == "svg" ]; then

	cmd="xmllint --noout $TMPDIR/$OUTFILE"
	eval $cmd
	if [ $? == 0 ]; then
	    echo "# OK: XML syntax"
	else
	    echo $cmd
	    echo "# FAIL: XML syntax error (xmllint $OUTFILE) "
	    exit 2
	fi

	local OUTFILE_PNG=$BASENAME.png
	cmd="inkscape $TMPDIR/$OUTFILE -o $TMPDIR/$OUTFILE_PNG"
	eval $cmd

	if [ $? == 0 ]; then
	    echo "# OK: created $TMPDIR/$OUTFILE_PNG"
	    OUTFILES+=($TMPDIR/$OUTFILE_PNG)
	    cp -v $TMPDIR/$OUTFILE_PNG ../doxygen/
	else
	    echo $cmd
	    echo "# FAIL: SVG->PNG conversion "
	    exit 3
	fi

	CAPTION=${CAPTION:-"SVG output '$OUTFILE_PNG'"}
	
	WRITE_DOC <<EOF
\code
$rack_cmd
\endcode

\image html  $OUTFILE_PNG "$CAPTION" 
\image latex $OUTFILE_PNG "$CAPTION" width=0.8\textwidth

EOF

	unset CAPTION
	
    fi
}



WRITE_DOC "Align two images horizontally (default)" 
RUN_TEST \\  volume.h5 --cSize 400 -Q DBZH -c -o gray.png --palette 'default' -o rgb.png \\  -o simple.svg

WRITE_DOC "Also legend outputs (SVG) are included"
RUN_TEST \\  volume.h5 --cSize 400 -Q DBZH -c -o gray.png --palette 'default' -o rgb.png \\  --legendOut legend.svg \\  -o simple2.svg

WRITE_DOC 'With several inputs, it is handier to use \c --script . (See: \ref scripts .)'
WRITE_DOC 'Output names must be distinct, which is achieved using variables. (See \ref templates .)'
RUN_TEST \\ --script "'--cReset --cSize 300 -Q DBZH -c --palette \"\" -o out-\${NOD}.png'" \\ data/pvol_fi{anj,kor,kuo}.h5  -o triple1.svg

WRITE_DOC 'Variables identifying the radar and timestamp often distinguish files sufficiently.'
WRITE_DOC 'Hence, consider using variables like \c ${NOD} , \c ${what:date}, and \c ${what:time} .'
WRITE_DOC '\c Rack supports grouping output images to rows or columns. Use \c --cGroup to set a distinguishing key.'
RUN_TEST  \\ --script "'--cReset --cSize 300 -Q DBZH -c --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gGroup "'Test-\${NOD}'" \\ 'data-kiira/*.h5'  -o series1.svg


WRITE_DOC 'In a grid of images, originating from several radars and times, the images can be labelled automatically with \c --gTitles command, with option \c IMAGE_TITLES . '
RUN_TEST  \\ --script "'--cReset --cSize 300 -Q DBZH -c --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gGroup "'Examples of \${PLC} (\${NOD}) on \${what:date|%Y/%m/%d}'" --gTitles IMAGE_TITLE \\ 'data-kiira/*.h5'  -o series-labelled.svg

echo -e "Created: \ndisplay ${OUTFILES[*]}"
