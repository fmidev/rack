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

#ifndef DRAIN_TREE_NAME
#warning "Use TreeOrdered.h or TreeUnordered.h to include this file."
#endif


#include <iterator>
#include <string>
#include <map>


#include <drain/Log.h>
#include <drain/Type.h>
#include <drain/TypeUtils.h>

#include "Path.h"

#include "TreeUtils.h"

namespace drain {



/// A templated class for directed rooted trees.
/**
 *  This file provides templates of for basic types of trees (directed rooted trees):
 *
 *   - OrderedTree – std::map<>
 *   - OrderedMultiTree – std::multimap
 *   - UnorderedTree – std::list
 *   - UnorderedMultiTree – std::list
 *
 * \tparam T - node type, containing user-designed data implementing clear(), size() and empty() for future compatibility.
 * \tparam EXCLUSIVE - node can contains either data or children but not both (JSON-like convention)
 * \tparam P - applied path structure, like std::list – must implement empty() both for path and keys (path elements)
 *
 * Data:
 *
 * The nodes in a tree contain user defined data, the type of which is given as template T.
 *
 *   Currenty, data Concept is requires the following methods:
 *   - clear() – resetting the data to default initial (empty) value.
 *   - empty() – returning true if the data is empty, or can be identified with empty.
 *
 *  A tree is EXCLUSIVE, if a node contains either data or children. For example, a JSON tree is EXCLUSIVE and an XML tree is not (a tag can containg attributes (data) and
 *  inner tages (children).
 *
 *   Further, each type listed above has two variants:
 *   - inclusive (implicit, default) – each node may contain data and child nodes.
 *   - exclusive – each node may contain data or child nodes, but not both.
 *   In the code, both data and children exist as members but in
 *   exclusive trees data is automatically cleared if children are added, and
 *   inversely, chilren are cleared (removed) if data is assigned.
 *
 *  Limitation: in OrderedTree the children are in a std::map, and threrefore each child
 *  of a node has a unique key.
 *  In UnorderedTree there may be children having the same key.
 *
 *  Addressing:
 *
 *  tree(<path>) return a descendant
 *  tree[<path_element>] returns childs only (complains separators, for some time)
 *
 *  Child nodes can be addressed with operator[] with std::string valued keys:
 *  \code
 *  OrderedTree<std::string> tree;
 *  t["first"] = "Hello!";
 *  t["second"] = "Another greeting...";
 *  \endcode
 *
 *  As the operator[] returns a reference, it may be used recursively:
 *  \code
 *  OrderedTree<std::string> & subTree1 = t["first"]["child"]["grandChild"];
 *  \endcode
 *
 *  Tree also recognizes a path separator, which is '/' by default.
 *  Hence, the node created in the above example could be likewise
 *  created or addressed as:
 *  \code
 *  OrderedTree<std::string> & subTree = t("first/child/grandChild");        // equivalent
 *  \endcode
 *
 *  One may iterate the children directly:
 *  \code
 *  for (auto & entry: tree){
 *    std::cout << entry.first << ':' << entry.second << '\n';
 *  }
 *  \endcode
 *
 *
 *  Each node contains data of type T. It may be unused (empty), that is, a node can be used only as a branching point.
 *
 *  \code
 *  OrderedTree<std::string> t;
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
 *
 *
 * Obsolete: template C - path element comparison functor, should be compatible with Path<P>::key_t.
 */

/**
#ifndef DRAIN_AMBIVALUE
#define DRAIN_AMBIVALUE "0.1beta"
template <class T, class K>
class CombiValue {
public:

	typedef T data_t;
	typedef K key_t;

	// T data; CONSIDER! But Hi5Node heavy?

	inline
	~CombiValue(){};

	/// True, if data is structured.
	virtual
	bool hasMultipleData() const = 0;


	virtual
	const data_t & getData() const = 0;

	virtual
	data_t & getData() = 0;

	virtual
	const data_t & getData(const key_t &) const = 0;

	virtual
	data_t & getData(const key_t &) = 0;

};
#endif
 *  \tparam K – key type
 *  \tparam K – key type
 */

template <class T, bool EXCLUSIVE=false, class P=drain::Path<std::string,'/'> > // , class C=std::less<std::string>
class DRAIN_TREE_NAME { // : public AmbiValue<T,typename P::elem_t> { //: public TreeBase {
public:

