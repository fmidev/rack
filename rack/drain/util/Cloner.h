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

#include <list>
#include <exception>


namespace drain {

/**
 *   \tparam T - base class (ie not to be used for derived class)
 */
template <class T>
struct ClonerBase {

	ClonerBase(){};

	virtual
	~ClonerBase(){
		for (typename ptrlist_t::iterator it=ptrs.begin(); it!= ptrs.end(); ++it){
			delete *it;
		}
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


protected:

	// Structure for storing pointers until the target objects should be deleted.
	typedef std::list<T *> ptrlist_t;

	std::list<T *> ptrs;

};


/// Wrapper for derived class T2, returning base class T.
/**
 *  \tparam T  - visible base class
 *  \tparam T2 - internal, wrapped class (not accessible as T2&)
 */
template <class T, class T2>
struct Cloner : public ClonerBase<T> {

	//typedef ClonerBase<T> clonerbase_t;

	virtual
	~Cloner(){};

	T & clone(){
		T *p = new T2(entry);  // NOTE: parameterReferences must be done
		this->ptrs.push_front(p);
		return *p;
	};


	/* Consider separating these to "Provider" or "Factory" etc. */

	/// Returns a const reference to a default instance
	/*
	 *  The returned object is possibly static.
	 */
	virtual
	const T & get() const {
		return entry;
	};

	/// Returns a reference to a default instance
	/*
	 *  The returned object is possibly static.
	 */
	virtual
	T & get() {
		return entry;
	};

protected:

	/// Default instance, also the source for cloning.
	static T2 entry;  // Consider non-static


};

template <class T, class T2>
T2 Cloner<T,T2>::entry;




}

#endif /* Cloner_H_ */

// Drain
