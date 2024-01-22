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
#ifndef TEXT_STYLE_H_
#define TEXT_STYLE_H_

#include <iostream>
// #include <sstream>
//#include <string>
#include <map>
#include <set>

#include "TypeUtils.h"

namespace drain
{

/// Utility for scanning text segments
/**
 *
 *
 */
class TextStyle {

public:

	enum Colour {NO_COLOR=0, BLACK, GRAY, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE};

	enum Style {NO_STYLE=0, ITALIC=1, BOLD=2, DIM=4, REVERSE=8}; // NO_STYLE not needed?

	enum Line {NO_LINE=0, UNDERLINE=1, DOUBLE_UNDERLINE=2, OVERLINE=4};

	Colour colour;

	std::set<Style> style;

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
		colour = Colour::NO_COLOR;
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




class TextStyleVT100 : public TextStyle {

public:

	template <typename ... TT>
	void write(std::ostream & ostr, const TT &... args) const{
		startWrite(ostr);
		append(ostr, args...);
		endWrite(ostr);
	};


	virtual
	void startWrite(std::ostream & ostr) const {
	}

	virtual
	void endWrite(std::ostream & ostr) const {
		ostr << "\033[0m"; // VT100 end marker
	}

	template <typename ... TT>
	void append(std::ostream & ostr, const TT &... args) const{
		_append(ostr, false, args...);
	};


	typedef std::map<drain::TextStyle::Colour,int> color_codemap_t;
	typedef std::map<drain::TextStyle::Style,int>  style_codemap_t;
	typedef std::map<drain::TextStyle::Line,int>   line_codemap_t;

	static
	const color_codemap_t color_codemap;

	static
	const style_codemap_t style_codemap;

	static
	const line_codemap_t line_codemap;

	template <class T>
	static
	const std::map<T,int> & getCodeMap();

	template <class T>
	static
	int getIntCode(const T & enumCode){

		typedef std::map<T,int> codemap_t;

		const codemap_t & map = getCodeMap<T>();

		typename codemap_t::const_iterator it = map.find(enumCode);
		if (it != map.end()){
			return it->second;
		}
		else {
			std::cerr << __FILE__ << '/' << __FUNCTION__ << ": no such enumCode: " << enumCode << std::endl;
			throw std::runtime_error("No such enumCode: "); // TYPE!
			return 0; // drain::TextDecorator::Colour::NO_COLOR;
		}
	}

protected:

	/**
	 *  Default implemenation for normal input elements.
	 *  For control elements, see following specializations.
	 */
	template <typename T, typename ... TT>
	// static
	void _append(std::ostream & ostr, bool init, const T & arg, const TT &... args) const{
		if (init){
			_appendControlSuffix(ostr);
		}
		ostr << arg;
		_append(ostr, false, args...);
	};

	template <typename ... TT>
	// static
	void _append(std::ostream & ostr, bool start, const Colour & colour, const TT &... args) const{
		_appendControlPrefix(ostr, start);
		ostr << getIntCode(colour);
		_append(ostr, true, args...);
	};

	template <typename ... TT>
	// static
	void _append(std::ostream & ostr, bool start, const Line & line, const TT &... args) const{
		_appendControlPrefix(ostr, start);
		ostr << getIntCode(line);
		_append(ostr, true, args...);
	};

	template <typename ... TT>
	// static
	void _append(std::ostream & ostr, bool start, const Style & style, const TT &... args) const{
		_appendControlPrefix(ostr, start);
		ostr << getIntCode(style);
		_append(ostr, true, args...);
	};

	inline
	void _append(std::ostream & ostr, bool init) const{
		if (init){
			_appendControlSuffix(ostr);
		}
	};

	virtual
	void _appendControlPrefix(std::ostream & ostr, bool start) const {
		if (!start){
			ostr << "\033["; // start VT100 def
		}
		else {
			ostr << ';'; // separator; continuing VT100 defs
		}
	}

	virtual
	void _appendControlSuffix(std::ostream & ostr) const {
		ostr << 'm'; // VT100 end marker
	}



};



template <>
const std::map<TextStyle::Colour,int> & TextStyleVT100::getCodeMap();

template <>
const std::map<TextStyle::Line,int> & TextStyleVT100::getCodeMap();

template <>
const std::map<TextStyle::Style,int> & TextStyleVT100::getCodeMap();

} // ::drain

#endif
