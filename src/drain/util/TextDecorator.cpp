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
#include <ostream>
#include <stdexcept>

//#include "Log.h"

#include "TextDecorator.h"


namespace drain
{


template <>
const drain::FlaggerDict drain::EnumDict<TextDecorator::Style>::dict = {
		{"ITALIC", TextDecorator::ITALIC},
		{"BOLD", TextDecorator::BOLD},
		{"DIM", TextDecorator::DIM},
		{"INVERSE", TextDecorator::REVERSE}, // VT100 8
//		{"", TextDecorator::},
};

template <>
const drain::FlaggerDict drain::EnumDict<TextDecorator::Colour>::dict = {
		{"BLACK", TextDecorator::BLACK},
		{"GRAY", TextDecorator::GRAY},
		{"WHITE", TextDecorator::WHITE},
		{"RED",   TextDecorator::RED},
		{"GREEN", TextDecorator::GREEN},
		{"YELLOW", TextDecorator::YELLOW},
		{"BLUE",  TextDecorator::BLUE},
		{"CYAN",  TextDecorator::CYAN},
		{"PURPLE", TextDecorator::PURPLE}
		//		{"", TextDecorator::}
};

template <>
const drain::FlaggerDict drain::EnumDict<TextDecorator::Line>::dict = {
		{"NO_LINE", TextDecorator::NO_LINE}, // deprecating
		{"UNDERLINE", TextDecorator::UNDERLINE},
		{"DOUBLE_UNDERLINE", TextDecorator::DOUBLE_UNDERLINE},
		{"OVERLINE", TextDecorator::OVERLINE},
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
	throw std::runtime_error(drain::StringBuilder(__FILE__, '/', __FUNCTION__, ": no such key: ", key));
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






template <>
const std::map<TextDecorator::Colour,int> & TextDecoratorVt100::getCodeMap(){

	static
	const std::map<TextDecorator::Colour,int> map = {
			{BLACK, 30},
			{RED, 31},
			{GREEN, 32},
			{YELLOW, 33},
			{BLUE, 34},
			{PURPLE, 35},
			{CYAN, 36},
			{WHITE, 37},
			{NO_COLOR, 39}
			/*
			{BLACK_BG, 40},
			{RED_BG, 41},
			{GREEN_BG, 42},
			{YELLOW_BG, 43},
			{BLUE_BG, 44},
			{MAGENTA_BG, 45},
			{CYAN_BG, 46},
			{WHITE_BG, 47},
			{DEFAULT_BG, 49}
			*/
	};

	return map;
}

template <>
const std::map<TextDecorator::Style,int> & TextDecoratorVt100::getCodeMap(){

	static
	const std::map<TextDecorator::Style,int> map = {
			{NO_STYLE, 0},
			{ITALIC, 3},
			{BOLD, 1},
			{DIM, 2},
			{REVERSE, 7}
	};

	return map;
}

// enum Style {NO_STYLE=0, ITALIC=1, BOLD=2, DIM=4, REVERSE=8}; // DEFAULT,
// enum Line {NO_LINE=0, UNDERLINE=1, DOUBLE_UNDERLINE=2, OVERLINE=4};

template <>
const std::map<TextDecorator::Line,int> & TextDecoratorVt100::getCodeMap(){

	static
	const std::map<TextDecorator::Line,int> map = {
			{NO_LINE, 0},
			{UNDERLINE, 4},
			{DOUBLE_UNDERLINE, 21}, // Double unnderline
			{OVERLINE, 9}
	};

	return map;
}


} // drain::
