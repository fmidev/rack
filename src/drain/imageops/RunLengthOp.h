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
#ifndef RUNLENGTHOP_H_
#define RUNLENGTHOP_H_

#include <stdexcept>


#include "ImageOp.h"


namespace drain
{

namespace image
{

/// Base class for RunLengthHorzOp and RunLengthVertOp
/**

 */
// c onvert  -frame 1 +append image.png runLengthHorz.png -resize 50% runLengthHorz2.png  #exec
class RunLengthOp : public ImageOp
{
public:
    
	RunLengthOp(const std::string &name, double threshold=0) :
		ImageOp(name, "Computes lengths of segments of intensity above threshold.") {
		parameters.link("threshold", this->threshold = threshold);
	};

	double threshold;

	inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL this->name+"[const ChannelTray &, ChannelTray &]", __FUNCTION__);
		//mout.error() << "not implemented; consider forwarding to processChannelsSeparately()" << mout.endl;
		traverseChannelsSeparately(src, dst);
	}


};


/// Computes lengths of horizontal segments.
/**
\code
drainage shapes1.png  --iRunLengthHorz 128  -o rleHorzAbs.png
drainage shapes1.png  --physicalRange 0:1 --iRunLengthHorz 0.75  -o rleHorz.png
\endcode

To detect line segments longer than 255 the user should use 16-bit result image, \c --type \c S .
Further, \c --iRescale or \c --iGamma command may be useful in making the results visible.
 */
class RunLengthHorzOp : public RunLengthOp {

public:

	/// Default constructor.
	/**
	 *   \param threshold - minimum absolute intensity in segments
	 */
	RunLengthHorzOp(double threshold = 0.0) : RunLengthOp(__FUNCTION__,  threshold) {};

protected:

	void traverseChannel(const Channel &src, Channel &dst) const ;


};

/// Computes lengths of vertical segments.
/**
\code
drainage shapes1.png  --iRunLengthVert 128  -o runLengthVertAbs.png
drainage shapes1.png  --physicalRange 0:1 --iRunLengthVert 0.5  -o runLengthVert.png
\endcode
 */
class RunLengthVertOp : public RunLengthOp {

public:

	/// Default constructor.
	/**
	 *   \param threshold - minimum absolute intensity in segments
	 */
	RunLengthVertOp(double threshold = 0.0) : RunLengthOp(__FUNCTION__, threshold) {};

protected:

	//void process(const ImageFrame &src, ImageFrame &dst) const ;
	void traverseChannel(const Channel &src, Channel &dst) const ;

};

}
}

#endif /* RUNLENGTHOP_H_*/

// Drain
