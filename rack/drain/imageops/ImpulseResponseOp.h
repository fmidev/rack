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

#include "image/Coordinates.h"
#include "image/FilePng.h"
#include "image/SegmentProber.h"

#include "ImageOp.h"

namespace drain
{
namespace image
{

/*
class Conf  : public BeanLike {


	inline
	Conf(const std::string & name, const std::string & description="") : BeanLike(name, description) {
	}


};
 */

/**
 *  \tparam C - conf type, implementing drain::BeanLike concept (getName, getDescription, getParameters)
 */
template <class C>
class ImpulseCumulator : public C {

public:

	~ImpulseCumulator(){}

	typedef C conf_t;

	/// Adapt to input geometry, type, and scaling.
	virtual inline
	void init(const Channel & src, bool horizontal = true){};

	/// Clear statistics before traversing each row or column.
	virtual
	void reset() = 0;

	/// When traversing down or right, add a value to bucket in position i.
	virtual
	void add1(int i, double value, double weight = 1.0) = 0;

	/// When traversing up or left, add a value to bucket in position i.
	virtual
	void add2(int i, double value, double weight = 1.0) = 0;

	/// Return value, typically final & filtered value, at position i.
	virtual
	double get(int i) = 0;

	/// Return weight (confidence) at position i.
	virtual
	double getWeight(int i) = 0;


};


// Important. Guarantees that bucket (parameters) have been initialized.

template <class T>
class ImpulseResponseOpBase : public ImageOp {
public:

	/*
	inline
	ImpulseResponseOpBase(const std::string & name, const std::string & description="") : ImageOp(name, description){
	};
	 */
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
 \code
   drainage shapes.png --impulseAvg  100,100,min=50,value=128 -o shapes-fill.png
 \endcode
 */
template <class T>
class ImpulseResponseOp : public ImpulseResponseOpBase<T> {

public:

	inline
	ImpulseResponseOp() {
		this->parameters.append(this->conf.getParameters());
		this->parameters.reference("extendHorz", extendHorz = 0, "pix"); // for avoiding border effects, include pixels beyond main area
		this->parameters.reference("extendVert", extendVert = 0, "pix"); // for avoiding border effects, include pixels beyond main area
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


protected:

	int extendHorz;
	int extendVert;

};


template <class T>
void ImpulseResponseOp<T>::traverseChannel(const Channel & src, Channel & dst) const {

	//drain::image::Image tmp;
	//tmp.copyShallow(dst);

	drain::image::Image empty;

	traverseChannelHorz(src, empty, dst, empty);
	traverseChannelVert(dst, empty, dst, empty);

}

template <class T>
void ImpulseResponseOp<T>::traverseChannel(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const {

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
	const double defaultWeight = srcWeight.getMax<double>();

	const drain::image::CoordinateHandler2D coordHandler(src.getGeometry(), src.getCoordinatePolicy());

	drain::image::Point2D<int> point;

	T bucket(this->conf);
	bucket.init(src, true);


	for (int j=0; j<height; ++j){

		bucket.reset();

		if (UNWEIGHTED){

			// Collect
			for (int i=-extendHorz; i<widthExt; ++i){

				point.setLocation(i, j);
				if (coordHandler.validate(point)){
					bucket.add1(point.x, src.get<double>(point), defaultWeight);
				}

				point.setLocation(width-1-i, j);
				if (coordHandler.validate(point)){
					bucket.add2(point.x, src.get<double>(point), defaultWeight);
				}

			}

			// Write
			for (int i=0; i<width; ++i){
				dst.put(i,j, bucket.get(i));
			}

		}
		else {

			// Collect
			for (int i=-extendHorz; i<widthExt; ++i){

				point.setLocation(i, j);
				if (coordHandler.validate(point)){
					bucket.add1(point.x, src.get<double>(point), srcWeight.get<double>(point));
				}

				point.setLocation(width-1-i, j);
				if (coordHandler.validate(point)){
					bucket.add2(point.x, src.get<double>(point), srcWeight.get<double>(point));
				}

			}

			// Write
			for (int i=0; i<width; ++i){
				dst.put(i,j, bucket.get(i));
				dstWeight.put(i,j, bucket.getWeight(i));
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
	const int heightExt = src.getHeight()+extendVert;
	const double defaultWeight = srcWeight.getMax<double>();

	const drain::image::CoordinateHandler2D coordHandler(src.getGeometry(), src.getCoordinatePolicy());

	drain::image::Point2D<int> point;

	T bucket(this->conf);
	bucket.init(src, false);

	for (int i=0; i<width; i++){

		bucket.reset();

		if (UNWEIGHTED){

			// Collect
			for (int j=-extendVert; j<heightExt; ++j){

				point.setLocation(i, j);
				if (coordHandler.validate(point)){
					bucket.add1(point.y, src.get<double>(point), defaultWeight);
				}

				point.setLocation(i, height-1-j);
				if (coordHandler.validate(point)){
					bucket.add2(point.y, src.get<double>(point), defaultWeight);
				}

			}

			// Write
			for (int j=0; j<height; ++j){
				dst.put(i,j, bucket.get(j));
			}

		}
		else {

			// Collect
			for (int j=-extendVert; j<heightExt; ++j){

				point.setLocation(i, j);
				if (coordHandler.validate(point)){
					bucket.add1(point.y, src.get<double>(point), srcWeight.get<double>(point));
				}

				point.setLocation(i, height-1-j);
				if (coordHandler.validate(point)){
					bucket.add2(point.y, src.get<double>(point), srcWeight.get<double>(point));
				}
			}

			// Write
			for (int j=0; j<height; ++j){
				dst.put(i,j, bucket.get(j));
				dstWeight.put(i,j, bucket.getWeight(j));
			}

		}


	}
}








// IMPLEMENTATIONS


struct ImpulseAvgConf : public BeanLike {

	inline
	ImpulseAvgConf() : BeanLike(__FUNCTION__, "yes"){
		this->parameters.reference("decay", decay = 0.9);
	};

	double decay;

};


/// Averaging operator. A simple example implementation of ImpulseCumulator
/**
 \code
   drainage image.png --impulseAvg  0.8       -o impulseAvg.png
   drainage image.png --impulseAvg  0.8,40,20 -o impulseAvgMarg.png
 \endcode
 */
class ImpulseAvg : public ImpulseCumulator<ImpulseAvgConf> {

public:


	inline
	ImpulseAvg() :  unDecay(0.0), prev1(0), prev2(0) {

	};

	inline
	ImpulseAvg(const ImpulseAvg & avg) : unDecay(0.0), prev1(0), prev2(0) {
		decay = avg.decay;
	}

	inline
	ImpulseAvg(const ImpulseAvgConf & conf) : unDecay(0.0), prev1(0), prev2(0) {
		decay = conf.decay;
	}


	virtual inline
	void init(const Channel & src, bool horizontal){
		const size_t n = horizontal ? src.getWidth() : src.getHeight();
		data.resize(n);
	}

	virtual
	void reset();

	virtual
	void add1(int i, double value, double weight = 1.0);

	virtual
	void add2(int i, double value, double weight = 1.0);

	virtual
	double get(int i);

	virtual
	double getWeight(int i);


protected:

private:

	struct entry {
		double value1;
		double weight1;
		double value2;
		double weight2;
	};

	double unDecay; // 1-decay;

	std::vector<entry> data;

	double prev1, prev2;

};



} // image::

} // drain::


#endif /* ImpulseResponse_H_ */
