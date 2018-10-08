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
#ifndef SEGMENT_OP_H
#define SEGMENT_OP_H

#include <math.h>

#include "util/FunctorBank.h"
#include "image/SegmentProber.h"

#include "ImageOp.h"
//#include "FloodFillOp.h"

namespace drain
{
namespace image
{

/// A base class for computing statistics of segments. As segment is an area of connected pixels.
/**
 *
 */
class SegmentOp: public ImageOp
{
public:


	double min;  // TODO: instead consider criterion method inside(i,j)
	double max;

	bool clearDst;

	std::string functorStr;  // consider internal split util


protected:

	inline
	SegmentOp(const std::string & name, const std::string & description) :
		ImageOp(name, description), min(0), max(0), clearDst(true), functor(identityFtor) {
	}

	SegmentOp(const std::string & name, const std::string & description, const drain::UnaryFunctor & ftor) :
		ImageOp(name, description + " ftor=" + ftor.getName()), min(0), max(0), clearDst(true), functor(ftor) {
	}



	/// Returns a functor: the member functor or retrieved from FunctorBank
	inline
	const UnaryFunctor & getFunctor(double scale = 0.0) const {

		drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

		if (functorStr.empty()){
			return this->functor;
		}
		else {
			UnaryFunctor & f = drain::getFunctor(functorStr, ':');
			if (scale > 0.0)
				f.setScale(scale, 0);
			mout.debug(2) << " ftor: " << f << mout.endl;
			//mout.warn() << "cftor: " << (const UnaryFunctor &)f << mout.endl;
			return f;
		}
	}

protected:

	const drain::IdentityFunctor identityFtor;
	const drain::UnaryFunctor & functor;




};

} // namespace image

} // namespace drain

#endif

// Drain
