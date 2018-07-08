/*

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef GAMMAOP_H_
#define GAMMAOP_H_

#include <math.h>

//#include <limits>

//#include "BufferedImage.h"
#include "../util/Functor.h"
//#include "SequentialImageOp.h"

// using namespace std;

namespace drain
{

namespace image
{

/// Gamma correction. Intensity is mapped as f' = f^(gamma)
/*!
   Adjusts the brightness such that intensities remain monotonously inside the original scale.
   Unline in direct linear scaling neither undeflow nor overflow occurs and details remain
   detectable down to applied bit resolution.

   \code
   drainage gray.png --physicalRange 0,1 --gamma 2.0 -o gamma-bright.png
   drainage gray.png --physicalRange 0,1 --gamma 0.5 -o gamma-dark.png
   \endcode


   \code
   drainage color.png --physicalRange 0,1 --gamma 2.0 -o gamma-color-bright.png
   drainage color.png --physicalRange 0,1 --gamma 0.5 -o gamma-color-dark.png
   \endcode

 *  NOTE. Design for parameters may vary in future, since multichannel image could be handled by giving
 *  a value for each: 1.2,1.4,0.7 for example. 
 */
/*
class GammaOp : public SequentialImageOp
{
public:

	GammaOp(double gamma = 1.0) : SequentialImageOp(__FUNCTION__, "Gamma correction for brightness."){
		parameters.reference("gamma", this->gamma = gamma, "0.0...");
		gammaInv = 1.0/gamma;
		scaleSrc = 1.0;
		scaleDst = 1.0;
	};

	inline
	void initializeParameters(const ImageFrame & src, const ImageFrame & src2, const ImageFrame & dst) const {
		gammaInv = 1.0 / gamma;
		scaleSrc = 1.0 / src.scaling.getMax<float>();
		scaleDst = dst.scaling.getMax<float>();
	};

	inline
	double filterValueD(double src, double src2) const {
		return scaleDst * pow(src*scaleSrc, gammaInv);
	};

	double gamma;

protected:

	mutable double gammaInv;
	mutable double scaleSrc;
	mutable double scaleDst;

};
*/
class GammaFunctor : public drain::UnaryFunctor
{

public:

	GammaFunctor(double gamma = 1.0) : UnaryFunctor(__FUNCTION__, "Gamma correction for brightness."){ // , fromValue(fromValue), toValue(toValue) {
		this->getParameters().reference("gamma", this->gamma = gamma, "0.0...");
	};

	//virtual
	inline
	double operator()(double s) const {
		return this->scale * pow(s, 1.0/gamma);
	};

	double gamma;


};


/*
class GammaOp : public SequentialImageOp {
	GammaOp(double gamma = 1.0) : SequentialImageOp(__FUNCTION__, "Gamma correction for brightness."){
		parameters.reference("gamma", this->gamma = gamma, "0.0...");
		gammaInv = 1.0/gamma;
		scaleSrc = 1.0;
		scaleDst = 1.0;
	};

	inline
	void initializeParameters(const ImageFrame & src, const ImageFrame & src2, const ImageFrame & dst) const {
		gammaInv = 1.0 / gamma;
		scaleSrc = 1.0 / src.scaling.getMax<float>();
		scaleDst = dst.scaling.getMax<float>();
	};

	inline
	double filterValueD(double src, double src2) const {
		return scaleDst * pow(src*scaleSrc, gammaInv);
	};

	double gamma;

protected:

	mutable double gammaInv;
	mutable double scaleSrc;
	mutable double scaleDst;

};
*/

}
}

#endif /*GAMMAOP_H_*/
