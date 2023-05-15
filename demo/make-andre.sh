#!/bin/bash

VOLUME=${VOLUME:'volume.h5'}

# Andre command with parameters embedded as ${parameter}
ANDRE_CNF_TEMPLATE='andre-default.cmd.tpl'
ANDRE_CNF_EMPTY='andre-default-empty.and'   # .cmd
ANDRE_CNF_FILLED='andre-default-filled.and' # .cmd
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

fgrep -v 'FORMAT' $ANDRE_CNF_FILLED  > tmp.and
mv tmp.and $ANDRE_CNF_FILLED
cat "$ANDRE_CNF_FILLED"

#cp andre.and andre.tmp
