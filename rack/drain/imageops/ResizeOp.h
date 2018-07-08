/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef ResizeOP_H_
#define ResizeOP_H_

#include "ImageOp.h"

namespace drain
{

namespace image
{
// help copy


/// Resizes the image, stretching the image correspondingly.
/**
  \see ScaleOp

  To shrink an image:
  \code
    drainage image.png --resize 100,100  -o resizeShrink.png
  \endcode

  To expand an image using nearest-pixel interpolation:
  \code
    drainage image.png --resize 1000,800    -o resizeExpand.png
    drainage image.png --resize 1000,800,n  -o resizeExpand.png
  \endcode

  To expand an image using bilinear interpolation:
  \code
  	  drainage image.png --resize 1000,800,b  -o resizeExpandBilinear.png
  \endcode

 */	
class ResizeOp : public ImageOp {

public:

	ResizeOp(size_t width=0, size_t height=0, const std::string & interpolation = "n") : ImageOp(__FUNCTION__, "Resize geometry and scale intensities") {
		this->parameters.reference("width",  this->width=width, "pix");
		this->parameters.reference("height", this->height=height, "pix");
		this->parameters.reference("interpolation", this->interpolation=interpolation, "n=nearest,b=bilinear");
		this->parameters.reference("scale", this->scale = 1.0, "rescaling factor");
	}

	inline
	void setWidth(size_t w){
		this->width = w;
	}

	inline
	void setHeight(size_t h){
		this->height = h;
	}

	inline
	void setGeometry(const AreaGeometry & area){
		this->width = area.getWidth();
		this->height = area.getHeight();
	}

	inline
	void setScale(double s){
		this->scale = s;
	}

	/// Sets scale to fraction d/n.
	/**
	 *   \param d - denominator
	 *   \param n - nominator
	 */
	inline
	void setScale(int d, int n){
		this->scale = static_cast<double>(d) / static_cast<double>(n);
	}


	size_t width;
	size_t height;
	std::string interpolation;
	double scale;
	//
	virtual
	void makeCompatible(const ImageFrame & src, Image & dst) const;

	inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		drain::Logger mout(this->name+"[const ChannelTray &, ChannelTray &]", __FUNCTION__);
		traverseChannelsEqually(src, dst);
	}

	/// The main functionality called by traverseFrame() after image compatibility check and tmp allocations
	/**
	 *  Other classes - like other operators - may call these also directly, but then compatibility must be ensured.
	 */
	virtual
	void traverseChannel(const Channel & src, Channel & dst) const;

	/// The main functionality called by traverseFrame() after image compatibility check and tmp allocations
	/**
	 *  Other classes - like other operators - may call these also directly, but then compatibility must be ensured.
	 */
	virtual  // TODO: deprecated?
	inline
	void traverseChannel(const Channel & src, const Channel & srcAlpha, Channel & dst, Channel & dstAlpha) const {
		drain::Logger mout(this->name+"[2+2]", __FUNCTION__);
		mout.note() << "delegating to: 2 x traverseChannel(2)" << mout.endl;
		traverseChannel(src, dst);
		traverseChannel(srcAlpha, dstAlpha);
	}

protected:

	inline
	double getRounds(double value, int & cFloor, int & cCeil) const {
		double f = floor(value);
		cFloor = static_cast<int>(f);
		cCeil  = static_cast<int>(ceil(value));
		return (value-f);
	}


};


} // image::

} // drain::

#endif /*ResizeOP_H_*/
