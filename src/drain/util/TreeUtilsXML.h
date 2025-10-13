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
 *  Created on: 2025/10
 *      Author: mpeura
 */



#ifndef DRAIN_TREE_UTILS_XML
#define DRAIN_TREE_UTILS_XML

#include "TreeUtils.h"
#include "TreeXML.h"


namespace drain {


template <class N>
class ElemPrinter {

public:

	inline
	ElemPrinter(const N & node){
		std::stringstream sstr;
		sstr << "<" << node.getTag();
		if (::atoi(node.getId().c_str())==0){
			sstr << " id=" << node.getId();
		}
		if (node.getName().isValid()){
			sstr << " name=" << node.getName();
		}
		if (!node.getClasses().empty()){
			sstr << " class=[" << node.getClasses() << ']';
		}
		sstr << '>';
		id = sstr.str();
	}

	const std::string & str() const {
		return id;
	}


protected:

	std::string id;

};

enum XmlEmptiness {
	CHILDREN = 1,
	TEXT = 2, // consider whitespace remover?
	ATTRIBUTES = 4,
	//DATA = 4,
	ALL = CHILDREN|TEXT|ATTRIBUTES,
	// require any or all?
	ANY = 128,
};

template <class T>
class TreePruner : public drain::TreeVisitor<T> {

public:


	typedef std::map<typename T::node_data_t::xml_tag_t, short unsigned int> tag_selector_t;

	tag_selector_t tagSelector;
	TreePruner(){
	}

	inline
	int visitPrefix(T & tree, const typename T::path_t & path) override {
		return 0;
	}

	int visitPostfix(T & tree, const typename  T::path_t & path) override;


};


template <class T>
int TreePruner<T>::visitPostfix(T & tree, const typename  T::path_t & path){
	drain::Logger mout(__FILE__, __FUNCTION__);

		T & current = tree(path);

		std::list<typename T::path_elem_t> empties;

		for (const auto & entry: current.getChildren()){

			drain::EnumFlagger<drain::MultiFlagger<XmlEmptiness> > flagger;

			const T & child = entry.second;

			if (child.getChildren().empty())
				flagger.add(CHILDREN);

			if (child.data.getAttributes().empty())
				flagger.add(ATTRIBUTES);

			if (child.data.getText().empty())
				flagger.add(TEXT);

			typename tag_selector_t::const_iterator it = tagSelector.find(child->getNativeType());
			if (it != tagSelector.end()){
				// mout.pending<LOG_WARNING>("found : ", child->getType(), " -> " , ElemPrinter<typename T::node_data_t>(child).str());
				// check rule
				if (flagger.isSet(it->second)){
					empties.push_back(entry.first);
				}
			}

			/*
			switch (entry.second->getType()){
			case svg::TEXT:
				if (entry.second.getChildren().empty() && entry.second->getText().empty()){
					empties.push_back(entry.first);
					// current.erase();
				}
				break;
			case svg::TSPAN:
				if (entry.second->getText().empty()){
					empties.push_back(entry.first);
				}
				break;
			default:
				break;
			}
			*/
		}

		for (const typename T::path_elem_t & elem: empties){
			// mout.reject<LOG_WARNING>("erasing: ", elem, " -> " , ElemPrinter<typename T::node_data_t>(current[elem]).str());
			current.erase(elem);
		}

		return 0;

}



}  // drain::

DRAIN_ENUM_DICT(drain::XmlEmptiness);

#endif /* TREEXML_H_ */

