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
#ifndef SEQUENTIALIMAGEOP_H_
#define SEQUENTIALIMAGEOP_H_


#include "ImageOp.h"

namespace drain
{

namespace image
{

/**! Base class for designing image processing based simple pixel iterations. 
 * 
 */
//   Consider renaming / re-designing. Semantics: pixel-by-pixel, including for i, for j
class SequentialImageOp : public ImageOp
{
public:

	SequentialImageOp(const std::string &name = "SequentialImageOp",
			const std::string &description="Image processing operator.") : // ,  int weight_supported=0, bool relative_scale=true) :
				ImageOp(name, description) {}; // , RELATIVE_SCALE(relative_scale) {};

	virtual ~SequentialImageOp(){};
	

	inline
	void traverse(const Image &src,Image &dst) const {
		traverse(src,dst,dst);
	}

	void traverse(const Image &src, const Image &src2, Image &dst) const;

protected:

	/// If the images have same geometry, they are processed sequentially in memory (quick).
	void traverseSequentially(const Image &src, const Image &src2, Image &dst) const;

	/// If the images have different geometry, they are processed spatially, in each pixel (i,j).
	void traverseSpatially(const Image &src, const Image &src2, Image &dst) const;

	
	virtual
	double filterValueD(double src, double src2) const = 0;

	inline
	virtual
	int filterValueI(int src, int src2) const {
		return static_cast<int>(filterValueD(src,src2));
	}

	static const int MAX_16B;

	// bool RELATIVE_SCALE;
};

//MAX_16B(0xffff)

}

}

#endif /*SEQUENTIALIMAGEOP_H_*/

// Drain
