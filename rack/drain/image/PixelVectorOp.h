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
#ifndef PixelVectorOP_H_
#define PixelVectorOP_H_

#include <cmath>

#include "ImageOp.h"
#include "util/Functor.h"
#include "util/Fuzzy.h"
#include "util/Registry.h"
#include "util/FunctorBank.h"

namespace drain
{

namespace image
{



///  A base class for operations between of intensity vectors of two images.
/**  In each point(i,j), performs  an operation for vectors constructed the intensities in different channels.
 *   Produces a flat, single-channel image.
 *
 *  \see DistanceOp
 *  \see ProductOp
 *  \see MagnitudeOp
 */
class PixelVectorOp : public ImageOp
{
public:

	/// NEW
	std::string functorName;
	std::string functorParams;

	double rescale;


	PixelVectorOp(const std::string & name, const std::string & description) :
		//ImageOp(name,description + " Computes L-norm or power of channel intensities. Euclidean L=2. "), SQRT(false) { //, l(1){ , rescale(0.0),
		ImageOp(name,description + " Post-scaling with desired functor."), rescale(0.0), SQRT(false) { //, l(1){ , rescale(0.0),
		parameters.separator = ':';
		parameters.reference("functor", this->functorName);
		parameters.reference("params", this->functorParams);
		//parameters.reference("", this->functorParams);
		/*
		parameters.reference("mapping", this->mapping = "linear");
		parameters.reference("l", this->l = 1.0);
		parameters.reference("lInv", this->lInv = 1.0);
		parameters.reference("halfWidth", this->halfWidth = 0.0);
		*/
	};


	virtual
	void makeCompatible(const Image &src, Image &dst) const;


	virtual
	inline
	void filter(const Image &src, Image &dst) const {

		ImageOp::filter(src, dst, dst);
	};

	virtual
	void filter(const Image &src, const Image &src2, Image &dst) const {
		ImageOp::filter(src, src2, dst);
	};




protected:

	virtual
	inline
	bool traverseChannelsSeparately(const Image & src, Image & dst) const {
		return false;
	}

	virtual
	inline
	bool traverseChannelsSeparately(const Image & src, const Image & srcAlpha, Image & dst, Image & dstAlpha) const {
		return false;
	}

	virtual
	void traverse(const Image &src1, const Image &src2, Image &dst) const;

	virtual
	double getValue(double src, double src2) const = 0;

	//bool ABS;
	/// take sqrt of the sum
	bool SQRT;


};

/**! Computes dot product of intensities of two images.
 *    dst(i,j) = \sqrt{ src(i,j,0)*src2(i,j,0) + ... + src(i,j,k)*src2(i,j,k) }.
 *
 *  \see DistanceOp
 *  \see MagnitudeOp
 *  \see ChannelVectorOp
 */
class ProductOp : public PixelVectorOp
{
public:

	ProductOp() : PixelVectorOp(__FUNCTION__, "Computes the dot product of pixel vectors."){
		SQRT = false;
	};

protected:

	inline
	double getValue(double src, double src2) const { return src*src2; };

};

/** Computes magnitude of channel intensities in a pixel.
    dst(i,j) = \sqrt{ src(i,j,0)^2 + ... + src(i,j,k)^2 }.

 \code
 drainage image.png --magnitude FuzzyStep:0,255   -o magnitude-step.png
 drainage image.png --magnitude FuzzyBell:0,-150  -o magnitude-bell.png
 \endcode

 \see DistanceOp
 \see ProductOp

 */
class MagnitudeOp : public PixelVectorOp
{
public:

	//MagnitudeOp(const std::string & mapping = "linear", double l=2.0) : PixelVectorOp(__FUNCTION__,"Computes the magnitude of a pixel vector."){
	MagnitudeOp() : PixelVectorOp(__FUNCTION__,"Computes the magnitude of a pixel vector."){
		SQRT = true;
	};

	virtual
	inline
	void filter(const Image &src, Image &dst) const {
		ImageOp::filter(src, src, dst);
	};


protected:

	inline
	double getValue(double src, double src2) const { return src*src2; };

private:

	//void filter(const Image & src1, const Image & src2, Image &dst) const {};

};

/** Computes distance of intensity vectors of two images.
 *    dst(i,j) = \sqrt{ src(i,j,0)*src2(i,j,0) + ... + src(i,j,k)*src2(i,j,k) }.

 \code
   drainage shapes1.png shapes2.png --distance FuzzyStep:0,255 -o distance.png
 \endcode

 \see DistanceOp
 \see MagnitudeOp
 */
class DistanceOp : public PixelVectorOp
{
public:

	//DistanceOp(const std::string mapping = "linear", double l = 2.0) :
	DistanceOp() :
		PixelVectorOp(__FUNCTION__, "Computes the distance of pixel vectors."){
		SQRT = true;
		/*
		this->mapping = mapping;
		this->l = l;
		this->lInv = 1.0/l;
		*/
	};

protected:

	inline
	double getValue(double src, double src2) const {
		return (src - src2)*(src - src2);
	};

};


}
}


#endif /*PixelVectorOP_H_*/

// Drain
