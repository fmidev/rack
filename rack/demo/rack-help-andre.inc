Rack fmi.fi - a radar data processing program

--aBiomet  <reflMax>,<maxAltitude>,<reflDev>,<devAltitude>
  Detects birds and insects.

--aBird  <dbzPeak>,<vradDevMin>,<rhoHVmax>,<zdrDevMin>,<windowWidth>,<windowHeight>
  Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.

--aCCor  <reflHalfWidth>
  Detects clutter. Based on difference of DBZH and TH. At halfwidth PROB=0.5. Universal.

--aClassThreshold  <threshold>
  Quality index value below which also CLASS information will be updated.

--aClutterMap  <decay>
  Reads a ground clutter map and scales it to sweeps.

--aClutterMapRead  <filename>
  Read a file containing CLUTTERMAP quantity.

--aDamper  <threshold>,<undetectThreshold>,<dbzMin>
  Removes anomalies with fuzzy deletion operator.

--aDopplerNoise  <vradDevMin>,<windowWidth>,<windowHeight>
  Detects variance Doppler (VRAD).

--aEmitter  <lengthMin>,<thicknessMax>,<sensitivity>
  Detects electromagnetic interference segments by means of window medians.

--aGapFill  <width>,<height>
  Removes low-quality data with gap fill based on distance transformation.

--aGapFillRec  <width>,<height>,<loops>
  Recursive, 'splinic' gap filler.

--aHydroClassBased  <nonMet>,<NOMET>
  Sets probabilities of non-meteorological echoes based on precomputed hydrometeor CLASS.

--aInsect  <dbzPeak>,<vradDevMin>,<rhoHVmax>,<zdrDevMin>,<windowWidth>,<windowHeight>
  Estimates probability from DBZH, VRAD, RhoHV and ZDR.

--aJamming  <smoothnessThreshold>,<distanceMin>,<weightLower>,<debugRow>
  Detects broad lines caused by electromagnetic interference. Intensities should be smooth, increasing by distance.

--aPrecip  <probMax>,<dbz>,<dbzSpan>
  Detects precipitation...

--aQualityCombiner  
  Combines detection probabilities to overall quality field QIND (and CLASS).

--aRemover  <threshold>
  Simple anomaly removal operator.

--aRhoHVLow  <threshold>,<thresholdWidth>,<windowWidth>,<windowHeight>,<medianPos>
  Detects clutter. Based on dual-pol parameter RhoHV . Optional postprocessing: morphological closing. Universal.

--aShip  <reflMin>,<reflDev>,<windowWidth>,<windowHeight>
  Detects ships based on their high absolute reflectivity and local reflectivity difference.

--aSpeckle  <reflMin>,<area>
  Detects speckle noise. Universal: uses DBZ data as input but applies to all data in a sweep group.

--aSun  <width>,<sensitivity>
  Draw the sun beam

--aTime  <time>,<decayPast>,<decayFuture>
  Created quality field based on measurement time for each beam.

--aUniversal  
  Toggle the support for universal ie. Dataset-wide quality indices.

For help on separate modules, type:
 --help [<command>]
 --help cart
 --help formulae
 --help imageOps
 --help prod
