/**

    Copyright 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef IMAGELIKE_H_
#define IMAGELIKE_H_ "ImageLike 0.1,  2017/10 Markus.Peura@fmi.fi"

#include <stddef.h>  // size_t


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
class ImageLike
{

public:

	//inline
    ImageLike() {};
    ImageLike(const ImageLike &image) : coordinatePolicy(image.coordinatePolicy) {};

	inline
 	virtual ~ImageLike(){};
 
	virtual
	inline
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
	bool isEmpty() const { return (getGeometry().getVolume()==0); };

	/// Does not set any CoordinateHandler object.
	template <class T>
	inline
	void setCoordinatePolicy(const T & policy){
	    coordinatePolicy.set(policy);
	}

	inline
	const CoordinatePolicy & getCoordinatePolicy() const {
	    return coordinatePolicy;
	}

	/// Copies image limits and coordinate overflow policy to external coordinate handler.
	inline
	void adjustCoordinateHandler(CoordinateHandler2D & handler) const {
		handler.setLimits(getWidth(), getHeight());
		handler.setPolicy(getCoordinatePolicy());
	};

	
protected:

	Geometry geometry;

	CoordinatePolicy coordinatePolicy;


};




} // image::
} // drain::

#endif /* IMAGE_SCALING_H_*/
