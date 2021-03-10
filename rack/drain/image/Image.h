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
#include <typeinfo>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>


#include "drain/util/Log.h"
#include "drain/util/Type.h"
#include "drain/util/TypeUtils.h"
#include "drain/util/Castable.h"
#include "drain/util/CastableIterator.h"
#include "drain/util/Point.h"
#include "drain/util/VariableMap.h"

#include "Geometry.h"
#include "Intensity.h"
#include "ImageFrame.h"
#include "ImageChannel.h"


#ifndef DRAIN_IMAGE
#define DRAIN_IMAGE  "drain::Image v3.0 2017/10 Markus.Peura@fmi.fi"


namespace drain {

namespace image {

/// Image with modifiable geometry.
class ModifiableImage : public MultiChannel {

public:

	virtual inline
	const ImageConf & getConf() const {
		return conf;
	}

	/*
	virtual inline
	ImageConf & getConf() {
		return conf;
	}
	*/


	inline
	void setConf(const ImageConf &conf) {
		this->conf.setConf(conf);
		//setCoordinatePolicy(conf.getCoordinatePolicy());
		//setGeometry(conf);
		update();
	}

	/// Resizes the image, keeps the current type. \see initialize().
	/**

	    The buffer will always have at least one element to guarantee a safe call to begin().

	 */
	// 	Consider old feature: \return true if geometry was changed, otherwise false.
	virtual inline
	void setGeometry(size_t width, size_t height, size_t imageChannels=1, size_t alphaChannels=0){
		conf.getGeometry().set(width, height, imageChannels, alphaChannels);
		// geometry.setGeometry(width, height, imageChannels, alphaChannels);
		update();
	}

	/// Resizes the image, keeps the current type. \see initialize().
	inline
	void setGeometry(const AreaGeometry &g){
		conf.channels.set(1, 0); // ? CONSIDER area geom?
		conf.area.set(g);
		//setGeometry(g.getWidth(), g.getHeight());
		update();
	};

	/// Resizes the image, keeps the current type. \see initialize().
	inline
	void setGeometry(const Geometry &g){
		conf.getGeometry().assign(g);
		update();
	};


	/// Collapses the image to undetectValue size, releasing memory. @see clear().
	void resetGeometry(){
		setGeometry(0,0,0,0);
	}


	inline
	void setChannelCount(size_t ni, size_t na = 0){
		conf.channels.set(ni, na);
		update();
	};


	inline
	void setAlphaChannelCount(size_t k){
		conf.channels.setAlphaChannelCount(k);
		update();
		//setGeometry(getWidth(), getHeight(), getImageChannelCount(), k);
	};


	/// Sets the type and allocates a data buffer.
	virtual inline
	void initialize(const std::type_info &t, const Geometry & g){
		setStorageType(t);
		setGeometry(g);
		//initialize(t, g.area.getWidth(), g.area.getHeight(), g.channels.getImageChannelCount(), g.channels.getAlphaChannelCount());
	}

	/// Sets the type and allocates a data buffer.
	virtual inline
	void initialize(const std::type_info &t, size_t width, size_t height, size_t imageChannels=1, size_t alphaChannels=0){
		setStorageType(t);
		setGeometry(width, height, imageChannels, alphaChannels);
	}

	/// Sets the type and allocates a data buffer.
	/*
	template <class T>
	inline
	void initialize(size_t width, size_t height, size_t imageChannels=1, size_t alphaChannels=0){
		setStorageType(typeid(T));
		setGeometry(width,height,imageChannels,alphaChannels);
	}
	*/
private:

	void update();

};



/// Class for multi-channel digital images. Supports dynamic typing with base types (char, int, float).
/**
 *   Contains an internal buffer for image data. The buffer is essentially a std::vector<char>.
 *   The size of the buffer is dynamically adjusted to the dimensions and bit resolution of the image.
 *
 *   Also, supports STL style iterators.
 */
class Image : public ModifiableImage { //, protected Castable {

public:

	/// The constructor for an empty image.
	inline
	Image(const std::type_info & t = typeid(unsigned char), const Geometry & g = Geometry()){  //: buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]) {
		//Logger mout(getImgLog(), "Image(T, g)", __FUNCTION__);
		initialize(t,g);
		//mout.warn() << *this << mout.endl;
	};

