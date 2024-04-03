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



#include "VariableBase.h"


namespace drain {

void VariableBase::setType(const std::type_info & t){ //, size_t n = 0

	reset(); // what if requested type already?

	if ((t == typeid(std::string)) || (t == typeid(const std::string)) || (t == typeid(const char *)) || (t == typeid(char *))){
		// std::cout << __FILE__ << "::" << __FUNCTION__ << ':' << t.name() << " is string \n";
		caster.setType(typeid(char));
		setOutputSeparator(0);
		updateSize(1);
		caster.put('\0');
	}
	else {
		// std::cout << __FILE__ << "::" << __FUNCTION__ << ':' << t.name() << " not a string \n";

		if (t == typeid(void)){  // why not unset type
			// std::cerr << __FUNCTION__ << ':' << t.name() << " is void " << std::endl;
			caster.unsetType(); // else infinite loop
			//resize(n);
		}
		else if (Type::call<drain::typeIsFundamental>(t)){
			// std::cerr << __FUNCTION__ << ':' << t.name() << " is fund " << std::endl;
			caster.setType(t);
		}
		else {
			// std::cerr << __FUNCTION__ << ':' << t.name() << " throw... " << std::endl;
			throw std::runtime_error(std::string(__FILE__) + __FUNCTION__ + ':' + t.name() + ": cannot convert to basic types");
			//caster.setType(t); // set(void) = unset  // else infinite loop
		}

	}
}


//bool
void VariableBase::updateSize(size_t elementCount){

	this->elementCount = elementCount;

	if (elementCount > 0)
		data.resize(elementCount * getElementSize());
	else {
		if (getElementSize() > 0)
			data.resize(1 * getElementSize());
		else
			data.resize(1);
		data[0] = 0; // for std::string toStr();
	}

	caster.ptr = &data[0]; // For Castable

	updateIterators();

	//return true;
}


void VariableBase::updateIterators()  {

	dataBegin.setType(getType());
	dataBegin = (void *) & data[0];

	dataEnd.setType(getType());
	dataEnd = (void *) & data[ getElementCount() * getElementSize() ]; // NOTE (elementCount-1) +1

}


} // drain::
