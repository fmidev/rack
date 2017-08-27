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
#ifndef SEGMENTAREAOP_H
#define SEGMENTAREAOP_H

#include <math.h>

#include "util/FunctorBank.h"

#include "ImageOp.h"
#include "FloodFillOp.h"
#include "SegmentProber.h"

namespace drain
{
namespace image
{

/// A base class for computing statistics of segments. As segment is an area of connected pixels.
/**
 *
 */
class SegmentOp: public ImageOp
{
public:

	double min;
	double max;
	/*
	std::string mapping;
	double scale; // floodFill(src,dst);
	double offset;
	 */
	bool clearDst;

	std::string functorName;
	std::string functorParams;

protected:

	virtual
	inline
	bool traverseChannelsSeparately(const Image & src, Image & dst) const {
		return false;
	}

	SegmentOp(const std::string & name, const std::string & description) : ImageOp(name, description), min(0), max(0), clearDst(true) {
	}




};

/// Computes sizes connected pixel areas.
/**

  \tparam T - prober class, eg. SegmentProber


  For more complicated statistics, see SegmentStatisticsOp.


 \code
   drainage shapes.png --segmentArea 64,255 -o segmentArea.png
 \endcode

 \code
   drainage shapes.png --segmentArea 64,255,FuzzyStep,300:0 -o segmentAreaInv.png
 \endcode

 \code
   drainage shapes.png --segmentArea 64,255,FuzzyStepsoid,100:20 -o segmentAreaStepsoid.png
 \endcode

 \code
   drainage shapes.png --segmentArea 32,255,FuzzyBell,200:50 -o segmentAreaBell.png
 \endcode

 */
template <class T>
class SegmentAreaOp: public SegmentOp
{
public:

	/**
	 * \par min - the smallest intensity in a segment to be traversed
	 * \par max - the largest intensity in a segment to be traversed
	 * \par mapping - determines how the retrieved area is rescaled: d=direct, i=inversed, s=fuzzyScale, p=fuzzyPeak (was: f=fuzzyPeak)
	 * \par scale - parameter depending on \c mapping
	 * \par offset - parameter depending on \c mapping
	 */
	inline
	SegmentAreaOp(double min=1, double max=255) : //, const std::string & mapping="d", double scale=1.0, double offset=0.0) :
    	SegmentOp(__FUNCTION__, "Computes segment sizes."), functor(identityFtor) {
		parameters.reference("min", this->min = min);
		parameters.reference("max", this->max = max);
		parameters.reference("functor", this->functorName);  //  = "FuzzyStep"
		parameters.reference("functorParams", this->functorParams);
    };

	inline
	SegmentAreaOp(const drain::UnaryFunctor & ftor, double min=1, double max=255) : //, const std::string & mapping="d", double scale=1.0, double offset=0.0) :
    	SegmentOp(__FUNCTION__, std::string("Computes segment sizes. Functor=")+ftor.getName()), functor(ftor) {
		parameters.reference("min", this->min = min);
		parameters.reference("max", this->max = max);
    };

    virtual
    void traverse(const Image & src , Image &dst) const;

	/// Resizes dst to width and height of src. Ensures integer type.
	void makeCompatible(const Image & src, Image & dst) const;

	/// Returns a functor: the member functor or retrieved from FunctorBank
	const UnaryFunctor & getFunctor(double dstMax) const;

protected:

	const drain::UnaryFunctor & functor;
	const drain::IdentityFunctor identityFtor;

};


template <class T>
void SegmentAreaOp<T>::makeCompatible(const Image & src, Image & dst) const  {

	//std::cerr << "SegmentAreaOp::makeCompatible\n";
	//dst.info(std::cerr);
	//if (!dst.typeIsSet())
	if (!dst.isIntegerType()){
		if (dst.typeIsSet())
			throw std::runtime_error("SegmentAreaOp: float valued destination image not supported.");
		dst.setType<unsigned short>();
	}


	if (dst.setGeometry(src.getWidth(), src.getHeight(),
			std::max(src.getImageChannelCount(),dst.getImageChannelCount()), dst.getAlphaChannelCount()))
		if (clearDst)
			dst.clear();

	//dst.info(std::cerr);
}

template <class T>
const UnaryFunctor & SegmentAreaOp<T>::getFunctor(double dstMax) const {

	drain::MonitorSource mout(iMonitor, name, __FUNCTION__);

	if (functorName.empty()){
		return functor;
	}
	else {

		FunctorBank & functorBank = getFunctorBank();

		if (!functorBank.has(functorName)){
			mout.error() << "functor not found: " << functorName << mout.endl;
			return functor;
		}

		mout.debug(2) << "functorName: " << functorName << mout.endl;

		UnaryFunctor & ftor = functorBank.get(functorName).get();
		//mout.warn() << functorBank.get(functorName).get() << mout.endl;
		ftor.setScale(dstMax, 0.0);
		ftor.getParameters().separator = ':';
		ftor.setParameters(functorParams);

		mout.debug() << ftor.getName() << ':' << ftor.getDescription() << mout.endl;

		return ftor;
	}

}

template <class T>
void SegmentAreaOp<T>::traverse(const Image & src, Image & dst) const {

	//if (traverseChannelsSeparately(src,dst))
	//	return;

	drain::MonitorSource mout(iMonitor, name, __FUNCTION__);

	const unsigned int w = src.getWidth();
	const unsigned int h = src.getHeight();

	/// TODO: Use actual types in SegmentProber.
	//SegmentProber<int,int> floodFill(src,dst);
	T floodFill(src,dst);

	//const size_t dMax = dst.getMax<size_t>();
	const UnaryFunctor & ftor = getFunctor(dst.getMax<double>());

	mout.debug(1) << "Functor: " << ftor << mout.endl;
	/*
	mout.warn() << "Functor: 1 =>" << ftor(1.0) << mout.endl;
	mout.warn() << "Functor: 100 =>" << ftor(100.0) << mout.endl;
	mout.warn() << "Functor: 10000 =>" << ftor(10000.0) << mout.endl;
	*/

	//mout.note(1) << *this << mout.endl;
	mout.debug(1);
	mout << "range:" << (float)min  << '-' << (float)max;
	mout << mout.endl;


	size_t sizeMapped;
	//double sizeMapped;
	for (size_t i=0; i<w; i++){
		for (size_t j=0; j<h; j++){
			//if ((drain::Debug > 5) && ((i&15)==0) && ((j&15)==0)) std::cerr << i << ',' << j << "\n";
			if (dst.get<int>(i,j) == 0){ // not visited

				// STAGE 1: detect size.
				floodFill.probe(i,j,1,  (int)min, (int)max);  // painting with '1' does not disturb dst

				if (floodFill.size > 0){

					sizeMapped = dst.limit<size_t>(ftor(floodFill.size));
					if (sizeMapped == 0)
						sizeMapped = 1;

					/*
					mout.warn() << "found segment at " << i << ',' << j << " f=" << src.get<float>(i,j);
					mout << " size=" << floodFill.size << " => "<< sizeMapped << mout.endl;
					*/

					floodFill.probe(i,j,sizeMapped,min,max);
					//floodFill.fill(i,j,size&254,min,max);
					//std::cerr << "filled\n";
				}
			}
		}
	}


}


} // namespace image

} // namespace drain

#endif

// Drain
