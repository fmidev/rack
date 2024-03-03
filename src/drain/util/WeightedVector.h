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
#ifndef WVECTOR_H_
#define WVECTOR_H_


/*
 g++ drain/examples/WeightedVector-example.cpp drain/util/{Caster,Variable,Debug}.cpp drain/image/{Geometry,Coordinates,Image,FilePng}.cpp  -lpng   -o WeightedVector-example
 */
#include <stdlib.h>


#include <iostream>
#include <vector>
#include <list>
#include <limits>
#include <sstream>

//


// using namespace std;

namespace drain {

/**
 */
template <class T>
class WeightedVector : public std::vector<T> {

public:

	WeightedVector(typename std::vector<T>::size_type size = 0){
		resize(size);
	}

	std::vector<T> weight;


	// Overrides the original.
	void resize(typename std::vector<T>::size_type n, typename std::vector<T>::value_type t = typename std::vector<T>::value_type()){  // const & value ?
		std::vector<T>::resize(n, t);
		weight.resize(n, 1.0); // (n,t)
	}

	void toStream(std::ostream &ostr) const {

	  char separator;

	  /// data
	  separator = 0;
	  for (size_t i=0; i < this->size(); i++){
	    if (separator)
	      ostr << separator;
	    else
	      separator = ',';
	    ostr << (*this)[i];
	  };

	  // weights
	  ostr << ' ' << '{';
	  separator = 0;
	  for (size_t i=0; i < this->size(); i++){
	    if (separator)
	      ostr << separator;
	    else
	      separator = ',';
	    ostr << (*this).weight[i];
	  };
	  ostr << '}';

	};


	//template <class T>
	static
	double defaultDistance2(const WeightedVector<T> & a, const WeightedVector<T> & b) {

	  double d;
	  double w;
	  double sumD2 = 0.0;
	  double sumW = 0.0;

	  for (int i = 0; i < a.size(); ++i) {
		  d = a[i] - b[i];
		  w = a.weight[i] * b.weight[i];
		  sumD2 += w*d*d;
		  sumW  += w;
	  }
	  if (sumW > 0.0)
	    return sumD2/sumW;
	  else
	    return std::numeric_limits<double>::max();
	}

	/// Given vectors a and a and a mixing coefficient coeff , outputs mixture vector m.
	/**
	 *   Does not check vector lengths.
	 *
	 */
	//template <class T>
	static
	void euclideanMixingFunction(const WeightedVector<T> & a, const WeightedVector<T> & b, double coeff, WeightedVector<T> & m) {

	  double alpha;
	  double beta;
	  const double coeff2 = 1.0 - coeff;
	  double w;
	  for (int i = 0; i < a.size(); ++i){
	    alpha = coeff2 * a.weight[i];
	    beta  = coeff  * b.weight[i];
	    w = alpha + beta;
	    if ( w > 0.0 ){
	      m[i] = (alpha*a[i] + beta*b[i]) / w;
	      m.weight[i] = w;  // consider sqrt?
	    }
	    else { // else ? 0, max, min ?
	      m.weight[i] = 0.0;
	    }
	      
	  }

	}



};



template <class T>
std::ostream & operator<<(std::ostream & ostr, const WeightedVector<T> & WeightedVector){
	WeightedVector.toStream(ostr);
	return ostr;
};


} // drain



/* namespace drain */

#endif /* WeightedVector_H_ */

// Drain
