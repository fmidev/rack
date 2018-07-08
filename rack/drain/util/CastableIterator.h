/**

    Copyright 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>

#include "Castable.h"

#ifndef CASTABLE_ITERATOR
#define CASTABLE_ITERATOR

// // using namespace std;

namespace drain {



class CastableIterator;

/// Implements iterator for Castable.
/**
 *   properties (++,--,*) for a
 *
 *   \example CastableIterator-example.cpp
 */
class CastableIterator : protected Castable { //private Castable {

public:

	CastableIterator(){
		setPtr(NULL, typeid(void));
	}

	CastableIterator(const CastableIterator &it){
		setPtr(it.ptr, it.getType());
	}

	CastableIterator(void *p, const std::type_info & t){
		setPtr(p, t);
	}


	template <class T>
	CastableIterator(T *p) : Castable(p) {
		//std::cerr << "CastableIterator(T *p), T:"<< typeid(T).name() << "\n";
	}



	CastableIterator(std::string *p){
		throw std::runtime_error("CastableIterator(std::string *p): std::string iterators not implemented");
	}



	CastableIterator(void *p){
		throw std::runtime_error("CastableIterator(void *p): void iterators not allowed");
	}

	/// Moves iterator to address pointed by &it. Changes the internal type accordingly.
	inline
	CastableIterator & operator=(const CastableIterator &it){
		// std::cerr << "CastableIterator operator=(const & CastableIterator) \n";
		setPtr(it.ptr, it.getType());
		return *this;
	}


	// Problematic. Compare with (void *p).
	template <class T>
	inline
	CastableIterator & operator=(T *p){
		//std::cerr << "CastableIterator::operator=, type:"<< typeid(T).name() << "\n";
		setPtr(p, typeid(T));
		return *this;
	}


	///  Moves iterator to address *p.
	inline
	CastableIterator & operator=(void *p){
		//std::cerr << "CastableIterator::operator=(void *):\n";
		//setPtr(p,getType());
		ptr = p;
		return *this;
	}


	inline
	void setType(const std::type_info &type){
		caster.setType(type);
	};

	inline
	const std::type_info &getType() const {
		return caster.getType();
	};

	inline
	size_t getByteSize() const {
		return caster.getByteSize();
	};


	inline
	bool operator==(const CastableIterator &it) const {
		return (ptr == it.ptr);
	};

	inline
	bool operator!=(const CastableIterator &it) const {
		return (ptr != it.ptr);
	};

	inline
	bool operator<(const CastableIterator &it) const {
		return (ptr < it.ptr);
	};

	inline
	bool operator>(const CastableIterator &it) const {
		return (ptr > it.ptr);
	};

	inline
	CastableIterator & operator++(){
		char *cptr = (char *)ptr;
		ptr = (cptr + caster.getByteSize());
		return *this;
	};

	/// Not recommended, use prefix operator++() instead.
	inline
	CastableIterator operator++(int){
		CastableIterator tmp = *this;
		char *cptr = (char *)ptr;
		ptr = (cptr + caster.getByteSize());
		return tmp;
	};

	inline
	CastableIterator & operator--(){
		char *cptr = (char *)ptr;
		ptr = (cptr - caster.getByteSize());
		return *this;
	};

	/// Not recommended, use prefix operator--() instead.
	inline
	CastableIterator operator--(int){
		CastableIterator tmp = *this;
		char *cptr = (char *)ptr;
		ptr = (cptr - caster.getByteSize());
		return tmp;
	};


	inline
	const Castable & operator*() const {
		return (const Castable &) *this;
	};


	inline
	Castable & operator*(){
		return (Castable &)*this;
	};

	inline
	operator const void*() const {
		return ptr;
	};

	inline
	operator long() const {
		return (long)ptr;
	};

	/*
	inline
	std::ostream & toOstr(std::ostream & ostr) const {
		ostr << (long)ptr;
		return ostr;
	};
	*/


	//CastableIterator & operator=(const T *p){


	/// Utility for casting ptr to a number (~ long int) for debugging.
	//  void * could be better.
	template <class T>
	operator T() const {
		return (T)(ptr);
	}


};

inline
std::ostream & operator<<(std::ostream & ostr, const CastableIterator & it){
	ostr << (long)it;
	return ostr;
}
/*
class CastableIterator : public CastableIteratorBase {

public:

	template <class T>
	CastableIterator(T *p) : CastableIteratorBase(p) {
	}


	inline
	Castable & operator*(){
		return (Castable &)*this;
	};


};
*/

/*
class CastableConstIterator : public CastableIteratorBase {

public:

	template <class T>
	CastableConstIterator(T *p) : CastableIteratorBase(p) {
	}

};
*/


}  // namespace drain


#endif

