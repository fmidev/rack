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

#include <drain/RegExp.h>
#include <drain/Sprinter.h>
#include <drain/Type.h>

//#include "hi5/Hi5.h"
#include "KeyMatcher.h"
#include "KeySelector.h"

//#include "ODIMPathTools.h"


namespace drain {

// Consider more general key matcher?


void KeySelector::setQuantities(const std::string & args){ //, const std::string & separators){
	drain::Logger mout(__FILE__, __FUNCTION__);
	clear();
	std::vector<std::string> argv;
	//const char s =
	drain::StringTools::split(args, argv, ",:"); //separators);
	for (const std::string & arg: argv){
		// mout.attention("adapting: ", arg, " split by '", separators, "'");
		adaptQuantity(arg);
	}

	/* problematic. Keep for a while
		if (s == ':'){
			mout.deprecating("usage of colon ':' as quantity separator ");
		}
	 */
}


/*
void QuantitySelector::setQuantityRegExp(const std::string & r){

	if (!empty()){ // quantities.
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.note("Adding regExp=", r, ", also listed quantities exist: "); // , quantities
	}

	// this->regExp.setExpression(r);
	adaptQuantity(r);
}
*/


/** In future, the recommended way to define desired/accepted quanties is a comma-separated list of keys.
 *
 */
void KeySelector::adaptQuantity(const std::string & s){


	if (s.empty()){
		return;
	}
	else {

		KeyMatcher matcher(s);
		for (const KeyMatcher & m: *this){
			if (m.value == matcher.value){
				// exists already, dont append...
				return;
			}
		}

		// quantities.
		push_back(KeyMatcher());
		// quantities.
		back().set(s);
	}

	/*
	else if (s.find_first_of("^?*[]()$") == std::string::npos){
		quantities.push_back(s);
	}
	else {
		regExp.setExpression(s);
	}
	*/
}




bool KeySelector::testQuantity(const std::string & s) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (empty()){
		return true; // or defaultValue? TODO
	}
	else {
		for (const auto & q: *this){
			// mout.experimental("testing [", s, "] vs [", q, "]");
			if (q == s){
				return true;
			}
		}
	}

	return false;
	/*
	for (const std::string & q: quantities){
		// mout.experimental("testing [", s, "] vs [", q, "]");
		if (s == q){
			return true;
		}
	}

	if (regExp.empty()){
		// Important here: if quantities listed, but none matched, return false.
		// Hence: return true only if both tests are empty.
		return quantities.empty();
	}
	else {
		//drain::Logger mout(__FILE__, __FUNCTION__);
		//mout.attention<LOG_NOTICE>("unmatched quantities, testing ", s ," regExp: ", regExp);
		return regExp.test(s);
	}
	*/

}

/*
void KeySelector::getQuantityMap(const Hi5Tree & srcDataset, ODIMPathElemMap & m){

	for (const auto & entry: srcDataset) {
		if (entry.first.is(ODIMPathElem::DATA)){
			const drain::VariableMap & attr = entry.second[ODIMPathElem::WHAT].data.attributes;
			m[attr.get("quantity","")] = entry.first;
		}
	}

};
*/

/*
void QuantitySelector::addQuantity(){
}
*/

void KeySelector::toStream(std::ostream & ostr) const {
	drain::Sprinter::sequenceToStream(ostr, *this, drain::Sprinter::cmdLineLayout);
	//if (regExp.isSet() && !empty())
	//	ostr << drain::Sprinter::cmdLineLayout.arrayChars.separator;
	// ostr << regExp.toStr();
}

DRAIN_TYPENAME_DEF(KeySelector);

}  // drain::
