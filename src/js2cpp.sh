#!/bin/bash

echo "Converts JavaScript code (file.js) to C++ (file.h and file.cpp)"

for INPUT in $*; do

    echo "INPUT: $INPUT"

    EXT=${INPUT##*.}
    if [ $EXT != 'js' ]; then
	echo "File '$INPUT' with extension '$EXT' not JavaScript?"
	exit -1
    fi

    echo "Step 1: Checking syntax"
    js --check $INPUT
    if [ $? == 0 ]; then
	echo OK
    else
	exit -1
    fi



    BASENAME=${INPUT%.*}
    echo "BASENAME: $BASENAME"
    STEM=${BASENAME##*/}
    LABEL=${STEM^^}
    LABEL=${LABEL//[-.]/_}


    
    VARIABLE="const char *${STEM}"
    
    OUTPUT=${BASENAME}.h
    echo "Step 2a: Create header: $OUTPUT"
    echo $LABEL
    #exit -1
    echo '/** Automatically generated from "' $INPUT '" */' > $OUTPUT
    echo >> $OUTPUT
    echo "#ifndef JS_INCLUDE_${LABEL}" >> $OUTPUT
    echo "#define JS_INCLUDE_${LABEL}" >> $OUTPUT
    echo "Adding includes:"
    cat $INPUT | grep '^//|' | cut -d'|' -f2-
    cat $INPUT | grep '^//|' | cut -d'|' -f2- >> $OUTPUT
    echo "Adding main code:"
    echo "namespace javascript {" >> $OUTPUT
    echo "extern" >> $OUTPUT
    echo "$VARIABLE;" >> $OUTPUT
    echo "} // javascript::" >> $OUTPUT
    echo "#endif" >> $OUTPUT
    #echo 'Checking version control...'
    #git status $INPUT

    #INCLUDE=${OUTPUT} ?
    OUTPUT=${BASENAME}.cpp
    echo "Step 2b: Create object file: $OUTPUT"
    echo '/** Automatically generated from "' $INPUT '" */' > $OUTPUT
    echo "namespace javascript {" >> $OUTPUT
    echo "$VARIABLE = R\"JS(" >> $OUTPUT
    cat $INPUT | grep -v '^//|' >> $OUTPUT
    echo ')JS";' >> $OUTPUT
    echo "} // javascript::" >> $OUTPUT
    echo >> $OUTPUT
    
    
done

exit 0

echo 'Checking syntax...'
	js --check $<
echo 'Ok'

echo 'Wrapping to a C++ char array...'
echo $(basename $@)
echo $(notdir $@)
echo $(notdir $(basename $@) )

echo '#ifndef JS_INCLUDE_'$(notdir $(basename $@) ) >> $@
echo '#define JS_INCLUDE_'$(notdir $(basename $@) ) >> $@
echo >> $@

echo >> $@
echo 'namespace javascript {' >> $@
#cat $<  >> $@
echo -n 'const char* ' >> $@
echo -n $(notdir $(basename $@) ) >> $@
#echo -n $* | tr '-' '_' | tr -d '/.'  >> $@
#       @ echo    ' = R"JS(//<![CDATA[' | tr '-' '_' | tr -d '/.'  >> $@
#echo    ' = R"JS(/* ' $< ' */' >> $@
echo    ' = R"JS(// Source: ' $< >> $@
	cat $< | grep -v '^//|' >> $@
echo ')JS";' >> $@
echo '} // javascript::' >> $@
echo '#endif' >> $@
echo 'Checking version control...'
	git status $*
