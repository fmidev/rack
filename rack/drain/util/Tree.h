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
*//**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 * Tree.h
 *
 *  Created on: Nov 25, 2010
 *      Author: mpeura
 */

#ifndef TREE2_H_
#define TREE2_H_

#include <iterator>
#include <string>
#include <map>

#include "String.h"
#include "RegExp.h"
//#include "Options.h"

namespace drain {

// // // using namespace std;

//temp late <class T> class Tree;

/// Unordered tree.
/**
 *  Child nodes can be addressed with operator[] with std::string-valued keys:
 *  \code
 *  Tree<float> t;
 *  t["firstChild"];
 *  \endcode
 *  The keys can be retrieved to a list with getKeys().
 *
 *  Alternatively, one may iterate the children directly:
 *  \code
 *  std::map<std::string,Tree<int> >::iterator it = tree.begin();
 *  while (it != tree.end()){
 *    std::cout << it->first << ' ' << it->second << '\n';
 *    ++it;
 *  }
 *  \endcode
 *
 *  As the operator[] returns a reference, it may be used recursively:
 *  \code
 *  t["firstChild"]["a"]["bb"];
 *  \endcode
 *
 *  Tree also recognizes a path separator, which is '/' by default.
 *  Hence, the node created in the above example could be likewise
 *  created or addressed as:
 *  \code
 *  t["firstChild/a/bb"];
 *  \endcode
 *  This behaviour will \b change in the future; paths with '/' will be addressed with tree::find() .
 *
 *  Each node contains data of type T. It may be unused (empty), that is, a node can be used only as a branching point.
 *
 *  \code
 *  Tree<std::string> t;
 *  t.data = "Hello world";
 *  std::string s = t;  // Now contains "Hello world".
 *  \endcode
 *  (Direct assignments to and from the node data has been supported, but is now \b disabled for c++
 *  compatibility reasons.)
 *
 *  Also direct referencing of the node data is now \b disabled:
 *  \code
 *  Tree<std::string> t;
 *  t = "Hello world";
 *  Tree<std::string> &r = t; // Refers to tree.
 *  std::string &s = t;       // Refers to data, "Hello world".
 *  \endcode
 *
 */
template <class T, class C=std::less<std::string> >
class Tree {
public:

	typedef T node_type;
	typedef std::map<std::string,Tree<T,C>, C> map_type;

	/// Default constructor.
	Tree(char separator = '/') :  separator(separator) {}; //parent(*this),

	/// Copy constructor
	Tree(const Tree &t) : data(t.data), separator(t.separator) {}; //parent(*this),

	/// Copies the data of another node. Does traverse the children.
	// TODO: what to do with the separator?
	inline
	Tree &operator=(const Tree &t){
		data = t.data;
		//separator = t.separator;
		return *this;
	};

	/// Contents (data) of the node.
	T data;

	/// New
	inline
	const T *operator->() const {
		return &data;
	};

	/// New
	inline
	T *operator->(){
		return &data;
	};

	/// Assigns value to contents.
	// Needed? See next.
	inline
	Tree &operator=(const T &v){
		data = v;
		return *this;
	};

	/// Assigns a value to contents.
	template <class T2>
	inline
	Tree &operator=(const T2 &v){
		data = v;
		return *this;
	}


	/// Returns a copy of the data of a node.
	/// inline operator T() const { return value;};

	/// Returns the reference to the contents of a node.
	/// inline operator T &(){ return value; };

	/// Returns the reference to the contents of a node.
	/// inline 	operator const T &() const { return value; };

	typedef typename map_type::iterator iterator;
	typedef typename map_type::const_iterator  const_iterator;


	/// Child iterator pointing to the first child.
	inline
	typename map_type::const_iterator begin() const { return children.begin(); };

	/// Child iterator end.
	inline
	typename map_type::const_iterator end() const { return children.end(); };

	/// Child iterator pointing to the first child.
	inline
	typename map_type::iterator begin(){ return children.begin(); };

