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


#include "Convert.h"
#include "Variable.h"
#include "Sprinter.h"

#ifndef DRAIN_REFERENCE_VARIABLE_NEW
#define DRAIN_REFERENCE_VARIABLE_NEW

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

// TODO: finally, rename to Referencer

/// Extends Castable's interface with link()
/**
 *   \tparam F Castable or Variable
 */
template <class T>
class ReferenceBase : public T {

public:

//	typedef T var_base_t; // needed

protected:

	/// Default constructor.
	ReferenceBase(){};

	/// Copy constructor.
	/**
	 *  If T is Castable, pointer is copied.
	 *  If T is Variable, data and type are copied.
	 *
	 */
	ReferenceBase(const ReferenceBase<T> & r) : T(r) { // (const T &)r
	};

	/*
	Reference(Reference<T> & r) : Castable((Castable &)r) {
	};

	template <class F>
	inline
	Reference(F *p) : Castable(p){}


	template <class F>
	inline
	Reference(F & x) : Castable(x){}
	*/

	virtual inline
	~ReferenceBase(){};

public:


	/// Set pointer to &p.
	/**
	 *  \tparam T - target object (if Castable or Reference redirected to relink() )
	 */
	template <class F>
	inline
	ReferenceBase<T> & link(F &p){
		try {
			this->setPtr(p);
		}
		catch (const std::exception & e){
			std::cerr << __FILE__ << ':' << __FUNCTION__ << ": unsupported type: " << typeid(F).name() << std::endl;
			// throw e;
			throw std::runtime_error("unsupported type");
			//throw DrainException(__FILE__, ':', __FUNCTION__,": unsupported type: ", typeid(F).name());
		}
		return *this;
	}

	/// Set pointer to &p.
	/**
	 *  \tparam T - target object (if Castable or Reference redirected to relink() )
	 */
	template <class F>
	inline
	ReferenceBase & link(F *p){
		this->setPtr(p);
		return *this;
	}

	/// Set pointer to p, of given type.
	inline
	ReferenceBase & link(void *p, const std::type_info &t, size_t count=1){
		this->setPtr(p, t);
		this->elementCount = count;
		return *this;
	}

	inline
	ReferenceBase & link(void *p){
		throw std::runtime_error(std::string("ReferenceBase::") + __FUNCTION__ + ": void type unsupported");
		return *this;
	}


	// Copied
	inline
	ReferenceBase & link(Castable &x){
		this->relink(x);
		return *this;
	}

	// Copied
	inline
	ReferenceBase & link(Variable &x){
		this->relink(x);
		return *this;
	}

	// Copied
	template <class F>
	inline
	ReferenceBase & link(ReferenceBase<F> &x){
		this->relink(x);
		return *this;
	}


};

/*
template <>
struct TypeName<ReferenceBase<Castable> > {
    static const char* get(){ return "ReferenceBase<Castable>"; }
};

template <>
struct TypeName<ReferenceBase<Variable> > {
    static const char* get(){ return "ReferenceBase<Variable>"; }
};
*/

template <>
const char* TypeName<ReferenceBase<Castable> >::get();

template <>
const char* TypeName<ReferenceBase<Variable> >::get();



class Referencer : public ReferenceBase<Castable> {

public:

	inline
	Referencer(){
	}

	inline
	Referencer(const Referencer & v){
		// Note: neither ptr nor type is set.
	}

	/// Copy pointer and type of the referenced variable.
	/**
	 *  This is the "parasite mode".
	 *
	 */
	inline
	Referencer(Referencer & v){
		setPtr(v.getPtr(), v.getType());
	}

	template <class T>
	inline
	Referencer(T *p){
		setPtr(p);
	}


	template <class T>
	inline
	Referencer(T & x){
		setPtr(x);
	}

	/// Methods shared with Variable, Referencer, FlexibleVariable
	#define SmartVariable Referencer
	#include "SmartVariable.inc"
	#undef  SmartVariable


};

//typedef ReferenceVariable Referencer;

/// drain:: reflection support.
template <>
struct TypeName<Referencer> {
    static const char* get(){ return "Referencer"; }
};

/// drain:: default implementation for output formatting
template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::Referencer & v, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &) v, layout);
};

template <>
template <class D>
inline
void Convert2<Referencer>::convert(const Referencer &src, D & dst){
	dst = (const D &)src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}


template <>
inline
void Convert2<Referencer>::convert(const char *src, Referencer & dst){
	dst = src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}



template <>
template <class S>
inline
void Convert2<Referencer>::convert(const S &src, Referencer & dst){
	dst = src;
	std::cout << "CONV:" << __FILE__ << src << " -> " << dst << '\n';
}


}  // namespace drain


#endif
