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

#include "SequentialImageOp.h"


namespace drain
{
namespace image
{

/// TODO: CopyOP should be quite similar.
/**! 
 *   
 * 
 */
class QuantizatorOp: public SequentialImageOp
// rename bit quantizator?
{
public:

	QuantizatorOp(int bits) : SequentialImageOp("QuantizatorOp",
				"Quantize to n bits. Makes sense for integer data only."){
			parameters.reference("bits", this->bits = bits);
	};


	inline
	void initializeParameters(const Image &src, const Image &src2, const Image &dst) const {
		
		//const unsigned int bits = this->parameters.get("bits",8*sizeof(T));
		
		mask = 0;
		
		// Create 111111...
      	for (unsigned int i = 0; i < bits; i++)
			mask = (mask << 1) | 1;
      
      	// Complete 1111110000
      	mask = mask << (sizeof(T)*8 - bits);
      
        bitShift = (dst.getByteSize()-src.getByteSize())*8;
	};
   
    inline
	int filterValueInt(const int &src, const int &src2) const {
		return (src&mask) << bitShift;
	};

	unsigned int bits;
	
protected:
	mutable long int mask;
	mutable int bitShift;
}; 





}
}

#endif /*QUANTIZE_H_*/

// Drain
