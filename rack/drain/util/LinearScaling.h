/**

    Copyright 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain.

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


   Created on: Aug 24, 2011
   Author: mpeura
*/

#ifndef DataScaling_H_
#define DataScaling_H_

#include <ostream>



// using namespace std;

namespace drain {

/// Utility for mapping \f$x\f$ to \f$x'\f$, where \f$x\f$ uses some scaling and $y$ in another scaling.
/**
 Given samples $x$ with gain $a$ and offset $b$,
 \code
  y = a*x + b
  y = A*x_2 + B
 \endcode

 Consider a measurement \f$y\f$ that has been mapped to  $x$ such that \f$y\f$ can be retrieved with a
 simple linear transformation
 \f[
  y = ax + b
 \f]
 Consider another similar mapping, producing $x'$.
 \f[
  y = a'x' + b'
 \f]
If we want to create a direct mapping \f$x' = Ax + B\f$, then, evidently
\f[
  y = ax + b = a'x' + b'
  \Leftrightarrow
  x' = (ax + b -b')/a' = (a/a')x + (b-b')/a' = Ax' + B
\]
 */
class LinearScaling {
public:

	inline
	LinearScaling(double gain1=1.0, double offset1=0.0, double gain2=1.0, double offset2=0.0) :
		gain(gain1/gain2), gainInv(gain2/gain1), offset((offset1-offset2)/gain2) {};

	inline
	void set(double gainIn=1.0, double offsetIn=0.0, double gainOut=1.0, double offsetOut=0.0){
		gain = gainIn/gainOut;
		offset = (offsetIn-offsetOut)/gainOut;
		gainInv = gainOut/gainIn;
	}

	/**
	 * \param minIn - internal minimum
	 * \param maxIn - internal maximum
	 * \param minOut - external minimum
	 * \param maxOut - external maximum
	 *
	 * Above, "internal" may refer to constraints derived from storage type.
	 */
	inline
	void setRange(double minIn, double maxIn, double minOut, double maxOut){
		gain = (maxOut - minOut)/(maxIn - minIn);
		offset = minOut;
		gainInv = 1.0/gain;
	}

	/*
	inline
	void set(double gainIn=1.0, double offsetIn=0.0, double gainOut=1.0, double offsetOut=0.0){
		gain = gainIn/gainOut;
		offset = (offsetIn-offsetOut)/gainOut;
		gainInv = gainOut/gainIn;
	}
	*/

	template <class T>
	inline T forward(const T &x) const{
		return static_cast<T>(offset + gain*x);
	}

	template <class T>
	inline void forward(const double &x, T &y) const{
		y = static_cast<T>(offset + gain*x);
	}

	template <class T>
	inline double inverse(const T &y) const {
		return (static_cast<double>(y) - offset)*gainInv;
	}

	template <class T>
	inline void inverse(const T &y, double &x) const {
		x = (static_cast<double>(y) - offset)*gainInv;
	}

	inline
	double getGain() const { return gain; }

	inline
	double getOffset() const { return offset; }

	/*
	inline
	double getInMin() const { return };

	inline
	int getUpperBoundIn() const { return inMax; };

	inline
	int getOutMin() const { return outMin; };

	inline
	int getUpperBoundOut() const { return outMax; };
	*/


protected:
	double gain;
	double gainInv;
	double offset;
};

inline
std::ostream & operator<<(std::ostream & ostr, const drain::LinearScaling & d){
	ostr << "gain=" << d.getGain() << ", offset=" << d.getOffset();
	return ostr;
}

} //  drain::

#endif /* DataScaling_H_ */
