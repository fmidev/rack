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



#ifndef DRAIN_VARIABLE_LIKE
#define DRAIN_VARIABLE_LIKE

/*
#include "Convert.h"

// #include "VariableBase.h"
#include "ReferenceT.h"
#include "VariableT.h"

*/

#include "VariableAssign.h"

namespace drain {


/*
template <>
template <class S>
void ReferenceT<VariableBase>::init(const S & src);


template <>
template <class S>
void ReferenceT<VariableBase>::init(const S & src){
	this->assign(src); // Safe for VariableBase
}
*/





}

namespace drain {

/*
template <>
bool Variable::requestType(const std::type_info & t);

template <>
bool Reference::requestType(const std::type_info & t);

template <>
bool FlexibleVariable::requestType(const std::type_info & t);


template <>
bool Variable::suggestType(const std::type_info & t);

template <>
bool Reference::suggestType(const std::type_info & t);


template <>
bool FlexibleVariable::suggestType(const std::type_info & t);


template <>
bool Variable::requestSize(size_t elementCount);

template <>
bool Reference::requestSize(size_t elementCount);

template <>
bool FlexibleVariable::requestSize(size_t elementCount);
*/




/*
template <>
const std::string TypeName<Variable>::name;

template <>
const std::string TypeName<Reference>::name;

template <>
const std::string TypeName<FlexibleVariable>::name;





/// drain:: default implementation for output formatting
template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::Reference & v, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &) v, layout);
};

template <>
template <class D>
inline
void Convert2<Reference>::convert(const Reference &src, D & dst){
	dst = (const D &)src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}


template <>
inline
void Convert2<Reference>::convert(const char *src, Reference & dst){
	dst = src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}



template <>
template <class S>
inline
void Convert2<Reference>::convert(const S &src, Reference & dst){
	dst = src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}



template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::Variable & x, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &)x, layout);
}



template <>
template <class D>
inline
void Convert2<Variable>::convert(const Variable &src, D & dst){
	dst = (const D &)src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}


template <>
inline
void Convert2<Variable>::convert(const char *src, Variable & dst){
	dst = src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}



template <>
template <class S>
inline
void Convert2<Variable>::convert(const S &src, Variable & dst){
	dst = src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}




template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::FlexibleVariable & v, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &) v, layout);
};



template <>
template <class D>
inline
void Convert2<FlexibleVariable>::convert(const FlexibleVariable &src, D & dst){
	dst = (const D &)src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}


template <>
inline
void Convert2<FlexibleVariable>::convert(const char *src, FlexibleVariable & dst){
	dst = src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}



template <>
template <class S>
inline
void Convert2<FlexibleVariable>::convert(const S &src, FlexibleVariable & dst){
	dst = src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}


*/

}

#endif
