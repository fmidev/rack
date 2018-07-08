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
	//std::string functorName;
	//std::string functorParams;


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
