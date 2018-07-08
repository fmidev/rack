/*


    Copyright 2011-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

   Created on: Sep 30, 2010
*/

#include <drain/util/Log.h>

#include "SourceODIM.h"

namespace rack {


const std::string & SourceODIM::getSourceCode() const {

	//sourceMap.setValues(source, ':');  // 2nd par: equal-sign
	#define TRY_RETURN(s) if (!s.empty()) return s
	TRY_RETURN(NOD); // TODO: add options for desired order
	TRY_RETURN(RAD);
	TRY_RETURN(WMO);
	TRY_RETURN(ORG);
	TRY_RETURN(CTY);
	TRY_RETURN(PLC);
	/*
	for (const_iterator it = this->begin(); it != this->end(); ++it) {
		if (it->second.getElementCount() > 0)
			return it->second.toStrRef(); // cast (const std::string &)
	}
	*/
	static std::string empty;
	return empty;
}


void SourceODIM::init(){
	// Considered prefix "where:source", but gets complicated for default constructor.  (?)
	reference("NOD", NOD);
	reference("RAD", RAD);
	reference("WMO", WMO);
	reference("ORG", ORG);
	reference("CTY", CTY);
	reference("PLC", PLC);
	reference("CMT", CMT);
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
				drain::Logger mout("SourceODIM", __FUNCTION__);
				//NOD = getSourceCode();
				//mout.toOStr() << "Site code 'NOD' not found, substituting with '" << NOD << "'" << mout.endl;
				mout.info() << "Site code 'NOD' not found, using '" << getSourceCode() << "' as node indicator " << mout.endl;
			}
		}
	}

}




}  // namespace rack


