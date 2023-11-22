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
#ifndef IMAGE_MODIFIER_H_
#define IMAGE_MODIFIER_H_

//#include "drain/util/LinearScaling.h"
#include "drain/util/BeanLike.h"
// #include "drain/image/ drain::ValueScaling.h"
#include "drain/image/Image.h"
//#include "ImageChannel.h"
#include "drain/image/ImageTray.h"
//#include "Sampler.h"



namespace drain {

namespace image {

/// Class for operations that modify an existing image instead of producing a new image.
/**
 *
 */
class ImageMod : public BeanLike {

public:

	bool physicalScale;

	/// Tell if alpha channel(s) is required in input.
	/**
	 *  Future option:
	 *  0: not used
	 *  1: optional, supported
	 *  2: required
	 */
	virtual inline
	int srcAlpha() const {
		return 0;
	};

	/// Run this modifier for an image.
	/**
	 *  The default implementation redirects to traverseChannel(Channel & dst) .
	 */
	virtual
	void process(Image & dst) const;


	/// Run this modifier for a set of channels.
	/**
	 *  The default implementation forwards to applyToEachChannel(ImageTray<Channel> & dst) & dst)
	 *
	 *  By definition of Channel, geometries and types will be unaltered.
	 */
	virtual
	void traverseChannels(ImageTray<Channel> & dst) const;


	/// Run this modifier for an image frame.
	/**
	 *  The default implementation calls applyToEachChannel(Channel &) for each image.
	 */
	//virtual 	void traverse rame(ImageFrame & dst) const;



	/// Run this modifier for an image frame.
	/**
	 *  The default implementation calls traverseFrame(ChannelView & dst) for each image.
	 */
	virtual
	inline
	void traverseChannel(Channel & dst) const {
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL this->name+"(ImageMod)", __FUNCTION__);
		mout.warn("(Channel &): not implemented" );
	};

	/// Run this modifier for an image frame.
	/**
	 *  The default implementation calls traverseFrame(ChannelView & dst) for each image.
	 */
	virtual
	inline
	void traverseChannel(Channel & dst, Channel & dstAlpha) const {
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL this->name+"(ImageMod)", __FUNCTION__);
		mout.debug("(Channel &, Channel &alpha): not implemented" );
	};



	virtual
	inline
	void help(std::ostream & ostr = std::cout) const {
		toStream(ostr);
	}

	/// Converts dst to ImageTray<Channel> and calls traverseFrame(ImageTray<Channel> &) .
	virtual
	void traverseChannels(ImageFrame & dst) const;

protected:

	/**
	 * \param  weight_supported - not supported (0), flexibly supported (1) or required (2)
	 *  \param in_place - 0 = separate dst required, 1 = separate dst encouraged, 2 = dst is allowed to be src
	 */
	inline
	ImageMod(const std::string &name = __FUNCTION__, const std::string &description="")  :
		// Remove trailing "Op", if it exists. Start find from the end-2
		BeanLike(name.substr(0, name.find("Op", name.size()-2)), description) , physicalScale(true) {

	};

	inline
	ImageMod(const ImageMod & op) : BeanLike(op) , physicalScale(true) {
	};

	inline
	virtual ~ImageMod(){};



	/// Run this modifier by calling traverseChannel(Channel &) for each image.
	virtual
	void processChannelsSeparately(ImageTray<Channel> & dst) const;


	/// Modifies the geometry and the type of dst such that traverseChannel(Channel &) can be called.
	/*  This default implementation does nothing.
	 */
	virtual
	inline
	void initialize(Image & dst) const {};

	/// Modifies the geometry and the type of dst such that traverseChannel(Channel &) can be called.
	/*  This default implementation does nothing.
	 */
	virtual
	inline
	void initializeAlpha(const Image & srcAlpha, Image & dstAlpha) const {
		//dstAlpha.initialize(src.getType(), src.getWidth(), src.getHeight(), 1);
	};



};






}  // namespace image

}  // namespace drain


#endif /* IMAGE_OP_H_ */

// Drain
