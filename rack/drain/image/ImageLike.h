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
#ifndef IMAGELIKE_H_
#define IMAGELIKE_H_ "ImageLike 0.1,  2017/10 Markus.Peura@fmi.fi"

#include <stddef.h>  // size_t

#include "../util/Caster.h"
#include "Geometry.h"
#include "Coordinates.h"
#include "ImageScaling.h"

namespace drain
{

/// Namespace for images and image processing tools.
namespace image
{



class Encoding { //: public drain::Caster, public ImageScaling {

public:

	inline
	Encoding(){
		setType(typeid(unsigned char));
	}

	Encoding(const Encoding & encoding){
		setType(encoding.caster.getType());
	}

	/// In base class(es), mainly for storing storage type. In derived classes, also for value conversions.
	Caster caster;

	// Size of the storage type (1 for 8 bits, 2 for 16 bits, etc.)
	size_t byteSize;

	/// Information of the current type.
	std::string type;  // synch?

	/// Linear scaling
	ImageScaling scaling;

	/// Get the storage type
	inline
	const std::type_info & getType() const {
		return caster.getType();
	}

	/// Set storage type
	void setType(const std::type_info & t){

		if (t == typeid(bool)){
			//mout.warn() << "storage type 'bool' not supported, using 'unsigned char'" << mout.endl;
			setType(typeid(unsigned char)); // re-invoke
		}
		else if (t == typeid(std::string)){
			//mout.error() << "storage type 'std::string' not applicable to images" << mout.endl;
			//setType(typeid(unsigned char));
			throw std::runtime_error("storage type 'std::string' not applicable to images");
			return;
		}

		caster.setType(t);
		byteSize = caster.getByteSize();
		type.resize(1);
		type.at(0) = drain::Type::getTypeChar(caster.getType());

	}

	/// Convenience
	template <class T>
	inline
	void setType(){
		setType(typeid(T));
	}

	template <class T>
	inline
	typename drain::typeLimiter<T>::value_t getLimiter() const {
		return drain::Type::call<drain::typeLimiter<T> >(caster.getType());
	}

	/// Returns the minimum value supported by the current storage type.
	/**
	 *   Does not check the actual image data.
	 *
	 *   \see getMax()
	 *   \see getLimiter()
	 *   \see setType()
	 */
	template <class T>
	inline
	T getTypeMin() const {
		return Type::call<typeMin, T>(caster.getType());
	}

	/// Returns the maximum value supported by the current storage type. \see setType()
	/**
	 *   Does not check the actual image data.
	 *
	 *   \see getMin()
	 *   \see getLimiter()
	 *   \see setType()
	 */
	template <class T>
	inline
	T getTypeMax() const {
		return Type::call<typeMax, T>(caster.getType());
	}

	/// Returns the size in bytes of the storage type (1 for unsigned char, 2 for 16-bit types, etc.)
	inline
	size_t getByteSize() const { return byteSize; };

	/*
	  problem: scaling may be linked ie different than that of encoding
	inline
	void setPhysicalScale(double min, double max){
		scaling.setPhysicalScale(caster.getType(), min, max);
	}
	*/






};


/// Struct for image data.
class ImageConf {

public:


	inline
	ImageConf(const drain::Type & t = typeid(unsigned char), size_t width=0, size_t height=0, size_t imageChannels=1, size_t alphaChannels=0) {
		encoding.setType(t);
		// encoding.scaling...
		geometry.setArea(width, height ? height : width);
		geometry.setChannelCount(imageChannels, alphaChannels);
	}

	//inline 	ImageConf() : byteSize(0) {	setType(typeid(void));}

	inline
	ImageConf(const ImageConf & conf) : encoding(conf.encoding), geometry(conf.geometry), coordinatePolicy(conf.coordinatePolicy) {
		//encoding.setType(conf.getType());
	}


	// A single channel may have specific scaling, or scaling linked to main image object.
	//ImageScaling scaling;
	Encoding encoding;

	/// Dimensions of the image: width, height, image channel count, alpha channel count
	Geometry geometry;

	/// Rules to handle under- and overflows of horizontal and vertical coordinates
	CoordinatePolicy coordinatePolicy;



};



/// A base class for images.
/*!
 *   Generally, an ImageLike does not have to have memory. It may be a two-dimensional function,
 *   for example geti,j) may return sinusoidal field or random noise.
 */
//  Consider: in future, may be merged to ImageConf ?
//  Consider : ImageLike<CONF> : protected CONF {
class ImageLike : protected ImageConf {  // or rename ImageBase

public:

	inline
	ImageLike() {};

	inline
	ImageLike(const ImageLike &image){
		setConf(image);
		/*
		encoding.setType(image.getType());
		encoding.scaling.set(image.encoding.scaling);
		geometry.setGeometry(image.getGeometry());
		coordinatePolicy.set(image.coordinatePolicy);
		 */
	};

