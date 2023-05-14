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

#include "drain/util/Log.h"

#include "SourceODIM.h"

namespace rack {

SourceODIM::SourceODIM(const std::string & source) : source(source) {
	init();

	drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.experimental("initialized: ", *this);

	importEntries(source, ':', ','); //, LOG_NOTICE);
	// mout.experimental("imported: '", source, "' => ", *this);

	setNOD();
};


SourceODIM::SourceODIM(const SourceODIM & s){

	init();
	// importCastableMap(s);
	updateFromMap(s);
	//updateFromCastableMap(s);
	setNOD();
};


void SourceODIM::init(){
	// Considered prefix "where:source", but gets complicated for default constructor.  (?)
	link("NOD", NOD);
	link("RAD", RAD);
	link("WMO", WMO);
	link("ORG", ORG);
	link("CTY", CTY);
	link("PLC", PLC);
	link("CMT", CMT);
}


const std::string & SourceODIM::getSourceCode() const {

	/*
	for (const std::string & key : getKeyList()){
		const drain::Variable & value = (*this)[key];
		if (!value.isEmpty()){
			return
		}
	}
	*/

	#define TRY_RETURN(s) if (!s.empty()) return s
	TRY_RETURN(NOD); // TODO: add options for desired order
	TRY_RETURN(RAD);
	TRY_RETURN(WMO);
	TRY_RETURN(WIGOS); // NEW 2020
	TRY_RETURN(ORG);
	TRY_RETURN(CTY);
	TRY_RETURN(PLC);
	/*
	for (const_iterator it = this->begin(); it != this->end(); ++it) {
		if (it->second.getElementCount() > 0)
			return it->second.toStrRef(); // cast (const std::string &)
	}
	*/
	const static std::string empty;
	return empty;
}


/// Tries to resolve NOD code from partial or deprecated metadata
/**
 *  Assigns NOD if empty, and CMT
 */
void SourceODIM::setNOD(){

	// drain::Logger mout("SourceODIM", __FUNCTION__);
	if (NOD.empty()){
		switch (get("WMO", 0)){
		case 26422:
			NOD = "lvrix";
			break;
		default:
			static drain::RegExp nodRegExp("^[a-z]{5}$");
			if (nodRegExp.test(CMT)){
				NOD = CMT;
			}
			else {
				drain::Logger mout(__FILE__, __FUNCTION__);
				//NOD = getSourceCode();
				//mout.toOStr() << "Site code 'NOD' not found, substituting with '" << NOD << "'" << mout.endl;
				const std::string & code = getSourceCode();
				if (!code.empty()){
					mout.info() << "Site code 'NOD' not found, using '" << code << "' as node indicator " << mout.endl;
				}
				else {
					mout.info(*this);
					mout.note() << "Site information (what:source) not found " << mout.endl;
				}
			}
		}
	}

}




}  // namespace rack



// Rack