	typedef drain::DRAIN_TREE_NAME<T,EXCLUSIVE,P> tree_t; //,C
	typedef T node_data_t;
	typedef P path_t;
	typedef typename path_t::elem_t key_t;

	typedef std::pair<key_t,tree_t> pair_t;

	typedef DRAIN_TREE_CONTAINER(key_t,tree_t) container_t;

	typedef typename container_t::iterator iterator;
	typedef typename container_t::const_iterator  const_iterator;

	/// Default constructor.
	inline
	DRAIN_TREE_NAME(){}; // :  separator(separator) {}; //parent(*this),

	/// Copy constructor; copy only node data at the root.
	inline
	DRAIN_TREE_NAME(const node_data_t &data) : data(data){}; //, separator(t.separator) {}; //parent(*this),

	/// Copy constructor; copy only node data at the root.
	inline
	DRAIN_TREE_NAME(const DRAIN_TREE_NAME &t) : data(t.data){}; //, separator(t.separator) {}; //parent(*this),

	virtual inline
	~DRAIN_TREE_NAME(){};


	/// Contents (data) of the node.
	node_data_t data;


	// AMBIVALUED
	virtual inline
	bool hasMultipleData() const {
		// Reteurn false, if own, local data
		return data.empty();
	};

	virtual inline
	const node_data_t & getData() const {return data;};

	virtual inline
	node_data_t & getData(){return data;};

	virtual inline
	const node_data_t & getData(const key_t & key) const {
		return retrieveChild(key).data;
	};

	virtual inline
	node_data_t & getData(const key_t & key){
		return retrieveChild(key).data;
	};



	/// Child iterator pointing to the first child.
	inline
	typename container_t::const_iterator begin() const { return children.begin(); };

	/// Child iterator pointing beyond the last child.
	inline
	typename container_t::const_iterator end() const { return children.end(); };

	/// Child iterator pointing to the first child.
	inline
	typename container_t::iterator begin(){ return children.begin(); };

	/// Child iterator end.
	inline
	typename container_t::iterator end(){ return children.end(); };


	// REMOVE
	static inline
	bool isExclusive(){
		return EXCLUSIVE;
	}

	// REMOVE
	static inline
	bool isMulti(){
		#ifdef DRAIN_TREE_MULTI
		return true;
		#else
		return false;
		#endif
	}

	static inline
	bool isOrdered(){
		#ifdef DRAIN_TREE_ORDERED
		return true;
		#else
		return false;
		#endif
	}

	/*
	static inline
	const std::string className(){
		const std::string s = drain::StringBuilder<>(
				isOrdered()?"Ordered":"Unordered",
						isMulti()?"Multi":"","Tree",
								isExclusive()?"(Exclusive)":"",
										'<', drain::Type::call<drain::simpleName>(typeid(node_data_t)), '>');
		return s;
	}
	*/

	/// Copies the data of another node. Does not copy the children.
	/**
	 *  Default use to reset a node (re-initialize its data).
	 *
	 */
	// TODO: what to do with the separator?
	inline
	tree_t &operator=(const tree_t &t){
		if (&t != this){
			data = t.data;
			//if (EXCLUSIVE){ ...or consider deep copy?
			clearChildren();
			//}
		}
		return *this;
	};

	/// Assigns value to contents.
	// Needed? See next.
	/* THIS CAUSES PROBLEMS. with int x = tree; vs. tree = x;
	inline
	tree_t & operator=(const node_data_t &v){
		data = v;
		if (EXCLUSIVE){
			clearChildren();
		}
		return *this;
	};
	*/

	template <class S>
	inline
	tree_t & operator=(const std::initializer_list<S> &l){
		data = l;
		if (EXCLUSIVE){
			clearChildren();
		}
		return *this;
	}

	/// Assigns a value to contents.
	template <class T2>
	inline
	tree_t & operator=(const T2 &v){
		data = v;
		if (EXCLUSIVE){
			clearChildren();
		}
		return *this;
	}

	/// Experimental. Given pair<elem, data> assigns child[elem] = data;
	typedef std::pair<key_t,node_data_t> node_pair_t;
	inline
	tree_t & operator<<(const node_pair_t & entry){
		if (EXCLUSIVE){
			clearData();
		}
		tree_t & child = retrieveChild(entry.first);
		child.data = entry.second;
		return child;
	}

