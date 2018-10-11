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
  Does not support arrays. See Variable and ReferenceMap.

  \example Castable-example.cpp
   
  \see ReferenceMap
  \see Variable


 */

class Variable;

/// Extends Castable's interface with link()
class Referencer : public Castable {

public:

	Referencer(){};

	//Referencer(Referencer & r) {};
	template <class T>
	inline
	Referencer(T *p) : Castable(p){}


	template <class T>
	inline
	Referencer(T & x) : Castable(x){}

	virtual inline
	~Referencer(){};


	template <class F>
	inline
	void link(F &p){
		setPtr(p);
	}

	template <class F>
	inline
	void link(F *p){
		setPtr<F>(p);
	}

	inline
	void link(const Castable & c){
		setPtr(c);
	}

	inline
	void link(const Referencer & r){
		setPtr((const Castable &)r);
	}

	inline
	void link(Referencer & r){
		setPtr((const Castable &)r);
	}


	template <class T>
	inline
	Referencer & operator=(const T &x){
		Castable::operator=(x);
		return *this;
	}

	inline
	Referencer & operator=(const Castable &x){
		Castable::operator=(x);
		return *this;
	}



};


}  // namespace drain


#endif

// Drain
