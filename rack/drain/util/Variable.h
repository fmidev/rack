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

#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <string.h>
#include <string>
#include <vector>
#include <map>

#include "String.h"
//#include "Castable.h"
#include "CastableIterator.h"
//#include "MultiCastable.h"
#include "Reference.h"


#include "SmartMap.h"

// // // // using namespace std;

namespace drain {

class Variable;
class Referencer;

//std::ostream & operator<<(std::ostream &ostr, const Variable &x);

/// An object for storing scalars, vectors and std::string using automatic type conversions.
/**
 *   Variable designed for storing values with automatic forward and backward conversion.
 *   Internally, Variable uses a byte vector which is dynamically resized.
 *   The data can be a single scalar value, an array, or a string.
 *
 *   Internally, a string is implemented as a vector of chars. This is different from
 *   drain::Castable with type std::string which assumes that the object behind (void *)
 *   is actually an instance of std::string.
 *
 *   The following examples illustrates its usage:
 *   \code
 *   Variable vField;
 *   vField = 123.456;  // First assignment sets the type implicitly (here double).
 *   int i = vField;    // Automatic cast
 *   \endcode
 *
 *  The type of a Variable can be set explicitly:
 *   \code
 *   Variable vField;
 *   vField.setType<std::string>();
 *   vField = 123.456; // Stored internally as char array "123.456"
 *   int i = vField;   // Automatic cast
 *   \endcode
 *
 *  Any number of scalars can be appended to a Variable, which hence becomes a vector:
 *   \code
 *   Variable vField;
 *   vField.setType<double>();
 *   vField << 1.0 << 123.456 << 9.999;
 *   std::cout << vField << std::endl;  // dumps 1.0,123.456,9.999
 *   \endcode
 *
 *  If the type is unset, Variable converts to undetectValue or an empty std::string.
 *   \code
 *   Variable vField;
 *   double d = vField; // becomes 0.0
 *   std::string s = vField; // becomes ""
 *   \endcode
 *
 *  Unlike with drain::Castable, the pointer contained by Variable is not visible or changeable by the user.
 *
 *  Note: tried skipping Castable, inheriting directly from Caster but needs Castable for cooperation with ReferenceMap.
 *
 *
 *   \example Variable-example.cpp
 *
 */
class Variable : public Castable {
public:

	/// Default constructor generates an empty array.
	Variable(const std::type_info &t = typeid(void)) {
		reset();
		setType(t);
	};

	/// Copies type, data and separator char.
	Variable(const Variable & v) {
		reset();
		this->outputSeparator = v.outputSeparator;
		this->inputSeparator = v.inputSeparator;
		assignCastable(v);
	};

	/// Copies type, data and separator char.
	Variable(const Castable & c) {
		reset();
		assignCastable(c);
	};

	/// Copies type, data and separator char.
	Variable(const char * s) {
		reset();
		assignString(s);
	};

	/// Copies type, data and separator char. Fails with Reference?
	template <class T>
	Variable(const T & value, const std::type_info &t = typeid(void)) {
		reset();
		setType(t);
		*this = value;
	}

	virtual ~Variable(){};

	/// Does not change separator chars.
	inline
	void reset(){
		caster.unsetType();
		//resize(0);
		updateSize(0);
		setSeparator(',');
		//this->separator = ','; // semantics ?
	}

	/// Sets basic type or std::string, or void.
	/**
	 *   Note: request of std::string is handled as request of \c char[]
	 */
	virtual inline
	void setType(const std::type_info & t){ //, size_t n = 0

		reset(); // what if requested type already?

		if ((t == typeid(std::string)) || (t == typeid(char *))){
			//std::cerr << "Variable::setType (std::string) forward, OK\n";
			caster.setType(typeid(char));
			// setOutputSeparator(0); NEW 2019/11
			setOutputSeparator(0);
			//resize(1);
			updateSize(1);
			//caster.put(ptr, '\0');
			caster.put('\0');
		}
		else {
			if (t != typeid(void)){  // why not unset type
				caster.setType(t);
				// IS_STRING = false;
				//resize(n);
			}
			else {
				caster.setType(t); // unset
			}
		}
	}

	/// Changes type by calling setType() directly. Always supported for Variable.
	/**
	 *  \param t - new type
	 *  \return - always true
	 *
	 *  \see suggestType()
	 *  \see setType()
	 */
	virtual inline
	bool requestType(const std::type_info & t){
		Variable::setType(t);
		return true;
	}

	/**
	 *
	 *  Semantics: should the type change always?
	 */
	inline
	Variable &operator=(const Variable &v){
		assignCastable(v);
		return *this;
	}


	inline
	Variable &operator=(const Referencer &r){
		assignCastable(r);
		return *this;
	}

	/* Handler by template, below
	inline
	Variable &operator=(const Castable &c){
		assignCastable(c);
		return *this;
	}
	*/

