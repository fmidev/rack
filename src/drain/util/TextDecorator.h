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
#ifndef TEXT_DECORATOR_H_
#define TEXT_DECORATOR_H_

#include <iostream>
#include <sstream>
#include <list>
#include <string>

#include "Flags.h"

namespace drain
{

/// Utility for scanning text segments
/**
 *
 *
 */
class TextDecorator {

public:

	//enum Colour {NONE=0, DEFAULT=1, BLACK=2, GRAY=4, RED=8, GREEN=16, YELLOW=32, BLUE=64, PURPLE=128, CYAN=256, WHITE=512};
	enum Colour {NO_COLOR=0, BLACK, GRAY, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE};

	enum Style {NO_STYLE=0, ITALIC=1, BOLD=2, DIM=4, REVERSE=8}; // DEFAULT,

	enum Line {NO_LINE=0, UNDERLINE=1, DOUBLE_UNDERLINE=2, OVERLINE=4};

	typedef drain::EnumFlagger<drain::SingleFlagger<Colour> > ColourFlag;

	typedef drain::EnumFlagger<drain::MultiFlagger<Style> > StyleFlags;

	typedef drain::EnumFlagger<drain::SingleFlagger<Line> > LineFlag;

	/// Read input stream until any char in \c endChars is encountered. The end char will not be included, but passed in input stream.
	/**
	 *
	 */

	ColourFlag color;
	StyleFlags style;
	LineFlag    line;
	//std::map<std::type_info,size_t> state;

	TextDecorator(){
		reset();
	}

	//std::list<>
	void reset(){
		style.reset();
		color.reset();
		line.reset();
	}

	template<typename T, typename ... TT>
	void set(T arg, const TT &... args) {
		//style.set()
		//state[typeid(T)] = arg;
		//std::cout << arg << '\n';
		add(arg);
		set(args...);
	};


	void set(const std::string & key){
		reset();

	}

	/**
	 *  Note: Uses add() and set()
	 *  Note: separate if's: accept DEFAULT and NONE - ?
	 */
	void add(const std::string & key){

		std::list<std::string> l;
		drain::StringTools::split(key, l, ",");
		for (const std::string & k: l){
			std::cout << "  +" << k << '\n';
			addKey(k);
		}


	}

	void add(Colour c){
		color.set(c);
	}

	void add(Line l){
		line.set(l);
	}

	void add(Style s){
		style.add(s);
	}


	void set(){
		//str = criterion.str() + separator + operation.str();
	}


	void debug(std::ostream & ostr) const ;

	//std::string str;

protected:

	void addKey(const std::string & key);


};

inline
std::ostream & operator<<(std::ostream & ostr, const TextDecorator & decorator){
	//decorator.debug(ostr);
	ostr << decorator.style << '=' << decorator.style.getValue();
	ostr << ':';
	ostr << decorator.color << '=' << decorator.color.getValue();
	ostr << ':';
	ostr << decorator.line  << '=' << decorator.line.getValue();
	return ostr;
}

} // ::drain

#endif
