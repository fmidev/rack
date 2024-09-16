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


#ifndef DRAIN_UNITUPLE_WRAPPER
#define DRAIN_UNITUPLE_WRAPPER

#include <cstddef>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include <string>
//#include <set>

#include <drain/StringBuilder.h>
#include <drain/Type.h>  // Utils
//   lower ==  cannot use Sprinter, as long as SprinterLayouit uses UniTuple...
// #include <drain/VariableT.h> lower
//#include <drain/Sprinter.h>
#include "TupleBase.h"

namespace drain {


/***
 *  \tparam C - base class
 *  \tparam S - storage class of members (int, double, char)
 *  \tparam N - number of the members included
 */
template <class C, typename T=typename C::value_t, size_t N=sizeof(C)/sizeof(T)>
class PseudoTuple : public C, public TupleBase<T,N> {

public:

	typedef T* iterator;
	typedef T const* const_iterator;

	static const size_t baseTypeSize;

	const size_t elementCount;


	inline
	PseudoTuple() : elementCount(N>0 ? N : baseTypeSize/TupleBase<T,N>::storageTypeSize) {
		// std::cerr << N << '*' << TupleBase<T,N>::storageTypeSize << " ... " << baseTypeSize << '\n';
		if (N*TupleBase<T,N>::storageTypeSize > baseTypeSize){
			throw std::runtime_error(StringBuilder<>(__FUNCTION__, ": conflicting geometry ", N, 'x',
					TupleBase<T,N>::storageTypeSize," in addressing base class ", TypeName<C>::str(), " size=", baseTypeSize));
		}
	};

	virtual inline
	const_iterator begin() const {
		return static_cast<const_iterator>((void *)(const C *)this); //
	}

	virtual inline
	const_iterator end() const {
		return static_cast<const_iterator>((void *)(const C *)this) + N; //
	}

	virtual inline
	iterator begin(){
		return static_cast<iterator>((void *)(const C *)this);
	}

	virtual inline
	iterator end(){
		return static_cast<iterator>((void *)(const C *)this) + N;
	}


protected:

};

template <class C, typename T, size_t N>
const size_t PseudoTuple<C,T,N>::baseTypeSize = sizeof(C);



template <class C, typename T, size_t N>
struct TypeName<PseudoTuple<C,T,N> > {

    static const std::string & str(){
    	if (N == 0){
        	static const std::string name = drain::StringBuilder<>("PseudoTuple<", drain::TypeName<C>::str(),',' , drain::TypeName<T>::str(),">");
            return name;
    	}
    	else {
        	static const std::string name = drain::StringBuilder<>("PseudoTuple<", drain::TypeName<C>::str(),',' , drain::TypeName<T>::str(),',' , N, ">");
            return name;
    	}
    }

};

/*
template <class T, size_t N>
std::ostream & operator<<(std::ostream & ostr, const UniTuple<T,N> & tuple){
	return tuple.toStream(ostr);
}
*/




/*
 // NOTE: cannot use Sprinter, as long as SprinterLayout uses UniTuple...
template <class T, size_t N>
std::ostream & Sprinter::toStream(std::ostream & ostr, const  UniTuple<T,N> & tuple, const SprinterLayout & layout) {
	return Sprinter::sequenceToStream(ostr, tuple, layout);
}
*/

}  // drain


#endif
