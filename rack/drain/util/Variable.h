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
 *   Variable stores conveniently values that need to be frequently type converted.
 *   Internally, Variable uses a byte vector which is dynamically resized to
 *   fit data in.
 *   The data can be a scalar of basic type, vector of a basic type or a std:.std::string.
 *
 *   Internally std::string type is implemented as a vector of char. This is different from
 *   drain::Castable with type std::string which assumes that the object behind (void *)
 *   is actually std::string.
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
 *   vField = 123.456; // Stored internally as std::string "123.456"
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
	Variable(const std::type_info &t = typeid(void)) { // : IS_STRING(false) {
		reset();
		setType(t);
		// separator = ','; // todo: static defaultSeparator ?
	};

	/// Copies type, data and separator char.
	Variable(const Variable & v) { //  : IS_STRING(false) {
		reset();
		this->outputSeparator = v.outputSeparator;
		this->inputSeparator = v.inputSeparator;
		assignCastable(v);
	};

	/// Copies type, data and separator char.
	Variable(const Castable & c) { //  : IS_STRING(false) {
		reset();
		//this->outputSeparator = c.outputSeparator;
		//this->inputSeparator = c.inputSeparator;
		assignCastable(c);
	};

	/// Copies type, data and separator char.
	Variable(const char * s) { //  : IS_STRING(false) {  // TODO: false?
		reset();
		assignString(s);
	};

	/// Copies type, data and separator char. Fails with Reference?
	template <class T>
	Variable(const T & value, const std::type_info &t = typeid(void)) { //  : IS_STRING(false) {
		reset();
		setType(t);
		*this = value;
	}

	virtual ~Variable(){};

	/// Does not change separator chars.
	inline
	void reset(){
		caster.unsetType();
		resize(0);
		setSeparator(',');
		//this->separator = ','; // semantics ?
	}

	/// Sets type to any of the basic types or std::string.
	/**
	 *   Internally, std::string type is anyway implemented as \c char[]
	 */
	virtual inline
	void setType(const std::type_info & t){ //, size_t n = 0

		reset(); // what if requested type already?

		if ((t == typeid(std::string)) || (t == typeid(char *))){
			//std::cerr << "Variable::setType (std::string) forward, OK\n";
			caster.setType(typeid(char));
			setOutputSeparator(0);
			resize(1);
			caster.put(ptr, '\0');
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
	Variable &operator=(const Castable &c){
		assignCastable(c);
		return *this;
	}

	inline
	Variable &operator=(const Referencer &r){
		assignCastable(r);
		return *this;
	}

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
	/** Important when other Castable wants to assing this, preserving the precision in floating point numbers for example.
	 *
	 */
	inline
	operator const Castable &() const {
		std::cerr << "operator const Castable, value=" << *this << std::endl;
		return *this;
	}



	/// Work-around for the bad behaviour of Variable::operator T() for std::string.
	/*
	 *
	 *  Could return directly (const char *)
	 *  but the target would be dynamically changing (be it  _sstr.toStr().c_str() or &data[0]).
	 */



	/// Extends the vector to include n elements of the current type.
	virtual inline
	bool resize(size_t elementCount){

		this->elementCount = elementCount;

		if (elementCount > 0)
			_data.resize(elementCount * getByteSize());
		else {
			if (getByteSize() > 0)
				_data.resize(1 * getByteSize());
			else
				_data.resize(1);
			_data[0] = 0; // for std::string toStr();
		}

		ptr = &_data[0]; // For Castable

		updateIterators();

		return true;
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

	std::vector<char> _data;



	mutable CastableIterator dataBegin;
	mutable CastableIterator dataEnd;

	/// Sets type, if unset.
	/**
	 */
	virtual inline
	bool requestType(const std::type_info & t){
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
		resize(elementCount);
		return true;
	}



	inline
	void updateIterators()  {

		dataBegin.setType(getType());
		dataBegin = (void *) & _data[0];

		dataEnd.setType(getType());
		dataEnd = (void *) & _data[ getElementCount() * getByteSize() ]; // NOTE (elementCount-1) +1

	}


};





} // drain

#endif /* VARIABLE_H_ */

// Drain
