/**

    Copyright 2016  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)
    Thanks to Mikael Kilpelainen


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

 */
#ifndef Cloner_H_
#define Cloner_H_

//#include <cmath>
#include <list>
#include <exception>

//#include "../util/Type.h"

// // using namespace std;

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
