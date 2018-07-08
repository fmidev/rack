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
		parameters.reference("threshold", this->threshold = threshold);
	};

	double threshold;

	inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		drain::Logger mout(this->name+"[const ChannelTray &, ChannelTray &]", __FUNCTION__);
		//mout.error() << "not implemented; consider forwarding to processChannelsSeparately()" << mout.endl;
		traverseChannelsSeparately(src, dst);
	}


};


/// Computes lengths of horizontal segments.
/**
\code
drainage shapes1.png  --runLengthHorz 128  -o rleHorzAbs.png
drainage shapes1.png  --physicalRange 0,1 --runLengthHorz 0.75  -o rleHorz.png
\endcode

To detect line segments longer than 255 the user should use 16-bit result image, \c --type \c S .
Further, \c --scale or \c --gamma command may be useful in making the results visible.
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
drainage shapes1.png  --runLengthVert 128  -o runLengthVertAbs.png
drainage shapes1.png  --physicalRange 0,1 --runLengthVert 0.5  -o runLengthVert.png
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