	/// Assignment from basic types and std::string.
	template <class T>
	inline
	Variable & operator=(const T &x){
		Castable::operator=(x);
		return *this;
	}


	/// Assignment from char * and std::string splits the input by separator.
	inline
	Variable &operator=(const char *s){
		Castable::operator=(s);
		return *this;
	}


	/// Extends the array by one element.
	inline
	Variable &operator<<(const char *s){
		Castable::operator<<(s);
		return *this;
	}

	/// Extends the array by one element.
	/*
	 *  Note: if a std::string is given, does not split it like with Variable::operator=(const char *).
	 */
	template <class T>
	inline
	Variable &operator<<(const T &x){
		Castable::operator<<(x);
		return *this;
	}


	/// Conversion to Castable.
	/** Important when str Castable wants to assing this, preserving the precision in floating point numbers for example.
	 *
	 */
	inline
	operator const Castable &() const {
		std::cerr << "operator const Castable, value=" << *this << std::endl;
		return *this;
	}



	/// Extends the array to include \c elementCount elements of current type.
	virtual inline
	bool setSize(size_t elementCount){
		return updateSize(elementCount);
	}


	/// Like with std::iterator.
	/**
	 *   Notice: in the case of std::strings, will iterate through characters.
	 */
	inline
	const CastableIterator & begin() const { return dataBegin; };

	/// Like with std::iterator.
	/*
	 *  Notice: in the case of std::strings, will iterate through characters.
	 */
	inline
	const CastableIterator & end() const { return dataEnd; };



protected:

	std::vector<char> data;

	mutable CastableIterator dataBegin;
	mutable CastableIterator dataEnd;


	/// Sets type, if unset.
	/**
	 */
	virtual inline
	bool suggestType(const std::type_info & t){
		if (!typeIsSet())
			setType(t);
		return true;
	}

	/// Request to change the array size. For Castable (and Reference) does nothing and returns false.
	/**
	 *   Does not apply to std::string;
	 */
	virtual inline
	bool requestSize(size_t elementCount){
		updateSize(elementCount);
		return true;
	}

	inline
	bool updateSize(size_t elementCount){

		this->elementCount = elementCount;

		if (elementCount > 0)
			data.resize(elementCount * getElementSize());
		else {
			if (getElementSize() > 0)
				data.resize(1 * getElementSize());
			else
				data.resize(1);
			data[0] = 0; // for std::string toStr();
		}

		caster.ptr = &data[0]; // For Castable

		updateIterators();

		return true;
	}


	inline
	void updateIterators()  {

		dataBegin.setType(getType());
		dataBegin = (void *) & data[0];

		dataEnd.setType(getType());
		dataEnd = (void *) & data[ getElementCount() * getElementSize() ]; // NOTE (elementCount-1) +1

	}


};


class FlexVariable : public Variable {

public:

	template <class T>
	inline
	FlexVariable & link(T &p){
		reset();
		setPtr(p);
		return *this;
	}

	inline
	void relink(Castable & c){
		Castable::relink(c);
	}

	template <class T>
	inline
	FlexVariable & operator=(const T &x){
		if (isReference())
			Castable::operator=(x);
		else
			Variable::operator=(x);
		return *this;
	}

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
			throw std::runtime_error(std::string("FlexVariable") + __FUNCTION__ + ": illegal for referenced variable");
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
				std::cerr << "FlexVariable::" << __FUNCTION__ << " warning: resize for linked Variable not yet implemented " << std::endl;
			}
			if (elementCount != getElementCount()){
				throw std::runtime_error(std::string("FlexVariable") + __FUNCTION__ + ": illegal for referenced variable");
				// = return false;
			}
			return true;
		}
	}

protected:

	/// Returns true, if the internal pointer directs to external data object, ie. internal data vector is not used.
	/**
	 *  (Derived classes, like FlexVariable, may alternatively point to external data array.)
	 *
	 *  \return - true, if the internal pointer directs to external data object
	 *
	 */
	inline
	bool isReference(){
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
inline
FlexVariable & FlexVariable::link<Castable>(Castable &c){
	std::cerr << "Referencer::" << __FUNCTION__ << "(Castable): discouraged, use relink(Castable &) " << std::endl;
	Castable::relink(c);
	return *this;
}

template <>
inline
FlexVariable & FlexVariable::link<Variable>(Variable &c){
	std::cerr << "Referencer::" << __FUNCTION__ << "(Castable): discouraged, use relink(Castable &) " << std::endl;
	Castable::relink(c);
	return *this;
}

template <>
inline
FlexVariable & FlexVariable::link<Referencer>(Referencer &r){
	//std::cerr << "Referencer::" << __FUNCTION__ << "(Referencer): deprecating, use relink(Referencer &) " << std::endl;
	Castable::relink(r);
	return *this;
}



} // drain

#endif /* VARIABLE_H_ */

// Drain
