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
