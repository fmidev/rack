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

#include "drain/util/FunctorBank.h"
#include "drain/image/SegmentProber.h"

//#include "ImageOp.h"
//#include "FloodFillOp.h"

#include "SegmentOp.h"

namespace drain
{
namespace image
{

/*
 *  Problem: --iSegmentArea 64:255,FuzzyStep:100:1
 *  actually is SegmentArea(64:255,FuzzyStep(100:100,1))
 *
 *  Consider:  FuzzyStep/100:100/1
 *
 *  Or prefix cmd:
 *  --functor FuzzyStep,100:100,1
 *  --functor FuzzyStep|100:100,1
 *  --functor FuzzyStep[100:100,1]
 *
 *  or last-arg-comma-support?
 *  a,b,1,2,3
 *  A=a,B=c,C=C,D,E
 */


/// Computes sizes of connected pixel areas.
/**

	\tparam S - source image type for SegmentProber<S,D>
	\tparam D - destination image type for SegmentProber<S,D>
	\tparam T - prober class, eg. SegmentProber

Note: current designed for SegmentProber.

\code
drainage shapes.png --iSegmentArea 64:255 -o segmentArea.png
drainage image-gray.png --physicalRange 0:1 --iSegmentArea 0.5:01 -o segmentAreaPhys.png
\endcode

\code
drainage shapes.png --iSegmentArea 64:255,FuzzyStep:300:0 -o segmentAreaInv.png
\endcode

\code
drainage shapes.png --iSegmentArea 64:255,FuzzyStepsoid:100:20 -o segmentAreaStepsoid.png
\endcode

\code
drainage shapes.png --iSegmentArea 32:255,FuzzyBell:200:50 -o segmentAreaBell.png
\endcode

\see SegmentStatisticsOp

 */
template <class S, class D, class T=SizeProber>
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
	inline  // static_cast<double>(0xffff)
	SegmentAreaOp(double min=1.0, double max=std::numeric_limits<double>::max()) :
	//SegmentAreaOp(double min=1.0, double max=static_cast<double>(0xffff)) :
    	SegmentOp(__FUNCTION__, "Computes segment sizes.") {
		parameters.link("intensity", this->intensity.tuple(), "min:max");
		parameters.link("functor", this->functorStr);  //  eg. "FuzzyStep:params"
		this->intensity.set(min, max);
    };

	inline // double max=static_cast<double>(0xffff)
	SegmentAreaOp(const drain::UnaryFunctor & ftor, double min=1.0, double max=std::numeric_limits<double>::max()) : //, const std::string & mapping="d", double scale=1.0, double offset=0.0) :
    	SegmentOp(__FUNCTION__, "Computes segment sizes", ftor) {
		parameters.link("intensity", this->intensity.tuple(), "min:max");
		this->intensity.set(min, max);
    };

	SegmentAreaOp(const SegmentAreaOp & op) : SegmentOp(op){
		parameters.copyStruct(op.getParameters(), op, *this);
	}

	/// Resizes dst to width and height of src. Ensures integer type.
	//void makeCompatible(const ImageFrame & src, Image & dst) const;
	void getDstConf(const ImageConf & src, ImageConf & dst) const;

	inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		//drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL this->name+"[const ImageTray &, ImageTray &]", __FUNCTION__);
		traverseChannelsSeparately(src, dst);
	}

    virtual
    void traverseChannel(const Channel & src, Channel & dst) const;


};


template <class S, class D, class T>
void SegmentAreaOp<S,D,T>::getDstConf(const ImageConf & src, ImageConf & dst) const {
	// void SegmentAreaOp<S,D>::makeCompatible(const ImageFrame & src, Image & dst) const  {

	drain::Logger mout(getImgLog(), __FUNCTION__, getName());

	const std::type_info & t = typeid(dst_t); // typeid(typename T::dst_t);

	/*if (!dst.typeIsSet()){
		dst.setType(t);
	}
	else*/
	if (Type::call<typeIsFloat>(dst.getType())){
		dst.setType(t);
		mout.warn("float valued destination data not supported, setting: " , Type::getTypeChar(t) );
		//throw std::runtime_error("SegmentAreaOp: float valued destination image not supported.");
	}

	dst.setArea(src);
	dst.setChannelCount(std::max(src.getImageChannelCount(),dst.getImageChannelCount()), dst.getAlphaChannelCount());

	//if (clearDst)
	//	dst.clear();

}

