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



#include "JSON.h"

namespace drain
{


FileInfo JSON::fileInfo("json");


template <>
const drain::JSONtree2 drain::JSONtree2::emptyNode;

void JSON::readValue(const std::string & s, Variable & v, bool keepType){
	std::istringstream istr(s);
	readValue(istr, v, keepType);
};

void JSON::readValue(std::istream & istr, Variable & v, bool keepType){

	drain::Logger mout(__FILE__, __FUNCTION__);

	std::string value;

	/*
	if ((keepType && v.isString()) || !v.typeIsSet()){
		std::getline(istr, value);
		value = StringTools::trim(value, "'\" \t\r\n");
		mout.warn("istr.readline: ", value);
		v = value;
		return;
	}
	*/

	TextReader::skipWhiteSpace(istr);

	int c = istr.peek();
	switch (c) {
	// consider: flag to support:
	// case '\'': // STRING
	case '"': // STRING
		istr.get();
		v = TextReader::scanSegment(istr, "\"");
		istr.get();
		break;
	case '{': // OBject, but yes, as STRING
		istr.get();
		v = TextReader::scanSegment(istr, "}");
		istr.get(); // swallow '}'
		break;
	case '[': // ARRAY TODO: chars/integer/float handling
		istr.get();
		value = TextReader::scanSegment(istr, "]");
		istr.get(); // swallow ']'
		if (value.find_first_of("[]") != std::string::npos){
			mout.warn("Arrays of arrays not supported (value='", value, "')");
		}
		JSON::readArray(value, v);
		break;
	default: // numeric
		value = TextReader::scanSegment(istr, "},\t\n\r"); // 2023/03 dropped space ' '
		//value = TextReader::scanSegment(istr, "}, \t\n\r"); // 2023/01 re-added ','
		//value = TextReader::scanSegment(istr, "} \t\n\r"); // 2023/01 dropped ','
		if (!(keepType && v.typeIsSet())){
			const std::type_info & type = Type::guessType(value);
			v.requestType(type);
		}
		v = value;
		/*
		mout.warn() << "Numeric: str '" << value <<"' => ";
		v.debug(mout);
		mout << " peek:" << (char)istr.peek();
		mout << mout.endl;
		*/
		//break;
	}
	//completed = true;

}


void JSON::readArray(const std::string & s, Variable & v){

	v.clear();

	std::vector<std::string> values;
	drain::StringTools::split(s, values, ',', " '\t\n\r");

	// TODO: recurse values twice such that they become converted trough friendly types (esp. true => 1)
	const std::type_info & atype = Type::guessArrayType(values);
	// Likewise, Type::getCompatibleType(typeid(bool), typeid(float));
	v.requestType(atype);
	v = values;

}



} // drain::



// Drain
