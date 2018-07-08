/**

    Copyright 2009 - 2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

    Author: mpeura
 */
#include <sstream>
#include "StringMapper.h"

namespace drain {

void StringMapper::parse(const std::string &s) {

	clear();  // clear list

	// Splits s to "<*><variable><*>"
	std::stringstream sstr;
	// NOTE: Dollar $ and curlies have to be consistent with variablePrefix/Postfix
	// WARNING: BRACES cause problems with newlines. And especially \\$ = \\ + $ was wrong...
	//sstr << "^([^\\$]*)\\$?\\{(" << validChars << ")\\}(.*)$";  // NOTE: has to be consistent with variablePrefix/Postfix
	//sstr << "^(.*[^$])?\\$?\\{(" << validChars << ")\\}(.*)$";
	sstr << "^([^${]*)\\$?\\{(" << validChars << ")\\}(.*)$";
	RegExp r(sstr.str(), REG_EXTENDED); //  | REG_NEWLINE |  RE_DOT_NEWLINE); // | RE_DOT_NEWLINE); //  | REG_NEWLINE |  RE_DOT_NEWLINE

	parse(s,r);
}

void StringMapper::parse(const std::string &s, RegExp &r) {

	r.execute(s);
	// std::cout << " StringMapper::parse size= " << r.result.size() << "\n";
	// std::cout << __FUNCTION__ << " r=" << r << '\t' << r.result.size() << std::endl;

	// The size of the result is 4, if ${variable} found, else 0, and result is literal.

	if (r.result.empty()){
		if (!s.empty())
			push_front(Stringlet (s, false));
	}
	else if (r.result.size() == 4){
		const std::string left     = r.result[1]; // leading part
		const std::string variable = r.result[2];
		const std::string right    = r.result[3]; // trailing part
		//std::cerr << __FUNCTION__ << " added Stringlet " << variable << std::endl;
		parse(right, r);
		push_front(Stringlet (variable, true));
		parse(left, r);
	}
	else {
		StringTools::join(r.result, std::cerr, '|');
		std::cerr << '\n';
		std::cerr << " validChars: '" << validChars << "'\n";
		std::cerr << " StringMapper warning: parsing oddity: " << s << " != " << r << "\n";
	}


}




}

