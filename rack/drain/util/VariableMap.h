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


namespace drain {

/// A map of Variables.
class VariableMap : public SmartMap<Variable> {

public:

	/// Default constructor
	inline
	VariableMap(char separator = ',') : SmartMap<Variable>(separator){
	};


	/// Copy constructor
	inline
	VariableMap(const VariableMap & v) : SmartMap<Variable>(v.separator){
		importMap(v);
	};

	/*
	inline
	VariableMap & operator=(const VariableMap & v){
		importMap(v);
		return *this;
	}
	*/

	template <class T>
	inline
	VariableMap & operator=(const std::map<std::string,T> & m){
		importMap(m); // CastableMap?
		return *this;
	}


};

/// A map of FlexVariable:s.
class FlexVariableMap : public SmartMap<FlexVariable> {

public:

	inline
	FlexVariableMap(char separator = '\0') : SmartMap<FlexVariable>(separator){
	};

	/// Copies a map like VariableMap does - creates an own entry for every input entry.
	/**
	 *   Does not try to create references (links), ie does not copy pointers even if input has referencing entries.
	 */
	inline
	FlexVariableMap(const FlexVariableMap & m) : SmartMap<FlexVariable>(m.separator){ // vField.ordered,
		importMap(m);
	};

	inline
	FlexVariableMap & operator=(const FlexVariableMap & v){
		importMap(v);
		return *this;
	}


	template <class T>
	inline
	FlexVariableMap & link(const std::string &key, T & target){
		(*this)[key].link(target);
		return *this;
	};


};

// std::ostream &operator<<(std::ostream &ostr, const VariableMap &m);

} // drain

#endif /* VARIABLE_H_ */


