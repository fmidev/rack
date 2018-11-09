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
#include <utility>

#include "image/Coordinates.h"
#include "image/FilePng.h"
//#include "image/SegmentProber.h"

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

	/// When traversing down or right, add a encoded value to bucket in position i.
	virtual
	void add1(int i, double value, double weight) = 0;

	/// When traversing up or left, add a encoded value to bucket in position i.
	virtual
	void add2(int i, double value, double weight) = 0;

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
   drainage shapes.png --impulseAvg  0.9,5,5 -o shapes-impulse-90.png
   drainage shapes.png --impulseAvg  0.5,5,5 -o shapes-impulse-50.png
   drainage shapes.png --impulseAvg  0.1,5,5 -o shapes-impulse-10.png
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
		this->parameters.reference("extendHorz", extendHorz = 0, "pix"); // for avoiding border effects, include pixels beyond main area
		this->parameters.reference("extendVert", extendVert = 0, "pix"); // for avoiding border effects, include pixels beyond main area
		this->parameters.reference("weightThreshold", weightThreshold = 0.05, "[0..1.0]"); //

	};

	inline
	~ImpulseResponseOp(){};


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
	Logger mout(getImgLog(), this->getName(), __FUNCTION__);
	mout.debug() << "delegating to traverseChannel(src, empty, dst, empty)" << mout.endl;
	//dst.properties.updateFromMap(src.getProperties());
	drain::image::Image empty;
	traverseChannel(src, empty, dst, empty);
	//traverseChannelHorz(src, empty, dst, empty);
	//traverseChannelVert(dst, empty, dst, empty);

}

template <class T>
void ImpulseResponseOp<T>::traverseChannel(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), this->getName(), __FUNCTION__);

	//dst.properties.updateFromMap(src.getProperties());
	//mout.note() << dst.getProperties() << mout.endl;

	traverseChannelHorz(src, srcWeight, dst, dstWeight);
	traverseChannelVert(dst, dstWeight, dst, dstWeight);

}


template <class T>
void ImpulseResponseOp<T>::traverseChannelHorz(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), this->getName(), __FUNCTION__);

	mout.debug() << *this << mout.endl;

	const bool UNWEIGHTED = (srcWeight.isEmpty() || dstWeight.isEmpty());

	const int width    = src.getWidth();
	const int widthExt = src.getWidth()+extendHorz;
	const int height   = src.getHeight();
	const double defaultWeight = 1.0; //srcWeight.getMax<double>();

	//const double weightThreshold = 0.1;
	double w;

	const drain::image::CoordinateHandler2D coordHandler(src.getGeometry(), src.getCoordinatePolicy());

	drain::image::Point2D<int> point;

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
				bucket.add1(point.x, src.get<double>(point.x, point.y), defaultWeight);
				//}

				point.setLocation(width-1-i, j);
				coordHandler.handle(point);
				// if (coordHandler.validate(point)){
				bucket.add2(point.x, src.get<double>(point.x, point.y), defaultWeight);
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
				bucket.add1(point.x, src.get<double>(point.x, point.y), srcWeight.getScaled(point.x, point.y));
				//}

				point.setLocation(width-1-i, j);
				coordHandler.handle(point);
				// if (coordHandler.validate(point)){
				bucket.add2(point.x, src.get<double>(point.x, point.y), srcWeight.getScaled(point.x, point.y));
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

	Logger mout(getImgLog(), this->getName(), __FUNCTION__);

	mout.debug() << *this << mout.endl;

	const bool UNWEIGHTED = (srcWeight.isEmpty() || dstWeight.isEmpty());

	const int width     = src.getWidth();
	const int height    = src.getHeight();
	const int heightExt = src.getHeight() + extendVert;
	const double defaultWeight   = 1.0; //srcWeight.getMax<double>();
	//const double weightThreshold = 0.1;
	double w;

	const drain::image::CoordinateHandler2D coordHandler(src.getGeometry(), src.getCoordinatePolicy());

	drain::image::Point2D<int> point;

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
				bucket.add1(point.y, src.get<double>(point.x, point.y), defaultWeight);

				point.setLocation(i, height-1-j);
				coordHandler.handle(point); //if (coordHandler.validate(point))
				bucket.add2(point.y, src.get<double>(point.x, point.y), defaultWeight);

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
				bucket.add1(point.y, src.get<double>(point.x, point.y), srcWeight.getScaled(point.x, point.y));

				point.setLocation(i, height-1-j);
				coordHandler.handle(point); // if (coordHandler.validate(point)){
				bucket.add2(point.y, src.get<double>(point.x, point.y), srcWeight.getScaled(point.x, point.y));

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








// IMPLEMENTATIONS


struct ImpulseAvgConf : public BeanLike {

	inline
	ImpulseAvgConf() : BeanLike(__FUNCTION__, "yes"){
		// this->parameters.reference("decayHorz", decayHorz = 0.9);
		// this->parameters.reference("decayVert", decayVert = 0.9);
		this->parameters.reference("decay", decay = 0.9);
		// this->parameters.reference("decayVert", decayVert = 0.9);
	};

	inline
	ImpulseAvgConf(const ImpulseAvgConf & conf) : BeanLike(__FUNCTION__, "yes"){
		this->parameters.reference("decay", decay = conf.decay);
		// this->parameters.reference("decayHorz", decayHorz = conf.decayHorz);
		// this->parameters.reference("decayVert", decayVert = conf.decayHorz);
	};

	double decay;
	//double decayHorz;
	//double decayVert;

};


/// Averaging operator. A simple example implementation of ImpulseBucket
/**
 \code
   drainage image.png --impulseAvg  0.5            -o impulseAvg.png
   drainage image.png --impulseAvg  0.2,20,20      -o impulseAvgMarg.png
   drainage image-rgba.png --target S --impulseAvg  0.5 -o impulseAvg-16b.png
 \endcode
 */
class ImpulseAvg : public ImpulseBucket<ImpulseAvgConf> {

public:


	inline
	ImpulseAvg(){

	};

	inline
	ImpulseAvg(const ImpulseAvg & avg){
		decay = avg.decay;
	}

	inline
	ImpulseAvg(const ImpulseAvgConf & conf){
		decay = conf.decay;
	}

	virtual
	void init(const Channel & src, bool horizontal);

	virtual
	void reset();

	/// Accumulate encoded value
	virtual
	void add1(int i, double value, double weight);

	/// Accumulate encoded value
	virtual
	void add2(int i, double value, double weight);

	virtual
	double get(int i);

	virtual
	double getWeight(int i);


protected:

private:

	ImageScaling scaling;

	/// Accumulating unit using natural values
	struct entry {

		double x;
		double w;

		inline void set(double value, double weight){
			x = value;
			w = weight;
		}

	};

	/*
	 *  \param xNew - value to be added
	 *  \param wNew - weight of xNew
	 */
	inline
	void mix(entry & prev, const entry & e, double decay){

		double w1 = decay*e.w;
		double w2 = (1.0-decay);

		if (decay < 1.0)
			prev.x =(w1*e.x + w2*prev.x) / (w1 + w2);
		else // decay==1.0
			prev.x = e.x;

		prev.w = w1 + w2*prev.w;

	}

	typedef std::pair<entry,entry> entryPair;
	typedef std::vector<entryPair> container;

	container data;

	entry e;
	entryPair latest; // utility


};



} // image::

} // drain::


#endif /* ImpulseResponse_H_ */
