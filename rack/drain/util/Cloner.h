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
#include <set>
#include <exception>
#include <iostream>

namespace drain {

/**
 *   \tparam T - base class (ie not to be used for derived class)
 */
template <class T>
struct ClonerBase {

	ClonerBase(){};

	virtual
	~ClonerBase(){
	};


	virtual
	T & clone() = 0;  // could be const (if mutable ptrs )

	/* Consider separating these to "Provider" or "Factory" etc. */
	/* Consider operator(T) */

	/// Returns a reference to an instance, possibly a static one.
	virtual
	T & get() = 0;

	/// Returns a reference to an instance, possibly a static one.
	virtual
	const T & get() const = 0;

	// Return the number of cloned instances.
	virtual
	size_t count() = 0;

protected:



};


/// Wrapper for derived class S, returning base class T.
/**
 *  \tparam T  - visible base class
 *  \tparam S - internal wrapped class derived from T
 *
 */
template <class T, class S>
struct Cloner : public ClonerBase<T> {

	//typedef ClonerBase<T> clonerbase_t;
	Cloner(){};

	/// Copy constructor
	Cloner(const Cloner<T,S> & c) : src(c.src){
	};

	/// Constructor with source object (copied)
	Cloner(const S & s) : src(s){
	};

	virtual
	~Cloner(){
		//std::cerr << "Goodbye" << std::endl;
		//clear();
		for (typename ptr_container_t::iterator it=ptrs.begin(); it!= ptrs.end(); ++it){
			S *ptr = *it;
			//std::cout << "deleting " << *ptr << ", at #" << (long int)ptr << '\n';
			std::cout << "deleting at #" << (long int)ptr << '\n';
			delete ptr;
		}
	};

	/// Implements interface
	T & clone(){
		return cloneOrig();
	};

	S & cloneOrig(){
		S *p = new S(src);  // NOTE: parameterReferences must be done
		this->ptrs.insert(p); //push_front(p);
		return *p;
	};


	/* Consider separating these to "Provider" or "Factory" etc. */

	/// Returns a const reference to a default instance
	/*
	 *  The returned object is possibly static.
	 */
	virtual
	const T & get() const {
		return src;
	};

	/// Returns a reference to a default instance
	/*
	 *  The returned object is possibly static.
	 */
	virtual
	T & get() {
		return src;
	};

	virtual inline
	size_t count(){
		return ptrs.size();
	}

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

	/// Remove cloned entry
	inline
	void clear(S *ptr){

		const typename ptr_container_t::iterator it = ptrs.find(ptr);

		if (it != ptrs.end()){
			//std::cout << "deleting " << *ptr << ", at #" << (long int)ptr << '\n';
			//std::cout << *ptr  << '\n';
			std::cout << "deleting " << (long int)ptr << '\n';
			delete ptr;
			//ptrs.erase(it);
		}
		else {
			std::cout << "already deleted: " << (long int)ptr << '\n';
			// warn?
		}

	}


//protected:

	/// Default instance, also the source for cloning.
	//static S entry;  // Consider non-static
	S src;  // Consider non-static

protected:

	// Structure for storing pointers until the target objects should be deleted.
	//typedef std::list<T *> ptrlist_t;
	typedef std::set<S *> ptr_container_t;

	ptr_container_t ptrs;


};

//template <class T, class S>
//S Cloner<T,S>::entry;




}

#endif /* Cloner_H_ */

// Drain
