/*

MIT License

Copyright (c) 2023 FMI Open Development / Markus Peura, first.last@fmi.fi

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
 * TreeElemUtilsHTML.h
 *
 *  Created on: 2026
 *      Author: mpeura
 */

#ifndef DRAIN_TREE_ELEM_UTILS_HTML
#define DRAIN_TREE_ELEM_UTILS_HTML


#include <drain/util/XML.h>
#include <drain/util/TreeXML.h>

#include "TreeHTML.h"

//namespace drain {

#define DRAIN_HTML_ELEM_CLS(E) template <> template <> class drain::NodeXML<drain::Html::tag_t>::Elem<drain::Html::tag_t::E>
#define DRAIN_HTML_ELEM(E)     typedef drain::NodeXML<drain::Html::tag_t>::Elem<drain::Html::tag_t::E> html##E

//#define DRAIN_SVG_SUPER(E) class drain::NodeXML<drain::image::svg::tag_t>::Elem<drain::image::svg::tag_t::E>; typedef drain::NodeXML<drain::image::svg::tag_t>::Elem<drain::image::svg::tag_t::E> svg##E;


namespace drain {




DRAIN_HTML_ELEM_CLS(IMG){
// class NodeXML<image::svg::tag_t>::Elem<image::svg::tag_t::RECT>{
public:

	inline
	Elem(NodeHTML & node) : node(node = Html::tag_t::IMG), src(node["src"]=""), alt(node["alt"]=""), title(node["title"]=""){
	};

	NodeXML<Html::tag_t> & node;

	FlexibleVariable & src;
	FlexibleVariable & alt;
	FlexibleVariable & title;

};
DRAIN_HTML_ELEM(IMG);



}  // drain::





#endif // TREESVG_H_

