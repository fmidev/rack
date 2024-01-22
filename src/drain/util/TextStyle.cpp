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

#include "TextStyle.h"

namespace drain
{

template <>
const std::map<TextStyle::Colour,int> & TextStyleVT100::getCodeMap(){

	static
	const std::map<TextStyle::Colour,int> map = {
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
const std::map<TextStyle::Style,int> & TextStyleVT100::getCodeMap(){

	static
	const std::map<TextStyle::Style,int> map = {
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
const std::map<TextStyle::Line,int> & TextStyleVT100::getCodeMap(){

	static
	const std::map<TextStyle::Line,int> map = {
			{NO_LINE, 0},
			{UNDERLINE, 4},
			{DOUBLE_UNDERLINE, 21}, // Double underline
			{OVERLINE, 9}
	};

	return map;
}

/*
template <>
struct TypeName<TextStyle::Style> {
    static const char* get(){ return "TextStyle::Style"; }
};
*/



} // drain::
