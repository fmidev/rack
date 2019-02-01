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
#include "Path.h"
//#include "Options.h"

namespace drain {



/// Unordered tree based on std::map.
/**
 *
 *  \tparam K - key type, implementing method empty() and cast from/to std::string.
 *  \tparam T - value type
 *  \tparam C - comparison functor, implementing less-than relation
 *
 *  tree[] returns childs only (complains separators, for some time)
 *  tree() returns descendants, allows deep paths.
 *
 *  Child nodes can be addressed with operator[] with std::string valued keys:
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
 *  t("firstChild/a/bb")
 *  \endcode
 *
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
template <class K, class T, class C=std::less<std::string> >
class Tree {
public:

	typedef K key_t;
	typedef T node_t;
	typedef drain::Path<K> path_t;
	typedef drain::Tree<K,T,C> tree_t;
	typedef std::map<K,Tree<K,T,C>, C> map_t;

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

	typedef typename map_t::iterator iterator;
	typedef typename map_t::reverse_iterator reverse_iterator;
	typedef typename map_t::const_iterator  const_iterator;


	/// Child iterator pointing to the first child.
	inline
	typename map_t::const_iterator begin() const { return children.begin(); };

	/// Child iterator end.
	inline
	typename map_t::const_iterator end() const { return children.end(); };

	/// Child iterator pointing to the first child.
	inline
	typename map_t::iterator begin(){ return children.begin(); };

	/// Child iterator end.
	inline
	typename map_t::iterator end(){ return children.end(); };


	/*
	tree_t & operator[](const std::string & key){
	}
	*/

	tree_t & operator[](const key_t & key){

		//if (key.empty())			return *this;

		iterator it = children.find(key);

		if (it != children.end()){
			return it->second;
		}
		else {
			/*
			if (key.find(separator) != std::string::npos){
				std::cerr << "op["<<key<<"] with '/', delegating to op()\n";
				return operator()(key);
			}
			*/
			children[key].separator = separator;
			return children[key];
		}

	}

	const tree_t & operator[](const key_t & key) const {

		//if (key.empty())			return *this;

		const const_iterator it = children.find(key);

		if (it != children.end()){
			return it->second;
		}
		else {
			/*
			if (key.find(separator) != std::string::npos){
				std::cerr << "op["<<key<<"] const  with '/', delegating to op() const \n";
				return operator()(key);
			}*/
			//std::cerr << "return dummy?\n";
			return dummy;
		}

	};


	/// Returns a descendant. Creates one if not existing already.
	/**
	 *  \tparam - K2 key type of applied Path class
	 */
	//template <class K2>
	inline
	//tree_t & operator()(const drain::Path<K2> & path){
	tree_t & operator()(const path_t & path){
		return get(path.begin(), path.end());
	}

	/// Returns a descendant.
	/**
	 *  \tparam - K2 key type of applied Path class
	 */
	//template <class K2>
	inline
	//const tree_t & operator()(const drain::Path<K2> & path) const {
	const tree_t & operator()(const path_t & path) const {
		return get(path.begin(), path.end());
	}

	/*
	/// Returns a descendant. Creates one if not existing already.
	inline
	tree_t & operator()(const path_t & path){
		return get(path.begin(), path.end());
	}

	/// Returns a descendant.
	inline
	const tree_t &operator()(const path_t & path) const {
		return get(path.begin(), path.end());
	}
	*/

	/// Returns a descendant. Creates one if not existing already.
	/*
	inline
	tree_t & operator()(const std::string & s){
		return operator()(path_t(s));
	}

	/// Returns a descendant. Creates one if not existing already.
	inline
	const tree_t & operator()(const std::string & s) const {
		return operator()(path_t(s));
	}

	inline
	tree_t & operator()(const char *s){
		return operator()(path_t(s));
	}

	/// Returns a descendant. Creates one if not existing already.
	inline
	const tree_t & operator()(const char *s) const {
		return operator()(path_t(s));
	}
	*/



	/// Returns a descendant.
	/**
	 *  \tparam - K2 key type of applied Path class
	 */
	//template <class K2>
	inline
	//tree_t & get(typename K2::const_iterator it, typename K2::const_iterator eit){
	tree_t & get(typename path_t::const_iterator it, typename path_t::const_iterator eit) {

		// Path empty => self-reference
		if (it == eit)
			return *this;

		// Path element is empty => proceed to next element
		if (it->empty())
			return get(++it, eit);


		tree_t & child = operator[](*it);
		return child.get(++it, eit);

	}



	/// Returns a descendant.
	/**
	 *  \tparam - K2 key type of applied Path class
	 */
	//template <class K2>
	inline
	//const tree_t & get(typename K2::const_iterator it, typename K2::const_iterator eit) const {
	const tree_t & get(typename path_t::const_iterator it, typename path_t::const_iterator eit) const {

		// Path empty => self-reference
		if (it == eit)
			return *this;

		// Path element is empty => proceed to next element
		if (it->empty())
			return get(++it, eit);

		if (!hasChild(*it))
			return dummy;

		const tree_t & child = operator[](*it);
		return child.get(++it, eit);
	}