	inline
	ImageLike(const ImageConf &conf){
		setConf(conf);
	}


	virtual inline
	~ImageLike(){};


	inline
	void setConf(const ImageConf &conf) {
		encoding.setType(conf.encoding.getType());
		encoding.scaling.set(conf.encoding.scaling);
		geometry.setGeometry(conf.geometry);
		coordinatePolicy.set(conf.coordinatePolicy);
	}
	// ENCODING/SCALING

	// REMOVE THIS..?
	virtual inline
	const Encoding & getEncoding() const {
		return encoding;
	}


	/// Returns the storage type of the image (typically unsigned char, unsigned int or float).
	/*
	 */
	inline
	const std::type_info & getType() const { return encoding.caster.getType(); };

	/// Returns true, if type is set.
	inline
	bool typeIsSet() const {
		return encoding.caster.typeIsSet();
	};

	// ENCODING/SCALING

	inline
	void setScaling(const ImageScaling &s) {
		getScaling().set(s); // note: may call useOwnScaling();
	}

	inline
	void setScaling(double scale, double offset=0.0) {
		getScaling().setScale(scale, offset); // note: may call useOwnScaling();
	}

	// ImageScaling
	virtual inline
	const ImageScaling & getScaling() const {
		return encoding.scaling;
	}

	virtual inline
	ImageScaling & getScaling() {
		return encoding.scaling;
	}


	// These must be here, because getScaling() may be behind a link.
	// Consider getEncoding

	/// Returns the actual or guessed maximum physical value,
	/**
	 *  Is physical range is unset, and the storage type is a "small" integer, the value is guessed.
	 */
	inline // double defaultMax = std::numeric_limits<double>::max()
	double requestPhysicalMax(double defaultMax = static_cast<double>(std::numeric_limits<short int>::max())) const {

		const ImageScaling & s = getScaling();

		if (s.isPhysical())
			return s.getMaxPhys();
		else {
			const std::type_info & t = getType();
			if (Type::call<drain::typeIsSmallInt>(t))
				return s.fwd(Type::call<typeMax, double>(t));
			else
				return defaultMax;
		}
	}

	/// Returns the actual or guessed minimum physical value,
	/**
	 *  Is physical range is unset, and the storage type is a "small" integer, the value is guessed.
	 */
	inline
	double requestPhysicalMin(double defaultMin = static_cast<double>(std::numeric_limits<short int>::min())) const {

		const ImageScaling & s = getScaling();

		if (s.isPhysical())
			return s.getMinPhys();
		else {
			const std::type_info & t = getType();
			if (Type::call<drain::typeIsSmallInt>(t))
				return s.fwd(Type::call<typeMin, double>(t));
			else
				return defaultMin;
		}
	}


	// GEOMETRY

	virtual	inline
	const Geometry & getGeometry() const {
		return geometry;
	}

	// Frequently needed convenience functions.

	inline
	size_t getWidth() const { return getGeometry().getWidth();};

	inline
	size_t getHeight() const { return getGeometry().getHeight();};

	inline
	size_t getChannelCount() const { return getGeometry().getChannelCount();};

	inline
	const size_t getImageChannelCount() const { return getGeometry().getImageChannelCount();};

	inline
	const size_t getAlphaChannelCount() const { return getGeometry().getAlphaChannelCount();};

	inline
	bool hasAlphaChannel() const { return (getGeometry().getAlphaChannelCount()>0);};

	inline
	const size_t getVolume() const { return getGeometry().getVolume();};

	inline
	bool isEmpty() const { return (getGeometry().getVolume() == 0);	};


	// Modifiable (read & write) properties.

	/// Does not set any CoordinateHandler object.
	template <class T>
	inline
	void setCoordinatePolicy(const T & policy){
		coordinatePolicy.set(policy);
	}

	inline
	void setCoordinatePolicy(int xUnderFlowPolicy, int yUnderFlowPolicy, int xOverFlowPolicy, int yOverFlowPolicy){
		coordinatePolicy.set(xUnderFlowPolicy, yUnderFlowPolicy, xOverFlowPolicy, yOverFlowPolicy);
	}

	inline
	const CoordinatePolicy & getCoordinatePolicy() const {
		return coordinatePolicy;
	}

	/// Copies image limits and coordinate overflow policy to external coordinate handler.
	inline
	void adjustCoordinateHandler(CoordinateHandler2D & handler) const {
		handler.setLimits(getGeometry().getWidth(), getGeometry().getHeight());
		handler.setPolicy(getCoordinatePolicy());
	};



protected:


};




} // image::
} // drain::

#endif /* IMAGE_SCALING_H_*/

// Drain
