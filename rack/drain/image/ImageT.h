/**

    Copyright 2001 - 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

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
	const T & at(const size_t &i) const {
		return *(const T *)&buffer[ address(i)*byteSize ];
	}

	inline
	T & at(const size_t &i) {
		return *(T *)&buffer[ address(i)*byteSize ];
	}

	inline
	const T & at(const size_t &i, const size_t &j) const {
		return *(const T *)&buffer[ address(i)*byteSize ];
	};

	inline
	T & at(const size_t &i, const size_t &j){
		return *(T *)&buffer[ address(i,j)*byteSize ];
	};

	inline
	const T & at(const size_t &i, const size_t &j, const size_t &k) const {
		return *(const T *)&buffer[ address(i,j,k)*byteSize ];
	};

	inline
	T & at(const size_t &i, const size_t &j, const size_t &k) {
		return *(T *)&buffer[ address(i,j,k)*byteSize ];
	};

	inline
	const T & at(const Point2D<int> &p) const {
		return (const T *)&buffer[ address(p.x,p.y) ];
	};

	inline
	T & at(const Point2D<int> &p) {
		return (T *) & buffer[ address(p.x,p.y) ];
	};



};







}

}

#endif
