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

		std::stringstream sstr;
		sstr << it->second;
		if (!sstr.str().empty()){
			ostr << it->first << "=\"" << it->second << '"' << ' ';
		}
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
