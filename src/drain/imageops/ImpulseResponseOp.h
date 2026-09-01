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

#include "drain/image/ValueHandler.h"
#include <drain/image/CoordinatePolicy.h>
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
template <class C, class H=ValueHandler>
class ImpulseBucket : public C {

public:

	~ImpulseBucket(){}

	typedef C conf_t;
	typedef ImpulseBucket<C> bucket_t;


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

	// Experimental

	typedef H value_handler_t;

	ValueHandler & getValueHandler() {
		return valueHandler;
	}

protected:

	value_handler_t valueHandler;

	//ImpulseBucket(conf_t){}

};


// Important. Guarantees that bucket (parameters) have been initialized.
/*
 *
 *  \param T - bucket class, containing ::conf_t
 */
template <class T>
class ImpulseResponseOpBase : public ImageOp {
public:

	typedef T bucket_t;
	typename T::conf_t conf;

	inline
	ImpulseResponseOpBase(){};

	inline
	ImpulseResponseOpBase(const ImpulseResponseOpBase & op) : conf(op.conf){
	};

	inline
	//ImpulseResponseOpBase(const typename T::conf_t & conf) : conf(conf){
	ImpulseResponseOpBase(const bucket_t & conf) : conf(conf){
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

};


/// A fill operation for one color.
/**

 \tparam - Accumulating unit that also handles decoding/encoding of the values, must define ::conf

See examples in ImpulseAvgOp.h

 */
template <class T>
class ImpulseResponseOp : public ImpulseResponseOpBase<T> {

public:

	typedef T bucket_t;
	typedef typename T::conf_t conf_t;


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
		this->parameters.link("extendHorz", extendHorz, "pix"); // for avoiding border effects, include pixels beyond main area
		this->parameters.link("extendVert", extendVert, "pix"); // for avoiding border effects, include pixels beyond main area
		this->parameters.link("weightThreshold", weightThreshold, "[0..1.0]"); //

	};

	inline
	~ImpulseResponseOp(){};


