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

function WRITE_TITLE(){
    echo $NEWLINE >> $DOCFILE
    echo '<b>' $* '</b>' >> $DOCFILE
    echo $NEWLINE >> $DOCFILE
}

function WRITE_HELP(){
    local cmd="$1"
    make $cmd.hlp
    echo $NEWLINE >> $DOCFILE
    echo "\include $cmd.hlp" >> $DOCFILE
    echo $NEWLINE >> $DOCFILE
}

function RUN_TEST(){

    local cmd="$RACK $*"
    local args=( $* )
    local argc=${#args[*]}

    # Assume last arg is an output file
    local DESC="${args[$(( argc - 1 ))]}"
    local OUTFILE=${DESC,,}'.svg'
    OUTFILE=${OUTFILE//,/}
    cmd="${cmd/$DESC/$OUTFILE}"

    DESC="${DESC//_/ }."
    
    #local OUTFILE=${DESC// /_}

    
    local BASENAME=${OUTFILE%.*}
    local FORMAT=${OUTFILE##*.}

    # With original newlines, "\\\n"
    # echo -e ${cmd//\/$TMPDIR/} > $TMPDIR/$BASENAME.cmd
    # echo "# CMD: $TMPDIR/$BASENAME.cmd"
    # rack_cmd=${cmd//\/$TMPDIR/}
    cmd="${cmd/$DESC/$OUTFILE}"
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

	CAPTION=${CAPTION:-"SVG panel example: $DESC"}
	
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

#WRITE_DOC '\subsection svg-panels Basic SVG panels'

#MAPFORMAT="--format 'nutshell maps.wms_GEOCONF=radar:\${NOD}_LAYERS=osm:osm_PROJ=\${where:EPGS}_Presets=OpenStreetMap_SIZE=\${where:xsize},\${where:xsize}.png --link maps/\${NOD}_\${where:EPGS}_\${where:xsize},\${where:xsize}.png'"

#CONF="--format metadata -o \"\${NOD}_\${where:EPSG}_\${where:xsize},\${where:xsize}.cnf\""

#exit 0

WRITE_DOC "Align images horizontally (default):" 
RUN_TEST \\  volume.h5 --cProj 3067 --cSize 400 -Q DBZH -c "$CONF" -o gray.png --palette "'default'" -o rgb.png \\  -o 'Basic_example'


WRITE_DOC "Also legend outputs are included in the resulting SVG panel."
RUN_TEST \\  volume.h5 --cProj 3067 --cSize 400 -Q DBZH -c "$CONF" -o gray.png --palette "'default'" -o rgb.png \\  --legendOut legend.svg \\  -o 'Example_with_a_legend'

WRITE_DOC 'With several inputs, it is handier to use \c --script . '
WRITE_DOC 'Output names must be distinct, which is achieved using variables.'
WRITE_DOC 'If generated products contain different metadata, titles will appear, automatically displaying distinguishing data.'
WRITE_DOC 'For details, see \ref scripts and \ref templates .'
RUN_TEST \\ $MAPFORMAT --script "'--cReset --cProj 3067 --cSize 300 -Q DBZH -c $CONF --palette \"\" -o out-\${NOD}.png'" \\ 'data/pvol_fi{anj,kor,kuo}.h5' \\  -o 'Three_radars,_with_automatic_titles'

#exit 0

WRITE_TITLE 'Grouping images by timestamp or other metadata'

WRITE_DOC '\c Rack supports grouping output images to rows or columns.' # Use \c --cGroup to set a distinguishing key.'
#WRITE_DOC 'Radar images can be organized to \e groups – on lines or columns.'
WRITE_DOC 'The groups are identified with \c --gGroupTitle \c arg , using distinguishing variables in the argument, for example '
WRITE_DOC '\c ${NOD} , \c ${what:date} or \c ${what:time} .'
RUN_TEST  \\ --script "'--cReset --cProj 3067 --cSize 300 -Q DBZH -c $CONF --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gGroupTitle "'Grouping by time: \${what:time|%H:%M} UTC'" \\ 'data-kiira/201708121?00*.h5' \\  -o 'Time_series'

WRITE_DOC 'Group layout can be set with \c --gLayout that sets orientation \c HORZ (rows) or \c VERT (column). Also direction can be changed.'
WRITE_HELP 'gLayout'
WRITE_DOC 'Example:'
RUN_TEST  \\ --script "'--cReset --cProj 3067 --cSize 300 -Q DBZH -c --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'"  \\  --gLayout 'VERT,DECR'  --gGroupTitle "'Grouping by time: \${what:time|%H:%M} UTC'"    'data-kiira/201708121?00*.h5'   -o 'Time_series2' 

# --gStyle .IMAGE_BORDER="'stroke:black;stroke-width:0'" --gStyle rect.GROUP_TITLE="'stroke:white;stroke-width:2'"

WRITE_DOC 'Grouping without displaying titles can be done with prefix \c NONE . For example, using plain (unformatted) timestamp as follows:'
RUN_TEST \\   --script "'--cReset --cProj 3067 --cSize 300 -Q DBZH -c $CONF --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gGroupId "'Invisible grouping criterion-\${what:date}\${what:time}'"  --gGroupTitle "''"  \\   'data-kiira/201708121?00_radar.polar.fi{ika,kor,van}.h5'  \\  -o 'Without_group_titles'
# RUN_TEST \\   --script "'--cReset --cProj 3067 --cSize 300 -Q DBZH -c $CONF --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gGroupTitle "'NONE:Invisible grouping criterion-\${what:date}\${what:time}'"  \\   'data-kiira/201708121?00_radar.polar.fi{ika,kor,van}.h5'  \\  -o 'Without_group_titles'


WRITE_DOC 'Similar example using radar site code for grouping:'
RUN_TEST  \\  --script "'--cReset --cProj 3067 --cSize 300 -Q DBZH -c $CONF --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gGroupTitle "'Grouping using radar name: \${PLC} (\${NOD})'"  \\ 'data-kiira/20170812*.h5' \\  -o 'Grouping_by_site_codes'


WRITE_DOC 'The main title can be set explicitly with \c --gTitle command . The default value, \c AUTO , displays main title as in the examples above.'
RUN_TEST \\  --script "'--cReset --cProj 3067 --cSize 300 -Q DBZH -c $CONF --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gTitle "'My Main Title for \${what:date|%A, %d %B %Y}'"   --gGroupTitle "'Radar \${PLC}'" \\ 'data-kiira/201708121?00_*{ika,kor,van}.h5'   \\  -o 'User-defined_title'

# --gStyle .IMAGE_BORDER="'stroke:black;stroke-width:1'" 


WRITE_DOC 'Main title can be removed with empty arg:'
RUN_TEST \\   --script "'--cReset --cSize 300 --cProj 3067 -Q DBZH -c --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gTitle "''"  --gGroupTitle "'\${what:time|%H:%M} ${what:time|%H:%M}'"   \\   'data-kiira/201708121?00_radar.polar.fi{kor,ika}.h5' \\  -o "Group_titles_only"

#WRITE_DOC 'User defined main title'
#RUN_TEST \\   --script "'--cReset --cSize 300 -Q DBZH -c --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gTitle "'User defined title with timestamp: \${what:date|%Y/%m/%d} \${what:time|%H:%M}' "  --gGroupTitle "'NONE:\${what:date}-\${what:time}'"   \\   'data-kiira/201708121?00_radar.polar.fi{kor,ika}.h5' \\  -o user-title.svg 


WRITE_DOC '<b>Changing style of graphic panels</b><p/>'

WRITE_DOC 'A further example, usage of styles'
RUN_TEST \\   --script "'--cReset --cSize 300 --cProj 3067 -Q DBZH -c $CONF --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gTitle "'Larger font here...'"  --gGroupTitle "'...but smaller here, with still readable timestamp \${what:date|%A, %d %B %Y} at \${what:time|%H:%M} UTC'" --gTitleHeights "'40,20,15'" \\  'data-kiira/201708121?00_radar.polar.fi{ika,kor,van}.h5'  \\  -o "User-defined_title_height"

WRITE_DOC 'A further example, usage of styles'
RUN_TEST \\   --script "'--cReset --cSize 300 --cProj 3067 -Q DBZH -c $CONF --palette \"\" -o out-\${what:date}T\${what:time}-\${NOD}.png'" \\ --gGroupTitle "'AUTO:\${what:time}'"  \\ --gStyle ".IMAGE_BORDER='stroke:black;stroke-width:1'" --gStyle "rect.MAIN='fill:forestgreen'" --gStyle "rect.GROUP='fill:lightgreen'"  \\   --gStyle "text.MAIN='font-family:Times'"  --gStyle ".LOCATION='fill:brown'" \\   'data-kiira/201708121?00_radar.polar.fi{ika,kor,van}.h5'  \\  -o "Multiple_styles"





WRITE_DOC 'Metadata panel. (Experimental, variants under development.)'

RUN_TEST \\  --gTitleHeights "'30,25,15'"  --gTitle "''" --gGroupTitle "''" \\  volume.h5 \\ --cProj 3067 --cSize 500 -Q DBZH -c \\ --palette "'default'" --legendOut legend.svg -o rbg.png \\  --gPanel TECH --gStyle text.IMAGE='opacity:0' \\  -o "Metadata_panel"


WRITE_DOC '\b Background maps. External images can be linked with \c --gLinkImage. For example, maps can be included, aligning the following radar image on top of it with \c --gAlign \c HORZ_FILL,VERT_FILL .'

#WRITE_DOC '\subsection svg-include Including and excluding images in SVG panels'
#make -B gInclude.hlp
#WRITE_DOC '\include gInclude.hlp'
RUN_TEST \\ --inputPrefix '$PWD/' \\ --gTitle "''" --gGroupTitle "'AUTO:'" \\  --script "'--cReset --cProj 3067 --cSize 300 -Q DBZH -c --gLinkImage maps/map-radar:\${NOD}-\${where:EPSG}-\${where:xsize}x\${where:ysize}.png  --imageTransp 0.0:0.1,0,1 --palette default --gAlign HORZ_FILL,VERT_FILL  -o out-\${what:date}T\${what:time}-\${NOD}.png'"  'data-kiira/201708121600_radar.polar.fi{kor,ika,van}.h5' -o 'Adding_background_maps'




ls -1t ${OUTFILES[*]//.png/.cmd}

echo -e "Created: \ndisplay ${OUTFILES[*]}"


exit 0
