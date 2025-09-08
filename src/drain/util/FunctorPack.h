/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
*/
#ifndef FUNCTOR_PACK_H_
#define FUNCTOR_PACK_H_

#include <cmath>

#include "Functor.h"


namespace drain
{




/// Rescales intensities linearly: f' = scale*f + offset.
/**
    Scale determines the contrast, offset determines the brightness of the resulting image.
	\code
	drainage image-gray.png  --iRescale 0.5              -o scale-dark.png
	drainage image-gray.png  --target S --iRescale 128   -o scale-dark16.png
	drainage image-gray.png  --iRescale 0.5,128          -o scale-dim.png
	drainage image.png --iRescale 0.5,128          -o scale-dim-image.png
	drainage image-gray.png  --iRescale 2.0,-128,LIMIT=1 -o scale-contrast.png
	\endcode
 */
// Inversely: f = (f'-offset)/scale = a*f+b, where a=1/scale and b=-offset/scale.
class ScalingFunctor : public UnaryFunctor {

public:

	ScalingFunctor(double scale = 1.0, double bias = 0.0) : UnaryFunctor(__FUNCTION__, "Rescales values linerarly: y = scale*x + bias", scale, bias){
		this->getParameters().link("scale", this->scale);
		this->getParameters().link("bias", this->bias);
		this->setScale(scale, bias);
	};

	ScalingFunctor(const ScalingFunctor & ftor) : UnaryFunctor(ftor) {
		this->parameters.copyStruct(ftor.getParameters(), ftor, *this);
		updateBean();
	}

	//virtual
	inline
	double operator()(double s) const {
		return this->scaleFinal*s + this->biasFinal;
	};

protected:

	ScalingFunctor(const std::string & name, const std::string & description, double scale = 1.0, double bias = 0.0) :
		UnaryFunctor(name, description, scale, bias){
		this->setScale(scale, bias);
	};

};



/// Inverts intensities: f' = f_max - f
/**
   Inverts image by subtracting the pixel intensities from the maximum intensity (255, 65535 or 1.0).
	\code
	drainage image-gray.png  --iNegate -o negate.png
	drainage image.png --iNegate -o negate-image.png
	\endcode
 */
class NegateFunctor : public ScalingFunctor {

public:

	NegateFunctor() : ScalingFunctor(__FUNCTION__, "Inverts values.", -1.0, 1.0) {
		//updateScale(); // needed?
	}



};

/// Maps a single intensity value to another value.
/**
	\code
	drainage image-gray.png  --iRemap 255,32       -o remap-abs.png
	drainage image-gray.png  --physicalRange 0:1 --iRemap 1,0.5  -o remap.png
	drainage image.png --physicalRange 0:1 --iRemap 1,0.5  -o remap-image.png
	\endcode
 */
class RemappingFunctor : public UnaryFunctor {

public:

	RemappingFunctor(double fromValue = 0.0, double toValue = 0.0) : UnaryFunctor(__FUNCTION__, "Rescales intensities linerarly") , fromValue(fromValue), toValue(toValue) {
		this->getParameters().link("fromValue", this->fromValue = fromValue);
		this->getParameters().link("toValue", this->toValue = toValue);
	};

	RemappingFunctor(const RemappingFunctor & ftor) : UnaryFunctor(ftor){
		parameters.copyStruct(ftor.parameters, ftor, *this);
	}


	inline
	double operator()(double s) const {
		// this->scale*s + this->bias;
		if (s == fromValue)
			return toValue;
		else
			return s;
	};

	double fromValue = 0.0;
	double toValue   = 0.0;

};


/// Thresholds intensity values.
/**
	\code
	drainage image-gray.png  --iThreshold 96 -o thresholdAbs.png
	drainage image-gray.png  --physicalRange 0:1 --iThreshold 0.5   -o thresholdRelative.png
	drainage image.png --physicalRange 0:1 --iThreshold 0.25  -o thresholdRelative-image.png
	\endcode

	\see BinaryThresholdFunctor
 */
class ThresholdFunctor : public UnaryFunctor {

public:

	ThresholdFunctor(double threshold = 0.5, double replace = 0.0) : UnaryFunctor(__FUNCTION__, "Resets values lower than a threshold") , threshold(threshold), replace(replace) {
		this->getParameters().link("threshold", this->threshold = threshold);
		this->getParameters().link("replace", this->replace = replace);
	};

	ThresholdFunctor(const ThresholdFunctor & ftor) : UnaryFunctor(ftor){
		parameters.copyStruct(ftor.parameters, ftor, *this);
	}


	virtual inline
	double operator()(double s) const override {
		if (s < threshold)
			return replace;
		else
			return s;
	};

