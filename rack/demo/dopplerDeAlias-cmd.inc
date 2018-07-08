--pDopplerDeAlias  <width>,<height>,<nyquist>,<altitudeWeight>,<VVP>
  Solves Doppler speed ambiguity, outputs (u,v).
  # Encoding: gain=0,nodata=0,offset=0,type=S,undetect=0, user modifiable: type,gain,offset,undetect,nodata,rscale,nrays,nbins
  # Input quantities: VRAD
	width=500 [metres]
	height=3 [degrees]
	nyquist=0 [m/s]
	altitudeWeight= [Functor:a:b:c...]
	VVP=0 [0|1]
