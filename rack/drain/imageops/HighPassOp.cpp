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
#include "drain/util/FunctorPack.h"
#include "drain/image/File.h"

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
	Logger mout(getImgLog(), __FUNCTION__, getName());

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

	Logger mout(getImgLog(), __FUNCTION__, getName());

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


// Drain
