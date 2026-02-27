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

//#include <sstream>
#include <drain/TypeUtils.h>
#include <ostream>
#include <stdexcept>

//#include "Log.h"
#include <drain/TextStyle.h>

#include "TextDecorator.h"



namespace drain
{


template <>
const drain::Enum<TextStyle::Style>::dict_t drain::Enum<TextStyle::Style>::dict = {
		{"ITALIC", TextStyle::ITALIC},
		{"BOLD", TextStyle::BOLD},
		{"DIM", TextStyle::DIM},
		{"INVERSE", TextStyle::REVERSE}, // VT100 8
//		{"", TextStyle::},
};

template <>
const drain::Enum<TextStyle::Colour>::dict_t drain::Enum<TextStyle::Colour>::dict = {
		{"BLACK", TextStyle::BLACK},
		{"GRAY", TextStyle::GRAY},
		{"WHITE", TextStyle::WHITE},
		{"RED",   TextStyle::RED},
		{"GREEN", TextStyle::GREEN},
		{"YELLOW", TextStyle::YELLOW},
		{"BLUE",  TextStyle::BLUE},
		{"CYAN",  TextStyle::CYAN},
		{"PURPLE", TextStyle::PURPLE}
		//		{"", TextStyle::}
};

template <>
const drain::Enum<TextStyle::Line>::dict_t drain::Enum<TextStyle::Line>::dict = {
		{"NO_LINE", TextStyle::NO_LINE}, // deprecating
		{"UNDERLINE", TextStyle::UNDERLINE},
		{"DOUBLE_UNDERLINE", TextStyle::DOUBLE_UNDERLINE},
		{"OVERLINE", TextStyle::OVERLINE},
//		{"", TextDecorator::},
};




/**
 *
 *
 *
 */
void TextDecorator::addKeys(const std::string & key){

	std::list<std::string> l;
	//drain::StringTools::split(key, l, ",");
	drain::StringTools::split(key, l, separator);
	for (const std::string & k: l){
		// std::cout << "  +" << k << '\n';
		addKey(k);
	}

}

/** Assumes \c key is a key (ie. "resolved", no string splitting needed)
 *
 */
void TextDecorator::addKey(const std::string & key){
	if (key.empty()){
		return;
	}
	if (style.getDict().hasKey(key)){
		style.add(key);
		return;
	}
	if (color.getDict().hasKey(key)){
		color.set(key);
		return;
	}
	if (line.getDict().hasKey(key)){
		line.set(key);
		return;
	}
	throw std::runtime_error(drain::StringBuilder<':'>(__FILE__, __FUNCTION__, " no such key", key));
	return;
}

/**
 *
 *
 *
 */
void TextDecorator::debug(std::ostream & ostr) const {
	style.debug(ostr);
	ostr << '\n';
	color.debug(ostr);
	ostr << '\n';
	line.debug(ostr);
	ostr << '\n';
}

/// Export style to VT100 numeric codes
/**
 *   First, extract style enums from the current flags (integer).
 *
 *   \tparam TS -
 *
 *   Not: TextStyle::Style, TextStyle::Line
 *
 */  //
/*
template <typename TS>
void appendCodes(const TS & styleFlags, std::list<int> & codes){

	if (styleFlags){ // is non-zero
		std::list<typename TS::value_t> l;
		FlagResolver::valuesToList(styleFlags.getDict(), styleFlags.value, l);
		for (typename TS::value_t v: l){
			codes.push_back(TextStyleVT100::getIntCode(v));
		}
	}

}
*/


std::ostream & TextDecoratorVt100::_begin(std::ostream & ostr) const {

	std::list<int> codes;

	appendCodes(style, codes);
	appendCodes(color, codes);
	appendCodes(line,  codes);

	if (!codes.empty()){
		ostr << "\033["; // VT100 decl start code
		ostr << drain::StringTools::join(codes,';'); // consider SprinterLayout(";");
		ostr << 'm'; // VT100 decl end code //  << "\]";
	}

	return ostr;
}

std::ostream & TextDecoratorVt100::_end(std::ostream & ostr) const {
	ostr << "\033[0m";
	return ostr;
}







} // drain::
