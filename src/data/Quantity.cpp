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

//#include "drain/util/Log.h"

#include "Quantity.h"

namespace rack {

Quantity::Quantity(const std::string & name,
		const drain::Range<double> & range,
		char defaultType,
		const list_t & l,
		double undetectValue):
			name(name),
			defaultType(defaultType),
			physicalRange(range),
			undetectValue(undetectValue) {

	drain::Logger mout(__FILE__, __FUNCTION__);

	for (const typename list_t::value_type & entry: l){
		if (entry.type.empty()){
			// WARN
			mout.warn("...");
		}
		else {
			const char type = entry.type.at(0);
			//list_t::value_type & newEntry = this->insert(this->end, list_t::value_type(ODIMPathElem::OTHER)); // (*this)[type];
			//this->insert({type, EncodingODIM(ODIMPathElem::OTHER)}); //(*this)[type];
			EncodingODIM & newEntry = (*this)[type];

			newEntry = entry;
			if (!this->defaultType)
				this->defaultType = type;
			if (newEntry.scaling.physRange.empty() && !range.empty()){
				//std::cout << name << ": empty range " << entry.scaling.physRange << " <-- " << range << '\n';
				newEntry.scaling.physRange.set(range); // also to floats?
			}
		}
	}

}


EncodingODIM & Quantity::set(char typecode) {

	if (!typecode)
		typecode = 'C';  // ???

	if (!defaultType)
		defaultType = typecode;

	EncodingODIM & odim = (*this)[typecode];
	odim.type = typecode;
	if (!odim.isSet())
		odim.setTypeDefaults();

	return odim;
}

std::ostream & Quantity::toStream(std::ostream & ostr) const {
	for (const_iterator it = begin(); it != end(); ++it){
		//ostr.width(6);
		if (it->first == defaultType)
			ostr << "  *";
		else
			ostr << "   ";
		ostr << it->first << ',' << it->second;
		if (drain::Type::call<drain::typeIsInteger>(it->first)){
			ostr << " (min=" << it->second.getMin() << ')';
		}
		ostr << '\n';
	}
	if (hasUndetectValue())
		ostr << '\t' << "virtual zero=" << undetectValue << '\n';
	return ostr;
}


}  // namespace rack



// Rack
