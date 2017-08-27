#!/bin/bash

# Creates a profile curve, saves it in text file and draws it with gnuplot.
# Additionally, creates a CAPPI image and catenates it with the profile image.

# Under construction...

FILE=${FILE:-'volume.h5'}
rack $FILE --pVerticalProfile 40,200,0,11000,100,0,360,1 -o profile.mat

#rack $FILE --pCappi 500 --cSize 500,500 -c --palette palette-DBZH.txt -o profile-cappi.png

# Pick some lines for text file illustration.
head -6 profile.mat >  profile-cut.mat
echo '...'          >> profile-cut.mat
tail -3 profile.mat >> profile-cut.mat


gnuplot <<EOF
set title 'Vertical profile (VAN r=40...200km, h=0...11000m)';
set xrange [-30:50]; 
set ylabel 'HGHT';
set terminal png size 480,500;
set output 'profile-curve.png' 
plot 'profile.mat' using 2:1 with lines title "DBZH" , 'profile.mat' using 3:1 with lines title "QIND" , 'profile.mat' using 4:1 with lines title "COUNT"
exit
EOF

# needed?
#convert +append profile-cappi.png profile-curve.png profile-image.png
