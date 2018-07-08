/*


    Copyright 2011-2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef SOURCE_ODIM_STRUCT
#define SOURCE_ODIM_STRUCT

#include <ostream>
#include <cmath>
#include <string>
#include <set>
//#include <drain/util/Options.h>
#include <drain/util/ReferenceMap.h>
#include <drain/util/Rectangle.h>
#include <drain/util/Time.h>

#include "hi5/Hi5.h"
#include "radar/Constants.h"

namespace rack {

class SourceODIM : public drain::ReferenceMap {

public:

	std::string source;
	std::string WMO;
	std::string RAD;
	std::string NOD;
	std::string PLC;
	std::string ORG;
	std::string CTY;
	std::string CMT;

	/// Sets NOD, WMO, RAD, CTY and ORG
	/**
	 *   \param source - ODIM source std::string separated with ':', eg. "what:source=WMO:02870,RAD:FI47,PLC:Utajärvi,NOD:fiuta"
	 *
	 *   Note: some radars have had semicolon as separator: RAD:NL51;PLC:nldhl
	 */
	inline
	SourceODIM(const std::string & source = "") : source(source) {
		init();
		setValues(source, ':');
		setNOD();
	};


	inline
	SourceODIM(const SourceODIM & s){
		init();
		updateFromMap(s);
		setNOD();
	};

	/// Sets NOD, WMO, RAD, CTY and ORG
	// ----

	/// Derives a most standard name. Returns the first-non empty value of NOD, RAD, WMO, PLC, ORG, CTY, CMT.
	/**
	 *  Codes in checked in their order of initialization, see init().
	 */
	const std::string & getSourceCode() const;

private:

	//inline
	void init();

	/// Assigns NOD if empty, and CMT
	//inline
	void setNOD();

};



}  // namespace rack


#endif
