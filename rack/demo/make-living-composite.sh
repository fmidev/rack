#!/bin/bash

WGET='wget --proxy=off --no-clobber'
VENISON='http://localhost:8080/venison/Venison'
#DATE=20140924
YEAR=2014
MONTH=09
DAY=24
CACHE=~/venison/cache/$YEAR/$MONTH/$DAY

SITES=fikor,fivan,fiika
# ~/venison/cache/2014/09/24/radar/raw/fikor/201409241200_radar.raw.fikor.h

#for HOUR in {0,1}{0,1,2,3,4,5,6,7,8,9} 2{0,1,2,3}; do # ALL
#for HOUR in ${HOURS[*]}; do
#for HOUR in 2{0,1,2,3}  1{0,1,2,3,4,5,6,7,8,9} ; do
for HOUR in 10 11 12 13 14; do
    for MINUTE in 00 15 30 45; do
#    for MINUTE in {0,1,2,3,4,5}{0,5}; do
#    for MINUTE in 00 30; do
	TIME=$YEAR$MONTH$DAY$HOUR$MINUTE
	for SITE in ${SITES//,/ }; do
	    FILE=${TIME}_radar.raw.$SITE.h5
	    $WGET $VENISON?product=$FILE
	    VOLUMES=(`ls -1 $CACHE/radar/raw/?????/${TIME}_radar.raw.$SITE.h5`)
	    echo ${VOLUMES[*]}
	done
    echo 
  done
done