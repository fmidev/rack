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
#ifndef TRANSPOSEOP2_H_
#define TRANSPOSEOP2_H_

#include <cstdarg>

#include "ImageOp.h"


namespace drain
{
namespace image
{

/// Transpose operator. Swaps the intensities over the diagonal exis.
/**! 

 \code
 drainage image.png --iTranspose -o transpose.png
 \endcode


 */
class TransposeOp: public ImageOp
{
public:

	/// Default constructor.
	/**
	 *  \par minDBZ - minimum intensity passed through.
	 *  \par replace   - substitute value for intensities not reaching minDBZ
	 */
	TransposeOp(): ImageOp(__FUNCTION__,"Flips image matrix around its corner."){
		// link("threshold", this->threshold = threshold);
	};

	virtual inline
	void getDstConf(const ImageConf & src, ImageConf & dst) const {
	//virtual void make Compatible(const ImageFrame &src,Image &dst) const  {

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		// unneeded if (!dst.typeIsSet())	dst.setType(src.getType());

		const size_t & width = src.getWidth();
		const size_t & height = src.getHeight();
		const size_t & iChannels = src.getImageChannelCount();
		const size_t & aChannels = src.getAlphaChannelCount();

		/*
		if ((dst.getHeight()==width) && (dst.getWidth()==height) &&
				(dst.getImageChannelCount()==iChannels) && (dst.getAlphaChannelCount()==aChannels) ){
			mout.debug() << "dst with ok geometry" << mout.endl;
			return;
		}
		*/

		/// Turn
		dst.setGeometry(height, width, iChannels, aChannels);

		const CoordinatePolicy & pol = src.getCoordinatePolicy();
		dst.setCoordinatePolicy(
				pol.yUnderFlowPolicy,
				pol.xUnderFlowPolicy,
				pol.yOverFlowPolicy,
				pol.xUnderFlowPolicy
		);

		mout.debug() << "dst:" << dst << mout.endl;

	};

	inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);  //REP (this->name+"[const ChannelTray &, ChannelTray &]", __FUNCTION__);
		traverseChannelsSeparately(src, dst);
	}


	inline
	void traverseChannel(const Channel & src, Channel & dst) const {

		for (size_t j = 0; j < src.getHeight(); ++j) {
			for (size_t i = 0; i < src.getWidth(); ++i) {
				dst.put(j, i, src.get<double>(i,j));
			}
		}

	}

	//double threshold;


protected:

}; 





}
}

#endif /*THRESHOLD_H_*/

// Drain