	/// The constructor for an image of given type and size.
	inline
	Image(const std::type_info & t, size_t width, size_t height, size_t channelCount=1, size_t alphaChannelCount=0){
		// : buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]) {
		//Logger mout(getImgLog(), "Image(T, w,h,c,ca)", __FUNCTION__);
		initialize(t, Geometry(width, height, channelCount, alphaChannelCount));
		//mout.warn() << *this << mout.endl;
	};

	/// The constructor for \c unsigned \c char image of given size.
	inline
	Image(size_t width, size_t height, size_t channelCount=1, size_t alphaChannelCount=0){
		// buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]) {
		//Logger mout(getImgLog(), "Image(w,h,c,ca)", __FUNCTION__);
		initialize(typeid(unsigned char), width, height, channelCount, alphaChannelCount);
		//mout.warn() << *this << mout.endl;
	};

	/// Copies the geometry of another image.
	inline
	Image(const Image & image){
		copyShallow(image);
	};

	/// Copies the geometry of another image.
	inline
	Image(const ImageFrame & image){
		copyShallow(image);
	};

	inline
	operator const Channel &() const {
		if (conf.getChannelCount() == 0){
			throw std::runtime_error("Image: no channels for cast");
		}
		else if (conf.getChannelCount() > 1){
			Logger mout(getImgLog(), "Image", __FUNCTION__);
			mout.warn() << "several channelCount=" << conf.getChannelCount() << ">1, returning first." << mout.endl;
		}
		return getChannel(0);
	}

	inline
	operator Channel &(){
		if (conf.getChannelCount() == 0){
			throw std::runtime_error("Image: no channels for cast");
		}
		else if (conf.getChannelCount() > 1){
			Logger mout(getImgLog(), "Image", __FUNCTION__);
			mout.warn() << "several channelCount=" << conf.getChannelCount() << ">1, returning first." << mout.endl;
		}
		return getChannel(0);
	}


	/// Sets the storage type of the image - typically unsigned char, unsigned int or float.
	/* Sets the type of pixel elements of the image.
	 *
	 */
	inline
	void setType(const std::type_info &type){
		Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		setStorageType(type);
		if (!conf.isEmpty()){
			mout.note() << "STYLE/ changing type of allocated image" << mout.endl;
		}
		conf.setGeometry(getGeometry());
	};

	/// Sets the storage type of the image - typically unsigned char, unsigned int or float.
	template <class T>
	inline
	void setType(){
		setType(typeid(T));
	}

	/// Sets type of the image, applying character mapping of Type::setType(char c) .  TODO: drain::Type
	/**

	 */
	template <class T>
	inline
	void setType(const T & t){
		setType(drain::Type::getTypeInfo(t));
	}



	/// Copies type, geometry and coordinate under/overflow policy of the given image. Does not copy the data.
	inline
	void copyShallow(const ImageFrame & src){
		setConf(src.getConf()); // update() must be called
		//updateChannelVector();
		/*
		setStorageType(src.getType());
		conf.setScaling(src.getScaling());  // NOTE: pointer-selected
		conf.setGeometry(src.getGeometry());
		setCoordinatePolicy(src.getCoordinatePolicy());
		*/
		//initialize(src.getType(), src.getGeometry());
		//setScaling(src.getScaling());
		//setCoordinatePolicy(src.getCoordinatePolicy());
	}

	/// Copies the type, geometry, coordinate policy and data of the given image.
	inline
	void copyDeep(const ImageFrame &src){
		copyShallow(src);
		copyData(src);
	}


	/// Prints images geometry, buffer size and type information, and dumps the array contents. Consider toOStr...
	void dump(std::ostream &ostr = std::cout) const;


	inline
	void adoptScaling(const ImageConf & src, const std::type_info & t = typeid(void)){
		//conf.useOwnScaling(); // needed?
		if (t == typeid(void))
			conf.adoptScaling(src.getScaling(), src.getType(), conf.getType());
		else
			conf.adoptScaling(src.getScaling(), src.getType(), t);
	}

	void swap(Image & img);

};

inline
std::ostream & operator<<(std::ostream &ostr, const Image &image){
	image.toOStr(ostr);
	//ostr << image.getWidth() << 'X' << image.getHeight() << '\n';
	return ostr;
}

}  // image::

}  // drain::


#endif

