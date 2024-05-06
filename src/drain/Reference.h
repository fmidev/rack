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
#include <typeinfo>
#include <stdexcept>
#include <iostream>


#include "Castable.h"
#include "Convert.h"
#include "Sprinter.h"

#include "VariableT.h"
#include "ReferenceT.h"

#ifndef DRAIN_REFERENCE__NEW
#define DRAIN_REFERENCE__NEW


namespace drain {

/// Variable-like that is linked to a standard variable: double, int, std::string . Supports multi-element arrays through drain::UniTuple
/**

drain::Reference does not have memory of its own. The type of reference cannot be changed – it is that of the referenced variable.

The only character string type supported is \c std::string .

Currently, the only multi-element structure supported is drain::UniTuple<>, which has a fixed start address for its elements.
In future, STL Sequences like std::vectors may be supported.

Reference does not support STL containers.

This class is best illustrated together with its counterparts supporting own memory, Variable and FlexibleVariable

\section ctors-var Reference constructors

\htmlinclude  VariableT-ctors-Reference.html

\section assignments-var Reference assignments

\htmlinclude  VariableT-assign-Reference.html

See the documentation of drain::VariableT template specialized by drain::Variable, drain::Reference and drain::FlexibleVariable .

\see VariableT
\see Variable
\see FlexibleVariable

*/
typedef VariableT<ReferenceT<Castable> >  Reference;


//VariableT<R>
/*
template <>
template <>
void ReferenceT<Castable>::init(const VariableT<ReferenceT<Castable> > & src){
	std::cerr << __FILE__ << ' ' << __LINE__ << ':' << __FUNCTION__ << " " << src << std::endl;
}

template <>
template <>
void ReferenceT<Castable>::init(const ReferenceT<Castable> & src){
	std::cerr << __FILE__ << ' ' << __LINE__ << ':' << __FUNCTION__ << " " << src << std::endl;
}
*/

template <>
const std::string TypeName<Reference>::name;

template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const Reference & v, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &) v, layout);
};

/*
template <class T, size_t N=2>
template <>
UniTuple<T,N> & UniTuple<T,N>::set(const Reference & t){
	// assignSequence(t, true); // by default LENIENT, or how should it be?
	return *this;
}
*/

}  // namespace drain

#define   VariableLike Reference
#include "VariableLike.inc"
#undef    VariableLike


#endif
