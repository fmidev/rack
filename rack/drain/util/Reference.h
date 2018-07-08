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

#ifndef REFERENCE
#define REFERENCE

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
		//std::cout << __FUNCTION__ << "Loponen "<< '\n';
		setPtr(p);
	}

	template <class F>
	inline
	void link(F *p){
		setPtr<F>(p);
	}


	/*
	inline
	void relink(const Castable & c){
		setPtr(c);
		std::cout << __FUNCTION__ << '\n';
		c.toJSON(std::cout);
		std::cout << '\n';
		toJSON(std::cout);
		std::cout << '\n';
	}
	*/

	inline
	void relink(Castable & c){
		setPtr(c);
		/*
		std::cout << __FUNCTION__ << '\n';
		c.toJSON(std::cout);
		std::cout << '\n';
		toJSON(std::cout);
		std::cout << '\n';
		*/
	}




	inline
	void link(const Referencer & r){
		setPtr((const Castable &)r);
	}

	inline
	void link(Referencer & r){
		relink(r);
	}

	/*
	inline
	void link(const Castable & c){
		setPtr((Castable &)c);
	}

	inline
	void link(Castable & c){
		setPtr((const Castable &)c);
	}

	inline
	void link(const Variable & r){
		setPtr((Castable &)r);
	}

	inline
	void link(Variable & r){
		setPtr((const Castable &)r);
	}
	*/


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

	/*
	inline
	Referencer & operator=(const Variable &x){
		//Castable::operator=(x);
		return this->operator=((const Castable &)x);
	}
	*/


};


}  // namespace drain


#endif
