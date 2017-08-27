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
#ifndef RECURSIVEREPAIREROP_H_
#define RECURSIVEREPAIREROP_H_

#include "ImageOp.h"

namespace drain
{

namespace image
{

/** Image restoration utility applying recursive interpolation from neighboring pixels. 

 \code
  drainage image-rgba.png --recursiveRepairer 15,15,3 --view i -o rec.png --view a -o rec-alpha.png
 \endcode
 */
class RecursiveRepairerOp : public ImageOp
{
public:
	
	RecursiveRepairerOp(int width=5, int height=5, int loops=3, float decay=1.0f): ImageOp("RecursiveRepairerOp",
			"Applies weighted averaging window repeatedly, preserving the pixels of higher weight.") {
		parameters.reference("width", this->width = width);
		parameters.reference("height", this->height = height);
		parameters.reference("loops", this->loops = loops);
		//parameters.reference("decay", this->decay = decay);

	};

	int width;
	int height;
	int loops;
	// float decay;

protected:

	/// The main functionality called by filter() after image compatibility check and tmp allocations
	/**
	 */
	//virtual
	void traverse(const Image &src, const Image &srcAlpha, Image &dst, Image &dstAlpha) const;

};




}

}

#endif /*RECURSIVEREPAIRER_H_*/

// Drain
