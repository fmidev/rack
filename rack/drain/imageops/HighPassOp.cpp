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
#include "util/FunctorPack.h"
#include "image/File.h"

#include "FunctorOp.h"
#include "FastAverageOp.h"

#include "HighPassOp.h"

namespace drain
{

namespace image
{


HighPassOp::HighPassOp(int width, int height, double scale, double offset) :  //  bool LIMIT=true ('false' would make no sense)
			ImageOp(__FUNCTION__,"High-pass filter for recognizing details."){
	parameters.reference("width", this->width = width);
	parameters.reference("height",this->height = height>0 ? height : width);
	parameters.reference("scale", this->scale = scale);
	this->parameters.reference("offset", this->offset = offset);
	this->parameters.reference("LIMIT", this->LIMIT = true);
}




/// Apply to single channel with alpha.
void HighPassOp::traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {
	Logger mout(getImgLog(), getName(), __FUNCTION__);

	FastAverageOp fop(width, height);

	fop.traverseChannel(src, srcAlpha, dst, dstAlpha);
	/*
	File::write(src,      getName()+"-S1.png");
	File::write(srcAlpha, getName()+"-SA.png");
	File::write(dst,      getName()+"-D1.png");
	File::write(dstAlpha, getName()+"-DA.png");
	*/

	mout.debug(3) << "subtraction" << mout.endl;
	BinaryFunctorOp<SubtractionFunctor> sub;
	sub.functor.setScale(scale, offset);
	sub.LIMIT = LIMIT;
	sub.initializeParameters(src, dst);
	sub.traverseChannel(src, dst, dst); // dstAlpha left intact

}


void HighPassOp::traverseChannel(const Channel & src, Channel & dst) const {

	Logger mout(getImgLog(), getName(), __FUNCTION__);

	// File::write(src,"HighPass-01-src.png");
	// mout.warn() << "src: " << src << mout.endl;
	FastAverageOp fop(width, height);
	//fop.initializeParameters(src, dst);
	fop.traverseChannel(src, dst);
	//mout.warn() << "src2: " << src2 << mout.endl;

	// File::write(dst, "HighPass-02-avg.png");

	mout.debug(3) << "subtraction" << mout.endl;
	BinaryFunctorOp<SubtractionFunctor> sub;
	sub.functor.setScale(scale, offset);
	sub.LIMIT = LIMIT;
	sub.initializeParameters(src, dst);
	sub.traverseChannel(src, dst, dst);
	// File::write(dst, "HighPass-03-diF.png");
}


}

}

