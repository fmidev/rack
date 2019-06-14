#!/bin/bash

# Copyright Markus Peura 2010-2014
# This script tests rack by running examples mentioned in the manual.
#
# Arguments:
# 1 - first test (1=first)
# 2 - last  test 
# 3 - if non-zero, exit on fail
#
# Pedantic:
# STOP_ON_ERROR=EXEC 
# 
# Input files in the directory
# volume.h5 sweep?.h5 volume-anomalous.h5
#
# Example commands will are collected from this file(s)
# Only considers examples which can be executed as such, including
# those with variables like $FILE but not those with <FILE>
# This was just a trick to exclude the latter from this test.
DOC_FILES="../main/*.dox" 

# Resulting list of example commands from *.dox
TEST_CMD_FILE=tests.lst


# Clean all the files which are not in svn. See also ../demo-src ...
if [ "$1" == 'clean' ]; then
    rm -v *~ `svn status . | grep '^\?\W' | cut -c2-`
    exit 0
fi

rm -v *.hlp

# Check that 'rack' is executable
#rack > /dev/null
RACK=${RACK:-`which rack`}
#if [ ! -x "$RACK" ]; then
#  echo "Error: could not execute 'rack'. No tests can be done..."
#  echo "RACK=$RACK"
#  exit -1
#fi


# Fixed filenames will be used: volume.h5 and sweep1.h5, sweep2.h5, ...  
# The user should copy or link them to this directory
echo 'Available test files:'
ls -1 volume.h5 sweep?.h5 volume-anomalous.h5
if (( $? > 0 )); then
    echo "Error: test file(s) missing in directory."
    echo "Link or copy volume.h5 and sweep1.h5, sweep2.h5, ...  and volume-anomalous.h5 in this directory."
    exit -1
fi



# Collect test cases
# Exclude commands containing '<', for examples of type cmd <param>
echo -n > $TEST_CMD_FILE
for file in ${DOC_FILES[*]}; do
#  grep '^[ ]*\(rack\|convert\) [^<]*' $file  | fgrep -v '<'  >>  $TEST_CMD_FILE
#  grep '^ *\([a-zA-Z]+\=[^ ]* +\)*\(rack\|convert\) [^<]*' $file  | fgrep -v '<'  >>  $TEST_CMD_FILE
    grep '^\s*\(rack \|convert \|make \|.*#exec\)[^<]*' $file  | fgrep -v '<'  >>  $TEST_CMD_FILE
done



# Append additional cases
echo ./test-content.sh >> $TEST_CMD_FILE
echo ./make-plot.sh    >> $TEST_CMD_FILE

#echo METHOD=WAVG,1,1  SIZE=300,300 BBOX=17.6,57.6,29.7,64.0 POLARPRODUCT=' ' ./make-composite.sh data/pvol_fi{kor,ika,van}.h5   >> $TEST_CMD_FILE

echo TEST_CMD_FILE=$TEST_CMD_FILE

N=( `wc -l $TEST_CMD_FILE` )

echo "Number of test examples: $N"

mv $TEST_CMD_FILE $TEST_CMD_FILE.tmp
cat -n $TEST_CMD_FILE.tmp > $TEST_CMD_FILE

#N=$#

#echo "Number of test examples: $N"
#echo "FILE=$FILE";
#echo "IMAGE=$IMAGE";
echo

rm -f test-*.log

STOP_ON_ERROR=${STOP_ON_ERROR:-'EXEC'} # or CMD
#STOP_ON_ERROR=${STOP_ON_ERROR:-'CMD'} # or EXEC
#STOP_ON_ERROR=${3:-'CMD'} # or EXEC
FAILS=0

#N=8
cat $TEST_CMD_FILE | head -5
echo '...'
cat $TEST_CMD_FILE | tail -5

# Index of tests and cmds
i=0
# Index of tests
t=0
line=${1:-'1'}
iEnd=${2:-$(( N - line )) }
iEnd=$(( line + iEnd ))
while (( $line <= iEnd )); do

    # filter by row number
    filter='^[[:space:]]\+'$line'[[:space:]]\+'
    #cmd="`cat -n $TEST_CMD_FILE | grep \"^[ \t]\+$line[ \t]\+\" | cut -c8-`"
    cmd="`cat $TEST_CMD_FILE | grep $filter | cut -c8-`"

    echo

    #continue 

    exec_only="`echo $cmd | fgrep '#exec' `"
    if [ "$exec_only" != '' ]; then
	echo -ne '\033[1;30m';
	echo "Executing only:"
	echo "  $cmd"
	#$cmd
	eval ${cmd%#*}
	OK=$?
	echo -ne '\033[0m'
	if [ $OK != 0 ] && [ $STOP_ON_ERROR == 'EXEC' ]; then
	    echo -e '\033[1;43mFAILED\033[0m'
	    exit 1
        fi
	line=$(( line + 1 ))
	continue
    fi
    #continue

    i=$(( i + 1 ))

    echo -e "\033[1;20mTEST #$i (line $line)\033[0m"
    #echo "TEST #$i (line $line):"
    #echo $cmd
    echo -e '\033[1;42m' $cmd '\033[0m'
    
    #echo "Test[$i]: '$cmd'"
    LOGFILE=test-$line.log
    if [ -f $LOGFILE ]; then 
	line=$(( line + 1 ))
	continue
    fi

    echo $cmd > tmp.log
    #  valgrind -v $cmd
    eval "$PRE $cmd"

    if (( $? != 0 )); then
	echo -e '\033[1;41mFAILED\033[0m'
	#echo FAILED
	FAILS=$(( FAILS + 1 ))
	if [ $STOP_ON_ERROR != 'NO' ]; then
	    eval "echo $cmd"
	    echo i=$i
	    if (( i == 1 )); then
		echo "check LD_LIBRARY_PATH"
            fi
	    break
	fi
    else

	# Repeat all the cmds with h5 output to png output
	cmdbase=${cmd%-o*} # without output
	output=${cmd##*-o} # remaining part (output file)
	extension=( ${output##*.} ) # extension, trailing white space trimmed
	#echo cmdbase=$cmdbase
	#echo output=$output
	#echo "extension='$extension'"
	if [ $extension == 'h5' ]; then
	    img=${output%.*}'.png'
	    echo -ne '\033[1;30m';
	    echo GENERATING IMAGE: $img
	    echo "$cmdbase -o $img"
	    eval "$cmdbase -o $img"
	    echo -ne '\033[0m'
	fi

	echo -e '\033[1;42mPASSED\033[0m'
	mv tmp.log $LOGFILE

    fi

    line=$(( line + 1 ))
    echo 

done



#for i in $*; do 
#    ls $i
#    FAIL=$(( $? != 0 ))
#    FAILS=$(( FAILS + FAIL ))
#done

#echo FILE=$FILE
echo
#echo "Result of i (out of N) tests:"
echo " PASSED: " $(( i - FAILS )) #' (' $(( N - FAILS )) ')'
echo " FAILED: " $FAILS

echo IMAGE=$IMAGE
#echo source $IMAGE_POSTPROC
