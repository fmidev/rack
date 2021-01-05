#!/bin/bash



# Functions START, TITLE, TEST, REQUIRE, EXCLUDE
source content-utils.sh

INFILE='volume.h5'
OUTFILE='volume-modified.h5'


START example-select.inc
OUTFILE='volume-select.txt'

TITLE "For text output, select paths STARTING with 'dataset2', 'dataset3', ..., 'dataset5'. "
TEST    --select /dataset2:5
REQUIRE dataset2/where
EXCLUDE dataset1/data3

TITLE "For text output, select paths ENDING with 'what', 'where', or 'how'. "
TEST    --select "'what|where|how'"
REQUIRE dataset1/where
EXCLUDE data1

TITLE "For text output, select 'where' groups of 3 lowest elevations from 0.5 degrees upwards "
TEST    --select where,elangle=0.5,count=3
REQUIRE dataset3/where
EXCLUDE data1

TITLE "For text output, select paths of elevations up to 6.0 degrees "
TEST    --select elangle=:6.0
REQUIRE dataset3/where
EXCLUDE data1

TITLE "For text output, select what and where groups of elevations from 0.5 to 6.0 degrees"
TEST    --select "'what|where,elangle=0.5:6'"
REQUIRE dataset4/{data1,where}
EXCLUDE dataset1 data1

TITLE "For single-image output, select DBZH  "
OUTFILE='volume-DBZH.png'
TEST    --select 'elangle=1.5,quantity=DBZH'
REQUIRE dataset3/data.
EXCLUDE dataset1/ data1/

TITLE "For a Pseudo CAPPI product, change default input selection"
OUTFILE='pCappi-TH-1500m.h5'
TEST    --select "'quantity=^TH$,elangle=1.5:10,count=3' --pCappi 1500"
REQUIRE 'dataset1/data1/what:quantity="TH"'


# rack volume.h5 --select 'dataset=2:5,quantity=DBZH'    
# rack volume.h5 --select 'quantity=DBZH,elangle=0.5:4.0'   <commands>
# rack volume.h5 --select 'dataset=2:5,quantity=DBZH'    
# rack volume.h5 --select 'quantity=DBZH,elangle=0.5:4.0'   <commands>

START example-assign.inc
OUTFILE='volume-modified.h5'

TITLE "Set beam resolution"
TEST    --/dataset1/where:rscale=500
REQUIRE    dataset1/where:rscale=500


TITLE "Set date and time (note string format)"
TEST   '--/what:date="20140827" --/what:time="093000"'
REQUIRE   'what:date="20140827"'
REQUIRE   'what:time="093000"'

TITLE "Set freezing level"
TEST    --/how:freeze=2.10
REQUIRE    how:freeze=2.1

TITLE "Set radar location"
TEST  --/where:lat=64.7749 --/where:lon=26.3189
REQUIRE where:lat=64.7749
REQUIRE where:lon=26.31




START example-delete.inc

#INFILE='volume.h5'
TITLE "Delete dataset[i] groups 2...10 "
TEST    --delete dataset2:10
REQUIRE dataset1/{data1,what,where}
EXCLUDE dataset2 dataset10

TITLE "Delete data[i] groups 3...20"
TEST    --delete data3:20
REQUIRE dataset1
EXCLUDE dataset1/data3 dataset1/data10

TITLE "Delete data[i] groups 3...20, in dataset[i] groups 2...10"
TEST    --delete dataset2:10/data3:20
REQUIRE dataset1 dataset2/data2 
EXCLUDE dataset2/data3

TITLE "Delete independently dataset[i] and data[i] groups with two commands"
TEST    --delete dataset2:6 --delete data2:5 
REQUIRE dataset1/data1 
EXCLUDE dataset1/data2
EXCLUDE dataset2/data1

#TITLE "Delete data[i] groups containing quantity DBZH"
#TEST    --delete quantity=DBZH
#REQUIRE dataset.*/data.*/ 

TITLE "Delete data[i] groups containing VRAD"
TEST    --delete quantity=VRAD
REQUIRE dataset.*/data.*/ 
EXCLUDE dataset2/data3  # VRAD here in volume.h5

TITLE "Delete data[i] groups containing VRAD in 1.0 degree sweeps and above"
TEST    --delete elangle=1.0:90,quantity=VRAD 
REQUIRE dataset.*/data.*/
REQUIRE dataset1/data3 
EXCLUDE dataset3/data3  # VRAD in data3 in volume.h5


TITLE "Delete actual data arrays"
TEST    --delete data
REQUIRE dataset.*/data.*/
EXCLUDE dataset.*/data.*/data

INFILE='volume-detected.h5'
TITLE "Delete quality[i] groups "
TEST    --delete quality:
REQUIRE dataset1 dataset1/data1 
EXCLUDE dataset1/quality dataset1/data1/quality




START example-keep.inc

INFILE='volume.h5'

TITLE "Keep data[i] groups containg DBZH"
TEST    --keep quantity=DBZH
REQUIRE dataset1/data2/{data,what}
REQUIRE dataset1/{what,where}
EXCLUDE dataset1/data1 # TH here in volume.h5

TITLE "Keep dataset[i] groups containing (at least) VRAD"
TEST    --keep quantity=VRAD
#,groups=dataset:data
REQUIRE dataset.*/data3/data

TITLE "Keep dataset[i] group with elangle 5.0 degrees, at lowest"
TEST    --keep elangle=5.0:90.0
REQUIRE dataset.*/data1
EXCLUDE dataset1/

TITLE "Keep three dataset[i] groups with lowest elangle "
TEST    --keep elangle=-90.0:90.0,count=3
REQUIRE dataset.*
EXCLUDE dataset4/

INFILE='volume-detected.h5'

TITLE "Keep quality groups only"
TEST    --keep quality
REQUIRE dataset1/quality{1,2}  dataset1/data1/quality{1,2}
REQUIRE dataset1/{what,where}
#REQUIRE dataset1/data1/what
EXCLUDE dataset1/data1/data

TITLE "Keep quality1 and quality2 groups"
TEST    --keep quality1:2
REQUIRE dataset1/quality{1,2}  dataset1/data1/quality{1,2}
REQUIRE dataset1/{what,where}
EXCLUDE dataset1/data1/data

TITLE "Keep data specific quality groups"
TEST    --keep data:/quality
REQUIRE dataset1/data1/quality{1,2}
REQUIRE dataset1/{what,where}
EXCLUDE dataset1/quality{1,2}
EXCLUDE dataset1/data1/data


TITLE "Keep dataset[] specific quality groups"
TEST    --keep dataset:/quality
REQUIRE dataset1/quality{1,2}
REQUIRE dataset1/{what,where}
REQUIRE dataset1/quality
EXCLUDE dataset1/data1


START example-move.inc
INFILE='volume-detected.h5'

TITLE "Move group"
TEST --move dataset1/quality1,dataset1/data1/quality3
REQUIRE dataset1/data1/quality3
EXCLUDE dataset1/quality1


TITLE "Rename only the attribute key"
TEST --move dataset1/how:task,:newtask
REQUIRE dataset1/how:newtask
EXCLUDE dataset1/how:task

TITLE "Move attribute in hierarchy"
TEST  --move dataset1/how:task,what
REQUIRE what:task
EXCLUDE dataset1/how:task


TITLE "Move and rename attribute"
TEST --move dataset1/where:nrays,dataset1/how:imageheight
REQUIRE dataset1/how:imageheight
EXCLUDE dataset1/where:nrays
