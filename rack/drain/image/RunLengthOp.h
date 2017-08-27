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
// convert  -frame 1 +append image.png runLengthHorz.png -resize 50% runLengthHorz2.png  #exec
class RunLengthOp : public ImageOp
{
public:
    
	RunLengthOp(const std::string &name, double threshold=0) :
		ImageOp(name,"Computes lengths of segments of intensity above threshold.") {
		parameters.reference("threshold", this->threshold = threshold);
	};

	double threshold;

	/*
	virtual
		inline
		void makeCompatible(const Image & src, Image & dst) const  {

			drain::MonitorSource mout(iMonitor, name, __FUNCTION__);

			mout.debug(2) << "src: " << src << mout.endl;

			if (!dst.typeIsSet()){
				dst.setType<unsigned char>();
			}

			if (!isIntegralType(dst.getType())){
				mout.warn() << "Adopting src scaling for dst, although different storage types" << mout.endl;
			}


			if (dst.isSame(src)){
				mout.debug() << "dst == src, ok" << mout.endl;
				return;
			}

			dst.useAbsoluteScale();

			dst.setGeometry(src.getGeometry());
			/// TODO: copy alpha, fill alpha?
			if (dst.getAlphaChannelCount()){
				mout.info() << "resetting alpha channel" << mout.endl;
				dst.getAlphaChannel().fill(dst.getMax<int>());
			}

			mout.debug(3) << "dst: " << dst << mout.endl;

		};
	*/

	void initializeParameters(const Image & src, const Image & dst) const {};

};


/// Computes lengths of horizontal segments.
/**
\code
drainage shapes1.png  --runLengthHorz 128  -o rleHorz.png
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

	void traverse(const Image &src, Image &dst) const ;


};

/// Computes lengths of vertical segments.
/**
\code
drainage shapes1.png  --runLengthVert 128  -o runLengthVert.png
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

	void traverse(const Image &src, Image &dst) const ;

};

}
}

#endif /* RUNLENGTHOP_H_*/

// Drain
