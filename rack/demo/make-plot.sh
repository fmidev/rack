BBOX='17,57.75,32.75,70'
SIZE='500,750'

rack  --cProj '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs' --cBBox $BBOX  --cSize $SIZE  \
  --cQuantity RATE --cPlotFile measurements.txt \
  --encoding C,0.01,undetect=1,nodata=0 \
  --cExtract dw \
  -o plot.png \
  --iDistanceTransformFill 5 \
  -o plot-spread1.png  \
  --iGaussianAverage 25 \
  --iAverage 25 \
  --iGaussianAverage 25 \
  -o plot-spread2.png  \
  --palette palette-RATE.txt --paletteRefine 64 --encoding 'C,0.2,-32,1,100' --imageAlpha -o plot-spread-color.png


