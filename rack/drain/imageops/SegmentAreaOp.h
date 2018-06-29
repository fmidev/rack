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
#include "image/SegmentProber.h"

//#include "ImageOp.h"
//#include "FloodFillOp.h"

#include "SegmentOp.h"

namespace drain
{
namespace image
{


/// Computes sizes connected pixel areas. For more complicated statistics, see SegmentStatisticsOp.
/**

	\tparam T - prober class, eg. SegmentProber

	Note: current designed matches SegmentProber quite explicitly.

 \code
   drainage shapes.png --segmentArea 64 -o segmentArea.png
   drainage gray.png --physicalRange 0.0,1.0 --segmentArea 0.5 -o segmentAreaPhys.png
 \endcode

 \code
   drainage shapes.png --segmentArea 64,255,FuzzyStep:300:0 -o segmentAreaInv.png
 \endcode

 \code
   drainage shapes.png --segmentArea 64,255,FuzzyStepsoid:100:20 -o segmentAreaStepsoid.png
 \endcode

 \code
   drainage shapes.png --segmentArea 32,255,FuzzyBell:200:50 -o segmentAreaBell.png
 \endcode

	\see SegmentStatisticsOp

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
	SegmentAreaOp(double min=1.0, double max=static_cast<double>(0xffff)) : //, const std::string & mapping="d", double scale=1.0, double offset=0.0) :
    	SegmentOp(__FUNCTION__, "Computes segment sizes.") {
		parameters.reference("min", this->min = min);
		parameters.reference("max", this->max = max);
		//parameters.reference("functor", this->functorName);
		parameters.reference("functor", this->functorStr);  //  eg. "FuzzyStep"
		//parameters.reference("functorParams", this->functorParams);
    };

	inline
	SegmentAreaOp(const drain::UnaryFunctor & ftor, double min=1, double max=static_cast<double>(0xffff)) : //, const std::string & mapping="d", double scale=1.0, double offset=0.0) :
    	SegmentOp(__FUNCTION__, "Computes segment sizes", ftor) {
		parameters.reference("min", this->min = min);
		parameters.reference("max", this->max = max);
    };

	/// Resizes dst to width and height of src. Ensures integer type.
	void makeCompatible(const ImageFrame & src, Image & dst) const;

	inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		//drain::Logger mout(this->name+"[const ImageTray &, ImageTray &]", __FUNCTION__);
		traverseChannelsSeparately(src, dst);
	}

    virtual
    void traverseChannel(const Channel & src, Channel & dst) const;


};


template <class T>
void SegmentAreaOp<T>::makeCompatible(const ImageFrame & src, Image & dst) const  {

	drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

	const std::type_info & t = typeid(typename T::dst_t);

	if (!dst.typeIsSet()){
		dst.setType(t);
	}
	else if (!dst.isIntegerType()){
		dst.setType(t);
		mout.warn() << "float valued destination data not supported, setting: " << Type::getTypeChar(t) << mout.endl;
		//throw std::runtime_error("SegmentAreaOp: float valued destination image not supported.");
	}

	dst.setGeometry(src.getWidth(), src.getHeight(),
				std::max(src.getImageChannelCount(),dst.getImageChannelCount()), dst.getAlphaChannelCount());

	if (clearDst)
		dst.clear();

}

template <class T>
void SegmentAreaOp<T>::traverseChannel(const Channel & src, Channel & dst) const {

	drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

	const unsigned int w = src.getWidth();
	const unsigned int h = src.getHeight();

	const typename T::src_t minRaw = src.getScaling().inv(min);
	const typename T::src_t maxRaw = (max == std::numeric_limits<double>::max()) ? src.getMax<typename T::src_t>() : src.getScaling().inv(max);

	mout.debug()  << "raw range: " << (double)minRaw << '-' << (double)maxRaw << mout.endl;
	mout.debug(1) << "src: " << src << mout.endl;
	mout.debug(1) << "dst: " << dst << mout.endl;

	/// TODO: Use actual types in SegmentProber.
	//SegmentProber<int,int> floodFill(src,dst);
	T floodFill(src,dst);

	//const size_t dMax = dst.getMax<size_t>();
	// const typename T::dst_t dMax = dst.getMax<typename T::dst_t>();
	// mout.warn() << "dMax " << (double)dMax << mout.endl;
	const double scale = drain::Type::call<drain::typeIsSmallInt>(dst.getType()) ? dst.getMax<double>() : 1.0;

	const UnaryFunctor & ftor = getFunctor(scale);
	//const UnaryFunctor & ftor = getFunctor(dst.getMax<T::dst_t>());

	mout.debug(1) << "Functor: " << ftor.getName() << '=' << ftor.getParameters() << mout.endl;
	/*
	mout.warn() << "Functor: 1 =>" << ftor(1.0) << mout.endl;
	mout.warn() << "Functor: 100 =>" << ftor(100.0) << mout.endl;
	mout.warn() << "Functor: 10000 =>" << ftor(10000.0) << mout.endl;
	*/
	mout.debug(1);

	mout << mout.endl;

	typedef drain::typeLimiter<typename T::dst_t> Limiter;
	typename Limiter::value_t limiter = dst.getLimiter<typename T::dst_t>();

	size_t sizeMapped;
	//double sizeMapped;
	for (size_t i=0; i<w; i++){
		for (size_t j=0; j<h; j++){
			//if ((drain::Debug > 5) && ((i&15)==0) && ((j&15)==0)) std::cerr << i << ',' << j << "\n";
			if (dst.get<int>(i,j) == 0){ // not visited

				// STAGE 1: detect size.
				floodFill.probe(i,j,1,  minRaw, maxRaw);  // painting with '1' does not disturb dst

				if (floodFill.size > 0){

					// STAGE 2: mark the segment with size

					sizeMapped = limiter(ftor(floodFill.size));
					if (sizeMapped == 0)
						sizeMapped = 1;

					/*
					mout.warn() << "found segment at " << i << ',' << j << " f=" << src.get<float>(i,j);
					mout << " size=" << floodFill.size << " => "<< sizeMapped << mout.endl;
					*/

					floodFill.probe(i,j,sizeMapped, minRaw, maxRaw);
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
