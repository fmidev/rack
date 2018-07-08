/**

    Copyright 2012-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef DRAIN_VARIABLEMAP_H
#define DRAIN_VARIABLEMAP_H

#include <string.h>
#include <string>
#include <vector>
#include <map>

#include "Castable.h"
#include "CastableIterator.h"
#include "String.h"

#include "Variable.h"
#include "SmartMap.h"

// // // using namespace std;

namespace drain {


//typedef std::map<std::string,Variable> VariableMap;

/// A map of Variables.
class VariableMap : public SmartMap<Variable> { //std::map<std::string,Variable> {

public:

	inline  // strictness_t s=OPEN,
	//VariableMap(bool ordered=true, char separator='\0') : SmartMap<Variable>(ordered, separator){
	VariableMap(char separator = '\0') : SmartMap<Variable>(separator){
	};

	inline
	// vField.ordered
	VariableMap(const VariableMap & v) : SmartMap<Variable>(v.separator){ // vField.ordered,
		importMap(v);
	};

	inline
	VariableMap & operator=(const VariableMap & v){
		importMap(v);
		return *this;
	}

};


//std::ostream &operator<<(std::ostream &ostr, const VariableMap &m);

} // drain

#endif /* VARIABLE_H_ */
