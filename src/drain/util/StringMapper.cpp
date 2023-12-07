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
#include <sstream>
#include "Sprinter.h"
#include "StringMapper.h"

namespace drain {


//const SprinterLayout StringMapper::layout(""); // no separator char, no braces etc.


StringMapper & StringMapper::parse(const std::string &s, bool convertEscaped) {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("regExp= " , regExp );

	clear();  // clear list

	// Splits s to "<*><variable><*>"
	//std::stringstream sstr;
	// NOTE: Dollar $ and curlies have to be consistent with variablePrefix/Postfix
	// WARNING: BRACES cause problems with newlines. And especially \\$ = \\ + $ was wrong...
	//sstr << "^([^${]*)\\$?\\{(" << validChars << ")\\}(.*)$";
	//sstr << "^([^${]*)\\$\\{(" << validChars << ")\\}(.*)$";
	//sstr << "^(.*)\\$\\{(" << validChars << ")\\}(.*)$";
	//RegExp r(sstr.str(), REG_EXTENDED); //  | REG_NEWLINE |  RE_DOT_NEWLINE); // | RE_DOT_NEWLINE); //  | REG_NEWLINE |  RE_DOT_NEWLINE

	if (convertEscaped){
		std::string s2(s);
		return parse(StringMapper::convertEscaped(s2), regExp);
	}
	else {
		return parse(s, regExp);
	}
}

StringMapper & StringMapper::parse(const std::string &s, RegExp &r) {

	RegExp::result_t result;

	r.execute(s, result);

	// std::cout << " StringMapper::parse size= " << r.result.size() << "\n";
	// std::cout << __FUNCTION__ << " r=" << r << '\t' << r.result.size() << std::endl;

	// The size of the result is 4, if ${variable} found, else 0, and result is literal.

	if (result.empty()){
		if (!s.empty())
			push_front(Stringlet (s, false));
	}
	else if (result.size() >= 4){  // 2021/01 allow () in allowedChars
		const std::string left     = result[1]; // leading part
		const std::string variable = result[2];
		const std::string right    = result.back(); // result[3]; // trailing part
		//std::cerr << __FUNCTION__ << " added Stringlet " << variable << std::endl;
		parse(right, r);
		push_front(Stringlet (variable, true));
		parse(left, r);
	}
	else {
		StringTools::join(result, std::cerr, '|');
		std::cerr << '\n';
		// std::cerr << " regexo: '" << validChars << "'\n";
		std::cerr << " StringMapper warning: parsing oddity: " << s << " != " << r << "\n";
	}

	return *this;

}

bool StringMapper::isLiteral() const {
	for (const Stringlet & s: *this){
		if (s.isVariable())
			return false;
	}
	return true;
}
/*
std::ostream &  StringMapper::toStream(std::ostream & ostr) const {
	SprinterLayout layout;
	layout.arrayCaps.set(0, separator.character,0);
	layout.stringCaps.fill(0);
	Sprinter::sequenceToStream(ostr, *this, layout.arrayCaps, layout);
	return ostr;
}
*/



}


// Drain
