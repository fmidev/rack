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
#ifndef QUANTIZATOROP_H_
#define QUANTIZATOROP_H_

#include "../util/Functor.h"


namespace drain
{
namespace image
{


//  Consider moving to FunctorOps
/// TODO: CopyOP should be quite similar.
/**!
 *
 *
 */
class QuantizatorFunctor: public UnaryFunctor
{
public:

	QuantizatorFunctor(int bits = 4) : UnaryFunctor(__FUNCTION__,
				"Quantize to n bits. (For integer images)"), mask(0), bitShift(0) {
			parameters.reference("bits", this->bits = bits);
	};


	virtual
	inline
	int operator()(int s) const {
		//return s;
		return (s & mask);
        //	return (s & mask) << bitShift;
	};


	virtual
	double operator()(double s) const {
		//return this->scaleFinal * s;
		return  (1/255.0)*static_cast<double>(operator ()(static_cast<int>(255.0*s)));
	}


	virtual
	inline
	void update() const {

		drain::Logger mout(getImgLog(), this->name, __FUNCTION__);

		mask = 0;
		// Create 111111...
		for (unsigned int i = 0; i < bits; i++)
			mask = (mask << 1) | 1;

		//mout.warn() << mask << mout.endl;
		mask = mask << (1*8 - bits); // assum uchar?
		//mout.warn() << mask << mout.endl;
		updateScale();
		/*
		mout.note();
		for (int i=0; i < 255; ++i) {
			mout << i << '\t' << operator ()(i) << '\t' << operator ()(static_cast<double>(i)) << '\n';
		}
		mout << mout.endl;
		*/
	}

	/*
	inline
	void initializeParameters(const ImageFrame &src, const ImageFrame & src2, const ImageFrame & dst) const {


		mask = 0;

		// Create 111111...
      	for (unsigned int i = 0; i < bits; i++)
			mask = (mask << 1) | 1;

      	// Complete 1111110000
      	mask = mask << (src.getByteSize()*8 - bits);

        bitShift = (dst.getByteSize()-src.getByteSize())*8;
	};
	*/


	unsigned int bits;

protected:

	mutable long int mask;

	mutable int bitShift;

};


}
}

#endif /*QUANTIZE_H_*/

// Drain