template <class S, class D, class T>
void SegmentAreaOp<S,D,T>::traverseChannel(const Channel & src, Channel & dst) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, getName());

	drain::Range<src_t> raw;
	raw.min = src.getScaling().inv(this->intensity.min);
	raw.max = (this->intensity.max == std::numeric_limits<double>::max()) ? src.getConf().getTypeMax<src_t>() : src.getScaling().inv(this->intensity.max);

	if (raw.min <= src.getConf().getTypeMin<src_t>()){
		mout.warn("src scaling:   "    , src.getScaling() );
		mout.warn("original range:   " , intensity );
		mout.warn("raw (code) range: " , raw );
		mout.warn("min value=" , (double)raw.min ,  " less or smaller than storage type min=" , src.getConf().getTypeMin<src_t>() );
	}

	mout.special("raw range: " , (double)raw.min , '-' , (double)raw.max );
	mout.debug2("src: " , src );
	mout.debug2("dst: " , dst );

	//SizeProber sizeProber(src, dst);
	T sizeProber(src, dst);
	sizeProber.conf.anchor.set(raw.min, raw.max);
	mout.debug2("areaProber:" , sizeProber );

	FillProber floodFill(src, dst);
	floodFill.conf.anchor.set(raw.min, raw.max); // Min = raw.min;
	mout.debug2("Floodfill: " , floodFill );

	const double scale = drain::Type::call<typeNaturalMax>(dst.getType());
	// const double scale = drain::Type::call<drain::typeIsSmallInt>(dst.getType()) ? dst.getEncoding().getTypeMax<double>() : 1.0;

	const UnaryFunctor & ftor = getFunctor(scale); // scale problematic, but needed for const Ftor

	mout.debug("Scale: " , scale );
	mout.debug("Final functor: " , ftor.getName() , '(' , ftor.getParameters() , ')' );
	/*
	mout.warn("Functor: 1 =>" , ftor(1.0) );
	mout.warn("Functor: 100 =>" , ftor(100.0) );
	mout.warn("Functor: 10000 =>" , ftor(10000.0) );
	*/
	//mout.debug2();

	const CoordinatePolicy & cp = src.getCoordinatePolicy();
	const bool HORZ_MODE = ((cp.xUnderFlowPolicy != EdgePolicy::POLAR) && (cp.xOverFlowPolicy != EdgePolicy::POLAR));

	if (HORZ_MODE){
		mout.attention("scan HORIZONTAL, coordPolicy=", cp);
	}
	else {
		mout.attention("scan VERTICAL, coordPolicy=", cp);
	}

	typedef drain::typeLimiter<dst_t> Limiter;
	typename Limiter::value_t limit = dst.getConf().getLimiter<dst_t>();

	const size_t width = src.getWidth();
	const size_t height = src.getHeight();

	size_t sizeMapped;
	for (size_t i=0; i<width; i++){
		for (size_t j=0; j<height; j++){

			// STAGE 1: detect size.
			//if (HORZ_MODE)
			sizeProber.probe(i,j, HORZ_MODE);

			if (sizeProber.size > 0){

				// STAGE 2: mark the segment with size
				sizeMapped = limit(ftor(sizeProber.size));
				if (sizeMapped == 0)
					sizeMapped = 1;

				// mout.warn() << "found segment at " << i << ',' << j << " f=" << src.get<float>(i,j);
				// mout << " size=" << sizeProber.size << " => "<< sizeMapped << mout.endl;

				floodFill.conf.markerValue = sizeMapped;
				floodFill.probe(i,j, HORZ_MODE);

			}
		}
	}


}


} // image

} // drain

#endif

// Drain
