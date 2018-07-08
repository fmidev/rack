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
 drainage image.png --transpose -o transpose.png
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
		// reference("threshold", this->threshold = threshold);
	};


	inline
	virtual void makeCompatible(const ImageFrame &src,Image &dst) const  {

		drain::Logger mout(getImgLog(), name + "(ImageOp::_makeCompatible)");

		if (!dst.typeIsSet())
			dst.setType(src.getType());

		const size_t & width = src.getWidth();
		const size_t & height = src.getHeight();
		const size_t & iChannels = src.getImageChannelCount();
		const size_t & aChannels = src.getAlphaChannelCount();

		if ((dst.getHeight()==width) && (dst.getWidth()==height) &&
				(dst.getImageChannelCount()==iChannels) && (dst.getAlphaChannelCount()==aChannels) ){
			mout.debug() << "dst with ok geometry" << mout.endl;
			return;
		}

		/// Turn
		dst.setGeometry(height, width, iChannels, aChannels);

		mout.debug() << "dst:" << dst << mout.endl;

	};

	inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		drain::Logger mout(this->name+"[const ChannelTray &, ChannelTray &]", __FUNCTION__);
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