	inline
	tree_t & ensureChild(const node_pair_t & entry){
		if (hasChild(entry.first)){
			return retrieveChild(entry.first);
		}
		else {
			return (*this << entry);
		}
	}


	inline
	operator const node_data_t &() const {
		return data;
	};

	inline
	operator node_data_t &(){
		return data;
	};


	/// Child addressing operator
	inline
	tree_t & operator[](const key_t & key){
		return retrieveChild(key);
	}

	/// Child addressing operator
	inline
	const tree_t & operator[](const key_t & key) const {
		return retrieveChild(key);
	}


	/// Returns a descendant. Creates one if not existing already.
	/**
	 *  \param path - the path from the current node to the child node.
	 *
	 *  This function, using native path_t type, should be reserved for this semantic
	 *  by not redefining (specialising) this function in template classes.
	 */
	inline
	tree_t & operator()(const path_t & path){
		/*
		if (superDebug){
			std::cout << __FILE__ << "tree_t & operator()(const path_t & path)" << '\n';
		}
		*/
		return get(path.begin(), path.end());
	}

	/// Returns a descendant. Creates one if not existing already.
	/**
	 *  \param path - the path from the current node to the child node.
	 *
	 *  This function, not using native path_t type, can be redefined in template classes.
	 *  For example, an important attribute can be set using this method.
	 *  Method operator()(const char *arg) should be redefined to produce equivalent
	 *  result with <code>const string &arg</code>.
	 */
	template <class S>
	inline
	tree_t & operator()(const S & arg){
		return operator()(path_t(arg));
	}

	/// Redirects the call to operator()(const std::string & arg) .
	/**
	 *  \param arg - by default, the path from the current node to the child node.
	 */
	inline
	tree_t & operator()(const char *arg){
		return operator()(path_t(arg));
		// return operator()(std::string(arg));
	}

	/// Returns a descendant if that exists, else returns an empty node. Otherwise like non-const counterpart.
	/**
	 *  \param path - the path from the current node to the child node.
	 *
	 *  This function, using *native* path_t type, should be reserved for this semantic
	 *  by not redefining (specialising) this function in template classes.
	 */
	inline
	const tree_t & operator()(const path_t & path) const {
		return get(path.begin(), path.end());
	}

	/// Returns a descendant if that exists, else returns an empty node. Otherwise like non-const counterpart.
	template <class S>
	inline
	const tree_t & operator()(const S & arg) const {
		return operator()(path_t(arg));
	}

	/// Redirects the call to operator()(const std::string & arg) .
	inline
	const tree_t & operator()(const char *arg) const {
		return operator()(path_t(arg));
		//return operator()(std::string(arg));
	}

	/// Clear children and node data.
	/**
	 *  \see erase()
	 *  \see clearChildren()
	 *  \see clearData()
	 */
	inline
	void clear(){
		clearData();
		clearChildren();
	};

	/**
	 *  \see clear()
	 *  \see clearChildren()
	 *  \see erase()
	 */
	inline
	void clearData(){
		// Future option: data.clear() (applies to many stl classes, like strings and containers)
		data = getEmpty().data;
	};

	/// Clears the children of this node. Does not clear data.
	/**
x	 *  \see clearData()
	 *  \see clear()
	 *  \see erase()
	 */
	inline
	void clearChildren(){ // RAISE/virtualize
		children.clear();
	};

	/*
	void eraseChild(const key_t & key){
		children.erase(key);
	}
	*/

	/// Deletes a descendant node and hence its subtrees.
	/**
	 *  As opposite of clear() which focuses on a (current) node, this function refers to a descendant.
	 *
	 *  \see clear()
	 */
	// ??-> If an ending slash is included, then groups but no datasets will be erased. (?)
	void erase(const path_t & path){ // RAISE/virtualize

		// drain::Logger mout(__FILE__, __FUNCTION__);

		// Idea: first assign the leaf, and step one back.
		typename path_t::const_iterator it = path.end();
		if (it == path.begin())
			return;
		else {
			// Move 'it' back one step, to point to the leaf (the last element in chain).
			--it;

			// Safe (identity) if it == path.begin():
			tree_t & parent = this->get(path.begin(), it);

			#ifdef DRAIN_TREE_ORDERED  // map

			// TODO: generalize "find" for find first, etc
			parent.children.erase(*it);

			#else

			drain::Logger(__FILE__, __LINE__, __FUNCTION__).error("unimplemented code");


			#endif

			//parent.eraseChild(*it);

		}

	}

