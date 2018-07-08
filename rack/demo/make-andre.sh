#!/bin/bash

VOLUME=${VOLUME:'volume.h5'}

# Andre command with parameters embedded as ${parameter}
ANDRE_CNF_TEMPLATE='andre-default.cmd.tpl'
ANDRE_CNF_EMPTY='andre-default-empty.inc'   # .cmd
ANDRE_CNF_FILLED='andre-default-filled.inc' # .cmd
FORMAT="--formatFile $ANDRE_CNF_TEMPLATE --formatOut $ANDRE_CNF_FILLED "

echo 
eval "cat > $ANDRE_CNF_EMPTY <<EOF
`cat $ANDRE_CNF_TEMPLATE`
EOF"



# Run with implicit default values (ie. '' arguments)
source $ANDRE_CNF_EMPTY
if [ $? != 0 ]; then
    echo failed $0
    exit 1
fi

fgrep -v 'FORMAT' $ANDRE_CNF_FILLED  > tmp.inc
mv tmp.inc $ANDRE_CNF_FILLED
cat "$ANDRE_CNF_FILLED"

#cp andre.inc andre.tmp
