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

// TODO: check
#ifndef DRAIN_TREE_UTILS
#define DRAIN_TREE_UTILS "2.1"


#include <iterator>
#include <string>
#include <map>


#include "Log.h"
#include "Path.h"
#include "String.h"

namespace drain {



/// Collection of functions for investigating and processing trees.
/**
 *
 *  \tparam K - key type, implementing method empty() and cast from/to std::string.
 *  \tparam T - value type
 *  \tparam C - comparison functor, implementing less-than relation
 *
 */
class TreeUtils {
public:

	/// Deletes a node (leaf) and its subtrees.
	/** If an ending slash is included, then groups but no datasets will be erased. (?)
	 *
	 */
	template <class TR>
	static
	void erase(TR & tree, const typename TR::path_t & path){ // RAISE/virtualize

		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.unimplemented("consider: tree(", path, ").clear()");
		/*
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
		*/
	}

	/// Returns a list of the node names matching a pattern. The trailing '/' is NOT appended ie. should not be tested by RegExp.
	/**
	 *  \param prefix - leading part of
	 */
	template <class TR, class S>
	static
	void getPaths(const TR & tree, S & container){
		//const path_t prefix(separator);
		for (const auto & entry: tree){
			typename TR::path_t p;
			p << entry.first;
			getPaths(entry.second, container, p); // recursion
		};
	}

	/// Returns a list of the node names matching a pattern. The trailing '/' is NOT appended ie. should not be tested by RegExp.
	/**
	 *  \tparam S - container (STL Sequence)
	 *  \param prefix - leading part of
	 */
	template <class TR, class S>
	static
	void getPaths(const TR & tree, S & container, const typename TR::path_t & path){
		container.push_back(path);
		for (const auto & entry: tree){
				//for (typename container_t::const_iterator it = begin(); it != end(); ++it){
			typename TR::path_t p = path;
			p << entry.first;
			getPaths(entry.second, container, p); // recursion
		};
	}

	template <class T1, class T2>
	static
	void deepCopy(const T1 & srcTree, T2 & dstTree){

		for (const auto & entry: srcTree){
			deepCopy(entry.second, dstTree.addChild(entry.first));
		};

		if (dstTree.empty() || !dstTree.isExclusive())
			dstTree.data = srcTree.data;

	}


	/// Traverse tree, visiting each node as a prefix operation.
	/**
	 *  Simple version.
	 *
	 *  \tparam H - object with method visit(tree, path).
	 *  \tparam T - tree type, can be const
	 */
	template <class T, class H>
	static void traverse(H & handler, T & tree, const typename T::path_t & path = typename T::path_t()){

		/// TODO: more codes than non-zero
		if (handler.visit(tree, path)){
			std::cout << "SKIP: " << path << ':'  << '\n'; // << tree(path).data
			return;
		}
		// std::cout << "OK:   " << path << ':'  << '\n'; // << tree(path).data


		// Recursion
		for (auto & entry: tree.getChildren()){
			const typename T::path_t p(path, entry.first);
			traverse(handler, entry.second, p);
		}

	};


	/// Default implementation for recursive dump()
	template <class T>
	static
	bool dataDumper(const T & data, std::ostream &ostr){
		ostr << data << ' ';
		return true;
	}

