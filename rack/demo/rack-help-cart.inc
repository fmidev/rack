Rack - a radar data processing program

--cAdd  
  Maps the current polar product to a Cartesian product.

--cAddWeighted  <weight>
  Adds current Cartesian product to the composite applying weight.

--cBBox  <llLon>,<llLat>,<urLon>,<urLat>
  Bounding box of the Cartesian product.

--cBBoxReset  
  Resets the bounding box (to be set again according to the next radar data).

--cBBoxTest  <mode>
  Tests whether the radar range is inside the composite.

--cBBoxTile  <llLon>,<llLat>,<urLon>,<urLat>
  Redefines bbox and compositing array size for several radars, applying original projection and resolution. See --cSize, --cBBox, --cProj.

--cCreate, -c  
  Maps the current polar product to a Cartesian product.

--cCreateTile  
  Maps the current polar product to a tile to be used in compositing.

--cDefaultQuality  <weight>
  Set default quality (for data without quality field)

--cExtract  <value>
  Extract data that has been composited on the accumulation array

--cGrid  <lonSpacing>,<latSpacing>,<width>,<intensity>
  Draw lat-lon grid onto a Cartesian product.

--cInit  
  Allocate memory to --cSize, applying --target and --select, if set.

--cMethod  <method>
  Method to be used in accumulating the (weighted) values.

--cPlot  <lon>,<lat>,<x>,<w>
  Add a single data point.

--cPlotFile  <file>
  Plot file containing rows '<lat> <lon> <value> [weight] (skipped...)'.

--cProj  <projstr>
  Set projection

--cSize  <width>,<height>
  Set size of the compositing array. Does not allocate memory.

--cSpread  <horz>,<vert>,<loops>
  Set Spread of the compositing array.

--cTime  <time>
  Modify the time of the current composite. See --cTimeDecay 

--cTimeDecay  <decay>
  Delay weight (0.0...1.0) per minute. 1=no decay. See --cTime

For help, type:
 --help [<command>]
 --help andre
 --help formulae
 --help prod
