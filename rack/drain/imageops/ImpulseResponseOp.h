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
#ifndef ImpulseResponse2_H
#define ImpulseResponse2_H

#include <sstream>
#include <ostream>
//#include "drain/utility>

#include "drain/image/Coordinates.h"
#include "drain/image/FilePng.h"
//#include "drain/image/SegmentProber.h"

#include "ImageOp.h"

namespace drain
{
namespace image
{


/**
 *  \tparam C - conf type, implementing drain::BeanLike concept (getName, getDescription, getParameters)
 */
template <class C>
class ImpulseBucket : public C {

public:

	~ImpulseBucket(){}

	typedef C conf_t;

	/// Adapt to input geometry, type, and scaling.
	virtual inline
	void init(const Channel & src, bool horizontal = true){};

	/// Clear statistics before traversing each row or column.
	virtual
	void reset() = 0;

	/// When traversing up or left, add a encoded value to bucket in position i.
	virtual
	void addLeft(int i, double value, double weight) = 0;

	/// When traversing down or right, add a encoded value to bucket in position i.
	virtual
	void addRight(int i, double value, double weight) = 0;

	/// When traversing down or right, add a encoded value to bucket in position i.
	virtual
	void addDown(int i, double value, double weight) = 0;

	/// When traversing up or left, add a encoded value to bucket in position i.
	virtual
	void addUp(int i, double value, double weight) = 0;


	/// Return natural (not encoded) value at position i.
	virtual
	double get(int i) = 0;

	/// Return weight at position i.
	/**
	 *   Weight should reflect quality, reliability or relevance of the value returned by get(int i).
	 */
	virtual
	double getWeight(int i) = 0;

protected:

	//ImpulseBucket(conf_t){}

};


// Important. Guarantees that bucket (parameters) have been initialized.

template <class T>
class ImpulseResponseOpBase : public ImageOp {
public:

	inline
	ImpulseResponseOpBase(){};

	inline
	ImpulseResponseOpBase(const typename T::conf_t & conf) : conf(conf){
		//std::cerr << "decay now:" << conf.decay << '\n';
	};

	virtual inline
	~ImpulseResponseOpBase(){};

	virtual
	const std::string & getName() const {
		return this->conf.getName();
	};

	virtual
	const std::string & getDescription() const {
		return this->conf.getDescription();
	};

	// instead, consider conf object of type T::conf_t
	typename T::conf_t conf;

};


/// A fill operation for one color.
/**

 \tparam - Accumulating unit that also handles decoding/encoding of the values, must define ::conf

 \code
   drainage shapes.png --iImpulseAvg  0.9,5,5 -o shapes-impulse-90.png
   drainage shapes.png --iImpulseAvg  0.5,5,5 -o shapes-impulse-50.png
   drainage shapes.png --iImpulseAvg  0.1,5,5 -o shapes-impulse-10.png
 \endcode

 */
template <class T>
class ImpulseResponseOp : public ImpulseResponseOpBase<T> {

public:


	inline
	ImpulseResponseOp() {
		init();
	};

	inline
	ImpulseResponseOp(const ImpulseResponseOp<T> & op) : ImpulseResponseOpBase<T>(op.conf) {
		init();
	};

	inline
	ImpulseResponseOp(const typename T::conf_t & conf) : ImpulseResponseOpBase<T>(conf) {
		init();
	};

	inline
	void init(){
		this->parameters.append(this->conf.getParameters());
		this->parameters.link("extendHorz", extendHorz = 0, "pix"); // for avoiding border effects, include pixels beyond main area
		this->parameters.link("extendVert", extendVert = 0, "pix"); // for avoiding border effects, include pixels beyond main area
		this->parameters.link("weightThreshold", weightThreshold = 0.05, "[0..1.0]"); //

	};

	inline
	~ImpulseResponseOp(){};


	/*
	virtual	inline
	void makeCompatible(const ImageFrame &src, Image &dst) const  {

		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		//mout.debug3() << "src: " << src << mout.endl;

		if (dst.getType() != src.getType()){
			mout.note() << " changing dst image type: " << dst.getType().name() << '>' << src.getType().name() << mout.endl;
		}

		dst.copyShallow(src);
		mout.warn() << dst << mout.endl;
		// mout.debug(3) << "dst: " << dst << mout.endl;

	};
	*/

	//virtual void makeCompatible(const ImageFrame & src,Image & dst) const;
	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		this->traverseChannelsSeparately(src, dst);
	}

	virtual
	void traverseChannel(const Channel & src, Channel & dst) const;

