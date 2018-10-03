Rack fmi.fi - a radar data processing program

--iAdd  <scale>,<bias>,<LIMIT>
  Adds values

--iAverage  <width>,<height>
  Window averaging with optional weighting support.

--iBlender  <width>,<height>,<smooth>,<mix>,<loops>
  Smoothes image repeatedly, mixing original image with the result at each round.

--iCatenate  <scale>,<offset>
  Catenates images

--iCopy  <dstView>,<scale>,<bias>,<LIMIT>
  Copies current view to: f=full image, i=image channels, a=alpha channel(s), 0=1st, 1=2nd,...

--iCrop  <width>,<height>,<i>,<j>
  Crop image.

--iDistance  <functor>,<params>
  Computes the distance of pixel vectors. Post-scaling with desired functor.

--iDistanceTransform  <width>,<height>,<topology>
  Linearly decreasing intensities. Set decrements.

--iDistanceTransformExp  <width>,<height>,<topology>
  Exponentially decreasing intensities. Set half-decay radii.

--iDistanceTransformFill  <width>,<height>,<topology>
  Spreads intensities linearly up to distance defined by alpha intensities.

--iDistanceTransformFillExp  <width>,<height>,<topology>
  Spreads intensities exponentially up to distance defined by alpha intensities.

--iDiv  <scale>,<bias>,<LIMIT>
  Rescales intensities linerarly

--iFloodFill  <i>,<j>,<min>,<max>,<value>
  Fills an area starting at (i,j) having intensity in [min,max], with a value.

--iFlowAverage  <width>,<height>
  

--iFuzzyBell  <location>,<width>,<scale>,<bias>,<LIMIT>
  Fuzzy bell function.

--iFuzzyBell2  <location>,<width>,<scale>,<bias>,<LIMIT>
  Fuzzy Gaussian-like bell function.

--iFuzzyStep  <startPos>,<endPos>,<scale>,<bias>,<LIMIT>
  Fuzzy step function.

--iFuzzyTriangle  <startPos>,<peakPos>,<endPos>,<scale>,<bias>,<LIMIT>
  Fuzzy triangle function.

--iGamma  <gamma>,<LIMIT>
  Gamma correction for brightness.

--iGaussianAverage  <width>,<height>,<radius>
  Gaussian blur implemented as quick Wx1 and 1xH filtering.

--iGradient  <radius>,<LIMIT>
  Computes horizontal and vertical derivatives: df/di and df/dj.

--iGradientHorz  <radius>,<LIMIT>
  Horizontal intensity difference

--iGradientVert  <radius>,<LIMIT>
  Vertical intensity difference

--iHighBoost  <width>,<height>,<coeff>
  Mixture of original and high-pass filtered image

--iHighPass  <width>,<height>,<scale>,<offset>,<LIMIT>
  High-pass filter for recognizing details.

--iHistogram  <width>,<height>,<valueFunc>,<percentage>,<bins>
  A pipeline implementation of window histogram; valueFunc=[asmdvNX] (avg,sum,median,stddev,variance,miN,maX)

--iImpulseAvg  <decay>,<extendHorz>,<extendVert>,<weightThreshold>
  yes

--iLaplace  <radius>,<LIMIT>
  Second intensity derivatives, (df/di)^2 and (df/dj)^2

--iLaplaceHorz  <radius>,<LIMIT>
  Second horizontal differential

--iLaplaceVert  <radius>,<LIMIT>
  Second vertical differential

--iMagnitude  <functor>,<params>
  Computes the magnitude of a pixel vector. Post-scaling with desired functor.

--iMarginStat  <mode>,<stat>,<medianPos>
  Computes statistics on <horz> or <vert> lines: average,sum,median,stdDev,variance,miN,maX

--iMax  
  Maximum of two values.

--iMedian  <width>,<height>,<percentage>,<bins>
  A pipeline implementation of window median.

--iMin  
  Minimum of two values.

--iMix  <coeff>,<scale>,<bias>,<LIMIT>
  Rescales intensities linerarly

--iMul  <scale>,<bias>,<LIMIT>
  Rescales intensities linerarly

--iNegate  
  Inverts values.

--iProduct  <functor>,<params>
  Computes the dot product of pixel vectors. Post-scaling with desired functor.

--iQualityMixer  <coeff>
  Weighted blending of two images.

--iQualityOverride  <advantage>,<decay>
  Compares two images, preserving pixels having higher alpha value. Src alpha is pre-multiplied with advantage.

--iQualityThreshold  <threshold>,<replace>
  Threshold data with quality

--iQuantizator  <bits>,<LIMIT>
  Quantize to n bits. (For integer images)

--iRecursiveRepairer  <width>,<height>,<loops>,<smoother>
  Applies weighted averaging window repeatedly, preserving the pixels of higher weight.

--iRemap  <fromValue>,<toValue>,<LIMIT>
  Rescales intensities linerarly

--iRescale  <scale>,<bias>,<LIMIT>
  Rescales values linerarly: y = scale*x + bias

--iResize  <width>,<height>,<interpolation>,<scale>
  Resize geometry and scale intensities

--iRunLengthHorz  <threshold>
  Computes lengths of segments of intensity above threshold.

--iRunLengthVert  <threshold>
  Computes lengths of segments of intensity above threshold.

--iSegmentArea  <min>,<max>,<functor>
  Computes segment sizes.

--iSegmentStats  <statistics>,<min>,<max>,<functor>
  Segment statistics: area, mx, my, variance, slimness, horizontality, verticality, elongation

--iSub  <scale>,<bias>,<LIMIT>
  Subtracts values

--iThreshold  <threshold>,<replace>,<LIMIT>
  Rescales intensities linerarly

--iThresholdBinary  <threshold>,<replace>,<replaceHigh>,<LIMIT>
  Rescales intensities linerarly

--iTranspose  
  Flips image matrix around its corner.

For help on separate modules, type:
 --help [<command>]
 --help andre
 --help cart
 --help formulae
 --help prod
