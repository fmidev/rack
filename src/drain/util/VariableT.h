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



#ifndef DRAIN_VARIABLE_T
#define DRAIN_VARIABLE_T

//#include "Castable.h"
//#include "CastableIterator.h"
//#include "Convert.h"

//#include "Referencer.h"
/*
#include "Sprinter.h"
#include "String.h"
#include "UniTuple.h" // "Friend class"
#include "Type.h"
*/
#include "UniTuple.h" // "Friend class"

namespace drain {


/// "Final" class, to be applied through typedefs
template <class T>
class VariableT : public T {

public:

	typedef T varbase_t;

	typedef std::pair<const char *,const drain::VariableT<T> > init_pair_t;

	/// Single constructor template, forwarded to init(args...) defined in base classes.
	/**
	 *   This is the basic design pattern for all the VariableLikes
	 */
	template <class ...TT>
	inline
	VariableT(const TT & ...args) {
		this->init(args...); // invokes  T::init(...)
	}

	/// Single constructor template, forwarded to init(arg).
	/**
	 *   For a Reference,
	 */
	template <class D>
	inline
	VariableT(D & arg) {
		this->init(arg); // invokes  T::init(...)
	}


	/// Sets or changes the type if possible - that is, object is not a Reference.
	/**
	 */
	virtual
	bool requestType(const std::type_info & t);


	/// Sets type, only if unset, and object is not a Reference.
	/**
	 *
	 *   Does not change the current type, if set.
	 *
	 *   Could be protected, but requestType() will not be, so for consistency this is also public.
	 */
	virtual
	bool suggestType(const std::type_info & t);


	/// Change the array size, if  For Castable (and Reference) does nothing and returns false.
	/**
	 *
	 *
	 *   Does not apply to std::string .
	 *
	 *   Could be protected, but requestType() will not be, so for consistency this is also public.
	 *
	 *   \return - \c true , if final the element count is what was requested.
	 *
	 */
	virtual
	bool requestSize(size_t elementCount);

	/// Assignment of objects of the same class.
	/**
	 *
	 */
	inline
	VariableT & operator=(const VariableT<T> &x){
		this->assignCastable(x);
		return *this;
	}

	/// Assignment of base classes (VariableLike or Castable)
	inline
	VariableT & operator=(const T &x){
		this->assignCastable(x);
		return *this;
	}

	/// General assignment operator.
	template <class T2>
	inline
	VariableT & operator=(const T2 &x){
		this->assign(x);
		return *this;
	}

	/// General assignment operator.
	template <class T2>
	inline
	VariableT & operator=(std::initializer_list<T2> l){
		this->assignContainer(l, false);
		return *this;
	}

	/// Assignment of C strings. [Obligatory]
	/**
	 *   Assignment from char * and std::string splits the input by separator.
	 *
	 *   STL strings will be handled by the default operator operator=(const T &x) .
	 */
	inline
	VariableT & operator=(const char *x){
		this->assign(x);
		return *this;
	}

	// Could be removed ? Handled by assign<T>() ?
	template <class T2, size_t N>
	inline
	VariableT &operator=(const UniTuple<T2,N> & unituple){
		this->assignContainer(unituple);
		return *this;
	}

	// Default
	inline
	void info(std::ostream & ostr = std::cout) const {
		Castable::info(ostr);
	}

};



}

#endif