	/// GENERAL (independent from container type)
	/// Empty "default" node; typically initialized with empty data as well.
	virtual inline
	const tree_t & getEmpty() const {
		return emptyNode;
	}

	/// Check if the tree structure is empty.
	// Note: Current implementation does not check node (data). Could apply:
	// Ambiguous?
	virtual inline
	bool empty() const {
		// consider #ifdef DRAIN_TREE_SMART_DATA
		// return (data.empty() && children.empty())
		// #else
		//  data == defaultNode.data?
		return (data.empty() && !hasChildren());
		// endif
	};

	inline
	bool hasChildren() const {
		return !children.empty();
	}

	/// The number of children with name key.
	/**
	 * 	Checks how many immediate descendants with name \c key exist.
	 *
	 *  A multiple tree may have more than one children with the same name.
	 *   \see OrderedMultiTree, UnorderedMultiTree
	 *
	 *  With simple trees, use the faster hasChild(const key_t &key).
	 *   \see OrderedTree
	 *   \see UnorderedTree.
	 *
	 *
	 *   \return – the number of children with name 'key'.
	 */
	virtual inline
	int hasChildren(const key_t &key) const {

		#ifdef DRAIN_TREE_ORDERED  // map

		if (!isMulti()){
			return (children.find(key) == children.end()) ? 0 : 1;
		}
		// no-break for OrderedMultipleTree

		#endif

		// OrderedMultipleTree (passed through from the above #ifdef-#endif)
		// OrderedMultipleTree
		// UnorderedMultipleTree

		size_t count = 0;
		for (const auto & entry: children){
			if (entry.first == key){
				++count;
			}
		}
		return count;

	};

	/// Check if the tree node has a direct descendant with name \c key.
	/**
	 *   A multiple tree may have more than one children with the same name.
	 *   With them, consider hasChildren(const key_t &key) .
	 *   \see OrderedMultiTree
	 *   \see UnorderedMultiTree
	 *
	 *   \return – the number of children with name 'key'.
	 */
	virtual inline
	bool hasChild(const key_t &key) const {
		#ifdef DRAIN_TREE_ORDERED  // map

		return (children.find(key) != children.end());

		#else

		for (const auto & entry: children){
			if (entry.first == key){
				return true;
			}
		}
		return false;

		#endif
	};


	inline
	bool hasPath(const path_t & path) const {
		return hasPath(path.begin(), path.end());
	}

	// protect:
	/*
	key_t getNewChildKey() const {
		// Consider error (unimplemented)
		return key_t();
	}
	*/

	/// Add a child node. If unordered and UNIQUE, reuse existing nodes.
	/**
	 *   Behaviour of this function varies as follows:
	 *
	 *   - OrderedTree: return child named \c key, if exists
	 *   - UnorderedTree, UNIQUE==true:  return the first child named \c key, if exists, else create one.
	 *   - UnorderedTree, UNIQUE==false: always create a new child named \c key and return it.
	 *
	 *
	 */
	virtual
	tree_t & addChild(const key_t & key = key_t()){ // Added default empty 2024/04

		if (key.empty()) // Should be exceptional... Warning?
			return *this;

		if (EXCLUSIVE)
			this->clearData();

		#ifdef DRAIN_TREE_ORDERED
		iterator it = children.find(key);
		if (it != children.end()){
			return it->second;
		}
		else {
			return children.insert(children.begin(), pair_t(key, tree_t()))->second;
		}
		//return children[key];

		#else

		// Try searching first
		if (!isMulti()){
			for (auto & entry: children){
				if (entry.first == key){
					return entry.second;
				}
			}
		}

		// Add:
		children.push_back(pair_t(key, tree_t()));
		return children.back().second;

		#endif
	};


	// retrieveChild(key, create ALWAYS / IF_NOT_FOUND