	/// Render a tree using character graphics.
	/**
	 *  Debugging utility - dumps the structure of the tree (not the contents).
	 *
	 *  \tparam TR - tree type
	 *  \tparam SKIP - skip empty branches (FUTURE OPTION)
	 *
	 *  \return - true if empty, false if groups exist or data non-empty (FUTURE OPTION)
	 */
	template <class TR, bool SKIP_EMPTY=false>
	static
	//void dump(const TR & tree, std::ostream &ostr = std::cout, bool nodes=false, const std::string &  indent="") { // int depth = 0) const {
	bool dump(const TR & tree, std::ostream &ostr = std::cout,
			bool (* callBack)(const typename TR::node_data_t &, std::ostream &) = TreeUtils::dataDumper, const std::string &  indent="") { // int depth = 0) const {

		// https://www.w3.org/TR/xml-entity-names/025.html
		/*
		static const std::string EMPTY(" ");
		static const std::string VERT("│");
		static const std::string VERT_RIGHT("├");
		static const std::string UP_RIGHT("└");
		static const std::string HORZ("─");
		static const std::string HORZ_DOWN("┬");
		*/

		bool empty = true;

		//if (nodes){
		if (callBack != nullptr){ // nodes){
			// std::stringstream sstr;
			if (tree.empty() || !tree.isExclusive()){
				// if (empty || !tree.isExclusive()){
				empty = (*callBack)(tree.data, ostr);
				//if (sstr.LENGTH)
				// ostr << sstr.str(); // See Logger for direct copy? Risk: pending
				// ostr << tree.data;
			}
		}
		ostr << '\n';

		// for (const auto & entry: *this){
		// for (const auto & entry: tree.begin()){
		for (typename TR::container_t::const_iterator it = tree.begin(); it != tree.end(); it++){

			std::ostream & ostrChild = ostr; // for now...

			std::string indent2;
			if (it == --tree.end()){
				ostrChild   << indent << "└──"; // UP_RIGHT << HORZ << HORZ; // "'––";
				indent2      = indent  + "   "; // EMPTY  + EMPTY + EMPTY;   // "   ";
			}
			else {
				ostrChild   << indent << "├──"; // VERT_RIGHT << HORZ << HORZ; // "¦––";
				indent2      = indent  + "│  "; // VERT + EMPTY + EMPTY; // "|  ";
			}
			ostrChild << it->first; // << '\n'; //' ' << depth << '\n';

			if (callBack != nullptr){ // nodes){
				//ostr << ':';
				ostrChild << ' ';
			}

			// dump(entry.second, ostr, nodes, indent2);
			bool empty2 = dump(it->second, ostrChild, callBack, indent2);

			if (!empty2)
				empty = false;

			//if (!(empty2 && SKIP_EMPTY))
			// if ((empty2 && SKIP_EMPTY)) ostr << "{\n";
			// ostr << sstr.str();
			//if ((empty2 && SKIP_EMPTY)) ostr << "}\n";
		};

		return empty;

	};

	/// Debugging utility - dumps the tree, also the contents.
	template <class TR>
	static
	void dumpContents(const TR & tree, std::ostream &ostr = std::cout, const typename TR::path_t & path = "") {
		ostr << path << '=';
		ostr << tree.data << '\n';
		for (const auto & entry: tree){
			ostr << entry.first << '\t';  // UNIMPLEMENTED: recursion?
			dumpContents(entry.second, ostr); //, path+"/"+it->first);
		};
	};

	/// Write a Windows INI file
	// TODO: move to TreeUtils, Sprinter-like?
	template <class TR>
	static inline
	void writeINI(const TR & t, std::ostream & ostr = std::cout, const typename TR::path_t & prefix = typename TR::path_t()){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.unimplemented("future extension");
	}



	template <class TR>
	static inline
	void readINI(TR & tree, std::istream & istr){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.unimplemented("future extension");
	}


};



/// Write a Windows INI file
/*

void JSONtree::writeINI(const tree_t & t, std::ostream & ostr, const tree_t::path_t & prefix){

	//for (tree_t::node_t::const_iterator dit = t.data.begin(); dit != t.data.end(); ++dit){
	for (const auto & entry: t.data){
		ostr << entry.first << '='; // << dit->second;
		Sprinter::toStream(ostr, entry.second, Sprinter::jsonLayout);
		// entry.second.valueToJSON(ostr);
		ostr << '\n';
	}
	ostr << '\n';


	// Traverse children (only)
	for (tree_t::const_iterator it = t.begin(); it != t.end(); ++it){

		tree_t::path_t path(prefix);
		path << it->first;

		ostr << '[' << path << ']' << '\n';

		writeINI(it->second, ostr, path);

		ostr << '\n';

	}

}
*/


#ifdef DRAIN_TYPE_UTILS
/*
template <>
struct TypeName<TreeUtils> {
	static const char* get(){
		return "SuperTree";
	}
};
*/
#endif


} // drain::

#endif


