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


#include "FlexibleVariable.h"

namespace drain {

template <>
bool FlexibleVariable::requestType(const std::type_info & t){
	if (isLinking()){
		return caster.requestType(t);
	}
	else {
		VariableBase::setType(t);
		return true;
	}
}


template <>
bool FlexibleVariable::suggestType(const std::type_info & t){
	if (isLinking()){
		return Castable::suggestType(t);
	}
	else if (!typeIsSet())
		setType(t);
	return true;
}

template <>
bool FlexibleVariable::requestSize(size_t elementCount){
	if (isLinking()){
		Logger mout(__FILE__, __FUNCTION__);
		if (this->elementCount != elementCount){
			/*  DEBUGGING... (This IS ok, it is only a request...)
			mout.warn("requesting resize (", this->elementCount, " -> ", elementCount, ") for linked flexVar");
			mout.warn();
			this->info(mout);
			// mout << " - requesting: " ;
			mout.end();
			*/
		}
		// mout.special("LINKING, current size: ", this->elementCount, ", requested:  ", elementCount);
		// mout.special("LINKING, current value: ", *this);
		return Castable::requestSize(elementCount);
	}
	else {
		// mout.special("LOCAL VAR, setting size:", elementCount);
		return setSize(elementCount);
	}
}


/**
 *
 *
 */
// Override for "ambivalent" FlexibleVariable
template <>
void FlexibleVariable::info(std::ostream & ostr) const {
	//void VariableT<ReferenceBase<VariableT<VariableBase> > >::info(std::ostream & ostr) const {
	Castable::info(ostr);
	if (this->isLinking())
		ostr << " <reference>";
	else
		ostr << " <own>";
}

//#define DRAIN_TYPENAME(tname) template <>  const std::string TypeName<tname>::name(#tname)

DRAIN_TYPENAME_DEF(FlexibleVariable);

// template <>
// const std::string TypeName<FlexibleVariable>::name("FlexibleVariable");


}
