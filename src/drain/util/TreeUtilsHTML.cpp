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
 * TreeHTML.cpp
 *
 *  Created on:2024
 *      Author: mpeura
 */

#include "TreeUtilsHTML.h"

namespace drain {

TreeHTML & TreeUtilsHTML::initHtml(drain::TreeHTML & html, const std::string & heading){

	html(drain::NodeHTML::HTML);

	drain::TreeHTML & head  = html[drain::NodeHTML::HEAD](drain::NodeHTML::HEAD);

	drain::TreeHTML & encoding = head["encoding"](drain::html::META);
	encoding->set("charset", "utf-8");

	// drain::TreeHTML & style =
	head[drain::html::STYLE](drain::html::STYLE);

	drain::TreeHTML & title = head[drain::html::TITLE](drain::html::TITLE);
	title = heading;

	drain::TreeHTML & body = html[drain::html::BODY](drain::html::BODY);

	if (!heading.empty()){
		drain::TreeHTML & h1 = body["title"](drain::html::H1);
		h1 = heading;
	}

	return body;
}

/*
drain::TreeHTML & TreeUtilsHTML::fillTableRow(drain::TreeHTML & table, drain::TreeHTML & tr, const std::string value){

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
*/
//int NodeXML::nextID = 0;


}  // drain::
