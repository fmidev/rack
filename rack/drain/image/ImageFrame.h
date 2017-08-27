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
#ifndef IMAGEFRAME_H_
#define IMAGEFRAME_H_ "ImageFrame 0.9,  2011.09.25 Markus.Peura@fmi.fi"

#include <stddef.h>  // size_t

#include "Geometry.h"
#include "Coordinates.h"

namespace drain
{

/// Namespace for images and image processing tools.
namespace image
{


/// A base class for images.
/*!
 * 
 */
class ImageFrame  //: public Image
{

public:

	//inline
    ImageFrame() {};
    ImageFrame(const ImageFrame &image) : coordinatePolicy(image.coordinatePolicy) {};

	inline
 	virtual ~ImageFrame(){};
 
 	inline
    virtual const Geometry & getGeometry() const {
		return geometry;
	}

	// Frequently needed convenience functions.
	inline const size_t & getWidth() const { return geometry.getWidth();};
	inline const size_t & getHeight() const { return geometry.getHeight();};
	inline const size_t & getChannelCount() const { return geometry.getChannelCount();};
	inline const size_t & getImageChannelCount() const { return geometry.getImageChannelCount();};
	inline const size_t & getAlphaChannelCount() const { return geometry.getAlphaChannelCount();};
	inline const size_t & getVolume() const { return geometry.getVolume();};
	
	inline
	bool isEmpty() const { return (geometry.getVolume()==0); };

	///
	inline
	void setCoordinatePolicy(const CoordinatePolicy &policy){
	    coordinatePolicy = policy;
	}

	inline
	const CoordinatePolicy & getCoordinatePolicy() const {
	    return coordinatePolicy;
	}

	
protected:
	Geometry geometry;

	CoordinatePolicy coordinatePolicy;
    //CoordinateLimiter defaultCoordinatePolicy;

};

// consider class ImageCoordinateHandler : public CoordinateHandler2D {



}

}

#endif /* IMAGE_FRAME_H_*/

// Drain