	/*
	virtual	inline
	void make Compatible(const ImageFrame &src, Image &dst) const  {

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		//mout.debug3("src: " , src );

		if (dst.getType() != src.getType()){
			mout.note(" changing dst image type: " , dst.getType().name() , '>' , src.getType().name() );
		}

		dst.copyShallow(src);
		mout.warn(dst );
		// mout .debug3() << "dst: " << dst << mout.endl;

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

	int extendHorz = 0;
	int extendVert = 0;
	double weightThreshold = 0.1;
	//double undetectQuality;

};


template <class T>
void ImpulseResponseOp<T>::traverseChannel(const Channel & src, Channel & dst) const {
	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug("delegating to traverseChannel(src, empty, dst, empty)" );

	drain::image::Image empty;
	traverseChannel(src, empty, dst, empty);

}

template <class T>
void ImpulseResponseOp<T>::traverseChannel(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	dst.setScaling(src.getScaling());
	// OLD dstWeight.setScaling(srcWeight.getScaling());
	// NEW
	dstWeight.setPhysicalRange({0.0, 1.0}, true);

	mout.warn(dst.getProperties() );

	traverseChannelHorz(src, srcWeight, dst, dstWeight);
	traverseChannelVert(dst, dstWeight, dst, dstWeight);
	// DEBUG traverseChannelVert(src, srcWeight, dst, dstWeight);

}


template <class T>
void ImpulseResponseOp<T>::traverseChannelHorz(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug(*this );

	const bool WEIGHTED = !(srcWeight.isEmpty() || dstWeight.isEmpty());
	mout.attention(DRAIN_LOG(WEIGHTED));

	const int width    = src.getWidth();
	const int widthExt = src.getWidth()+extendHorz;
	const int height   = src.getHeight();
	const double defaultWeight = 1.0; //srcWeight.getMax<double>();


	const CoordinateHandler2D coordHandler(src.getGeometry().area, src.getCoordinatePolicy());
	drain::Point2D<int> point;

	T bucketOrig(this->conf); // instance
	ImpulseBucket<typename T::conf_t> & bucket = bucketOrig;
	bucket.init(src, true);

	const ValueHandler & valueHandler = bucket.getValueHandler();

	// mout.attention(DRAIN_LOG(weightThreshold));

	double value;
	double weight;

	for (int j=0; j<height; ++j){

		bucket.reset();

		if (WEIGHTED){

			for (int i=-extendHorz; i<widthExt; ++i){

				point.setLocation(i, j);
				if (coordHandler.validate(point)){
					value = src.get<double>(point); // src.getScaled(point);
					if (valueHandler.validate(value)){
						weight = srcWeight.get<double>(point);
						bucket.addLeft(point.x, value, weight);
					}
				}

				point.setLocation(width-1-i, j);
				if (coordHandler.validate(point)){
					value = src.get<double>(point); // src.getScaled(point);
					if (valueHandler.validate(value)){
						weight = srcWeight.get<double>(point);
						bucket.addRight(point.x, value, weight);
					}
				}

			}

			// Write
			for (int i=0; i<width; ++i){
				weight = bucket.getWeight(i);
				if (weight > weightThreshold){
					dst.put(i,j, bucket.get(i)); // dst.putScaled(i,j, bucket.get(i));
					dstWeight.put(i,j, bucket.getWeight(i));
				}
				else {
					dstWeight.put(i,j, 0); // TODO zero code
				}
			}

		}
		else {

			// Collect
			for (int i=-extendHorz; i<widthExt; ++i){

				point.setLocation(i, j);
				if (coordHandler.validate(point)){
					value = src.get<double>(point);
					if (valueHandler.validate(value)){
						bucket.addLeft(point.x, value, defaultWeight);
					}
				}

				point.setLocation(width-1-i, j);
				if (coordHandler.validate(point)){
					value = src.get<double>(point);
					if (valueHandler.validate(value)){
						bucket.addRight(point.x, value, defaultWeight);
					}
				}

			}

			// Write
			for (int i=0; i<width; ++i){
				dst.put(i,j, bucket.get(i));
				// dst.putScaled(i,j, bucket.get(i));
			}

		}

	}
}

template <class T>
void ImpulseResponseOp<T>::traverseChannelVert(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug(*this );

	const bool WEIGHTED = !(srcWeight.isEmpty() || dstWeight.isEmpty());

	mout.attention(DRAIN_LOG(WEIGHTED));

	const int width     = src.getWidth();
	const int height    = src.getHeight();
	const int heightExt = src.getHeight() + extendVert;
	const double defaultWeight   = 1.0; //srcWeight.getMax<double>();

	const drain::image::CoordinateHandler2D coordHandler(src.getGeometry(), src.getCoordinatePolicy());
	drain::Point2D<int> point;

	T bucketOrig(this->conf);
	ImpulseBucket<typename T::conf_t> & bucket = bucketOrig;
	bucket.init(src, false);

	const ValueHandler & valueHandler = bucket.getValueHandler();

	// NOTE: raw data values, but scaled weight values.

	double value;
	double weight;

	for (int i=0; i<width; i++){

		bucket.reset();

		if (WEIGHTED) {

			for (int j=-extendVert; j<heightExt; ++j){

				point.setLocation(i, j);
				if (coordHandler.validate(point)){
					value = src.get<double>(point); // src.getScaled(point);
					if (valueHandler.validate(value)){
						weight = srcWeight.get<double>(point); // srcWeight.getScaled(point);
						bucket.addDown(point.y, value, weight);
					}
				}

				point.setLocation(i, height-1-j);
				if (coordHandler.validate(point)){
					value = src.get<double>(point); // src.getScaled(point);
					if (valueHandler.validate(value)){
						weight = srcWeight.get<double>(point); // srcWeight.getScaled(point);
						bucket.addUp(point.y, value, weight);
					}
				}

			}

			// Write
			for (int j=0; j<height; ++j){
				weight = bucket.getWeight(j);
				if (weight > weightThreshold){ // check which are scaled?
					dst.put(i,j, bucket.get(j));
					dstWeight.put(i,j, weight);
				}
				else {
					dstWeight.put(i,j, 0); // TODO code
				}
			}

		}
		else { // (UNWEIGHTED)

			// Collect
			for (int j=-extendVert; j<heightExt; ++j){

				point.setLocation(i, j);
				if (coordHandler.validate(point)){
					value = src.get<double>(point);
					if (valueHandler.validate(value)){
						weight = srcWeight.get<double>(point); // srcWeight.getScaled(point);
						bucket.addLeft(point.y, value, defaultWeight);
					}
				}

				point.setLocation(i, height-1-j);
				if (coordHandler.validate(point)){
					value = src.get<double>(point);
					if (valueHandler.validate(value)){
						bucket.addRight(point.y, value, defaultWeight);
					}
				}

			}

			// Write
			for (int j=0; j<height; ++j){
				dst.putScaled(i,j, bucket.get(j)); // CHECK scaled
			}

		}

	}
}



} // image::

} // drain::


#endif /* ImpulseResponse_H_ */
