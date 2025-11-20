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


//#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string.h> // plain C strlen

#include <map>
#include <list>
#include <vector>
#include <set>

#include "drain/StringBuilder.h"

#ifndef DRAIN_SERIALIZER
#define DRAIN_SERIALIZER

namespace drain {

// consider formatter from T-inherited class: template <class F>
struct Serializer { // : public Formatter {

public:


	inline
	Serializer(){};

	virtual	inline
	~Serializer(){};

	static const char SINGLE_QUOTE; // = '\'';
	static const char DOUBLE_QUOTE; // = '"';

	typedef std::map<char, const char *> conv_map_t;

	static
	const conv_map_t conversion;


	// Object rendering
	typedef int objType;
	static const objType NONE=0;
	static const objType KEY=1;
	static const objType VALUE=2;
	static const objType PAIR=KEY|VALUE;
	static const objType LIST=4;
	static const objType SET=8;
	static const objType MAP=16;
	static const objType MAP_KEY=MAP|KEY;
	static const objType MAP_VALUE=MAP|VALUE;
	static const objType MAP_PAIR=MAP|PAIR;
	static const objType TUPLE=64;
	static const objType STRING=128;

	// Object rendering
	// Object rendering
	// Object rendering

	// Virtual
	virtual inline
	void writePrefix(std::ostream & ostr, objType type) const {};

	virtual inline
	void writeSeparator(std::ostream & ostr, objType type) const {
		if (type!=STRING)
			ostr << ',';
	};

	virtual inline
	void writeSuffix(std::ostream & ostr, objType type) const {};

	/// Default handler
	template <class T>
	void toStream(std::ostream & ostr, const T & x, objType hint=NONE) const ;

	// Sequences are partially templated

	template <class T>
	void toStream(std::ostream & ostr, const std::list<T> & x) const {
		this->iterableToStream(ostr, x, LIST);
	}

	template <class T>
	void toStream(std::ostream & ostr, const std::set<T> & x) const {
		this->iterableToStream(ostr, x, SET);
	}

	template <class K,class V>
	void toStream(std::ostream & ostr, const std::map<K,V> & x) const {
		this->iterableToStream(ostr, x, MAP);
	}

	template <class K,class V>
	void toStream(std::ostream & ostr, const std::pair<K,V> & x) const {
		this->pairToStream(ostr, x);
	}

	/// This is a map entry.
	template <class K,class V>
	void entryToStream(std::ostream & ostr, const std::pair<K,V> & x) const {
		this->pairToStream(ostr, x, MAP_PAIR);
	}

	template <class T>
	void entryToStream(std::ostream & ostr, const T & x) const {
		this->toStream(ostr, x);
	}


	// Consider indent level(s)
	template <class T>
	void iterableToStream(std::ostream & ostr, const T & x, objType iterableType = NONE) const {
		writePrefix(ostr, iterableType);
		objType separatorType = NONE;
		for (const auto & entry: x){
			this->writeSeparator(ostr, separatorType);
			//this->toStream(ostr, entry); // may be a pair.
			this->entryToStream(ostr, entry); // may be a pair.
			separatorType = iterableType;
		}
		writeSuffix(ostr, iterableType);
	}

	template <class K,class V>
	void pairToStream(std::ostream & ostr, const std::pair<K,V> & x, objType containerType= NONE) const {
		// DEBUG ostr << "@" << containerType << '|';
		this->writePrefix(ostr, PAIR|containerType);
		{
			this->writePrefix(ostr, KEY|containerType); // PAIR_KEY?
			this->toStream(ostr, x.first);
			this->writeSuffix(ostr, KEY|containerType); // PAIR_KEY?
		}
		this->writeSeparator(ostr, PAIR|containerType);
		{
			this->writePrefix(ostr, VALUE|containerType); // PAIR_VALUE?
			this->toStream(ostr, x.second);
			this->writeSuffix(ostr, VALUE|containerType); // PAIR_VALUE?
		}
		this->writeSuffix(ostr, PAIR|containerType);
	}


	inline
	void toStream(std::ostream & ostr, char c) const {
		this->charToStream(ostr, c);
	}

	inline
	void toStream(std::ostream & ostr, const char *x) const {
		this->stringToStream(ostr, x);
	}

	inline
	void toStream(std::ostream & ostr, const std::string &x) const {
		this->stringToStream(ostr, x);
	}

	inline
	void toStream(std::ostream & ostr, bool x) const {
		this->boolToStream(ostr, x);
	}

	inline
	void toStream(std::ostream & ostr, const nullptr_t & t) const {
		this->nullToStream(ostr);
	}

