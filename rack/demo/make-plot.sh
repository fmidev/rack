BBOX='17,57.75,32.75,70'
SIZE='500,750'

rack --cProj '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs' --cBBox $BBOX  --cSize $SIZE  \
  --cPlotFile measurements.txt \
  --encoding C,0.01 --cExtract dw \
  --/dataset1/data1/what:quantity=RATE --/what:date=20120808 --/what:time=1200  \
  -o plot.png --cSpread 50  -o plot-spread.png  \
  --cSpread 100 --palette palette-RATE.txt --encoding 'C,0.2,-32,1,100' --imageAlpha -o plot-spread-color.png