	/// Child iterator end.
	inline
	typename map_type::iterator end(){ return children.end(); };


	// TODO: Some day, change semantics such that
	// tree[] returns childs only (complains separators, for some time)
	// tree() returns descendants, allows deep paths.

	Tree<T,C> & operator[](const std::string &key){

		if (key.empty())
			return *this;

		const iterator it = children.find(key);

		if (it != children.end()){
			return it->second;
		}
		else {
			if (key.find(separator) != std::string::npos){
				std::cerr << "op["<<key<<"] with '/', delegating to op()\n";
				return operator()(key);
			}
			children[key].separator = separator;
			return children[key];
		}

	}

	const Tree<T,C> & operator[](const std::string &key) const {

		if (key.empty())
			return *this;

		const const_iterator it = children.find(key);

		if (it != children.end()){
			return it->second;
		}
		else {
			if (key.find(separator) != std::string::npos){
				std::cerr << "op["<<key<<"] const  with '/', delegating to op() const \n";
				return operator()(key);
			}
			//std::cerr << "return dummy?\n";
			return dummy;
		}

	};

	/// Returns a descendant. Creates one if not existing already.
	/*
	inline
	Tree<T,C> &operator()(const char *key){
		return operator()(std::string(key));
	}
	*/

	/// Returns a descendant. Creates one if not existing already.
	inline
	Tree<T,C> &operator()(const std::string &key){

		// Self-reference
		if (key.empty())
			return *this;

		// Skip leading separator(s)
		/*
		if (key.at(0) == separator)
			return (*this)[key.substr(1)];
		 */

		const size_t i = key.find(separator);

		// Leaves
		if (i == std::string::npos)
			return operator[](key);
		// Subtrees
		else
			return operator[](key.substr(0,i))(key.substr(i+1));

	};


	/// Returns a descendant.
	/*
	inline
	const Tree<T,C> &operator()(const char *key) const {
		return operator()(std::string(key));
	}
	*/

	/// Returns a descendant, or the dummy one if not existing.
	inline
	const Tree<T,C> &operator()(const std::string &key) const {

		// Self-reference
		if (key.empty())
			return *this;

		// Skip separator at root
		/*
		if (key.at(0) == separator)
			return (*this)(key.substr(1));
		 */

		const size_t i = key.find(separator);
		// Leaves
		if (i == std::string::npos)
			return operator[](key);
		// Subtrees
		else
			return operator[](key.substr(0,i))(key.substr(i+1));

	};


#ifdef DRAIN_PATH_H_
	inline
	const Tree<T,C> &operator[](const Path & path) const {

		// Self-reference
		if (path.size() == 0)
			return *this;

		/*
		// Skip separator at root
		if (key.at(0) == separator)
			return (*this)[key.substr(1)];

		const size_t i = key.find(separator);

		// Leaves
		if (i == std::string::npos)
			return operator()(key);
		// Subtrees
		else
			return operator()(key.substr(0,i))[key.substr(i+1)];
		 */
	};
#endif

	bool hasChild(const std::string &key) const {
		return (children.find(key) != children.end());
	};

	bool hasDescendant(const std::string &path) const {
		return &((*this)(path)) != &dummy;
	};


	/// Returns the map containing the children.
	/**
	 *   This is useful for example for map::swap.
	 */
	inline
	map_type & getChildren() { return children; };

	/// Returns the map containing the children.
	inline
	const map_type & getChildren() const { return children; };

	/// Clears the children of this node.
	void clear(){
		children.clear();
		//value = T();
	};

	/// Deletes a node and its subtrees.
	/** If an ending slash is included, then groups but no datasets will be erased. (?)
	 *
	 */
	// This is obscure, check some day. What about erase() -> children.clear();
	void erase(const std::string &path){

		const size_t i = path.rfind(separator);

		const std::string prefix = (i != std::string::npos) ? path.substr(0,i) : "";
		const std::string child  = (i != std::string::npos) ? path.substr(i+1) : path;

		if (child.empty())
			return;

		if (prefix.empty())
			children.erase(child);
		else if (hasDescendant(prefix)){
			(*this)(prefix).children.erase(child);
			//std::cerr << "Tree::erase OK " << parent << ": " << child << '\n';
		}
		else {
			std::cerr << "Tried to erase inexistent prefix=" << prefix << ", child=" << child << '\n';
		}
	};

