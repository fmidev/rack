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


#include "util/Log.h"
#include "util/Type.h"
#include "util/TypeUtils.h"
#include "util/Castable.h"
#include "util/CastableIterator.h"


#include "util/VariableMap.h"

#include "Geometry.h"
#include "Point.h"
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

	/// Resizes the image, keeps the current type. \see initialize().
	/**

	    The buffer will always have at least one element to guarantee a safe call to begin().

	 */
	// 	Consider old feature: \return true if geometry was changed, otherwise false.
	virtual inline
	void setGeometry(size_t width, size_t height, size_t imageChannels=1, size_t alphaChannels=0){

		// const bool result =
		geometry.setGeometry(width, height, imageChannels, alphaChannels);
		adjustBuffer();
		/*
		const size_t s = geometry.getVolume() * encoding.byteSize;


		if (s > 0)
			buffer.resize(s);
		else
			buffer.resize(1);

		bufferPtr = &buffer[0];
		segmentBegin = (void *)&(*buffer.begin());
		segmentEnd   = (void *)&(*buffer.end());
		*/

		this->channelVector.clear();
		//updateChannelVector();

	}

	/// Resizes the image, keeps the current type. \see initialize().
	inline
	void setGeometry(const AreaGeometry &g){
		setGeometry(g.getWidth(), g.getHeight());
	};

	/// Resizes the image, keeps the current type. \see initialize().
	inline
	void setGeometry(const Geometry &g){
		setGeometry(g.getWidth(), g.getHeight(), g.getImageChannelCount(), g.getAlphaChannelCount());
	};


	/// Collapses the image to undetectValue size, releasing memory. @see clear().
	void resetGeometry(){
		setGeometry(0,0,0,0);
	}


	inline
	void setChannelCount(size_t ni, size_t na = 0){
		setGeometry(getWidth(), getHeight(), ni, na);
	};


	inline
	void setAlphaChannelCount(size_t k){
		setGeometry(getWidth(), getHeight(), getImageChannelCount(), k);
	};


//protected:

	/// Sets the type and allocates a data buffer.
	inline
	void initialize(const std::type_info &t, const Geometry & geometry){
		// setType(t); warns
		setStorageType(t);
		setGeometry(geometry);
	}

	/// Sets the type and allocates a data buffer.
	inline
	void initialize(const std::type_info &t, size_t width, size_t height, size_t imageChannels=1, size_t alphaChannels=0){
		// setType(t); warns
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
		//setType(t);  ///
		//setGeometry(width, height, channelCount, alphaChannelCount);
		//mout.warn() << *this << mout.endl;
	};

	/// The constructor for \c unsigned \c char image of given size.
	inline
	Image(size_t width, size_t height, size_t channelCount=1, size_t alphaChannelCount=0){
		// buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]) {
		//Logger mout(getImgLog(), "Image(w,h,c,ca)", __FUNCTION__);
		initialize(typeid(unsigned char), width, height, channelCount, alphaChannelCount);
		// setType<unsigned char>();  ///
		//setGeometry(width, height, channelCount, alphaChannelCount);
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
		if (getChannelCount() == 0){
			throw std::runtime_error("Image: no channels for cast");
		}
		else if (getChannelCount() > 1){
			Logger mout(getImgLog(), "Image", __FUNCTION__);
			mout.warn() << "several channelCount=" << getChannelCount() << ">1, returning first." << mout.endl;
		}
		return getChannel(0);
	}

	inline
	operator Channel &(){
		if (getChannelCount() == 0){
			throw std::runtime_error("Image: no channels for cast");
		}
		else if (getChannelCount() > 1){
			Logger mout(getImgLog(), "Image", __FUNCTION__);
			mout.warn() << "several channelCount=" << getChannelCount() << ">1, returning first." << mout.endl;
		}
		return getChannel(0);
	}


	/// Sets the storage type of the image - typically unsigned char, unsigned int or float.
	/* Sets the type of pixel elements of the image.
	 *
	 */
	inline
	void setType(const std::type_info &type){
		Logger mout(getImgLog(), "Image", __FUNCTION__);
		setStorageType(type);
		if (!isEmpty()){
			mout.note() << "STYLE/ changing type of allocated image" << mout.endl;
		}
		setGeometry(getGeometry());
	};

	/// Sets the storage type of the image - typically unsigned char, unsigned int or float.
	template <class T>
	inline
	void setType(){
		//Castable::setType(typeid(T));
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
		initialize(src.getType(), src.getGeometry());
		//setType(src.getType());
		setScaling(src.getScaling());
		//setGeometry(src.getGeometry());
		setCoordinatePolicy(src.getCoordinatePolicy());
	}

	/// Copies the type, geometry, coordinate policy and data of the given image.
	inline
	void copyDeep(const ImageFrame &src){
		copyShallow(src);
		copyData(src);
		/*
		const_iterator sit = src.begin();
		for (iterator it = begin(); it != end(); ++it,++sit)
		 *it = *sit;
		 */
	}



	/// Prints images geometry, buffer size and type information, and dumps the array contents. Consider toOStr...
	void dump(std::ostream &ostr = std::cout) const;



	inline
	void adoptScaling(const ImageFrame & src, const std::type_info & t = typeid(void)){
		useOwnScaling(); // needed?
		if (t == typeid(void))
			encoding.scaling.adoptScaling(src.getScaling(), src.getType(), getType());
		else
			encoding.scaling.adoptScaling(src.getScaling(), src.getType(), t);
	}



	void swap(Image & img);

protected:


	//ImageScaling scaling;


};

inline
std::ostream & operator<<(std::ostream &ostr, const Image &image){
	image.toOStr(ostr);
	//ostr << image.getWidth() << 'X' << image.getHeight() << '\n';
	return ostr;
}


//typedef ImageList std::list<Image &>;


}  // image
}  // drain




/*
std::ostream & operator<<(std::ostream &ostr, const Image &image){
  ostr << "Image size: " << image.width << 'x' << image.height << " buffer: "<< image.buffer.size() << '\n';
  ostr << "Buffer type: " << image.getType().name() << "(" << image.getByteSize() << "byte=" << (image.getByteSize()*8) << "bit) \n";
  for (size_t j=0; j<image.height; j++){
    ostr << "Row "<< j << '\n';
    for (size_t i=0; i<image.width; i++)
      //ostr << i << '|' << ' '; // << image.get<double>(i,j) << '\t';
      ostr << image.get<double>(i,j) <<'\t';
    ostr << '\n';
  };
  //std::cerr << " adios \n";// << std::endl;
  return ostr;
}
 */

#endif

// Drain
