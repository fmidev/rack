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

#ifndef TRANSPARENT2_OP_H_
#define TRANSPARENT2_OP_H_

#include "drain/util/Fuzzy.h"
#include "FunctorOp.h"

namespace drain
{

namespace image
{
// he


class TransparentOp : public UnaryFunctorOp<drain::FuzzyStep<double> > {

	drain::Range<double> range;

	/// Default constructor.
	/**
	 *  \par minDBZ - minimum intensity passed through.
	 *  \par replace   - substitute value for intensities not reaching minDBZ
	 */
	TransparentOp(): UnaryFunctorOp<drain::FuzzyStep<double> >(__FUNCTION__, "Adds transparency."){

		range.set(0.5);
	};


	//virtual void make Compatible(const ImageFrame &src,Image &dst) const ;
	virtual
	void getDstConf(const ImageConf & src, ImageConf & dst) const;

	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;
	/// Uses that of UnaryFunctorOp<drain::FuzzyStep<double> >!
	/**
	 *   IDEA: only dst.alpha will be forwarded here.
	 */
	//void traverseChannel(const Channel &src, Channel & dst) const

	/// Policy: exclude alpha channels, ie. apply to image channels only.
	void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const;


};

} // image::

} // drain::

#endif
