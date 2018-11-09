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
#include "Castable.h"


namespace drain {


void Castable::clear(){
	if (caster.typeIsSet()){
		if (caster.getType() == typeid(std::string))
			caster.put(ptr, "");
		else {
			for (size_t i = 0; i < getElementCount(); ++i) {
				caster.put(getPtr(i), 0);
			}
		}
	}
}

std::ostream & Castable::toStream(std::ostream & ostr, char separator) const {

	const std::type_info & t = caster.getType();
	//const size_t n = getElementCount();

	if (t == typeid(std::string)){  // or any single-element?
		//ostr << "# type==std::string\n";
		caster.toOStream(ostr, ptr);  // no cast needed...
		//ostr << "{std::string}";
	}
	else if (isCharArrayString()){  // char array without outputSeparator
		ostr << getCharArray();
	}
	else if (!isEmpty()) {

		// Output element using given separator or default (outputSeparator)
		if (!separator)
			separator = this->outputSeparator;
		char sep = 0;
		for (size_t i = 0; i < getElementCount(); ++i) {
			if (sep)
				ostr << sep;
			caster.toOStream(ostr, getPtr(i));
			sep = separator;
		}
	}
	else {
		// Output nothing!
		//ostr << '[' << "Ø" << ']';  // consider array.begin, end etc
		//ostr << "alert: getElementCount()==0, elementCount=" << elementCount;
		//ostr << " byteSize=" << getByteSize();
		//ostr << " typeChar=" << drain::Type::getTypeChar(getType()) << '=' << getType().name() << '\n';
	}

	//ostr << '{' << getElementCount() << ',' << getByteSize() << '}';
	return ostr;

}

std::string Castable::toStr() const {

	if (isStlString()){
		return *(const std::string *)ptr;
	}
	else if (isCharArrayString()){
		return std::string(getCharArray());
	}
	else { // "default"
		std::stringstream sstr;
		toStream(sstr);
		return sstr.str();
	}
	//return caster.get<std::string>(ptr);
}

void Castable::toJSON(std::ostream & ostr, char fill) const {
	ostr << '{' << fill << " \"value\": ";
	valueToJSON(ostr);
	ostr << ',' << fill; //",\n";
	ostr << " \"type\": \"";
	if (isCharArrayString())
		ostr << '$';
	else
		ostr << drain::Type::getTypeChar(getType());
	// Debug:
	if (getElementCount() != 1)
		ostr << '[' << getElementCount() << ']';
	ostr << "\"";
	ostr << ',' << fill << " \"inputSeparator\": \""  << inputSeparator  << "\"";
	ostr << ',' << fill << " \"outputSeparator\": \"" << outputSeparator << "\"";
	ostr << fill << '}' << fill;
}

std::ostream & Castable::valueToJSON(std::ostream & ostr) const {

	if ((getType() == typeid(char)) || isStlString()){
		ostr << '"';
		toStream(ostr, ','); // use JSON separator
		ostr << '"';
	}
	else {
		if (getElementCount() != 1){
			ostr << '[';
			toStream(ostr);
			ostr << ']';
		}
		else
			toStream(ostr, ',');
	}
	return ostr;
}

const void Castable::typeInfo(std::ostream & ostr) const {
	ostr << '[';
	if (isString())
		ostr << '#';
	else
		ostr << drain::Type::getTypeChar(getType());
	//if (size_t n = getElementCount() != 1)
	ostr << getElementCount();
	ostr << '@' << getByteSize() << ']';
}






Castable & Castable::assignCastable(const Castable &c){

	// this is a string
	if (isString()){
		// Should call resize()
		//std::cerr << "target is string or char arrray" << '\n';
		assignString(c.toStr());
	}
	// c is a string, same thing
	else if (c.isString()){
		// Should call resize()
		// std::cerr << "src is string or char arrray" << '\n';
		assignString(c.toStr()); // when to split?
	}
	else {
		requestType(c.getType());
		requestSize(c.getElementCount());
		const size_t n = std::min(getElementCount(), c.getElementCount());
		for (size_t i = 0; i < n; ++i) {
			c.castElement(i, caster, getPtr(i));
		}
	}
	/*
	else {
		std::cerr << __FUNCTION__ << ": could not convert:\n";
		toJSON(std::cerr);
		std::cerr << '\n';
		c.toJSON(std::cerr);
		std::cerr << '\n';
	}
	 */
	return *this;
}

void Castable::assignToCharArray(const std::string & s){
	requestSize(s.size() + 1);
	if (isEmpty())
		throw std::runtime_error(std::string(__FUNCTION__) + ": array resize failed for: " + s);
	const size_t n = std::min(s.size(), getElementCount()-1);
	if (n < s.size()){
		std::cerr << __FUNCTION__ << ": truncating '" << s << "' to '" << s.substr(0, n) << "'\n";
	}
	for (size_t i=0; i < n; ++i){
		caster.put(getPtr(i), s[i]);
	}
	caster.put(getPtr(getElementCount()-1), '\0');
}


void Castable::assignString(const std::string &s){

	requestType(typeid(std::string));

	if (isStlString()){
		caster.put(ptr, s);  // does not split!
	}
	else if (isCharArrayString()){
		assignToCharArray(s);  // does not split!
	}
	else {
		//std::cerr << __FUNCTION__ << ": assigning '" << s << "' to type "<< drain::Type::getTypeChar(getType()) << std::endl;
		/// list<string> ok here, simply a tmp container for split string
		std::list<std::string> l;
		StringTools::split(s, l, std::string(1, inputSeparator));
		//requestType(typeid(double));
		requestSize(l.size()); // check if true?
		if (l.size() > getElementCount())
			std::cerr << __FUNCTION__ << ": provided " << l.size() << " elements, assigning: "<< getElementCount() << std::endl;

		assignContainer(l);

	}

}

}  // namespace drain


// Drain
