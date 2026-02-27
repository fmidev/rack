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

#include <cstddef> // nullptr
#include <iterator>
#include <string>
#include <map>

#include <drain/Log.h>
#include <drain/StringTools.h>
#include "Path.h"

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

	/// Retrieve all the paths.
	/**
	 *  \tparam S - container (STL Sequence)
	 *  \param path - starting point
	 */
	template <class TR, class S>
	static
	void getPaths(const TR & tree, S & container){
		for (const auto & entry: tree){
			typename TR::path_t p;
			p << entry.first;
			getPaths(entry.second, container, p); // recursion
		};
	}

	/// Retrieve all the paths.
	/**
	 *  \tparam S - container (STL Sequence)
	 *  \param path - starting point
	 */
	template <class TR, class S>
	static
	void getPaths(const TR & tree, S & container, const typename TR::path_t & path){ // consider empty() and checking empty
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
	 *  \tparam H - object with method visitPrefix(tree, path) and visitPostfix(tree, path)
	 *  \tparam T - tree type, can be const
	 *
	 *  \see TreeVisitor<>
	 */
	template <class T, class H>
	static void traverse(H & visitor, T & tree, const typename T::path_t & path = typename T::path_t()){

		/// TODO: more codes than non-zero
		if (visitor.visitPrefix(tree, path)){
			return;
		}


		// Recursion
		for (auto & entry: tree(path).getChildren()){
			// NOTICE: tree stays intact, path expands...
			traverse(visitor, tree, typename T::path_t(path, entry.first));
		}

		if (visitor.visitPostfix(tree, path)){
			// What to do? Not much commands below ...
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
			// if ((!tree.hasChildren()) || !tree.isExclusive()){
			if (! (tree.isExclusive() && tree.hasChildren())){
			// if (tree.empty()){
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


/// Default implementation of a tree visitor (concept) compatible TreeUtils::traverser()
/**
 *
 *   Notice that the first argument (visitor) of TreeUtils::traverser() is templated,
 *   hence does not have to be derived from this class.
 *
 */
template <class T>
class TreeVisitor {

public:

	inline
	TreeVisitor(){};

	virtual inline
	~TreeVisitor(){};

	/// Tasks to be executed before traversing child nodes.
	virtual inline
	int visitPrefix(T & tree, const typename T::path_t & path){
		return 0;
	}

	/// Tasks to be executed after traversing child nodes.
	virtual inline
	int visitPostfix(T & tree, const typename T::path_t & path){
		return 0;
	}

};




#ifdef DRAIN_TYPE_UTILS
DRAIN_TYPENAME_STR(TreeUtils);
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


