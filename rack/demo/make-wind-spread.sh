#!/bin/bash


rack --/dataset1/data1/data:image=1,1 --cSize 500,500 -c --/dataset1/data1/what:quantity=DBZH -o empty.h5
rack vectors.h5 -Q AMVU -c -Q AMVV -c -o vectors.txt

rack vectors.txt -o empty.h5

rack empty.h5 -Q AMV* --plot 250,255,255,255 -Q AMVU --iGaussianAverage 5,3  -o koe.png
