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
/*
 * TreeXML.h
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */



#ifndef DRAIN_SELECTOR_XML
#define DRAIN_SELECTOR_XML

//#include <ostream>
#include <drain/StringBuilder.h>

namespace drain {


/// Currently used only as CSS element selector.
class SelectorXML : public std::string {

public:

	static
	const char CLASS = '.';

	static
	const char ID = '#';


	inline
	SelectorXML(const std::string &s) : std::string(s){
	}

	inline
	SelectorXML(const char *s) : std::string(s){
	}

	template <class ...T>
	inline
	SelectorXML(T... args) : std::string(StringBuilder<>(args...)){
	}

};

// CSS class selector.
/**
 *
 */
class SelectorXMLcls : public SelectorXML {
public:

	template <class C>
	inline
	SelectorXMLcls(const C &cls) : SelectorXML(CLASS, cls){
	}

	template <class E, class C>
	inline
	SelectorXMLcls(const E &elem, const C &cls) : SelectorXML(elem, CLASS, cls){
	}

};

class SelectorXMLid : public SelectorXML {
public:

	template <class T>
	inline
	SelectorXMLid(const T & arg) : SelectorXML(ID, arg){
	}

};


/*
inline
std::ostream & operator<<(std::ostream &ostr, const TreeXML & t){
	// DOC def? TODO: preamble/prologToStream()
	TreeXML::node_data_t::docTypeToStream(ostr); // must be member, to support virtual?
	TreeXML::node_data_t::toStream(ostr, t, "");
	return ostr;
}
*/


}  // drain::

#endif /* TREEXML_H_ */

