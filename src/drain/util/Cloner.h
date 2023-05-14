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
#ifndef Cloner_H_
#define Cloner_H_

//#include <list>
#include <map>
#include <exception>
#include <iostream>
#include <sstream>

namespace drain {

/**
 *   \tparam T - base class (ie not to be used for derived class)
 */
template <class T>
struct ClonerBase {

	/// Each cloned entry has an index.
	/**
	 *  User of the cloned object can use it for deleting the object before the automatical destruction takes place
	 */
	typedef size_t index_t;

	// If
	typedef std::pair<index_t, T*> entry_t;

	ClonerBase(){};

	virtual
	~ClonerBase(){
	};


	virtual
	T & getCloned() const = 0;  // could be const (if mutable ptrs )

	virtual
	entry_t getClonerEntry() const = 0;


	/// Returns an instance at index i.
	// virtual const T & getCloned(index_t i) const = 0;

	/// Returns an instance at index i.
	virtual
	T & getCloned(index_t i) const = 0;


	/// Returns a reference to an instance (possibly a static one?).
	virtual
	const T & getSource() const = 0;

	/// Returns a reference to an instance.
	virtual
	T & getSource() = 0;



	// Return the number of cloned instances.
	virtual
	size_t count() const = 0;

	/// Remove cloned entry (experimental)
	/**
	 *  \return - true, if intance was found and succesfully deleted, false otherwise.
	 */
	virtual // clear
	bool drop(index_t i) const = 0;

	//experimental
	// virtual	bool dropUnique() = 0;

	protected:



};


/// Wrapper for derived class S, returning base class T.
/**
 *  \tparam T  - visible base class
 *  \tparam S - internal wrapped class derived from T
 *
 */
template <class B, class S>
struct Cloner : public ClonerBase<B> {

	typedef ClonerBase<B> clonerbase_t;

	// "inherit" a type
	typedef typename clonerbase_t::index_t index_t;

	// "inherit" a type
	typedef typename clonerbase_t::entry_t entry_t;

	typedef std::map<index_t,S *> ptr_container_t;


	Cloner(){};

	/// Copy constructor
	Cloner(const Cloner<B,S> & c) : src(c.src){
	};

	/// Constructor with source object (copied)
	Cloner(const S & s) : src(s){
	};

	virtual
	~Cloner(){
		for (typename ptr_container_t::iterator it=ptrs.begin(); it!= ptrs.end(); ++it){
			//std::cerr << "deleting instance, id=" << it->first << '\n';
			delete it->second;
		}
	};

	/// Implements interface
	B & getCloned() const {
		return getClonedSrc();
	};


	/// Returns
	/*
	 *
	virtual
	const B & getCloned(index_t i) const {
		typename ptr_container_t::const_iterator it = ptrs.find(i);
		if (it == ptrs.end()){
			std::stringstream sstr;
			sstr << __FILE__ << ':' << __FUNCTION__ << ": index not found: "<< i;
			throw std::runtime_error(sstr.str());
		}
		return *it->second;
	};
	*/

	/// Returns a reference to the instance with index i.
	/*
	 *  \param i - index (id) of the instance.
	 */
	virtual
	B & getCloned(index_t i) const {
		typename ptr_container_t::iterator it = ptrs.find(i);
		if (it == ptrs.end()){
			std::stringstream sstr;
			sstr << __FILE__ << ':' << __FUNCTION__ << ": index not found: "<< i;
			throw std::runtime_error(sstr.str());
		}
		return *it->second;
	};


	/// Returns a object of class S, initialized with the source.
	S & getClonedSrc() const{
		S *ptr = new S(src);  // NOTE: parameterReferences must be done
		//this->ptrs.insert(ptr); //push_front(p);
		this->ptrs[getNewIndex()] = ptr;
		return *ptr;
	};


	virtual
	entry_t getClonerEntry() const {
		S *ptr = new S(src);  // NOTE: parameterReferences must be done
		index_t i = getNewIndex();
		this->ptrs[i] = ptr; //.insert(ptr); //push_front(p);
		return entry_t(i, ptr);
	};


	/* Consider separating these to "Provider" or "Factory" etc. */

	/// Returns a const reference to a default instance
	/*
	 *  The returned object is possibly static.
	 */
	virtual
	const B & getSource() const {
		return getSourceOrig();
	};

	/// Returns a reference to a default instance
	/*
	 *  The returned object is possibly static.
	 */
	virtual
	B & getSource() {
		return getSourceOrig();
	};

	/// Returns a const reference to a default instance, in actual class.
	/*
	 *  The returned object is possibly static.
	 */
	virtual
	const S & getSourceOrig() const {
		return src;
	};

	/// Returns a reference to a default instance, in actual class
	/*
	 *  The returned object is possibly static.
	 */
	virtual
	S & getSourceOrig() {
		return src;
	};


	virtual inline
	size_t count() const {
		return ptrs.size();
	}


	/// Remove cloned entry
	virtual inline  // clear
	bool drop(index_t i) const {

		const typename ptr_container_t::iterator it = ptrs.find(i);

		if (it != ptrs.end()){
			// std::cout << "deleting " << *ptr << ", at #" << (long int)ptr << '\n';
			// std::cout << "deleting " << (long int)ptr << '\n';
			delete it->second;
			ptrs.erase(it);  // TODO!
			return true;
		}
		else {
			std::cerr << "already deleted or does not exist: " << i << '\n';
			// warn?
			return false;
		}

	}


	//protected:

	/// Default instance, also the source for cloning.
	//static S entry;  // Consider non-static
	S src;  // Consider non-static

	/// Remove cloned entries
	/*
	inline
	void clear(){
		for (typename ptr_container_t::iterator it=ptrs.begin(); it!= ptrs.end(); ++it){
			S *ptr = *it;
			std::cout << "deleting " << *ptr << ", at #" << (long int)ptr << '\n';
			delete ptr;
		}
		ptrs.clear();
	}
	*/



protected:

	// TODO: re-allocate empty slots
	inline
	index_t getNewIndex() const {
		// pra gma omp critical
		{
			if (this->ptrs.empty())
				return 1;
			else {
				// std::cerr << "getNewIndex: " << this->ptrs.rbegin()->first << '\n';
				return this->ptrs.rbegin()->first + 1;
			}
		}
	}

	// Structure for storing pointers until the target objects should be deleted.
	mutable
	ptr_container_t ptrs;


};




}

#endif /* Cloner_H_ */

// Drain