	virtual
	tree_t & retrieveChild(const key_t & key){

		if (key.empty())
			return *this;

		// ! if (EXCLUSIVE) this->clearData();

		#ifdef DRAIN_TREE_ORDERED
		// Old policy restored; keys must implement empty(), an empty key is indentified to the current node.

		// return children[key];
		iterator it = children.find(key);
		if (it != children.end()){
			return it->second;
		}
		else {
			return children.insert(children.begin(), pair_t(key, tree_t()))->second;
		}

		#else

		for (auto & entry: children){
			if (entry.first == key){
				return entry.second;
			}
		}
		// if (EXCLUSIVE)	this->clearData();
		children.push_back(pair_t(key, tree_t()));
		return children.back().second;

		#endif
	};


	virtual
	const tree_t & retrieveChild(const key_t & key) const {

		if (key.empty())
			return *this;

		#ifdef DRAIN_TREE_ORDERED
		const const_iterator it = children.find(key);
		if (it != children.end()){
			return it->second;
		}
		#else  // traverse
		for (const auto & entry: children){
			if (entry.first == key){
				return entry.second;
			}
		}
		#endif

		return getEmpty();
	};


	// Functions perhaps less relevant ....................................

	/// Returns the map containing the children.
	/**
	 *   This is useful for example for map::swap ?
	 */
	inline
	container_t & getChildren() { return children; };

	/// Returns the map containing the children.
	inline
	const container_t & getChildren() const { return children; };



	// New
	/// Fast access to data, applied widely in TreeXML (HTML/SVG)
	inline
	const node_data_t *operator->() const {
		return &data;
	};

	/// Fast access to data, applied widely in TreeXML (HTML/SVG)
	inline
	node_data_t *operator->(){
		return &data;
	};

	/// Replace children (but no data?)
	inline
	void swap(tree_t &t){
		children.swap(t.children);
	}

protected:

	container_t children;

	static
	const tree_t emptyNode;

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

	/// Returns a descendant. Creates one, if not present
	/**
	 */
	inline
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



	/// Given the descendant pointed to by a given path segment.
	/**
	 *  \return – descendant node, or the node itself if the path is empty.
	 */
	inline
	const tree_t & get(typename path_t::const_iterator it, typename path_t::const_iterator eit) const {

		// Path empty => self-reference
		if (eit == it) // order, 2023/04
			return *this;

		// Path element is empty => skip it (proceed to next element)
		if (it->empty())
			return get(++it, eit);

		// NEW ~faulty? data1/data1/data1/...

		if (hasChild(*it)){
			//return retrieveChild(*it); <- ADD <- .get(++it, eit); ?
			return retrieveChild(*it).get(++it, eit);
		}
		else {
			return getEmpty();
		}

		// OLD (limited to map container, reconsider NEW, above!)
		/*
		typename tree_t::const_iterator child_it = this->children.find(*it);
		if (child_it == this->children.end())
			return getEmpty();
		else
			return child_it->second.get(++it, eit);
		*/

		/*
		if (!hasChild(*it))
			return getEmpty();
		const tree_t & child = operator[](*it); // use find? Or better, direct child[key]
		//const tree_t & child = this->children[*it]; // use find? Or better, direct child[key] 2023/04

		return child.get(++it, eit);
		*/
	}



};

template <class T, bool EXCLUSIVE, class P>
const DRAIN_TREE_NAME<T,EXCLUSIVE, P> DRAIN_TREE_NAME<T,EXCLUSIVE,P>::emptyNode;



// Certified.
template <class T, bool EXCLUSIVE, class P>
struct TypeName<DRAIN_TREE_NAME<T,EXCLUSIVE, P> > {

	typedef DRAIN_TREE_NAME<T,EXCLUSIVE, P> tree_t;

    static
	const std::string & str(){

    	static const std::string name = drain::StringBuilder<>(
    			tree_t::isOrdered()?"Ordered":"Unordered",
    					tree_t::isMulti()?"Multi":"","Tree",
    							tree_t::isExclusive()?"(Exclusive)":"",
    									'<', TypeName<typename tree_t::node_data_t>::get(), '>'); // recursion...
    											//'<', drain::Type::call<drain::simpleName>(typeid(typename tree_t::node_data_t)), '>');
    	return name;
    }

    static
	const char* get(){
    	return str().c_str();
    };

};


} // drain::


