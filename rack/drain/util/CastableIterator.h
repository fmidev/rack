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
		Castable::setType(type);
	};

	inline
	const std::type_info &getType() const {
		return Castable::getType();
	};

	inline
	const size_t & getByteSize() const {
		return byteSize;
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
		ptr = (cptr + byteSize);
		return *this;
	};

	/// Not recommended, use prefix operator++() instead.
	inline
	CastableIterator operator++(int){
		CastableIterator tmp = *this;
		char *cptr = (char *)ptr;
		ptr = (cptr + byteSize);
		return tmp;
	};

	inline
	CastableIterator & operator--(){
		char *cptr = (char *)ptr;
		ptr = (cptr - byteSize);
		return *this;
	};

	/// Not recommended, use prefix operator--() instead.
	inline
	CastableIterator operator--(int){
		CastableIterator tmp = *this;
		char *cptr = (char *)ptr;
		ptr = (cptr - byteSize);
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


// Drain
