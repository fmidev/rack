Rack fmi.fi - a radar data processing program

--pAdd  
  Add polar data to accumulation array.

--pAddWeighted  <weight>
  Adds the current product to the composite applying weight.

--pBeamAltitude  <altitudeReference>
  Computes the altitude at each bin

--pBird  <dbzPeak>,<vradDevMin>,<rhoHVmax>,<zdrDevMin>,<windowWidth>,<windowHeight>
  Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.

--pCappi  <altitude>,<weightMin>,<weightExponent>,<aboveSeaLevel>
  Constant-altitude planar position indicator

--pConv  <maxEchoThreshold>,<cellDiameter>,<echoTopThreshold>,<echoTopDBZ>,<smoothAzm>,<smoothRad>
  Computes the probability of convection based on fuzzy cell intensity, area and height.

--pDoppler  <azm1>,<azm2>,<range1>,<range2>,<ray1>,<ray2>,<bin1>,<bin2>
  Projects Doppler speeds to unit circle. Window corners as (azm,range) or (ray,bin)

--pDopplerAvg  <width>,<height>,<threshold>,<compensate>,<relativeScale>
  Smoothens Doppler field

--pDopplerDeAlias  <width>,<height>,<nyquist>,<altitudeWeight>,<VVP>,<matchAliased>
  Inverts Doppler speed to (u,v), optionally de-aliases.

--pDopplerDev  <width>,<height>,<threshold>,<compensate>,<relativeScale>
  Computes std.dev.[m/s] of Doppler field

--pDrawing  <shape>,<p1>,<p2>,<p3>,<p4>,<marker>
  Visualise a direction[azm,r,r2], range[r,r2], sector[azm,azm2,r,r2] or arc[azm,azm2,r].

--pEchoTop  <minDBZ>,<dbzCoord>,<altitudeCoord>,<altitudeReference>
  Computes maximum (or minimum) altitude of echo.

--pExtract  <channels>
  Extract polar-coordinate data that has been accumulated.

--pFunctor  <ftor>
  Computes the altitude at each bin

--pMaxEcho  <altitude>,<devAltitude>,<accumulationMethod>
  Computes...

--pPseudoRhi  <az_angle>,<xsize>,<ysize>,<minRange>,<range>,<minHeight>,<maxHeight>,<beamWidth>,<beamPowerThreshold>
  Computes vertical intersection in a volume in the beam direction.

--pRainRate  
  Estimates precipitation rate [mm/h] from reflectance [dBZ].

--pRainRateDP  <freezingLevel>,<freezingLevelThickness>,<dataThresholdParams>,<dbzParams>,<kdpParams>,<zzdrParams>,<kdpzdrParams>
  Estimates on precipitation rate [mm/h] from dual-pol parameters.

--pRainRateDPSimple  <freezingLevel>,<freezingLevelThickness>,<dataThresholdParams>,<dbzParams>,<kdpParams>
  Estimates on precipitation rate [mm/h] from dual-pol parameters.

--pSunShine  <timestamp>,<quantity>
  Computes the sun position

--pVerticalProfile  <minRange>,<range>,<minHeight>,<maxHeight>,<levels>,<startaz>,<stopaz>,<azSlots>
  Computes vertical dBZ distribution in within range [minRange,maxRange] km.

For help on separate modules, type:
 --help [<command>]
 --help andre
 --help cart
 --help formulae
 --help imageOps
