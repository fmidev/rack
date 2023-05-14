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
#define IMAGELIKE_H_ "ImageLike 1.1,  2020/02 Markus.Peura@fmi.fi"

/*
#include <stddef.h>  // size_t

##include "../util/Caster.h"
#include "../util/ValueScaling.h"

#include "Geometry.h"
#include "Coordinates.h"
*/
#include "ImageConf.h"

namespace drain
{

/// Namespace for images and image processing tools.
namespace image
{


/// A base class for images.
/*!
 *   Generally, an ImageLike does not have to have memory. It may be a two-dimensional function,
 *   for example geti,j) may return sinusoidal field or random noise.
 */
//  Consider: in future, may be merged to ImageConf ?
//  Consider : ImageLike<CONF> : protected CONF {

class ImageLike { // public ImageConf {  // or rename ImageBase

public:

	inline
	ImageLike() {};

	inline
	ImageLike(const ImageLike &image){
		std::cerr << __FILE__ << __FUNCTION__ << " setConf/Geom has no effect\n";
		conf.setConf(image.conf);
		//conf.setEncoding(image.getEncoding());
		//conf.setCoordinatePolicy(image.getCoordinatePolicy());
	};

	/*
	inline
	ImageLike(const ImageConf &conf){
		std::cerr << __FILE__ << __FUNCTION__ << " setConf has no effect\n";
		//setConf(conf);
	}
	*/

	virtual inline
	~ImageLike(){};

	/*
	inline
	const ImageConf & getConf() const {
		return conf; // *this;
	}

	virtual inline
	const Encoding & getEncoding() const {
		return conf; // *this;
	}
	*/
	/// Get the storage type
	inline
	const std::type_info & getType() const {
		return conf.caster.getType();
	}

	inline
	bool typeIsSet() const {
		return conf.caster.typeIsSet();
	};


	virtual inline // CHECK if virtual needed
	const Geometry & getGeometry() const {
		return conf;
	}


	inline
	size_t getWidth() const {
		return conf.getWidth();
	};

	inline
	size_t getHeight() const {
		return conf.getHeight();
	};

	inline
	size_t getArea() const {
		return conf.getArea();
	};

	inline
	size_t getChannelCount() const {
		return conf.getChannelCount(); // getGeometry unneeded?
	};


	inline
	const size_t getImageChannelCount() const {
		return conf.getImageChannelCount();
	};


	inline
	const size_t getAlphaChannelCount() const {
		return conf.getAlphaChannelCount();
	};


	inline
	bool hasAlphaChannel() const {
		return conf.hasAlphaChannel();
	};


	inline
	size_t getVolume() const {
		return conf.getVolume();
	};

	inline
	bool isEmpty() const {
		return conf.isEmpty();
	};

	/// Coord policy
	inline
	const CoordinatePolicy & getCoordinatePolicy() const {
		return conf.coordinatePolicy;
	}

	/// Coord policy
	inline
	CoordinatePolicy & getCoordinatePolicy(){
		return conf.coordinatePolicy;
	}

	// TODO: lower?
	template <class T>
	inline
	void setCoordinatePolicy(const T & policy){
		conf.coordinatePolicy.set(policy);
	}

	// TODO: lower?
	inline
	void setCoordinatePolicy(coord_pol_t xUnderFlowPolicy, coord_pol_t yUnderFlowPolicy, coord_pol_t xOverFlowPolicy, coord_pol_t yOverFlowPolicy){
		conf.coordinatePolicy.set(xUnderFlowPolicy, yUnderFlowPolicy, xOverFlowPolicy, yOverFlowPolicy);
	}

	virtual inline
	const drain::ValueScaling & getScaling() const {
		return conf.getScaling();
	}


	// Notice: scaling is modifiable.
	virtual inline
	drain::ValueScaling & getScaling() {
		return conf.getScaling();
	}

	// Notice: scaling is modifiable.
	virtual inline
	void setScaling(const drain::ValueScaling & scaling){
		conf.setScaling(scaling);
	}

	// Notice: scaling is modifiable.
	virtual inline
	void setScaling(double scale, double offset){
		conf.setScaling(scale, offset);
	}



	template <class T>
	inline
	typename drain::typeLimiter<T>::value_t getLimiter() const {
		return drain::Type::call<drain::typeLimiter<T> >(conf.caster.getType());
	}





protected:

	ImageConf conf;

};


} // image::
} // drain::

#endif /* IMAGELIKE_H_*/
