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

#include "Variable.h"


namespace drain {


void Variable::setType(const std::type_info & t){ //, size_t n = 0

		reset(); // what if requested type already?

		if ((t == typeid(std::string)) || (t == typeid(char *))){
			//std::cerr << "Variable::setType (std::string) forward, OK\n";
			caster.setType(typeid(char));
			// setOutputSeparator(0); NEW 2019/11
			setOutputSeparator(0);
			//resize(1);
			updateSize(1);
			//caster.put(ptr, '\0');
			caster.put('\0');
		}
		else {
			if (t != typeid(void)){  // why not unset type
				caster.setType(t);
				// IS_STRING = false;
				//resize(n);
			}
			else {
				caster.setType(t); // unset
			}
		}
	}

bool Variable::updateSize(size_t elementCount){

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

	return true;
}


void Variable::updateIterators()  {

	dataBegin.setType(getType());
	dataBegin = (void *) & data[0];

	dataEnd.setType(getType());
	dataEnd = (void *) & data[ getElementCount() * getElementSize() ]; // NOTE (elementCount-1) +1

}

template <>
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::Variable & v, const SprinterLayout & layout) {

	if ((v.getElementCount() > 1) && !v.isString()){
		return Sprinter::sequenceToStream(ostr, v, layout);
	}
	else {
		return Sprinter::toStream(ostr, (const Castable &)v, layout); //
	}

	/*
	else if (v.getElementCount() > 1) {
		Sprinter::sequenceToStream(ostr, v, layout);
	}
	else if (v.empty()) {
		Sprinter::toStream(ostr, (double *)0, layout);
	}
	else if (v.getType() == typeid(bool)) {
		Sprinter::toStream(ostr, (bool)v, layout);
	}
	else {
		Sprinter::basicToStream(ostr, v, ""); // ?
		// ostr << v; // Infinite loop? Sprinter::basicToStream(ostr, v, myChars);
	}
	*/
	return ostr;
}

void FlexVariable::info(std::ostream & ostr) const {
	Castable::info(ostr);
	if (isReference())
		ostr << " <reference>";
	else
		ostr << " <own>";
}



} // drain
