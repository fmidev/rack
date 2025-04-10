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
#ifndef DRAIN_TEXT_STYLE_VT100
#define DRAIN_TEXT_STYLE_VT100

#include <iostream>
#include <map>
#include <set>

#include "StringBuilder.h"
#include "TextStyle.h"

namespace drain
{


class TextStyleVT100 : public TextStyle {

public:

	template <typename ... T>
	inline
	TextStyleVT100(const T &... args){
		//set(args...);
	};

	inline virtual
	~TextStyleVT100(){};

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


	/// Given an enum value, returns the corresponding numeric VT100 code.
	template <class E>
	static
	int getIntCode(const E & enumCode){

		typedef std::map<E,int> codemap_t;

		const codemap_t & map = getCodeMap<E>();

		typename codemap_t::const_iterator it = map.find(enumCode);
		if (it != map.end()){
			return it->second;
		}
		else {

			for (const auto & entry: map){
				std::cerr << entry.first << '=' << (int)entry.first << " VT100=" << entry.second << std::endl;
			}


			std::cerr << StringBuilder<>(TypeName<E>::str(), ": no such enumCode: ", enumCode) << std::endl;

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

inline
std::ostream & operator<<(std::ostream & ostr, const TextStyleVT100 &t){
	ostr << __FILE__ << ':' << __LINE__ << '?';
	return ostr;
}

DRAIN_TYPENAME(TextStyleVT100);


} // ::drain

#endif
