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
#ifndef DRAIN_TEXT_STYLE
#define DRAIN_TEXT_STYLE

#include <iostream>
#include <map>
#include <set>

#include "Type.h"
//#include "Sprinter.h"
//#include "StringBuilder.h"

namespace drain
{

/// Utility for scanning text segments
/**
 *
 *
 */
class TextStyle {

public:

	enum Colour {DEFAULT_COLOR=0, BLACK, GRAY, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE};

	enum Style {NO_STYLE=0, ITALIC=1, BOLD=2, DIM=4, REVERSE=8}; // NO_STYLE not needed?

	enum Line {NO_LINE=0, UNDERLINE=1, DOUBLE_UNDERLINE=2, OVERLINE=4};


	// Could be: EnumFlagger<SingleFlagger> ? -> see TextDecorator
	Colour colour;

	// Could be: EnumFlagger<MultiFlagger>  -> see TextDecorator
	std::set<Style> style;

	// Could be: EnumFlagger<SingleFlagger> ? -> see TextDecorator
	Line line;

	template <typename ... T>
	inline
	void set(const T &... args) {
		reset();
		//this->value = 0;
		add(args...);
	}

	template <typename T, typename ... TT>
	inline
	void add(const T & arg, const TT &... args) {
		_add(arg);
		add(args...);
	}

protected:

	inline
	void reset(){
		colour = Colour::DEFAULT_COLOR;
		line  = Line::NO_LINE;
		style.clear();
	};


	inline
	void set(){};

	inline
	void add(){};

	inline
	void _add(const Colour & c){
		colour = c;
	};

	inline
	void _add(const Line & l){
		line = l;
	};

	inline
	void _add(const Style & s){
		style.insert(s);
	};

};

DRAIN_TYPENAME(TextStyle);

DRAIN_TYPENAME(TextStyle::Colour);
DRAIN_TYPENAME(TextStyle::Line);
DRAIN_TYPENAME(TextStyle::Style);

inline
std::ostream & operator<<(std::ostream & ostr, const TextStyle &t){
	ostr << __FILE__ << ':' << __LINE__ << '?';
	return ostr;
}


} // ::drain

#endif
