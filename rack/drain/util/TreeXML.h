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



#ifndef TREEXML_H_
#define TREEXML_H_

#include <ostream>
#include "ReferenceMap.h"
#include "Tree.h"

namespace drain {


class NodeXML : protected ReferenceMap {


public:

	NodeXML(){
		id = nextID;
		++nextID;
	};

	~NodeXML(){};

	//std::string id;  // int?
	//std::string name;

	inline
	const std::string & getTag() const {return tag;};

	template <class T>
	inline
	void set(const std::string & key, const T &value){
		(*this)[key] = value;
	}

	//std::ostream & toOstr(std::ostream &ostr);

	template <class N> //, class C>
	static
	std::ostream & toOStr(std::ostream &ostr, const drain::Tree<N> & t, const std::string & name = "");

	std::string ctext;

protected:

	static int nextID;
	std::string tag;
	int id;

};



#define TreeXML drain::Tree<NodeXML>  // , std::less<std::basic_std::string<char>

template <class N> //, class C>
std::ostream & NodeXML::toOStr(std::ostream &ostr, const drain::Tree<N> & tree, const std::string & tag){
	const std::map<std::string, Tree<N> > & children = tree.getChildren();

	// OPEN TAG
	ostr << '<';
	//<< tree.data.tag << ' ';
	if (tree->getTag().empty())
		ostr << tag << ' ';
	else {
		ostr << tree->getTag() << ' ';
		// TODO if (tree.data.name.empty())
		ostr << "name=\"" << tag << '"' << ' ';
	}
	//ostr << "name='";
	ostr << "id=\"" << tree.data.id << '"' << ' ';

	/// iterate attributes
	for (ReferenceMap::const_iterator it = tree->begin(); it != tree->end(); it++){
		ostr << it->first << "=\"" << it->second << '"' << ' ';
	}

	if ((children.size() == 0) && tree->ctext.empty() ){ // OR no ctext!
		// close TAG
		ostr << '/' << '>';
		ostr << '\n';
	}
	else {
		// close starting TAG
		ostr << '>';
		if (tree->ctext.empty())
			ostr << '\n'; // TODO nextline
		else
			ostr << tree->ctext;

		/// iterate children
		for (typename std::map<std::string, Tree<N> >::const_iterator it = children.begin(); it != children.end(); it++){
			toOStr(ostr, it->second, it->first);
			//ostr << *it;
		}
		// add end </TAG>
		ostr << '<' << '/' << tree->getTag() << '>';
		ostr << '\n';  // TODO nextline
		ostr << "<!-- " << tree.data.id << " /-->\n";
	}
	return ostr;
}


inline
std::ostream & operator<<(std::ostream &ostr, const TreeXML & t){
	  return NodeXML::toOStr(ostr, t, "");
}


}  // namespace drain

#endif /* TREEXML_H_ */

// Rack
