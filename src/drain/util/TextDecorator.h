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

protected:

	// input sep
	std::string separator;

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

	inline
	TextDecorator() : separator(","), color(NO_COLOR){
		reset(); // ??
	}

	inline
	TextDecorator(const TextDecorator & deco) :separator(deco.separator), color(deco.color), style(deco.style), line(deco.line){
		reset(); // ??
	}

	virtual inline
	~TextDecorator(){
		//reset(); ???
	}

	inline
	std::ostream & begin(std::ostream & ostr) const {
		return _begin(ostr);
	}

	inline
	std::ostream & begin(std::ostream & ostr, const std::string & conf){
		set(conf);
		return _begin(ostr);
	}


	inline
	std::ostream & end(std::ostream & ostr) const {
		return _end(ostr);
	}

	inline
	void setSeparator(const std::string & separator){
		this->separator = separator;
	}

	//std::list<>
	void reset(){
		style.reset();
		color.reset();
		line.reset();
	}

	template<typename T, typename ... TT>
	void set(T arg, const TT &... args) {
		//set(arg);
		reset();
		add(arg, args...);
	};

	/// Sets given keys
	/*
	template<typename ... TT>
	void set(const std::string & keys, const TT &... args){
		reset();
		addKeys(keys);
		add(args...);
	}
	*/

	/// Adds separated keys. Essentially a string arg handler.
	/**
	 *  Note: separate if's: accept DEFAULT and NONE - ?
	 */
	template<typename ... TT>
	inline
	void add(const std::string & keys, const TT &... args){
		// std::cout << "Handling1: " << keys << '\n';
		addKeys(keys); // string handler
		add(args...);
	}

	/// Second case of strings...
	template<typename ... TT>
	inline
	void add(const char *keys, const TT &... args){
		// std::cout << "Handling2: " << keys << '\n';
		addKeys(keys); // string handler
		add(args...);
	}

	/// Third case of strings...
	template<typename ... TT>
	inline
	void add(char *keys, const TT &... args){
		// std::cout << "Handling3: " << keys << '\n';
		addKeys(keys); // string handler
		add(args...);
	}

	/// Adds several keys
	template<typename T, typename ... TT>
	void add(T arg, const TT &... args) {
		// std::cout << "Adding:" << arg << ':' << typeid(arg).name() << '\n';
		add(arg);
		add(args...);
	};

	/// Change the current color setting.
	inline
	void add(Colour c){
		color.set(c);
	}

	/// Change the current line setting.
	inline
	void add(Line l){
		line.set(l);
	}

	/// Change the current current style setting.
	inline
	void add(Style s){
		style.add(s);
	}




	void debug(std::ostream & ostr) const;

	//std::string str;

protected:


	virtual inline
	std::ostream & _begin(std::ostream & ostr) const {
		return ostr;
	}

	virtual inline
	std::ostream & _end(std::ostream & ostr) const {
		return ostr;
	}

	inline
	void set(){}; // could be reset? No! This is the last one called..

	inline
	void add(){};

	void addKey(const std::string & key);

	void addKeys(const std::string & keys);

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






class TextDecoratorVt100 : public drain::TextDecorator {

public:

	typedef std::map<drain::TextDecorator::Colour,int> color_codemap_t;
	typedef std::map<drain::TextDecorator::Style,int>  style_codemap_t;
	typedef std::map<drain::TextDecorator::Line,int> line_codemap_t;

	static
	const color_codemap_t color_codemap;

	static
	const style_codemap_t style_codemap;

	static
	const line_codemap_t line_codemap;

	virtual inline
	~TextDecoratorVt100(){
	}

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
			throw std::runtime_error(drain::StringBuilder(__FILE__, '/', __FUNCTION__, ": no such enumCode: ", enumCode)); // TYPE!
			return 0; // drain::TextDecorator::Colour::NO_COLOR;
		}
	}

	/*
	static inline
	int getColourCode(drain::TextDecorator::Colour colour){ // drain::TextDecorator::Colour colour
		color_codemap_t::const_iterator it = color_codemap.find(colour);
		if (it != color_codemap.end()){
			return it->second;
		}
		else {
			throw std::runtime_error(drain::StringBuilder(__FILE__, '/', __FUNCTION__, ": no such colour: ", colour));
			return drain::TextDecorator::Colour::NO_COLOR;
		}
	}

	static inline
	int getStyleCode(drain::TextDecorator::Style style){ // drain::TextDecorator::Colour colour
		style_codemap_t::const_iterator it = style_codemap.find(style);
		if (it != style_codemap.end()){
			return it->second;
		}
		else {
			throw std::runtime_error(drain::StringBuilder(__FILE__, '/', __FUNCTION__, ": no such colour: ", style));
			return drain::TextDecorator::Style::NO_STYLE;
		}
	}

	static inline
	int getLineCode(drain::TextDecorator::Line line){ // drain::TextDecorator::Colour colour
		line_codemap_t::const_iterator it = line_codemap.find(line);
		if (it != line_codemap.end()){
			return it->second;
		}
		else {
			throw std::runtime_error(drain::StringBuilder(__FILE__, '/', __FUNCTION__, ": no such colour: ", line));
			return drain::TextDecorator::Line::NO_LINE;
		}
	}
	*/



	virtual inline
	std::ostream & _begin(std::ostream & ostr) const {

		std::list<int> codes;

		if (style)
			codes.push_back(getIntCode<Colour>(color.value));

		if (color)
			codes.push_back(getIntCode<Colour>(color.value));

		if (line)
			codes.push_back(getIntCode<Colour>(color.value));

		if (!codes.empty()){
			ostr << "\033[";
			ostr << drain::StringTools::join(codes,';'); // consider SprinterLayout(";");
			ostr << 'm'; //  << "\]";
		}

		return ostr;
	}

	virtual inline
	std::ostream & _end(std::ostream & ostr) const {
		ostr << "\033[0m";
		return ostr;
	}

};

template <>
const std::map<TextDecorator::Colour,int> & TextDecoratorVt100::getCodeMap();

} // ::drain

#endif
