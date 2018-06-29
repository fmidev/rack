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

// Drain