	/// Returns a descendant, or the dummy one if not existing.
			/*
	inline
	const tree_t &operator()(const path_t & path) const {


		// Self-reference
		if (path.empty())
			return *this;

		const key_t & root = path.front();

		typename path_t::const_iterator it = path.begin();
		const tree_t & child = operator[](*it);
		return child(*(++it));


	};
	*/


	bool hasChild(const key_t &key) const {
		return (children.find(key) != children.end());
	};

	// TODO: hasKey = hasPath
	bool hasDescendant(const path_t &path) const {
		std::cerr << "obsolete " << __FUNCTION__ << std::endl;
		return hasPath(path);
		//return &((*this)(path)) != &dummy;
	};


	/// Returns the map containing the children.
	/**
	 *   This is useful for example for map::swap.
	 */
	inline
	map_t & getChildren() { return children; };

	/// Returns the map containing the children.
	inline
	const map_t & getChildren() const { return children; };

	/// Clears the children of this node.
	inline
	bool isEmpty(){
		return (children.empty());
	};

	/// Clears the children of this node.
	void clear(){
		children.clear();
		//value = T();
	};

	/// Deletes a node (leaf) and its subtrees.
	/** If an ending slash is included, then groups but no datasets will be erased. (?)
	 *
	 */
	void erase(const path_t & path){

		typename path_t::const_iterator it = path.end();
		if (it == path.begin())
			return;
		else {
			--it;
			// Now 'it' points to the leaf
			if (it == path.begin()) // path size = 1 (direct child path)
				children.erase(*it);
			else { // path size > 1
				tree_t & parent = this->get(path.begin(), it);
				parent.children.erase(*it);
			}
		}

	}
	/*
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
	*/

	/* not elegant
	inline
	void swapChildren(Tree<T,C> &t){
		children.swap(t.children);
	}
	*/
	inline
	void swap(tree_t &t){
		children.swap(t.children);
	}


	/// Returns a list of the node names matching a pattern. The trailing '/' is NOT appended ie. should not be tested by RegExp.
	/**
	 *  \param prefix - leading part of
	 */
	template <class S>
	void getPaths(S & container) const {
		//const path_t prefix(separator);
		for (typename map_t::const_iterator it = begin(); it != end(); ++it){
			path_t p;
			p << it->first;
			it->second.getPaths(container, p); // recursion
		};
	}

	/// Returns a list of the node names matching a pattern. The trailing '/' is NOT appended ie. should not be tested by RegExp.
	/**
	 *  \param prefix - leading part of
	 */
	template <class S>
	void getPaths(S & container, const path_t & path) const {
		container.push_back(path);
		for (typename map_t::const_iterator it = begin(); it != end(); ++it){
			path_t p = path;
			p << it->first;
			it->second.getPaths(container, p); // recursion
		};
	}





	/// Checks if there is a node with a given path name.
	inline
	bool hasKey(const path_t & path) const {
		std::cerr << "obsolete " << __FUNCTION__ << ", forwarding to hasPath() " << std::endl;
		return hasPath(path);
	};

	inline
	bool hasPath(const path_t & path) const {
		return hasPath(path.begin(), path.end());
	};


	inline
	char getSeparator() const { return separator; }

	//char separator;
	/// Debugging utility - dumps the structure of the tree (not the contents).
	void dump(std::ostream &ostr = std::cout, int depth = 0) const {
		//if (depth==0)
		//ostr << ' ' << depth << '\n';
		for (typename map_t::const_iterator it = begin(); it != end(); it++){
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

		ostr << path << '=';
		ostr << this->data << '\n';
		for (typename map_t::const_iterator it = begin(); it != end(); it++){
			ostr << it->first << '\t';
			it->second.dumpContents(ostr); //, path+"/"+it->first);
		};
	};


	// Tree<T,C> & parent;

protected:

	/// Constructor for a child.
	//Tree(char separator, Tree<T,C> &parent) :   separator(separator) {}; // parent(parent),


	static const tree_t dummy;

	char separator;

	map_t children;


	/// Checks if there is a node with a given path name.
	/** Could be called hasDescendant; hence is like hasChild() but calls children recursively.
	 *
	 *  \param eit - end()
	 */
	bool hasPath(typename path_t::const_iterator it, typename path_t::const_iterator eit) const {

		if (it == eit) // empty path
			return true;

		const typename path_t::elem_t elem = *it;

		if (elem.empty())
			return hasPath(++it, eit);
		else if (!hasChild(elem))
			return false;

		return this->operator [](elem).hasPath(++it, eit);

	}

	/*
	bool hasKey(const std::string &path,const std::string &prefix) const {

		for (typename map_t::const_iterator it = begin(); it != end(); it++){
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
	*/

	//static char separator;
	// cooperator[]
};

template <class K, class T, class C>
const Tree<K,T,C> Tree<K,T,C>::dummy;


}

#endif /* TREE2_H_ */
