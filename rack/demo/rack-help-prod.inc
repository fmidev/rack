Rack - a radar data processing program

--pAdd  
  Add polar data to accumulation array.

--pBeamAltitude  <altitudeReference>
  Computes the altitude at each bin
  # Encoding: gain=0.1,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: 

--pBird  <dbzPeak>,<vradDevMin>,<rhoHVmax>,<zdrDevMin>,<windowWidth>,<windowHeight>
  Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: ^(DBZH|VRAD|RHOHV|ZDR)$

--pCappi  <altitude>,<weightMin>,<weightExponent>,<aboveSeaLevel>
  Constant-altitude planar position indicator
  # Encoding: gain=0,nodata=0,offset=0,type=,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--pConv  <maxEchoThreshold>,<cellDiameter>,<echoTopThreshold>,<echoTopDBZ>,<smoothAzm>,<smoothRad>
  Computes the probability of convection based on fuzzy cell intensity, area and height.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins,undetect,nodata
  # Input quantities: ^DBZH$

--pDoppler  <azm1>,<azm2>,<range1>,<range2>,<ray1>,<ray2>,<bin1>,<bin2>
  Projects Doppler speeds to unit circle. Window corners as (azm,range) or (ray,bin)
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: VRAD

--pDopplerAvg  <width>,<height>,<areaThreshold>,<compensate>
  Smoothens Doppler field
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: gain
  # Input quantities: VRAD

--pDopplerDeAlias  <width>,<height>,<nyquist>,<altitudeWeight>
  Solves Doppler speed ambiguity, outputs (u,v).
  # Encoding: gain=0,nodata=0,offset=0,type=S,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: VRAD

--pDopplerDev  <width>,<height>,<areaThreshold>,<compensate>
  Computes std.dev.[m/s] of Doppler field
  # Encoding: gain=0.5,nodata=0,offset=0,type=C,undetect=0, user modifiable: gain
  # Input quantities: VRAD

--pDrawing  <shape>,<p1>,<p2>,<p3>,<p4>,<marker>
  Visualise a direction[azm,r,r2], range[r,r2], sector[azm,azm2,r,r2] or arc[azm,azm2,r].
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: 

--pEchoTop  <minDBZ>,<dbzCoord>,<altitudeCoord>,<altitudeReference>
  Computes maximum (or minimum) altitude of echo.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--pExtract  <channels>
  Extract polar-coordinate data that has been accumulated.

--pFunctor  <ftor>
  Computes the altitude at each bin
  # Encoding: gain=0.1,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: 

--pMaxEcho  <altitude>,<devAltitude>,<accumulationMethod>
  Computes...
  # Encoding: gain=0,nodata=0,offset=0,type=,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--pPseudoRhi  <az_angle>,<xsize>,<ysize>,<minRange>,<range>,<minHeight>,<maxHeight>,<beamWidth>,<beamPowerThreshold>
  Computes vertical intersection in a volume in the beam direction.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset
  # Input quantities: ^DBZH$

--pRainRate  
  Estimates precipitation rate [mm/h] from reflectance [dBZ].
  # Encoding: gain=0,nodata=0,offset=0,type=S,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: ^DBZH$

--pRainRateDP  <freezingLevel>,<freezingLevelThickness>,<dataThresholdParams>,<dbzParams>,<kdpParams>,<zzdrParams>,<kdpzdrParams>
  Estimates on precipitation rate [mm/h] from dual-pol parameters.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset
  # Input quantities: ^(RHOHV|KDP|DBZH|ZDR|QIND)$

--pRainRateDPSimple  <freezingLevel>,<freezingLevelThickness>,<dataThresholdParams>,<dbzParams>,<kdpParams>
  Estimates on precipitation rate [mm/h] from dual-pol parameters.
  # Encoding: gain=0,nodata=0,offset=0,type=C,undetect=0, user modifiable: type,gain,offset
  # Input quantities: ^(RHOHV|KDP|DBZH|HCLASS|QIND)$

--pSunShine  <timestamp>,<quantity>
  Computes the sun position
  # Encoding: gain=0.005,nodata=0,offset=-0.1,type=C,undetect=0, user modifiable: type,gain,offset,rscale,nrays,nbins
  # Input quantities: 

--pVerticalProfile  <minRange>,<range>,<minHeight>,<maxHeight>,<levels>,<startaz>,<stopaz>,<azSlots>
  Computes vertical dBZ distribution in within range [minRange,maxRange] km.
  # Encoding: gain=0,nodata=0,offset=0,type=d,undetect=0, user modifiable: type
  # Input quantities: ^DBZH$

For help, type:
 --help [<command>]
 --help andre
 --help cart
 --help formulae