	// weighted
	virtual
	void traverseChannel(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const;

	virtual
	void traverseChannelHorz(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const;

	virtual
	void traverseChannelVert(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const;

	inline
	void setExtensions(int horz, int vert){
		extendHorz = horz;
		extendVert = vert;
	}

protected:

	int extendHorz;
	int extendVert;
	double weightThreshold;
	//double undetectQuality;

};


template <class T>
void ImpulseResponseOp<T>::traverseChannel(const Channel & src, Channel & dst) const {
	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	mout.debug() << "delegating to traverseChannel(src, empty, dst, empty)" << mout.endl;

	drain::image::Image empty;
	traverseChannel(src, empty, dst, empty);

}

template <class T>
void ImpulseResponseOp<T>::traverseChannel(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	dst.setScaling(src.getScaling());
	dstWeight.setScaling(srcWeight.getScaling());

	mout.warn() << dst.getProperties() << mout.endl;

	traverseChannelHorz(src, srcWeight, dst, dstWeight);
	traverseChannelVert(dst, dstWeight, dst, dstWeight);

}


template <class T>
void ImpulseResponseOp<T>::traverseChannelHorz(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	mout.debug() << *this << mout.endl;

	const bool UNWEIGHTED = (srcWeight.isEmpty() || dstWeight.isEmpty());

	const int width    = src.getWidth();
	const int widthExt = src.getWidth()+extendHorz;
	const int height   = src.getHeight();
	const double defaultWeight = 1.0; //srcWeight.getMax<double>();

	//const double weightThreshold = 0.1;
	double w;

	const drain::image::CoordinateHandler2D coordHandler(src.getGeometry().area, src.getCoordinatePolicy());

	drain::Point2D<int> point;

	T bucket(this->conf);
	bucket.init(src, true);

	// NOTE: raw data values, but scaled weight values.

	for (int j=0; j<height; ++j){

		bucket.reset();

		if (UNWEIGHTED){

			// Collect
			for (int i=-extendHorz; i<widthExt; ++i){

				point.setLocation(i, j);
				coordHandler.handle(point);
				//if (coordHandler.validate(point)){
				bucket.addLeft(point.x, src.get<double>(point.x, point.y), defaultWeight);
				//}

				point.setLocation(width-1-i, j);
				coordHandler.handle(point);
				// if (coordHandler.validate(point)){
				bucket.addRight(point.x, src.get<double>(point.x, point.y), defaultWeight);
				//}

			}

			// Write
			for (int i=0; i<width; ++i){
				dst.putScaled(i,j, bucket.get(i));
			}

		}
		else {

			// Collect
			for (int i=-extendHorz; i<widthExt; ++i){

				point.setLocation(i, j);
				coordHandler.handle(point);
				//if (coordHandler.validate(point)){
				bucket.addLeft(point.x, src.get<double>(point.x, point.y), srcWeight.getScaled(point.x, point.y));
				//}

				point.setLocation(width-1-i, j);
				coordHandler.handle(point);
				// if (coordHandler.validate(point)){
				bucket.addRight(point.x, src.get<double>(point.x, point.y), srcWeight.getScaled(point.x, point.y));
				// }

			}

			// Write
			for (int i=0; i<width; ++i){
				w = bucket.getWeight(i);
				if (w > weightThreshold){
					dst.putScaled(i,j, bucket.get(i));
					dstWeight.putScaled(i,j, w);
				}
				else
					dstWeight.putScaled(i,j, 0);
				//dst.putScaled(i,j, bucket.get(i));
				//dstWeight.putScaled(i,j, bucket.getWeight(i));
			}

		}

	}
}

template <class T>
void ImpulseResponseOp<T>::traverseChannelVert(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	mout.debug() << *this << mout.endl;

	const bool UNWEIGHTED = (srcWeight.isEmpty() || dstWeight.isEmpty());

	const int width     = src.getWidth();
	const int height    = src.getHeight();
	const int heightExt = src.getHeight() + extendVert;
	const double defaultWeight   = 1.0; //srcWeight.getMax<double>();
	//const double weightThreshold = 0.1;
	double w;

	const drain::image::CoordinateHandler2D coordHandler(src.getGeometry(), src.getCoordinatePolicy());

	drain::Point2D<int> point;

	T bucket(this->conf);
	bucket.init(src, false);

	// NOTE: raw data values, but scaled weight values.

	for (int i=0; i<width; i++){

		bucket.reset();

		if (UNWEIGHTED){

			// Collect
			for (int j=-extendVert; j<heightExt; ++j){

				point.setLocation(i, j);
				coordHandler.handle(point); //if (coordHandler.validate(point)){
				bucket.addLeft(point.y, src.get<double>(point.x, point.y), defaultWeight);

				point.setLocation(i, height-1-j);
				coordHandler.handle(point); //if (coordHandler.validate(point))
				bucket.addRight(point.y, src.get<double>(point.x, point.y), defaultWeight);

			}

			// Write
			for (int j=0; j<height; ++j){
				dst.putScaled(i,j, bucket.get(j));
			}

		}
		else {

			// Collect
			for (int j=-extendVert; j<heightExt; ++j){

				point.setLocation(i, j);
				coordHandler.handle(point); //if (coordHandler.validate(point)){
				bucket.addLeft(point.y, src.get<double>(point.x, point.y), srcWeight.getScaled(point.x, point.y));

				point.setLocation(i, height-1-j);
				coordHandler.handle(point); // if (coordHandler.validate(point)){
				bucket.addRight(point.y, src.get<double>(point.x, point.y), srcWeight.getScaled(point.x, point.y));

			}

			// Write
			for (int j=0; j<height; ++j){
				w = bucket.getWeight(j);
				if (w > weightThreshold){
					dst.putScaled(i,j, bucket.get(j));
					dstWeight.putScaled(i,j, w);
				}
				else
					dstWeight.putScaled(i,j, 0);
			}

		}


	}
}







} // image::

} // drain::


#endif /* ImpulseResponse_H_ */
