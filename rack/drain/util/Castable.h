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

#include "Caster.h"

#ifndef CASTABLE
#define CASTABLE

// // using namespace std;

namespace drain {


//class Variable;

/// An object that can be automatically casted to and from a base type or std::string. Designed for objects returned by CastableIterator.
/** Contains type info (like Caster) and also a pointer to external object. The object ie. the memory resource is provided by the user.
    Does not support arrays. See Variable and ReferenceMap.

  \example Castable-example.cpp
   
  \see ReferenceMap
  \see Variable


 */
class Castable : protected Caster {

public:

	inline
	Castable() : ptr(NULL) {
                 unsetType();
              //setType<char>();
	};

	/// Copy constructor
	Castable(const Castable &c) : Caster() {
		//setType(c.getType());
		setPtr(c.ptr, c.getType());  // TODO LINK
	}

	// Obsolete?
	template <class F>
	Castable(F *p){
		setPtr(p,typeid(F));
		// std::cerr << "Castable(F *p) type=" << typeid(F).name() << " value=" << *p << " (" << (double)*p << ")" << std::endl;
	}

	/// Constructor for an object pointing to a variable.
	template <class F>
	Castable(F &p) : ptr(NULL){
		link(p);
	}

	inline
	void link(Castable & c){
		setPtr(c.ptr, c.getType());
	}

	template <class F>
	inline
	void link(F &p){
		setPtr(&p, typeid(F));
		// TODO: REFERENCE = true; ?
	}


	/* TODO:
	 	 resize(){
	 	 	 if (REFERENCE)
	 	 	 	 throw ...
	 	 }
	 */




	inline
	const std::type_info &getType() const { return *type; };

	inline
	const size_t & getByteSize() const { return byteSize; };


	/// Copies the value referred to by Castable. Notice that if type is unset (void), no operation is performed.
	inline
	Castable &operator=(const Castable &x){
		// std::cerr << "Castable & Castable::operator=(const Castable &)" << std::endl;
		(this->*castP)(x, x.ptr, ptr);
		return *this;
	}

	/*
	inline
	Castable &operator=(const Variable &x){
		// std::cerr << "Castable & Castable::operator=(const Castable &)" << std::endl;
		//(this->*castP)(x, x.ptr, ptr);
		const Castable &c = x;
		return (*this).operator=(c);
	}
	*/


	/// Copies an arbitrary base type or std::string value.
	template <class T>
	inline
	Castable &operator=(const T &x){
		put(ptr, x);
		return *this;
	}


	/// Conversion from the internal type.
	/*
	inline
	operator std::string() const {
		return get<string>(ptr);
	}
	*/


	/// Conversion from the internal type.
	template <class T>
	inline
	operator T() const {
		return get<T>(ptr);
	}


	inline
	bool operator==(const Castable &x){
		//(this->*putCastable)(x);
		throw std::runtime_error("Castable: operator== not implemented.");
		return false;
	}

	/// Compares a value to internal data.
	template <class T>
	bool operator==(const T &x){
		return (get<T>(ptr) == x);
	}

	/// Compares a value to inner data.
	/**
	 *
	 */
	template <class T>
	bool operator!=(const T &x){
		return (get<T>(ptr) != x);
	}

	/// Compares a value to inner data.
	template <class T>
	bool operator<(const T &x){
		return (get<T>(ptr) < x);
	}

	/// Compares a value with inner data.
	template <class T>
	bool operator>(const T &x){
		return (get<T>(ptr) > x);
	}


	inline
	std::ostream & toOStream(std::ostream &ostr) const {
		return (this->*toOStreamP)(ostr, ptr);
	}

	inline
	std::string toStr() const {
		return get<std::string>(ptr);
	}


	/// Clears std::strings or sets numeric values to undetectValue.
	inline
	void clear(){
		if (typeIsSet()){
			if (getType() == typeid(std::string))
				put(ptr, "");
			else
				put(ptr, 0);
		}
	}



protected:


	/// Sets the storage type. If a target value is available, use link() directly.
	template <class F>
	inline
	void setType(){ Caster::setType<F>(); }


	/// Sets the storage type. If a target value is available, use link() directly.
    inline
	void setType(const std::type_info &t){
	    Caster::setType(t);
	}

	template <class F>
	inline
	void setPtr(void *p){
		setPtr(p,typeid(F));
	}

	/// Sets the data pointer. Does not change type. (Cannot, because of void *.)
	/*
	 *  Function of this kind must be available for general (8bit) memory allocators.
	 */
	inline
	void setPtr(void *p, const std::type_info &t){
		ptr = p;
		setType(t);
	}


	/// Pointer to the internal variable.
	void *ptr;

	//void (Castable::* putCastable)(const Castable &x);

	// This is a trick.
	/*
	template  <class F>
	inline
	void _putCastable(const Castable & x){
	  *(F*)ptr = (const F)x;
		// new = x.get<F>(x->ptr);
	}
	*/

	/*
	inline
	void _putCastableString(const Castable & x){
		//   *(std::string*)ptr = Data(x);
		*(std::string*)ptr = (const std::string &)x; // TODO?
	}
	*/



};


  /*
template <>
void Castable::setType<string>();


template <class F>
void Castable::setType(){
	//std::cerr << "Warning:  setType() deprecating, use setPtr\n";
	Caster::setType<F>();
	// if (typeIsSet()) VOID
		putCastable = & Castable::_putCastable<F>;
	//toOStreamP  = & Castable::_toOStreamP<F>;
}
  */


inline
std::ostream & operator<<(std::ostream &ostr, const Castable &x){
	return x.toOStream(ostr);
}



}  // namespace drain


#endif

// Drain
