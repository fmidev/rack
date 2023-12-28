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


//#include "Variable.h"
#include "Referencer.h"
#include "SmartMap.h"
#include "Sprinter.h"


#ifndef DRAIN_FLEXIBLE_VARIABLE
#define DRAIN_FLEXIBLE_VARIABLE

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

/** Combination of Variable and ReferenceVariable: the target can be internal or external (reference).
 *
 */
class FlexibleVariable : public ReferenceBase<Variable> {

public:

	inline
	FlexibleVariable(){
	}

	inline
	FlexibleVariable(const FlexibleVariable & v){
		assignCastable(v);
	}

	template<typename T>
	inline
	FlexibleVariable(const T & x){
		assign(x);
	}

	/// Assignment of C strings. [Obligatory]
	/**
	 *   STL strings will be handled by the default operator operator=(const T &x) .
	 */
	inline
	FlexibleVariable(const char *x){
		assign(x);
	}


	template<typename T>
	inline
	FlexibleVariable(T *p){
		this->setPtr(p);
	}


	template<typename T>
	inline
	FlexibleVariable(std::initializer_list<T> l, const std::type_info &t = typeid(void)){
		//std::cerr << __FILE__ << ": Check " << __FUNCTION__ << '\n';
		//reset();
		setType(t);
		assignContainer(l, true);
	};

	template <class T, size_t N>
	inline
	FlexibleVariable(const UniTuple<T,N> & unituple){
		assignContainer(unituple);
	}

	inline
	~FlexibleVariable(){
	}



#define SmartVariable FlexibleVariable
#include "SmartVariable.inc"
#undef  SmartVariable

	/*
	inline
	FlexibleVariable & operator=(const FlexibleVariable &x){
		assignCastable(x);
		return *this;
	}

	inline
	FlexibleVariable & operator=(const Castable &x){
		assignCastable(x);
		return *this;
	}

	inline
	FlexibleVariable & operator=(const Variable &x){
		assignCastable(x);
		return *this;
	}

	template <class T, size_t N>
	inline
	FlexibleVariable &operator=(const UniTuple<T,N> & unituple){
		assignContainer(unituple);
		return *this;
	}

	template <class T>
	inline
	FlexibleVariable & operator=(const T &x){
		assign(x);
		return *this;
	}

	inline
	FlexibleVariable & operator=(const char *x){
		assign(x);
		return *this;
	}

	template<typename T>
	inline
	FlexibleVariable & operator=(const std::initializer_list<T> & l){
		assignContainer(l, false); // don't append
		return *this;
	};

*/

	/*
	inline
	FlexibleVariable & operator=(const ReferenceVariable &x){
		assignCastable(x);
		return *this;
	}
	*/

	// Deprecating?
	/*
	inline
	FlexibleVariable & operator=(const Referencer &x){
		assignCastable(x);
		return *this;
	}
	*/


	/// Tries to change type with requestType. Throws exception if not possible.
	/**
	 *  \param t - new type
	 *
	 *  \see requestType()
	 *  \see Variable::suggestType()
	 */
	virtual inline
	void setType(const std::type_info & t){
		if (!requestType(t)){
			throw std::runtime_error(std::string("FlexibleVariable::") + __FUNCTION__ + ": illegal for referenced variable");
		}
	}


	/// Changes type, if possible. See also suggestType()
	/**
	 *  \param t - new type
	 *  \return - true if requested type obtained
	 *
	 *  \see Castable::suggestType()
	 */
	virtual inline
	bool requestType(const std::type_info & t){

		if (!isReference()){ // is in Variable mode
			Variable::setType(t);
			return true;
		}
		else { // is in Referencer mode (points to external array)
			return (getType() == t);
		}
	}

	/// Extends the vector to include n elements of the current type.
	virtual inline
	bool setSize(size_t elementCount){
		if (!isReference()){ // is in Variable mode
			updateSize(elementCount);
			return true;
		}
		else {
			if (getType() == typeid(drain::Variable)){
				std::cerr << "FlexibleVariable::" << __FUNCTION__ << " warning: resize for linked Variable not yet implemented " << std::endl;
			}
			if (elementCount != getElementCount()){
				throw std::runtime_error(std::string("FlexibleVariable::") + __FUNCTION__ + ": illegal for referenced variable");
				// = return false;
			}
			return true;
		}
	}

	virtual inline
	void info(std::ostream & ostr = std::cout) const {
		Castable::info(ostr);
		if (isReference())
			ostr << " <reference>";
		else
			ostr << " <own>";
	}



protected:

	/// Returns true, if the internal pointer directs to external data object, ie. internal data vector is not used.
	/**
	 *  (Derived classes, like FlexibleVariable, may alternatively point to external data array.)
	 *
	 *  \return - true, if the internal pointer directs to external data object
	 *
	 */
	inline
	bool isReference() const {
		return (caster.ptr != (void *) &data[0]);
	}

	/// Request to change the array size. For Castable (and Reference) does nothing and returns false.
	/**
	 *
	 */
	virtual inline
	bool requestSize(size_t elementCount){
		if (isReference()){
			return Castable::requestSize(elementCount);
		}
		else {
			return Variable::requestSize(elementCount);
		}
	}


};

template <>
struct TypeName<FlexibleVariable> {
    static const char* get(){ return "FlexibleVariable"; }
};


template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::FlexibleVariable & v, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &) v, layout);
};





}  // namespace drain


#endif

// Drain
