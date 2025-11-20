/*

    Copyright 2001 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */

/*
  RES-QUIRE: drain/util/{Log,TextStyle,TextStyleVT100,Time}.cpp
 */

#include "./Serializer.h"

namespace drain {

const Serializer::objType Serializer::NONE;  // =0;
const Serializer::objType Serializer::KEY;   // =1;
const Serializer::objType Serializer::VALUE; // =2;
const Serializer::objType Serializer::PAIR;  // =KEY|VALUE;
const Serializer::objType Serializer::LIST;  // =4;
const Serializer::objType Serializer::SET;   // =8;
const Serializer::objType Serializer::MAP;   // =16;
const Serializer::objType Serializer::MAP_KEY; // =MAP|KEY;
const Serializer::objType Serializer::MAP_VALUE; // =MAP|VALUE;
const Serializer::objType Serializer::MAP_PAIR; // =MAP|PAIR;
const Serializer::objType Serializer::TUPLE;  // =64;
const Serializer::objType Serializer::STRING; // =128;


// static
const char Serializer::SINGLE_QUOTE = '\'';
const char Serializer::DOUBLE_QUOTE = '"';

// Consider ENUM  (NEWLINE=, TAB)
const Serializer::conv_map_t Serializer::conversion = {
		{'\t', "\\t"},
		{'\n', "\\n"},
};


// SimpleFormatter SimpleSerializer::defaultFormatter(",");
const SimpleFormatter & SimpleSerializer::getFormatter(Serializer::objType t) const{

	char_map_t::const_iterator it = this->fmtChars.find(t);
	if (it != fmtChars.end()){
		return it->second;
	}
	else if (t == Serializer::STRING){
		static const SimpleFormatter sfmt("''");
		return sfmt;
	}
	else if (t == Serializer::NONE){ // applies also to "first separator"
		static const SimpleFormatter sfmt("");
		return sfmt;
	}
	else {
		static const SimpleFormatter fmt(",");
		return fmt;
	}
};




void SimpleSerializer::handleChars(std::ostream & ostr, char c) const{
	typename conv_map_t::const_iterator it = conversion.find(c);
	if (it != conversion.end()){
		ostr << it->second;
	}
	else {
		ostr << c;
	}
}
void SimpleSerializer::handleChars(std::ostream & ostr, const char *s) const{
	for (const char *c = s; *c; ++c){
		handleChars(ostr, *c);
	}
}
/*
void SimpleSerializer::writePrefix(std::ostream & ostr, objType type) const {
	writePrefix(ostr, getFormatter(type));
};


void SimpleSerializer::writeSeparator(std::ostream & ostr, objType type) const {

	const SimpleFormatter & fmt = getFormatter(type);
	if (fmt.separator){
		ostr << fmt.separator;
	}

	//if (type != NONE)
	//	ostr << ',';
};


void SimpleSerializer::writeSuffix(std::ostream & ostr, objType type) const {

	const SimpleFormatter & fmt = getFormatter(type);
	if (fmt.suffix){
		ostr << fmt.suffix;
	}

	//...
	switch(type){
	case MAP:
		ostr << '}';
		break;
	case LIST:
	case SET:
		ostr << ']';
		break;
	case PAIR:
	case TUPLE:
		ostr << ')';
		break;
	default:
		return;
	}
	...//

};
*/



}
