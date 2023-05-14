#!/bin/bash

#
# Markus.Peura@fmi.fi
#

QUANTITY=${QUANTITY:-'DBZH'}

# mkdir -p cluttermaps

# Intermediate file
# (Cannot use special variable $NOD in input file name, as $NOD is known only after reading
ACCFILE=${ACCFILE:-"accumulated-$QUANTITY.h5"} 

# Final, "clutter probability" obtained by fuzzifying the accumulation field.
OUTFILE=${OUTFILE:-'cluttermap-${NOD}.h5'} 
#echo $OUTFILE

if [ $# == 0 ]; then
    echo "Accumulates data from a single site time series and creates a clutterMap"
    echo "by fuzzyfying the accumulated data."
    echo 
    echo "Default input quantity ($QUANTITY) can be changed with a leading QUANTITY=<qty> (in bash)"
    echo
    echo "Usage:"
    echo "$0 <volumes.h5>"
    echo
    echo "Examples:"
    echo "$0 data-acc/*fivim*.h5"
    echo "QUANTITY=DBZH OUTFILE='cluttermaps/clutterMap-\${NOD}.h5' $0 data-acc/*fivim.h5"
    echo "QUANTITY=DBZH ACCFILE=$ACCFILE OUTFILE=$OUTFILE $0 data-acc/*fivim.h5"
    exit 1
fi



echo "# Accumulating data to '$ACCFILE'..."
# 'S' => use 16bit accuracy"
rack --encoding S  -Q $QUANTITY --undetectWeight 1  --script '--pAdd' $*  --pExtract 'd' -o $ACCFILE



echo "# Fuzzifying the data to '$OUTFILE', +/-20dBZ "
cmd="rack $ACCFILE -Q $QUANTITY --encoding C,0.004,-0.004,quantity=CLUTTER --iFuzzyStep -20,20 --keep quantity=CLUTTER --expandVariables -o '$OUTFILE'"
echo $OUTFILE
echo $cmd
eval $cmd



#rack accumulated-DBZH.h5 -Q DBZH --encoding C,quantity=PROB --iPhysical 1 --iFuzzyStep -20,20 --keep quantity=PROB -o koe.png ; xv koe.png 
