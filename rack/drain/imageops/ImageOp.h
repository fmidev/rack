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
#ifndef IMAGENEWOP_H_
#define IMAGENEWOP_H_

//#include <drain/util/LinearScaling.h>
#include "util/BeanLike.h"
// #include "util/LinearScaling.h"
//#include "util/Data.h"
//#include "util/Options.h"
//#include "util/Tree.h"
//#include "util/ReferenceMap.h"
#include "ImageMod.h"
//#include "ImageView.h"
//#include "Image.h"
//#include "ImageTray.h"



namespace drain {

namespace image {

/// Base class for image processing functions.
/**
 *
 *  TODO:
 */
class ImageOp : public ImageMod {

public:


    inline
    virtual ~ImageOp(){};

	/// Main interface. Typically splits processing to each channel.
	/**
	 *   The default implementation calls makeCompatible() and
	 *
	 *  \see processWithTemp()
	 */
	virtual
	void process(const ImageFrame & src, Image & dst) const;

	virtual
	void process(const ImageFrame & src, const ImageFrame & srcWeight, Image & dst, Image & dstWeight) const;

	/*
	virtual inline
	void process(const ImageTray<const Channel> & src, ImageTray<Image> & dst) const {
		processConditional(src, dst, true);
	}
	*/


	/// Run the operator on a series of images. Geometry and type may be changed. UNDER CONSTR
	/**
	 *   Checks that images in dst are not overlapping with those of src. If yes, create tmp images.
	 */
	virtual
	void process(const ImageTray<const Channel> & src, ImageTray<Image> & dst, bool checkOverlap = true) const;  // could be Tray<ModifiableChannel> ?

	// Note: only a default implementation.
	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		drain::Logger mout(__FUNCTION__, __FILE__); //REPL this->name+"(ImageOp::)[const ChannelTray &, ChannelTray &]", __FUNCTION__);
		mout.error() << "not implemented; consider processChannels[Equally|Repeated|Separately]" << mout.endl;
		// traverseChannelsEqually(src, dst);
		// traverseChannelsRepeated(src, dst);
		// traverseChannelsSeparately(src, dst);
		// traverseMultiChannel(src, dst);
	}


	/// Apply to single channel.
    virtual inline
    void traverseChannel(const Channel & src, Channel &dst) const {
    	throw std::runtime_error(name+"(ImageOp)::"+__FUNCTION__+"(src,dst) unimplemented.");
    };

    /// Apply to single channel with alpha.
    virtual inline
    void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {
    	throw std::runtime_error(name+"(ImageOp)::"+__FUNCTION__+"(src,srcAlpha,dst,dstAlpha) unimplemented.");
    };



	/// Depending on the operator, modifies the geometry and type of dst.
	/*  This default implementation
	 *  -# returns immediately, if dst==src.
	 *  -# sets dst type, if unset, to that of src
	 *  -# sets dst geometry to that of src
	 */
	virtual
	void makeCompatible(const ImageFrame & src, Image & dst) const;

	/// Modifies the geometry and type of dst to fit the computation result.
	/*
	 * This default implementation
	 *  - returns immediately, if dst has overlap with src1 or src2.
	 *  - else, calls makeCompatible(src1, dst); src2 has no effect.
	 */
	virtual
	void makeCompatible(const ImageFrame & src1, const ImageFrame & src2, Image & dst) const;


	/// Prints name, description and parameters using BeanLike::toOStr(). Virtual, so derived classes may extend output.
	virtual inline
	void help(std::ostream & ostr = std::cout) const {
		toOStream(ostr);
	}

protected:

    /**
     *  \param  weight_supported - not supported (0), flexibly supported (1) or required (2)
     *  \param  tmp_required - temporary image needed, if src and dst are same or overlapping.
     */
    inline  // , bool tmp_required=false    // , TMP_REQUIRED(tmp_required)
    ImageOp(const std::string &name = __FUNCTION__, const std::string &description="")  : ImageMod(name, description)
    {};

	virtual
	bool processOverlappingWithTemp(const ImageFrame & src, Image & dst) const;

	virtual
	bool processOverlappingWithTemp(const ImageTray<const Channel> & src, ImageTray<Image> & dst) const;


	// Final ?
	bool traverseOverlappingWithTemp(const Channel & src, Channel & dst) const;

	// Final ?
	bool traverseOverlappingWithTemp(const Channel & src, const Channel & srcWeight, Channel & dst, Channel & dstWeight) const;


    /// Calls processWithTemp() if the frames overlap
	//  Final
    /// Creates a copy of dstImage, runs the operator and copies the result to dstImage.
	//bool processOverlappingWithTemp(const ImageFrame & srcFrame, Image & dstImage) const; // needed?

	/// Calls processWithTemp() if the frames overlap
	//  Final
	//bool processOverlappingWithTemp(const ImageFrame & src, const ImageFrame & srcWeight, Image & dst, Image & dstWeight) const; // needed?

	/// Apply to a frame. Target dst must be compatible..
	//virtual	void traverse rame(const ImageFrame & src, ImageFrame & dst) const;
    // USE

	/// Process image channel and alpha channels equally. Eg. resizing operations may use this.
	/**
	 *   Neutral
	 */
	void traverseChannelsEqually(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	/// Recycle channels until all dst channels completed.
	void traverseChannelsRepeated(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	/// Process each (src,dst) channel pair independently. Raise error if their counts differ.
	/**
	 */
	void traverseChannelsSeparately(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;


	/// Redirect to processing as trays. This is the opposite of processChannels...() functions.
	inline
	void traverseAsChannelTrays(const ImageFrame &src, ImageFrame &dst) const {
		Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.debug() << "restoring to trays" << mout.endl;

		ImageTray<const Channel> s; //
		s.setChannels(src);

		ImageTray<Channel> d;
		d.setChannels(dst);

		traverseChannels(s, d);
	}

	/// Redirect to processing as trays. This is the opposite of processChannels...() functions.
	inline
	void traverseAsChannelTrays(const ImageFrame &src, const ImageFrame & srcWeight, ImageFrame &dst, ImageFrame &dstWeight) const {

		Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.debug() << "restoring to trays" << mout.endl;

		ImageTray<const Channel> s;
		s.setChannels(src, srcWeight);

		ImageTray<Channel> d;
		d.setChannels(dst, dstWeight);

		traverseChannels(s, d);
	}



    static inline
    void adaptCoordinateHandler(const Channel & src, CoordinateHandler2D & handler){
    	handler.setLimits(src.getWidth(), src.getHeight());
    	handler.setPolicy(src.getCoordinatePolicy());
    }




protected:


	/// Set applicable internal parameters before calling traverse().
	virtual inline
	void initializeParameters(const ImageFrame &src, const ImageFrame &dst) const {
		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__); //  name+"(ImageOp) src,dst"
		mout.debug() << "nothing defined (ok)" << mout.endl;
	}

	/// Set applicable internal parameters before calling traverse().
	inline virtual
	void initializeParameters(const ImageFrame &src, const ImageFrame &src2, const ImageFrame &dst) const {
		//drain::Logger mout(__FUNCTION__, __FILE__); //REPL getImgLog(), name+"(ImageOp) src,src2,dst", __FUNCTION__);
		drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		mout.debug() << "nothing defined (ok)" << mout.endl;
	}


};



inline
std::ostream & operator<<(std::ostream & ostr, const ImageOp &op){
	//op.help(ostr); // todo: name & params only
	ostr << op.getName() << '(' << op.getParameters() << ')';
	return ostr;
}

}  // namespace image

}  // namespace drain


#endif /* IMAGE_OP_H_ */

// Drain
