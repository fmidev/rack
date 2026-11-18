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


#include "StringMapper.h"
#include "drain/Log.h"


namespace drain {


DRAIN_ENUM_DICT(StringMapper::handleMissing) = {
		DRAIN_ENUM_ENTRY(StringMapper, REMOVE_MISSING_VARIABLE),
		DRAIN_ENUM_ENTRY(StringMapper, KEEP_MISSING_VARIABLE),
};


void StringMapper::updateRegExp(){
	std::stringstream sstr;
	/*
	sstr << "^(.*)\\$\\{(" << validChars; //  << ")\\}(.*)$";
	if (formatting){
		//sstr << "(\\|[^}]*)?";
		sstr << "(\\|([^}]*))?";
	}
	sstr << ")\\}(.*)$";
	*/
	sstr << "^(.*)\\$\\{(" << validChars << ')'; //  << ")\\}(.*)$";
	if (formatting){
		//sstr << "(\\|[^}]*)?";
		sstr << "(\\|([^}]*))?";
	}
	sstr << "\\}(.*)$";
	// std::cout << "REGEXP::" << sstr.str() << '\n';
	regExp.setExpression(sstr.str());
}

StringMapper & StringMapper::parse(const std::string &s, bool convertEscaped) {

	drain::Logger mout(__FILE__, __FUNCTION__);


	clear();  // clear list

	if (!regExp.isSet()){
		updateRegExp();
		mout.debug("updated regExp: ", DRAIN_LOG(regExp));
	}

	// Splits s to "<*><variable><*>"
	// std::stringstream sstr;
	// NOTE: Dollar $ and curlies have to be consistent with variablePrefix/Postfix
	// WARNING: BRACES cause problems with newlines. And especially \\$ = \\ + $ was wrong...
	// sstr << "^([^${]*)\\$?\\{(" << validChars << ")\\}(.*)$";
	// sstr << "^([^${]*)\\$\\{(" << validChars << ")\\}(.*)$";
	// sstr << "^(.*)\\$\\{(" << validChars << ")\\}(.*)$";
	// RegExp r(sstr.str(), REG_EXTENDED); //  | REG_NEWLINE |  RE_DOT_NEWLINE); // | RE_DOT_NEWLINE); //  | REG_NEWLINE |  RE_DOT_NEWLINE

	if (convertEscaped){
		std::string s2(s);
		return parse(StringTools::convertEscaped(s2), regExp);
		// mout.warn("converted:  ", s2);
		// return *this;
	}
	else {
		// mout.warn("direct: ", s);
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
		if (!s.empty()){
			push_front(Stringlet(s,false));
		}
	}
	//else if (result.size() >= 4){  // 2021/01 allow () in allowedChars
	else if (result.size() >= 4){  // 2021/01 allow () in allowedChars

		// std::cerr << __FUNCTION__ << result.size() << std::endl;
		// std::cerr << sprinter(result) << std::endl;
		const std::string & left     = result[1]; // leading part
		const std::string & variable = result[2];
		const std::string & right    = result.back(); // result[3]; // trailing part
		// std::cerr << __FUNCTION__ << " added Stringlet " << variable << std::endl;
		parse(right, r);
		// std::cerr << __FUNCTION__ << result.size() << " added Stringlet " << variable << " FORMAT=" << result[4] << std::endl;
		// push_front(Stringlet (variable, true));
		if (result.size() >= 5){
			// Format string detected: pipe '|' followed by a string (possibly empty)
			const std::string & format   = result[4];
			// std::cerr << __FUNCTION__ << " added Stringlet " << variable << ", FORMAT=" << format << std::endl;
			push_front(Stringlet(variable, format));
		}
		else {
			// Variable without formatting
			push_front(Stringlet(variable,true));
			// std::cerr << __FUNCTION__ << " added Stringlet " << variable << ", no formatting" << std::endl;
		}

		parse(left, r);
	}
	else {
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.warn("parsing oddity: ", s, " != ", r, " result=", sprinter(result));
		/*
		StringTools::join(result, std::cerr, '|');
		std::cerr << '\n';
		// std::cerr << " regexo: '" << validChars << "'\n";
		std::cerr << " StringMapper warning: parsing oddity: " << s << " != " << r << "\n";
		*/
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