	/* not elegant
	inline
	void swapChildren(Tree<T,C> &t){
		children.swap(t.children);
	}
	*/
	inline
	void swap(Tree<T,C> &t){
		children.swap(t.children);
	}

	/// Returns the node names matching a regular expression.
	/*  For example, following wildcards can be used:
	 *  - . (period) matches any single character, excluding the path separator
	 *  - .* matches any std::string, including empty
	 *  - [0-5] matches a number
	 *  - ^ matches the start of the std::string
	 *  - $ matches the end of the std::string
	 */
	template <class L>
	inline
	void getKeys(L & list, const std::string &regexp = "") const {
		return getKeys(list, RegExp(regexp));
	}


	/// Returns a list of the node names matching a pattern.
	template <class L>
	void getKeys(L & list, const RegExp & r, const std::string & path = "") const {
		for (typename map_type::const_iterator it = begin(); it != end(); it++){
			//std::string p = path.empty() ? (it->first) : (path + separator + it->first);  // without leading '/'
			std::string p = path + separator + it->first;                                   // with   leading '/'
			if (r.test(p))
				list.push_back(p);
			it->second.getKeys(list, r, p); // recursion
		};
	}


	/// Checks if there is a node with a given path name.
	inline
	bool hasKey(const std::string &path) const {
		return hasKey(path,"");
	};

	inline
	char getSeparator() const { return separator; }

	//char separator;
	/// Debugging utility - dumps the structure of the tree (not the contents).
	void dump(std::ostream &ostr = std::cout, int depth = 0) const {
		//if (depth==0)
		//ostr << ' ' << depth << '\n';
		for (typename map_type::const_iterator it = begin(); it != end(); it++){
			for (int i = 0; i < depth; ++i)
				ostr << "  ";
			if (it == --end())
				ostr << "'––";
			else
				ostr << "¦––";
			ostr << it-> first; // << '\n'; //' ' << depth << '\n';
			ostr << '\n';
			it->second.dump(ostr,depth+1);
		};

	};



	/// Debugging utility - dumps the tree, also the contents.
	void dumpContents(std::ostream &ostr = std::cout, const std::string & path = "") const {

		ostr << path << '\n';
		ostr << this->data << '\n';
		for (typename map_type::const_iterator it = begin(); it != end(); it++){
			//ostr << it->first;
			it->second.dumpContents(ostr, path+"/"+it->first);
		};
	};


	// Tree<T,C> & parent;

protected:

	/// Constructor for a child.
	//Tree(char separator, Tree<T,C> &parent) :   separator(separator) {}; // parent(parent),


	static const Tree<T,C> dummy;

	char separator;

	map_type children;


	/// Checks if there is a node with a given path name.
	/** Could be called hasDescendant; hence is like hasChild() but calls children recursively.
	 *
	 */
	bool hasKey(const std::string &path,const std::string &prefix) const {

		for (typename map_type::const_iterator it = begin(); it != end(); it++){
			std::string p = prefix + separator + it->first;
			//path.

			if (path.compare(p) == 0){
				// std::cerr << "Found path: " << p << '\n';
				return true;
			}
			else
				if (it->first.compare(0,p.length(),p)){ // path starts with p
					// std::cerr << "Ascending path: " << p << '\n';
					return it->second.hasKey(path,p);
				}
				else { // no hope
					//std::cerr << "Skipped remaining path: " << p << '\n';
				}
		};
		return false;
	};

	//static char separator;
	// cooperator[]
};

template <class T, class C>
const Tree<T,C> Tree<T,C>::dummy;


}

#endif /* TREE2_H_ */
