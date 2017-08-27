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
#ifndef INTENSITY_H_
#define INTENSITY_H_


#include <limits>
//#include <math.h>
//#include <cmath>
#include <algorithm>

namespace drain
{

namespace image
{


//template <class T>
class Intensity
{
public:

	/// For setting mode of conversions between unsigned and signed numbers, and between integers and floats.
	/**
	 *   Consider scaling a signed char image to unsigned short image .
	 *   The former one takes values in range [-127,128], the latter in range [0,65535].
	 *
	 *   - NONE: negative values will be mapped to undetectValue, positive ones as such to [0,128].
	 *   - HALF: negative values will be mapped to undetectValue, [0,128] linearly to [0,65535].
	 *   - RANGE: -127 will be mapped to undetectValue, 128 to 65535, and linearly between these.
	 *
	 *   Assume that the latter will
	 *
	 *
	 *
	 */
	  //  TODO: CAST (low-level binary cast)?
	typedef enum {NONE,DEFAULT,RANGE} scalingMode;

	/// TODO: is this [0...1] limitation for floats valid?

	/// For floating-point values, returns 0.0, otherwise the lower limit of the range.
	template <class T>
	inline
	static T min(){ return std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::min() : 0; }

	/// For floating-point values, returns 1.0, otherwise the upper limit of the range.
	template <class T>
	inline
	static T max(){ return std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::max() : 1; }

	/// Value to be used in inverting numbers; returns 0.0 for floating-point values.
	template <class T>
	inline
	static T maxOrigin(){ return std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::max() : 0; }


	/// Ensures that the value stays within the numeric range of T2.
	template <class T>
	static T limit(double x){
		static const double xMin = static_cast<double>(
				std::numeric_limits<T>::is_integer ?  std::numeric_limits<T>::min() : -std::numeric_limits<T>::max() );
		static const double xMax = static_cast<double>( std::numeric_limits<T>::max() );
		x = std::max(xMin,x);
		x = std::min(x,xMax);
		return static_cast<T>(x);
	}

};



}

}

#endif /*INTENSITY_H_*/

// Drain
