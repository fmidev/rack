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
    drainage image.png --iResize 100,100  -o resizeShrink.png
  \endcode

  To expand an image using nearest-pixel interpolation:
  \code
    drainage image.png --iResize 1000,800    -o resizeExpand.png
    drainage image.png --iResize 1000,800,n  -o resizeExpand.png
  \endcode

  To expand an image using bilinear interpolation:
  \code
  	  drainage image.png --iResize 1000,800,b  -o resizeExpandBilinear.png
  \endcode

 */	
class ResizeOp : public ImageOp {

public:

	ResizeOp(size_t width=0, size_t height=0, const std::string & interpolation = "n") : ImageOp(__FUNCTION__, "Resize geometry and scale intensities") {
		this->parameters.link("width",  this->width=width, "pix");
		this->parameters.link("height", this->height=height, "pix");
		this->parameters.link("interpolation", this->interpolation=interpolation, "n=nearest,b=bilinear");
		this->parameters.link("scale", this->scale = 1.0, "rescaling factor");
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
	// void make Compatible(const ImageFrame & src, Image & dst) const;
	void getDstConf(const ImageConf &srcConf, ImageConf & dstConf) const;

	inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL this->name+"[const ChannelTray &, ChannelTray &]", __FUNCTION__);
		traverseChannelsEqually(src, dst);
	}

	/// The main functionality called by traverseFrame() after image compatibility check and tmp allocations
	/**
	 *  Other classes - like str operators - may call these also directly, but then compatibility must be ensured.
	 */
	virtual
	void traverseChannel(const Channel & src, Channel & dst) const;

	/// The main functionality called by traverseFrame() after image compatibility check and tmp allocations
	/**
	 *  Other classes - like str operators - may call these also directly, but then compatibility must be ensured.
	 */
	virtual  // TODO: deprecated?
	inline
	void traverseChannel(const Channel & src, const Channel & srcAlpha, Channel & dst, Channel & dstAlpha) const {
		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL this->name+"[2+2]", __FUNCTION__);
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

// Drain
