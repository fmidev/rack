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

#include "VariableBase.h"
#include "ReferenceT.h"
#include "VariableT.h"
//#include "Convert.h"

//#include "Referencer.h"
// #include "Sprinter.h"
//#include "String.h"
//#include "UniTuple.h" // "Friend class"
//#include "Type.h"


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





typedef VariableT<VariableBase>           Variable;
typedef VariableT<ReferenceT<Castable> >  Reference;
typedef VariableT<ReferenceT<Variable> >  FlexibleVariable;

template <>
template <>
inline
Variable & Variable::operator=(const Reference &x){
		this->assignCastable(x);
		return *this;
}

template <>
template <>
inline
Variable & Variable::operator=(const FlexibleVariable &x){
		this->assignCastable(x);
		return *this;
}


template <>
template <>
inline
Reference & Reference::operator=(const Variable &x){
		this->assignCastable(x);
		return *this;
}

template <>
template <>
inline
Reference & Reference::operator=(const FlexibleVariable &x){
		this->assignCastable(x);
		return *this;
}


template <>
template <>
inline
FlexibleVariable & FlexibleVariable::operator=(const Variable &x){
		this->assignCastable(x);
		return *this;
}

template <>
template <>
inline
FlexibleVariable & FlexibleVariable::operator=(const Reference &x){
		this->assignCastable(x);
		return *this;
}




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



// Override for "ambivalent" FlexibleVariable
template <>
void FlexibleVariable::info(std::ostream & ostr) const;


template <>
const std::string TypeName<Variable>::name;

template <>
const std::string TypeName<Reference>::name;

template <>
const std::string TypeName<FlexibleVariable>::name;






}

#endif
