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

#include <drain/TextStyle.h>
#include <drain/TypeUtils.h>

#include "Flags.h"

namespace drain
{


// Compare this with TextStyle. Join or separate?

/// Utility for ...
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

	typedef drain::EnumFlagger<drain::SingleFlagger<TextStyle::Colour> > ColourFlag;

	typedef drain::EnumFlagger<drain::MultiFlagger<TextStyle::Style> > StyleFlags;

	typedef drain::EnumFlagger<drain::SingleFlagger<TextStyle::Line> > LineFlag;


	/// Read input stream until any char in \c endChars is encountered. The end char will not be included, but passed in input stream.
	/**
	 *
	 */

	ColourFlag color;
	StyleFlags style;
	LineFlag    line;
	//std::map<std::type_info,size_t> state;

	inline
	TextDecorator() : separator(","), color(TextStyle::DEFAULT_COLOR){
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
	void add(TextStyle::Colour c){
		color.set(c);
	}

	/// Change the current line setting.
	inline
	void add(TextStyle::Line l){
		line.set(l);
	}

	/// Change the current current style setting.
	inline
	void add(TextStyle::Style s){
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

	virtual inline
	~TextDecoratorVt100(){
	}

	/// Internal utility: convert given abstract style to numeric VT100 codes.
	/**
	 *  \tparam - ColourFlag, StyleFlags, or LineFlag
	 *  \param  - styleFlags,
	 *  \param  - codes
	 */
	template <typename F>
	static
	void appendCodes(const EnumFlagger<F> & styleFlags, std::list<int> & codes);


protected:

	virtual
	std::ostream & _begin(std::ostream & ostr) const;

	virtual
	std::ostream & _end(std::ostream & ostr) const;


};

template <typename F>
void TextDecoratorVt100::appendCodes(const EnumFlagger<F> & styleFlags, std::list<int> & codes){

	if (styleFlags){ // is non-zero

		std::list<typename EnumFlagger<F>::value_t> l;

		FlagResolver::valuesToList(styleFlags.value, styleFlags.getDict(), l);

		for (typename EnumFlagger<F>::value_t v: l){
			codes.push_back(TextStyleVT100::getIntCode(v));
		}

	}

}


} // ::drain

#endif
