Rack fmi.fi - a radar data processing program

--aBiomet  <reflMax>,<maxAltitude>,<reflDev>,<devAltitude>
  Detects birds and insects.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: DBZH$

--aBird  <dbzPeak>,<vradDevMin>,<rhoHVmax>,<zdrDevMin>,<windowWidth>,<windowHeight>
  Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^(DBZH|VRAD|VRADH|RHOHV|ZDR)$

--aCCor  <reflHalfWidth>
  Detects clutter. Based on difference of DBZH and TH. At halfwidth PROB=0.5. Universal.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^(TH|DBZH)$

--aClassThreshold  <threshold>
  Quality index value below which also CLASS information will be updated.

--aClutterMap  <decay>
  Reads a ground clutter map and scales it to sweeps.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: 

--aClutterMapRead  <filename>
  Read a file containing CLUTTERMAP quantity.

--aDamper  <threshold>,<undetectThreshold>,<dbzMin>
  Removes anomalies with fuzzy deletion operator.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--aEmitter  <lengthMin>,<thicknessMax>,<sensitivity>
  Detects electromagnetic interference segments by means of window medians.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--aGapFill  <width>,<height>
  Removes low-quality data with gap fill based on distance transformation.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--aGapFillRec  <width>,<height>,<loops>
  Recursive, 'splinic' gap filler.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--aHydroClassBased  <nonMet>,<NOMET>
  Sets probabilities of non-meteorological echoes based on precomputed hydrometeor CLASS.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: HCLASS

--aInsect  <dbzPeak>,<vradDevMin>,<rhoHVmax>,<zdrDevMin>,<windowWidth>,<windowHeight>
  Estimates probability from DBZH, VRAD, RhoHV and ZDR.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^(DBZH|VRAD|VRADH|RHOHV|ZDR)$

--aJamming  <smoothnessThreshold>,<distanceMin>,<weightLower>,<debugRow>
  Detects broad lines caused by electromagnetic interference. Intensities should be smooth, increasing by distance.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--aNoise  <minRange>,<minLength>,<sensitivity>
  Detects sectors contaminated by electromagnetic interference.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--aPrecip  <probMax>,<dbz>,<dbzSpan>
  Detects precipitation...
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: DBZH$

--aQualityCombiner  
  Combines detection probabilities to overall quality field QIND (and CLASS).
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0,
  # Input quantities: ^DBZH$

--aRemover  <threshold>
  Simple anomaly removal operator.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--aRhoHVLow  <threshold>,<thresholdWidth>,<windowWidth>,<windowHeight>,<medianPos>
  Detects clutter. Based on dual-pol parameter RhoHV . Optional postprocessing: morphological closing. Universal.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: RHOHV

--aShip  <reflMin>,<reflDev>,<windowWidth>,<windowHeight>
  Detects ships based on their high absolute reflectivity and local reflectivity difference.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--aSpeckle  <reflMin>,<area>
  Detects speckle noise. Universal: uses DBZ data as input but applies to all data in a sweep group.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--aSun  <width>,<sensitivity>
  Draw the sun beam
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--aTime  <time>,<decayPast>,<decayFuture>
  Created quality field based on measurement time for each beam.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: ^[A-Z]+$

--aUniversal  
  Toggle the support for universal ie. Dataset-wide quality indices.

For help on separate modules, type:
 --help [<command>]
 --help cart
 --help formulae
 --help imageOps
 --help prod
