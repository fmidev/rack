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

//#include <drain/util/Log.h>

#include "Quantity.h"

namespace rack {

//const std::list<std::string> & compatibleVariants,
Quantity::Quantity(const std::string & name,
		const drain::Range<double> & range,
		char defaultType,
		const list_t & l,
		double undetectValue):
			name(name),
			defaultType(defaultType),
			physicalRange(range),
			undetectValue(undetectValue) {

	addEncodings(l);

}


Quantity::Quantity(const std::string & name,
		const std::list<std::string> & compatibleVariants,
		const drain::Range<double> & range,
		char defaultType,
		const list_t & l,
		double undetectValue):
			name(name),
			defaultType(defaultType),
			physicalRange(range),
			undetectValue(undetectValue) {

	for (const std::string & key: compatibleVariants){
		variants.addKey(key);
	}
	variants.addKey("TEST");

	// std::cerr << name << " koe1 " << drain::sprinter(compatibleVariants) << " -> " << variants << '\n';

	addEncodings(l);

}

Quantity::Quantity(const std::string & name,
		const std::list<std::string> & compatibleVariants,
		char defaultType,
		const list_t & l,
		double undetectValue):
			name(name),
			defaultType(defaultType),
			// physicalRange(0.0, 0.0),
			undetectValue(undetectValue) {

	for (const std::string & key: compatibleVariants){
		variants.addKey(key);
	}
	// variants.addKey("KOE");

	// std::cerr << name << " koe2 " << drain::sprinter(compatibleVariants) << " -> " << variants << '\n';

	addEncodings(l);

}

//void Quantity::addVariants(const std::list<std::string> & l){
//}


void Quantity::addEncodings(const list_t & l){

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
			if (newEntry.scaling.physRange.empty() && !physicalRange.empty()){
				//std::cout << name << ": empty range " << entry.scaling.physRange << " <-- " << range << '\n';
				newEntry.scaling.physRange.set(physicalRange); // also to floats?
			}
		}
	}

}

void Quantity::setZero(const std::string & value){

	// drain::Logger mout(__FILE__, __FUNCTION__);

	std::stringstream sstr(value);
	sstr >> undetectValue;
	if (sstr.fail()){
		undetectValue = std::numeric_limits<double>::signaling_NaN();
	}
	// mout.error(undetectValue, " good:", sstr.good(), ", bad:", sstr.bad(), " fail:", sstr.fail());

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

/// Retrieve the scaling for a given storage type.
const EncodingODIM & Quantity::get(char typecode) const {

	if (!typecode)
		typecode = defaultType;

	const const_iterator it = find(typecode);

	if (it != end()){ // null ok
		return it->second;
	}
	else {
		//drain::Logger mout("Quantity", __FUNCTION__);
		//mout.warn("undefined code for this quantity, code=" , typecode );
		// TODO return default
		static EncodingODIM empty;
		return empty;
	}

}

/// Retrieve the scaling for a given storage type.
const EncodingODIM & Quantity::get(const std::string & t) const {
	if (t.length() != 1)
		//hrow (std::runtime_error(t + "<= illegal std::string in EncodingODIM::"+__FUNCTION__+" line "+__LINE__));
		throw (std::runtime_error(t+" <= illegal std::string, "+ __FUNCTION__));
	else
		return get(t.at(0));
}




std::ostream & Quantity::toStream(std::ostream & ostr) const {
	for (const_iterator it = begin(); it != end(); ++it){
		//ostr.width(6);
		if (it->first == defaultType)
			ostr << "  *";
		else
			ostr << "   ";
		ostr << it->first << ','; //  << it->second;
		drain::Sprinter::toStream(std::cout, it->second, drain::Sprinter::cmdLineLayout);
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
