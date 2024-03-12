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
#ifndef VARIABLE_FORMATTER_ODIM
#define VARIABLE_FORMATTER_ODIM

#include <ostream>
#include <cmath>
#include <string>
#include <set>
#include <algorithm>
//
#include "drain/util/VariableFormatter.h"  // for VariableHandler
#include "drain/util/Time.h"

//#include "EncodingODIM.h"

namespace rack {


template <class T>
class VariableFormatterODIM : public drain::VariableFormatter<T>{

public:

	virtual inline
	~VariableFormatterODIM(){};

	/// Checks if variables have ODIM names (keys), and apply special formatting (currently with time stamps)
	virtual
	bool formatVariable(const std::string & key, const std::map<std::string,T> & variables, const std::string & format, std::ostream & ostr) const {

		// drain::Logger mout(__FILE__, __FUNCTION__);
		// mout.warn("trying time format: ", key, " + ", format);
		if (format.find('%') != std::string::npos){
			// Time formatting (instead of C-stype printf formatting)
			if (drain::StringTools::endsWith(key, "date")){
				std::string s;
				drain::MapTools::get(variables, key, s);
				// mout.warn("time format: ", key, " -> ", s, '+', format); //  " -> ", t.str(), " => ", t.str(key));
				ostr << drain::Time(s, "%Y%m%d").str(format);
				return true;
			}
			else if (drain::StringTools::endsWith(key, "time")){
				std::string s;
				drain::MapTools::get(variables, key, s);
				// mout.warn("time format: ", key, " -> ", s, '+', format); // , " -> ", t.str(), " => ", t.str(key));
				ostr << drain::Time(s, "%H%M%S").str(format);
				return true;
			}
		}

		return drain::VariableFormatter<T>::formatVariable(key, variables, format, ostr); // basic/trad.
	}


};


}  // namespace rack


#endif

// Rack
 // REP // REP // REP
