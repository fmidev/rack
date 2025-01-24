#!/bin/bash

DOCFILE=~/eclipse-workspace/rack/src/main/graphics.inc
TMPDIR=out
#RACK="rack --outputPrefix \$PWD/$TMPDIR/ --outputConf svg:absolutePaths=true "
RACK="rack --outputPrefix \$PWD/$TMPDIR/ "

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

	CAPTION=${CAPTION:-"SVG output '$OUTFILE'"}
	
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



# --gTitles main=AUTO,groups=NONE|AUTO|ID,images=NONE
# --gGroupId ''  # sets groups=ID
# --gImageTitles TIME=UR:....., one at time, like imageConf   LOCATION:  


WRITE_DOC "Align two images horizontally (default)" 
RUN_TEST \\  volume.h5 --cSize 400 -Q DBZH -c -o gray.png --palette "'default'" -o rgb.png \\  -o simple.svg

WRITE_DOC "Also legend outputs are included in the resulting SVG panel."
RUN_TEST \\  volume.h5 --cSize 400 -Q DBZH -c -o gray.png --palette "'default'" -o rgb.png \\  --legendOut legend.svg \\  -o simple2.svg

WRITE_DOC 'With several inputs, it is handier to use \c --script . '
WRITE_DOC 'Output names must be distinct, which is achieved using variables. For details, see \ref scripts and \ref templates .'
WRITE_DOC 'If generated products contain varying metadata, titles appear, automatically grouping distinguishing data.'
RUN_TEST \\ --script "'--cReset --cSize 300 -Q DBZH -c --palette \"\" -o out-\${NOD}.png'" \\ 'data/pvol_fi{anj,kor,kuo}.h5' \\  -o triple1.svg

WRITE_DOC '\c Rack supports grouping output images to rows or columns.' # Use \c --cGroup to set a distinguishing key.'
#WRITE_DOC 'Radar images can be organized to \e groups – on lines or columns.'
WRITE_DOC 'The groups are identified with \c --gGroupTitle \c arg , using distinguishing variables in the argument, for example '
WRITE_DOC '\c ${NOD} , \c ${what:date} or \c ${what:time} .'

RUN_TEST  \\ --script "'--cReset --cSize 300 -Q DBZH -c --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gGroupTitle "'Sky conditions at \${what:time|%H:%M} UTC'" \\ 'data-kiira/20170812*.h5' \\  -o time-series1.svg


WRITE_DOC 'Similar example using originating radar as the distinguishing metadata.'
RUN_TEST  \\  --script "'--cReset --cSize 300 -Q DBZH -c --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gGroupTitle "'Examples of \${PLC} (\${NOD})'"  \\ 'data-kiira/20170812*.h5' \\  -o radar-series.svg


WRITE_DOC 'A further example, with three levels of titles.'
RUN_TEST \\  --script "'--cReset --cSize 300 -Q DBZH -c --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\   --gGroupTitle "'Examples of Kiira case'" --gStyle .IMAGE_BORDER="'stroke:black;stroke-width:1'"  \\  data-kiira/201708121530_radar.polar.fikor.h5 data-kiira/201708121600_radar.polar.fiika.h5 \\  -o series-labelled2.svg

WRITE_DOC 'Titles can be removed with respective empty command values:'
RUN_TEST \\   --script "'--cReset --cSize 300 -Q DBZH -c --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gTitle "''"  --gGroupTitle "''" --gStyle .IMAGE_BORDER="'stroke:black;stroke-width:1'"  \\   'data-kiira/201708121?00_radar.polar.fi{kor,ika}.h5' \\  -o series-labelled3.svg 



WRITE_DOC 'A further example, usage of styles'
RUN_TEST \\   --script "'--cReset --cSize 300 -Q DBZH -c --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gTitle "'AUTO'"  --gGroupTitle "'AUTO:Examples of \${what:time}'" --gTitleBoxHeight "'30,20,10'" \\ --gStyle ".IMAGE_BORDER='stroke:black;stroke-width:1'" --gStyle "'rect.MAIN_TITLE=fill:red'"  \\   --gStyle ".MAIN_TITLE='font-size:15;font-family:Times'"  --gStyle ".LOCATION='fill:green'" \\   'data-kiira/201708121??0_radar.polar.fi{ika,kor,van}.h5'  \\  -o series-styled.svg 

ls -1t ${OUTFILES[*]//.png/.cmd}

echo -e "Created: \ndisplay ${OUTFILES[*]}"


exit 0
