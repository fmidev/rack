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

//#include <algorithm>
//#include <syslog.h>  // levels: LOG_ERROR etc.

#include "drain/util/RegExp.h"
#include "drain/util/Sprinter.h"
#include "drain/util/Type.h"

#include "hi5/Hi5.h"
#include "DataSelector.h"

#include "ODIMPathTools.h"


namespace rack {


/*
void QuantitySelector::setQuantity(const std::string & quantity){
	this->quantity = quantity;
	//updateQuantity();
}
*/


void QuantitySelector::setQuantityRegExp(const std::string & r){

	if (!quantities.empty()){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.note("Adding regExp=", r, ", also listed quantities exist: "); // , quantities
	}

	this->regExp.setExpression(r);

}


/** In future, the recommended way to define desired/accepted quanties is a comma-separated list of keys.
 *
 */
void QuantitySelector::adaptQuantity(const std::string & s){

	if (s.empty()){
		return;
	}
	else if (s.find_first_of("^?*[]()$") == std::string::npos){
		quantities.push_back(s);
	}
	else {
		/*
		if (regExp.isSet()){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.warn("Overriding regExp '", regExp, " with ", s, "'");
		}
		*/
		regExp.setExpression(s);
	}

}




bool QuantitySelector::testQuantity(const std::string & s) const {

	for (const std::string & q: quantities){
		//mout.experimental("testing [", s, "] vs [", q, "]");
		if (s == q){
			return true;
		}
	}

	if (regExp.empty()){
		// Important here: if quantities listed, but none matched, return false.
		// That is, return true only if both tests are empty.
		return quantities.empty();
	}
	else {
		//drain::Logger mout(__FILE__, __FUNCTION__);
		//mout.attention<LOG_NOTICE>("unmatched quantities, testing ", s ," regExp: ", regExp);
		return regExp.test(s);
	}

}

void QuantitySelector::getQuantityMap(const Hi5Tree & srcDataset, ODIMPathElemMap & m){

	for (const auto & entry: srcDataset) {
		if (entry.first.is(ODIMPathElem::DATA)){
			const drain::VariableMap & attr = entry.second[ODIMPathElem::WHAT].data.attributes;
			m[attr.get("quantity","")] = entry.first;
		}
	}

};


void QuantitySelector::addQuantity(){
}

void QuantitySelector::toStream(std::ostream & ostr) const {
	drain::Sprinter::sequenceToStream(ostr, quantities, drain::Sprinter::cmdLineLayout);
	if (regExp.isSet() && !quantities.empty())
		ostr << drain::Sprinter::cmdLineLayout.arrayChars.separator;
	ostr << regExp;
}

}  // rack::
