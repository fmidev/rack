 #!/bin/bash

outfile=out.kml
dir=kml

# --format <string contaning ${VARIABLES}>                ()
#         Sets a format string.
# --formatFile <filename>         ()
#         Reads a format string form a file.
# --formatOut <filename>|-                ()
#         Dumps the formatted string to a file or stdout.

if (( $# == 0)); then 
   echo usage: $0 
   exit
else 
    if (( $# == 1)); then 
	basename=${1##*/}
	outkml=$dir/${basename%.*}.kml
	outpng=$dir/${basename%.*}.png
    fi
fi

mkdir -p $dir

echo "Writing file $outkml"
echo "<kml>" > $outkml

while [[ $1 != '' ]]; do
    echo $1
    echo rack $1 -c --image 1 -o $outpng --formatFile make-kml.kml --formatOut tmp.kml
    cat tmp.kml >> $outkml
    echo >> $outkml
    shift
done

echo "</kml>" >> $outfile



