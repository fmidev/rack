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
#ifndef IMAGETEMPLATE_H_
#define IMAGETEMPLATE_H_ "Image 1.3,  2010.05.05 Markus.Peura@fmi.fi"


// TODO: rename to WritableImage ?

#include <stdlib.h>  // for exit(-1) TODO: obsolete, use std::exceptions.
#include <stdexcept>
#include <vector>
#include <ostream>

#include "util/Log.h"
//#include "util/Options.h"

//#include "Geometry.h"
#include "Image.h"
#include "Intensity.h"
#include "Point.h"

namespace drain
{

/// General debugging // TODO: move to more general place
//static size_t Debug;

/// Namespace for images and image processing tools.
namespace image
{


/// A template class for images with static storage type.
/*!
 *
 *
 *  @see Image
 * 
 */
template <class T=unsigned char>
class ImageT : public ModifiableImage
{
public:


	ImageT(){
		initialize(typeid(T), 0, 0, 0, 0);
		//setStorageType(typeid(T));
	}

	ImageT(size_t width, size_t height, size_t channelCount=1, size_t alphaChannelCount=0){
		initialize(typeid(T), width,height,channelCount,alphaChannelCount);
	}

	// Creates a new image having the geometry of \code image. 
	ImageT(const ImageT<T> &image){
		initialize(typeid(T), image.getWidth(),image.getHeight(),image.getImageChannelCount(),image.getAlphaChannelCount());
		setCoordinatePolicy(image.getCoordinatePolicy());
	}

	inline
	virtual ~ImageT(){};

	inline
	const T & at(size_t i) const {
		return *retrieve<const T>(address(i));
		//return *(const T *)&buffer[ address(i)*encoding.byteSize ];
	}

	inline
	T & at(size_t i) {
		return *retrieve<T>(address(i));
		//return *(T *) retrieve(i);
		//return *(T *)&buffer[ address(i)*encoding.byteSize ];
	}

	inline
	const T & at(size_t i, size_t j) const {
		return *(T *) retrieve<T>(i);
		//return *(const T *)&buffer[ address(i)*encoding.byteSize ];
	};

	inline
	T & at(size_t i, size_t j){
		return *(T *)&buffer[ address(i,j)*encoding.byteSize ];
	};

	/*
	inline
	const T & at(size_t i, size_t j, size_t k) const {
		return *(const T *)&buffer[ address(i,j,k)*encoding.byteSize ];
	};

	inline
	T & at(const size_t i, const size_t j, const size_t &) {
		return *(T *)&buffer[ address(i,j,k)*encoding.byteSize ];
	};

	inline
	const T & at(const Point2D<int> &p) const {
		return (const T *)&buffer[ address(p.x,p.y) ];
	};

	inline
	T & at(const Point2D<int> &p) {
		return (T *) & buffer[ address(p.x,p.y) ];
	};
	*/


};







}

}

#endif

// Drain
