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



#ifndef DRAIN_TREE_UTILS_HTML
#define DRAIN_TREE_UTILS_HTML

// #include <ostream>

#include "TreeHTML.h"

namespace drain {



class TreeUtilsHTML {

public:

	/// Initialize a HTML object with "head" (including "title", "style") and "body" elements.
	/**
	 *  If a title is given, it will be also assigned as H1 element.
	 */
	static
	drain::TreeHTML & initHtml(drain::TreeHTML & html, const std::string & key = "");



	static inline
	drain::TreeHTML & getFirstElem(drain::TreeHTML & elem, drain::html::tag_t tagType){
		if (elem.hasChildren()){
			return elem.getChildren().begin()->second; // last
		}
		else {
			//
			return elem.addChild()(tagType); // addChild(elem, tagType);
		}
	}



	/// Add element of given type. The path key is generated automatically, unless given.
	/**
	 *
	 *
	 */
	// static  // compare with TreeHTML::addChild( - is needed?
	//drain::TreeHTML & addChild(drain::TreeHTML & elem, drain::html::tag_t tagType, const std::string & key);

	template <class T>
	static inline
	drain::TreeHTML & appendElem(drain::TreeHTML & elem, drain::html::tag_t tagType, const T & arg){
		drain::TreeHTML & child = elem.addChild()(tagType); //  addChild(elem,tagType);
		child = arg;
		return child;
	};


	template <class T, class ...TT>
	static inline
	drain::TreeHTML & appendElem(drain::TreeHTML & elem, drain::html::tag_t tagType, const T & arg, const TT & ...args) {
		appendElem(elem, tagType, arg);
		return appendElem(elem, tagType, args...);
	}

	static inline
	drain::TreeHTML & createTable(drain::TreeHTML & body, const std::list<std::string> & columnTitles){ // "var", "ref", no whitespace.

		drain::TreeHTML & table =  body.addChild()(drain::NodeHTML::TABLE);   //       drain::TreeUtilsHTML::addChild(body, drain::NodeHTML::TABLE);

		drain::TreeHTML & tr    = table["header"](drain::NodeHTML::TR);

		for (const auto & title: columnTitles){
			drain::TreeHTML & th = tr[title](drain::NodeHTML::TH);
			th = title;
		}

		return table;
	}

	/// Creates a new table row (TD) using first row as a template.
	/**
	 *   \param T - HTML object, string or TAG type.
	 */
	template <class T>
	static
	// drain::TreeHTML & fillTableRow(drain::TreeHTML & table, drain::TreeHTML & tr, const std::string value = "");
	drain::TreeHTML & fillTableRow(drain::TreeHTML & table, drain::TreeHTML & tr, const T & value){

		for (const auto & entry: table.getChildren()){
			// Using keys of the first row, create a new row. Often, it is the title row (TH elements).
			for (const auto & e: entry.second.getChildren()){
				tr[e.first]->setType(drain::NodeHTML::TD);
				tr[e.first] = value;
			}
			// Return after investigating the first row:
			return tr;
		}

		// If table is empty, also tr is.
		return tr;

	}

	static
	drain::TreeHTML & addTableRow(drain::TreeHTML & table, const std::string value = ""){
		drain::TreeHTML & tr = table.addChild()(html::TR); // addChild(table, HTML::TR);
		return fillTableRow(table, tr, value);
	}


protected:

	// Dummy end... TODO: redesign logic, perhaps addChild();
	template <class T>
	static inline
	drain::TreeHTML & appendElem(drain::TreeHTML & elem, drain::html::tag_t tagType){
		if (elem.hasChildren()){
			return elem.getChildren().rbegin()->second; // last
		}
		else {
			//
			return elem;
		}
	}

};

}  // drain::

#endif /* TREE_UTILS_XML_H_ */

