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


//#include "PythonSerializer.h"

#ifndef DRAIN_PYTHON_UTILS
#define DRAIN_PYTHON_UTILS

#include <list>
//#include <drain/prog/Command.h>
#include <drain/prog/CommandBank.h>

namespace drain {




class PythonConverter {

public:

	static
	const std::string TRIPLE_HYPHEN;

	// Whitespace
	std::string indentStr = std::string(4, ' ');

	static
	const std::string & getType(const drain::Castable & c);

	// std::string content = "pass";
	std::list<std::string> content = {"return self._make_cmd(locals())"};

	mutable
	int counter = 100;

	template <class T>
	void exportImports(const std::initializer_list<T> & container, std::ostream & ostr=std::cout) const;

	void exportCommands(const std::string &name, const drain::CommandBank & commandBank, std::ostream & ostr=std::cout, int indentLevel=0) const;

	void exportCommand(const std::string & name, const drain::Command & command, std::ostream & ostr=std::cout, int indentLevel=1, bool implicit=false) const;


	/** Writes a doc title - a string underlined with a hyphen.
	inline
	void indent(std::ostream & ostr, int indentLevel=0) const {
		for (int i=0; i<indentLevel; ++i){
			ostr << this->indentStr;
		}
	}

	template <typename ... TT>
	void write(std::ostream & ostr, int indentLevel, TT... args) const {
		indent(ostr, indentLevel);
		flush(ostr, args...);
	};

	void writeTitle(std::ostream & ostr, int indentLevel, const std::string & title) const;


protected:

	template <typename T, typename ... TT>
	void flush(std::ostream & ostr, const T & arg, TT... args) const {
		ostr << arg;
		flush(ostr, args...);
	};

	inline
	void flush(std::ostream & ostr) const {
		ostr << '\n';
	}

	 *
	 */

};

template <class T>
void PythonConverter::exportImports(const std::initializer_list<T> & container, std::ostream & ostr) const {
	for (const auto & entry: container){
		ostr << "import " << entry << '\n';
	}
}

class PythonIndent {

public:

	std::ostream & ostr = std::cout;
	int indentLevel = 0;
	std::string indentStr = "    ";

	PythonIndent(std::ostream & ostr=std::cout, int indentLevel=0, std::string indentStr="    ") : ostr(ostr), indentLevel(indentLevel), indentStr(indentStr) {
	};

	inline
	void setIndent(int indentLevel){
		if (indentLevel < 0)
			indentLevel = 0;
		this->indentLevel = indentLevel;
	}

	inline
	void operator ++(){
		++this->indentLevel;
	}

	inline
	void operator --(){
		--this->indentLevel;
		if (this->indentLevel < 0)
			this->indentLevel = 0;
	}

	inline
	void indent() const {
		for (int i=0; i<this->indentLevel; ++i){
			ostr << this->indentStr;
		}
	}

	template <typename ... TT>
	void write(TT... args) const {
		this->indent();
		this->flush(args...);
	};

	/** Writes a doc title - a string underlined with a hyphen.
	 *
	 */
	void writeTitle(const std::string & title) const;


protected:

	template <typename T, typename ... TT>
	void flush(const T & arg, TT... args) const {
		this->ostr << arg;
		this->flush(args...);
	};

	inline
	void flush() const {
		this->ostr << '\n';
	}

};


}

#endif
