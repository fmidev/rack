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

    SegmentProber
	\tparam S - source type for SegmentProber<S,D>
	\tparam D - destination type for SegmentProber<S,D>

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
template <class S, class D>
class SegmentAreaOp: public SegmentOp
{
public:

	typedef S src_t;
	typedef D dst_t;
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


template <class S, class D>
void SegmentAreaOp<S,D>::makeCompatible(const ImageFrame & src, Image & dst) const  {

	drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

	const std::type_info & t = typeid(dst_t); // typeid(typename T::dst_t);

	if (!dst.typeIsSet()){
		dst.setType(t);
	}
	else if (Type::call<typeIsFloat>(dst.getType())){
		dst.setType(t);
		mout.warn() << "float valued destination data not supported, setting: " << Type::getTypeChar(t) << mout.endl;
		//throw std::runtime_error("SegmentAreaOp: float valued destination image not supported.");
	}

	dst.setGeometry(src.getWidth(), src.getHeight(),
				std::max(src.getImageChannelCount(),dst.getImageChannelCount()), dst.getAlphaChannelCount());

	if (clearDst)
		dst.clear();

}

template <class S, class D>
void SegmentAreaOp<S,D>::traverseChannel(const Channel & src, Channel & dst) const {

	drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

	const size_t w = src.getWidth();
	const size_t h = src.getHeight();

	const src_t minRaw = src.getScaling().inv(min);
	const src_t maxRaw = (max == std::numeric_limits<double>::max()) ? src.getEncoding().getTypeMax<src_t>() : src.getScaling().inv(max);

	if (minRaw <= src.getEncoding().getTypeMin<src_t>()){
		mout.warn()  << "min value=" << (double)minRaw <<  " less or smaller than storage type min=" << src.getEncoding().getTypeMin<src_t>() << mout.endl;
	}

	mout.debug()  << "raw range: " << (double)minRaw << '-' << (double)maxRaw << mout.endl;
	mout.debug(1) << "src: " << src << mout.endl;
	mout.debug(1) << "dst: " << dst << mout.endl;

	//SegmentProber<S,D> floodFill(src, dst);
	SizeProber sizeProber(src, dst);
	sizeProber.conf.anchorMin = minRaw;
	sizeProber.conf.anchorMax = maxRaw;
	mout.debug(1) << "areaProber:" << sizeProber << mout.endl;

	FillProber floodFill(src, dst);
	floodFill.conf.anchorMin = minRaw;
	floodFill.conf.anchorMax = maxRaw;
	mout.debug(1) << "floodFill: " << floodFill << mout.endl;
	//floodFill.init();
	//T floodFill(src, dst);

	//const size_t dMax = dst.getMax<size_t>();
	// const typename T::dst_t dMax = dst.getMax<typename T::dst_t>();
	// mout.warn() << "dMax " << (double)dMax << mout.endl;
	const double scale = drain::Type::call<drain::typeIsSmallInt>(dst.getType()) ? dst.getEncoding().getTypeMax<double>() : 1.0;

	mout.debug() << "Scale: " << scale << mout.endl;

	const UnaryFunctor & ftor = getFunctor(scale);
	//const UnaryFunctor & ftor = getFunctor(dst.getMax<T::dst_t>());

	mout.debug() << "Functor: " << ftor.getName() << '(' << ftor.getParameters() << ')' << mout.endl;
	/*
	mout.warn() << "Functor: 1 =>" << ftor(1.0) << mout.endl;
	mout.warn() << "Functor: 100 =>" << ftor(100.0) << mout.endl;
	mout.warn() << "Functor: 10000 =>" << ftor(10000.0) << mout.endl;
	*/
	//mout.debug(1);
	mout << mout.endl;

	typedef drain::typeLimiter<dst_t> Limiter;
	typename Limiter::value_t limit = dst.getEncoding().getLimiter<dst_t>();

	size_t sizeMapped;
	for (size_t i=0; i<w; i++){
		for (size_t j=0; j<h; j++){

			// STAGE 1: detect size.
			sizeProber.probe(i,j);

			if (sizeProber.size > 0){

				// STAGE 2: mark the segment with size
				sizeMapped = limit(ftor(sizeProber.size));
				if (sizeMapped == 0)
					sizeMapped = 1;

				// mout.warn() << "found segment at " << i << ',' << j << " f=" << src.get<float>(i,j);
				// mout << " size=" << sizeProber.size << " => "<< sizeMapped << mout.endl;

				floodFill.conf.markerValue = sizeMapped;
				floodFill.probe(i,j);

			}
		}
	}


}


} // namespace image

} // namespace drain

#endif

// Drain