	double threshold = 0.5;
	double replace   = 0.0;

};

/// Thresholds intensity values.
/**
	\code
	drainage image-gray.png  --iThresholdBinary 128,64,192 -o thresholdBinaryAbs.png
	drainage image-gray.png  --physicalRange 0:1 --iThresholdBinary 0.65 -o thresholdBinaryRelative.png
	drainage image.png --physicalRange 0:1 --iThresholdBinary 0.5  -o thresholdBinaryRelative-image.png
	\endcode

	\see BinaryThresholdFunctor

 */
class BinaryThresholdFunctor : public UnaryFunctor { // : public ThresholdFunctor {

public:

	BinaryThresholdFunctor(double threshold = 0.5, double replaceLow = 0.0, double replaceHigh = 1.0) : UnaryFunctor(__FUNCTION__, "Resets values lower and higher than a threshold")  {
		this->getParameters().link("threshold", this->threshold.tuple(threshold,threshold), "min[:max]").fillArray = true;
		this->getParameters().link("replace",   this->replace.tuple(replaceLow, replaceHigh), "min[:max]");
	};

	BinaryThresholdFunctor(const BinaryThresholdFunctor & ftor) : UnaryFunctor(ftor){
		parameters.copyStruct(ftor.parameters, ftor, *this);
	}

	virtual inline
	double operator()(double s) const override {
		if (s < threshold.min)
			return replace.min;
		else if (s > threshold.max)
			return replace.max;
		else
			return s;
	};

	Range<double> threshold  = {0.5, 0.5};
	Range<double> replace    = {0.0, 1.0};

};



/// Gamma correction. Intensity is mapped as f' = f^(gamma)
/*!
   Adjusts the brightness such that intensities remain monotonously inside the original scale.
   Unlike in direct linear scaling neither underflow nor overflow occurs and visual details remain
   detectable down to applied bit resolution.

	Prior to calling this function it should be ensured that the source image has normalized scale
	(eg. with \c--physicalRange, \c -R ).

   \code
   drainage image-gray.png -R 0:1 --iGamma 1.5 -o gamma-bright.png
   drainage image-gray.png -R 0:1 --iGamma 2.0 -o gamma-bright.png
   drainage image-gray.png -R 0:1 --iGamma 0.5 -o gamma-dark.png
   \endcode


   \code
   drainage image-color.png -R 0:1 --iGamma 2.0 -o gamma-color-bright.png
   drainage image-color.png -R 0:1 --iGamma 0.5 -o gamma-color-dark.png
   \endcode

   \see FunctorOp

 *  NOTE. Design for parameters may vary in future, since multichannel image could be handled by giving
 *  a value for each: 1.2,1.4,0.7 for example.
 */
class GammaFunctor : public drain::UnaryFunctor
{

public:

	GammaFunctor(double gamma = 1.0) : UnaryFunctor(__FUNCTION__, "Gamma correction for brightness."){
		this->getParameters().link("gamma", this->gamma = gamma, "0.0...");
	};

	GammaFunctor(const GammaFunctor & ftor) : UnaryFunctor(ftor){
		this->getParameters().link("gamma", this->gamma = ftor.gamma, "0.0..");
		//this->getParameters().copyStruct(ftor.getParameters(), ftor, *this);
	};

	//virtual
	inline
	double operator()(double s) const {
		return this->scale * pow(s, 1.0/gamma);
	};

	double gamma = 1.0;

};


/// Adds a intensity values .
/**
 *
   \~exec
   make image-gray-rot.png #exec
   \~

	\code
	drainage shapes1.png shapes2.png --iAdd 0.5  -o add1.png
	drainage image-gray.png image-gray-rot.png   --iAdd 0.5  -o add2.png
	\endcode
 */
class AdditionFunctor : public BinaryFunctor {

public:

	AdditionFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Adds values", scale, bias){
		this->getParameters().link("scale", this->scale);
		this->getParameters().link("bias", this->bias);
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scaleFinal*(s1 + s2) + this->biasFinal;
	};
};


/// Subtracts image from another image
/**
 Scaling applies to the result.
 The following subtracts image.png from image2.png :
 \code
  drainage shapes1.png -R 0:1 shapes2.png -R 0:1 --iSub LIMIT=1 -o sub-limit.png
  drainage shapes1.png -R 0:1 shapes2.png -R 0:1 --iSub 0.4,0.5 -o sub-scaled.png
 \endcode
 */
class SubtractionFunctor : public BinaryFunctor {

public:

	SubtractionFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Subtracts values", scale, bias){  // , bool LIMIT=false
		this->getParameters().link("scale", this->scale);
		this->getParameters().link("bias", this->bias);
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scaleFinal*(s1 - s2) + this->biasFinal;
	};
};