	/// Default handler
	template <class T>
	inline
	void toStr(std::string & str, const T & x) const {
		std::stringstream sstr;
		toStream(sstr, x);
		str = sstr.str();
	}

	// Type specific handlers

	void floatToStream(std::ostream & ostr, double x) const {
		ostr << x;
	}

	void intToStream(std::ostream & ostr, int x) const {
		ostr << x;
	}

	virtual
	void boolToStream(std::ostream & ostr, bool x) const {
		ostr << std::ios_base::boolalpha;
		ostr << x;
	}

	virtual inline
	void nullToStream(std::ostream & ostr) const  {
		ostr << "null";
	}

	virtual inline
	void charToStream(std::ostream & ostr, char c) const {
		ostr << c;
	}

	virtual inline
	void stringToStream(std::ostream & ostr, const char *x) const {
		ostr << x;
	}

	virtual inline
	void stringToStream(std::ostream & ostr, const std::string & x) const final {
		stringToStream(ostr, x.c_str());
	}


};

template <class T>
void Serializer::toStream(std::ostream & ostr, const T & x, objType hint) const {

	if (std::is_floating_point<T>()){
		this->floatToStream(ostr, x);
	}
	else if (std::is_integral<T>()){
		this->intToStream(ostr, x);
	}
	else {
		ostr << x;
	}
}

struct SimpleFormatter {

	char prefix    = 0;
	char separator = 0;
	char suffix    = 0;
	//char quote     = 0;

	inline
	SimpleFormatter(const char *conf=""){

		switch (strlen(conf)){
		case 3:
			prefix = conf[0];
			separator = conf[1];
			suffix = conf[2];
			break;
		case 2:
			prefix = conf[0];
			suffix = conf[1];
			break;
		case 1:
			separator = conf[0];
			break;
		case 0:
			break;
		default:
			throw std::runtime_error(StringBuilder<':'>(__FILE__, "unsupported conf", conf));
		}

	}

};

class SimpleSerializer : public Serializer {
public:

	typedef std::map<objType,SimpleFormatter> char_map_t;

	inline
	SimpleSerializer(){};

	inline
	SimpleSerializer(const char_map_t & chars) : fmtChars(chars){};

	virtual inline
	~SimpleSerializer(){};



	virtual inline
	void writePrefix(std::ostream & ostr, objType type) const override {
		writePrefix(ostr, getFormatter(type));
	}

	inline
	void writePrefix(std::ostream & ostr, const SimpleFormatter & fmt) const {
		if (fmt.prefix){
			ostr << fmt.prefix;
		}
	};

	virtual
	void writeSeparator(std::ostream & ostr, objType type) const override {
		writeSeparator(ostr, getFormatter(type));
	}

	inline
	void writeSeparator(std::ostream & ostr, const SimpleFormatter & fmt) const {
		if (fmt.separator){
			ostr << fmt.separator;
		}
	};

	virtual
	void writeSuffix(std::ostream & ostr, objType type) const override {
		writeSuffix(ostr, getFormatter(type));
	}

	inline
	void writeSuffix(std::ostream & ostr, const SimpleFormatter & fmt) const {
		if (fmt.suffix){
			ostr << fmt.suffix;
		}
	};


	virtual
	void stringToStream(std::ostream & ostr, const char * x)  const override {
		charsToStream(ostr, x);
	}


	/** Default implementation works like with std:string
	 *
	 *  Derived classes:
	 *  - C/Cpp  Serializer: single hyphens for char='c', doubles for string="str".
	 *  - Python Serializer: supports toggling single and double
	 */
	virtual inline
	void charToStream(std::ostream & ostr, char c) const override {
		charsToStream(ostr, c);
	}



protected:

	char_map_t fmtChars = {
			{LIST, "[,]"},
			{PAIR, "<,>"},
			{SET, "{,}"},
			{MAP,"{,}"},
			{STRING,"''"},
	};

	virtual
	const SimpleFormatter & getFormatter(objType t) const;


	template <class S>
	void charsToStream(std::ostream & ostr, const S &s)  const;

	virtual
	void handleChars(std::ostream & ostr, char c) const;

	virtual
	void handleChars(std::ostream & ostr, const char *c) const;



};

template <class S>
void SimpleSerializer::charsToStream(std::ostream & ostr, const S &s)  const {

	const SimpleFormatter & fmt = getFormatter(Serializer::STRING);
	writePrefix(ostr, fmt);
	handleChars(ostr, s);
	writeSuffix(ostr, fmt);

}


}

#endif


