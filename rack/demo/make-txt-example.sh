#!/bin/bash
#
# Markus.Peura@fmi.fi

if [ $# == 0 ]; then
    echo "Given text data, creates a shortened excerpt with extension '.inc'"
    echo "Example:"
    echo "$0 out.dat"
    exit 1
fi

INFILE=$1
OUTFILE=${INFILE%.*}.inc

# Pick header (all of it)
head -20 $INFILE | fgrep '#' > $OUTFILE

# Pick 5 first lines of data
fgrep -v '#' $INFILE | head -5 >> $OUTFILE

# Add clip-clip marker
echo -e  '...\n\t...\n\t\t...' >> $OUTFILE

# Pick 5 last lines of data
tail  -5 $INFILE >> $OUTFILE

echo "Created $OUTFILE"