/// Multiplies two images, with optional post scaling and offset
/**

  \code
  drainage shapes1.png shapes2.png -R 0:1 --iMul 1.0       -o mul.png
  drainage shapes1.png  -R 0:1 shapes2.png -R 0:1 --iMul 0.5,0.25  -o mul2.png
 \endcode

 The order of the images counts; the destination dimensions are determined from the image last read.
 \code
  drainage image.png shapes.png  --physicalRange 0:1 --iMul 1 -o mul-shapes.png
  drainage image-gray.png image-gray-rot.png --physicalRange 0:1 --iMul 1 -o mul-gray.png
  drainage shapes.png image.png  --physicalRange 0:1 --iMul 1 -o mul-image.png
 \endcode

 The coordinate handler can be applied as well.
 \code
  drainage --coordPolicy 3 image.png shapes.png --physicalRange 0:1 --iMul 1 -o mul-image-mirror.png
 \endcode

 */
class MultiplicationFunctor : public BinaryFunctor {

public:

	MultiplicationFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Rescales intensities linerarly", scale, bias){
		this->getParameters().link("scale", this->scale);
		this->getParameters().link("bias", this->bias);
		// updateBean();
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scaleFinal*(s1*s2) + this->biasFinal;
	};
};

/// Divides image by another image
/**
 Scaling applies to the result.
 The following divides image2.png by image.png :
 \code
  drainage shapes1.png -R 0:1 shapes2.png -R 0:1 --iDiv 0.5,LIMIT=1 -o div.png
 \endcode
 */
class DivisionFunctor : public BinaryFunctor {

public:

	DivisionFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Rescales intensities linerarly", scale, bias){
		this->getParameters().link("scale", this->scale);
		this->getParameters().link("bias", this->bias);
		//updateBean();
	};

	inline
	double operator()(double s1, double s2) const {
		if (s2 != 0.0)
			return this->scale*(s1/s2) + this->bias;
		else
			return NAN;
	};
};


/// Blends an image to another with given proportion.
/**

   This operator produces:
   <CODE>
   F2 = cF + (1-c)M{ cF + (1-c)M{F} }
      = cF + (1-c)cM{F} + (1-c)^2M^2{F}
   </CODE>
   where
   <CODE>F</CODE> is an image,
   <CODE>M{}</CODE> is a <CODE>W x W</CODE> WindowAverage operator, and
   <CODE>c</CODE> is a coefficient.

 \exec~
  make image-rot.png #exec
 \~
 \code
   drainage shapes1.png shapes2.png --iMix 0.75 -o mix.png
   drainage image.png image-rot.png --iMix 0.25 -o mix-image.png
 \endcode

  \see DoubleSmootherOp
  \see QuadraticSmootherOp

 */
class MixerFunctor : public BinaryFunctor {

public:

	MixerFunctor(double coeff=0.5, double scale=1.0, double bias=0.0) : BinaryFunctor(__FUNCTION__, "Rescales intensities linerarly", scale, bias), coeff(coeff), scaleFinal2(1.0){
		this->getParameters().link("coeff", this->coeff);
		this->getParameters().link("scale", this->scale);
		this->getParameters().link("bias", this->bias);
		updateBean();
		//updateScale();
	};

	MixerFunctor(const MixerFunctor & ftor) : BinaryFunctor(ftor){
		parameters.copyStruct(ftor.parameters, ftor, *this);  // ~coeff
		//std::cerr << "copy const" << __FUNCTION__ << std::endl;
	}


	inline
	double operator()(double s1, double s2) const {
		return this->scaleFinal*s1 + this->scaleFinal2*s2 + this->biasFinal;
	};

	inline
	virtual
	void updateScale() const {
		this->scaleFinal  = this->scale*this->coeff;
		this->scaleFinal2 = this->scale*(1.0-this->coeff);
		this->biasFinal   = this->bias;
	}

	double coeff = 1.0;

protected:

	mutable
	double scaleFinal2 = 1.0;

};




///
/**
 *
 *   Prescales src to dst scale. See also MinimumOp, MaximumOp and MultiplicationOp
 \code
 drainage shapes1.png  shapes2.png --iMax -o max.png
 drainage image.png  image-rot.png --iMax -o max-image.png
\endcode
*/
class MaximumFunctor : public BinaryFunctor {

public:

	MaximumFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Maximum of two values.", scale, bias){
		updateBean();
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scale * std::max(static_cast<double>(s1), static_cast<double>(s2)) + this->bias;
	};
};



/// Minimum intensity. Prescaled input.
/**
 *
 *   Prescales src to dst scale.  See also MaximumOp, MinimumOp and MultiplicationOp.
 \code
 drainage shapes1.png shapes2.png --iMin -o min.png
 drainage image.png image-rot.png --iMin -o min-image.png
 \endcode
*/
class MinimumFunctor : public BinaryFunctor {

public:

	MinimumFunctor(double scale = 1.0, double bias = 0.0) : BinaryFunctor(__FUNCTION__, "Minimum of two values.", scale, bias){
		updateBean();
	};

	inline
	double operator()(double s1, double s2) const {
		return this->scale * std::min(static_cast<double>(s1), static_cast<double>(s2)) + this->bias;
	};
};




}

#endif /*  MATH_OP_PACK */

// Drain
