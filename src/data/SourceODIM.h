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

//class SourceODIM : public drain::ReferenceMap {
class SourceODIM : public drain::FlexVariableMap {

public:

	std::string source;

	std::string WMO;
	std::string RAD;
	std::string NOD;
	std::string PLC;
	std::string ORG;
	std::string CTY;
	std::string CMT;
	std::string WIGOS;

	/// Sets NOD, WMO, RAD, CTY and ORG
	/**
	 *   \param source - ODIM source std::string separated with ':', eg. "what:source=WMO:02870,RAD:FI47,PLC:Utajärvi,NOD:fiuta"
	 *
	 *   Note: some radars have had semicolon as separator: RAD:NL51;PLC:nldhl
	 */
	SourceODIM(const std::string & source = "");

	SourceODIM(const SourceODIM & s);


	/// Derives a most standard name. Returns the first-non empty value of NOD, RAD, WMO, WIGOS, PLC, ORG, CTY, CMT.
	/**
	 *  Codes in checked in their order of initialization, see init().
	 */
	const std::string & getSourceCode() const;

	/// Return radar identifier; a single code retrieved in the order of preference.
	/**
	 * Example:
	   \code
	   getPreferredSourceCode(NOD, RAD, WMO, WIGOS, ORG, CTY, PLC);
	   \endcode
	 */
	template <class ...TT>
	const std::string & getPreferredSourceCode(const std::string & arg, const TT& ...args) const {
		//std::cerr << __FILE__ << ':' << __LINE__ << __FUNCTION__ << " (variadic args)" << std::endl;
		if (!arg.empty())
			return arg;
		else
			return getPreferredSourceCode(args...);
	}

private:

	const std::string & getPreferredSourceCode() const;


	void init();

	/// Assigns NOD if empty, and CMT
	/// Sets NOD, WMO, RAD, CTY and ORG
	void setNOD();

};



}  // namespace rack


#endif

// Rack
