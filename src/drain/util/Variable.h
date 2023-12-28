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

#include "Castable.h"
#include "CastableIterator.h"
//#include "Referencer.h"
#include "Sprinter.h"
#include "String.h"
#include "UniTuple.h" // "Friend class"



namespace drain {

class Referencer;
class FlexibleVariable;
//class ReferenceVariable;

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

	// Like in images. Should be fixed.
	typedef CastableIterator const_iterator;
	typedef CastableIterator iterator;

	/// Default constructor generates an empty array.
	inline
	Variable(const std::type_info &t = typeid(void)) {
		reset();
		setType(t);
	};

	/// Copies type, data and separator char.
	inline
	Variable(const Variable & v) {
		reset();
		this->outputSeparator = v.outputSeparator;
		this->inputSeparator = v.inputSeparator;
		assignCastable(v);
	};

	/// Copies type, data and separator char.
	inline
	Variable(const Castable & c) {
		reset();
		//this->outputSeparator = c.outputSeparator;
		//this->inputSeparator  = c.inputSeparator;
		assignCastable(c);
	};

	/// Copies type, data and separator char.
	inline
	Variable(const char * s) {
		reset();
		assignString(s);
	};

	/// Initialisation with type of the first element or explicit type argument.
	template<typename T>
	inline
	Variable(std::initializer_list<T> l, const std::type_info &t = typeid(void)) {
		reset();
		setType(t);
		assignContainer(l, true);
	};

	// Initialisation, using type of argument or explicit type argument.
	// Copies type, data and separator char. Fails with Reference?
	template <class T>
	Variable(const T & value, const std::type_info &t = typeid(void)) {
		reset();
		setType(t);
		*this = value;
	}

	virtual ~Variable(){};

	//template <class T>
	Variable & append(){
		return *this;
	}

	template <class T, class ...TT>
	Variable & append(const T &x, const TT& ...rest){
		Castable::operator<<(x);
		append(rest...);
		return *this;
	}

	template <class T, class ...TT>
	Variable & set(const T &x, const TT& ...rest){
		clear();
		(*this) = x;
		return append(rest...);
	}

	/// Does not change separator chars.
	inline
	void reset(){
		caster.unsetType();
		//resize(0);
		updateSize(0);
		setSeparator(',');
		//this->separator = ','; // semantics ?
	}

	/// Sets basic type or void.
	/**
	 *   Also std::string is accepted, but handled as request of \c char array.
	 */
	virtual
	void setType(const std::type_info & t);

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


#define SmartVariable Variable
#include "SmartVariable.inc"
#undef  SmartVariable


	/// Extends the array by one element.
	/*
	inline
	Variable &operator<<(const char *s){
		// Castable::operator<<(s);
		append(s);
		return *this;
	}
	*/

	/// Extends the array by one element.
	/*
	 *  Note: if a std::string is given, does not split it like with Variable::operator=(const char *).
	 */
	template <class T>
	inline
	Variable &operator<<(const T &x){
		//Castable::operator<<(x);
		append(x);
		return *this;
	}


	/// Conversion to Castable.
	/** Important when str Castable wants to assing this, preserving the precision in floating point numbers for example.
	 *
	inline
	operator const Castable &() const {
		std::cerr << "operator const Castable, value=" << *this << std::endl;
		return *this;
	}
	 */



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

	bool updateSize(size_t elementCount);

	void updateIterators();

};

template <>
struct TypeName<Variable> {
    static const char* get(){ return "Variable"; }
};


template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::Variable & x, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &)x, layout);
}





} // drain

#endif /* VARIABLE_H_ */

// Drain
