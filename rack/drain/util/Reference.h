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

#ifndef REFERENCE2
#define REFERENCE2

// // using namespace std;

namespace drain {


//class Variable;

/// An object that can be automatically casted to and from a base type or std::string. Designed for objects returned by CastableIterator.
/**

  Saves type info (like Caster) and also a pointer to external object.
  The object ie. the memory resource is provided by the user.
  Supports arrays.

  \example Castable-example.cpp
   
  \see ReferenceMap
  \see Variable


 */

//class Variable;

/// Extends Castable's interface with link()
class Referencer : public Castable {

public:

	/// Default constructor.
	Referencer(){};

	/// Copy constructor.
	Referencer(const Referencer & r) : Castable((const Castable &)r) {
	};

	// Needed?
	Referencer(Referencer & r) : Castable((Castable &)r) {
	};

	template <class T>
	inline
	Referencer(T *p) : Castable(p){}


	template <class T>
	inline
	Referencer(T & x) : Castable(x){}

	virtual inline
	~Referencer(){};

	/// Set pointer to &p.
	/**
	 *  \tparam T - target object (if Castable or Referencer redirected to relink() )
	 */
	template <class F>
	inline
	void link(F &p){
		setPtr(p);
	}

	/// Set pointer to &p.
	/**
	 *  \tparam T - target object (if Castable or Referencer redirected to relink() )
	 */
	template <class F>
	inline
	void link(F *p){
		setPtr(p);
	}

	/// NEW. Set pointer to p, of given type.
	inline
	void link(void *p, const std::type_info &t){
		setPtr(p, t);
	}


	/// Explicit linking for Castable class(es).
	/**
	 *   Essentially, makes Castable::relink visible.
	 */
	inline
	void relink(Castable & c){
		Castable::relink(c);
	}

	template <class T>
	inline
	Referencer & operator=(const T &x){
		//std::cout << "Yleis\n";
		Castable::operator=(x);
		return *this;
	}

	inline
	Referencer & operator=(const Castable &x){
		//std::cout << "Kasta\n";
		Castable::operator=(x);
		return *this;
	}



};

template <>
inline
void Referencer::link<void>(void *p){
	throw std::runtime_error(std::string("Referencer::") + __FUNCTION__ + ": void type unsupported");
}

template <>
inline
void Referencer::link<Castable>(Castable &c){
	// Warning removed, because link(T) is handy for ReferenceMap::append()
	// std::cerr << "Referencer::" << __FUNCTION__ << "(): deprecating, use relink() for this type" << std::endl;
	relink(c);
}

template <>
inline
void Referencer::link<Referencer>(Referencer &r){
	// Warning removed, because link(T) is handy for ReferenceMap::append()
	// std::cerr << "Referencer::" << __FUNCTION__ << "(): deprecating, use relink() for this type" << std::endl;
	relink(r);
}

template <>
inline
std::ostream & JSONwriter::toStream(const Referencer & v, std::ostream &ostr, unsigned short indentation){
	return JSONwriter::toStream((const Castable &) v, ostr, indentation);
}


}  // namespace drain


#endif

// Drain
