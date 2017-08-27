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
#include "String.h"

#include "SmartMap.h"

// // // // using namespace std;

namespace drain {

class Variable;

std::ostream & operator<<(std::ostream &ostr, const Variable &x);

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
 *   Variable v;
 *   v = 123.456;  // First assignment sets the type implicitly (here double).
 *   int i = v;    // automatic cast
 *   \endcode
 *
 *  The type of a Variable can be set explicitly:
 *   \code
 *   Variable v;
 *   v.setType<std::string>();
 *   v = 123.456; // stores internally as std::string "123.456"
 *   int i = v;  // automatic cast
 *   \endcode
 *
 *  Any number of scalars can be appended to a Variable, which hence becomes a vector:
 *   \code
 *   Variable v;
 *   v.setType<double>();
 *   v << 1.0 << 123.456 << 9.999;
 *   std::cout << v << std::endl;  // dumps 1.0,123.456,9.999
 *   \endcode
 *
 *  If the type is unset, Variable converts to undetectValue or an empty std::string.
 *   \code
 *   Variable v;
 *   double d = v; // becomes 0.0
 *   std::string s = v; // becomes ""
 *   \endcode
 *
 *
 *
 *   Replaces the older drain::Data.
 *   Does not much "need" ptr. Unlike in drain::Castable, the pointer contained by Variable is not visible or changeable by the user.
 *
 *   Note: tried skipping Castable, inheriting directly from Caster but needs Castable for cooperation with ReferenceMap.
 *
 *
 *   \example Variable-example.cpp
 *
 */
//class Variable : protected Caster {
class Variable : protected Castable { //protected Castable {
public:

	/// Default constructor generates an empty array.
	Variable(const std::type_info &t = typeid(void)) :  separator(','), IS_STRING(false) {
		reset();
		setType(t);
		// separator = ','; // todo: static defaultSeparator ?
	};



	/// Copies type, data and separator char.
	Variable(const Variable & e) : separator(e.separator), IS_STRING(false) {
		reset();
		separator = e.separator;
		*this = e;  // Handles std::string via setType/getType
	};

	/// Copies type, data and separator char.
	Variable(const Castable & e) : separator(','), IS_STRING(false) {
		reset();
		setType(e.getType());
		(Castable &)(*this) = e;
	};

	/// Copies type, data and separator char.
	Variable(const char * s) : separator(','), IS_STRING(false) {  // TODO: false?
		reset();
		*this = s;
	};

	/// Copies type, data and separator char.
	template <class T>
	Variable(const T & e, const std::type_info &t = typeid(void)) : separator(','), IS_STRING(false) {
		reset();
		setType(t);
		*this = e;
	}

	virtual ~Variable(){};

	/// Does not change separator char.
	inline
	void reset(){
		unsetType();
		resize(0);
		separator = ','; // semantics ?
	}

	template <class T>
	inline
	void setType(size_t n = 0){ setType(typeid(T), n); }

	/// Sets type to any of the basic types or std::string.
	/**
	 *   Internally, std::string type is anyway implemented as \c char[]
	 */
	inline
	void setType(const std::type_info & t, size_t n = 0){
		if ((t == typeid(std::string)) || (t == typeid(char *)))  {  // todo char[xx] ?
			Castable::setType<char>();
			IS_STRING = true;
			clear();
			if (n > 0)
				std::cerr << "Variable::setType<std::string> init fill (n="<< n <<") discarded \n";
			//resize(n);
			//fill('\0');
		}
		else if (t == typeid(void)){
			reset();
		}
		else {
			Castable::setType(t);
			IS_STRING = false;
			//_resize(getElementCount());
			resize(n);
			//clear();
		}
	}

	inline
	const std::type_info & getType() const {
		return IS_STRING ? typeid(std::string) : Caster::getType();
	}

	inline
	size_t getByteSize() const {
		return Caster::getByteSize();
	}


	inline
	bool typeIsSet() const {
		return Castable::typeIsSet();
	};

	/// Removes all the elements, leaving type info intact.
	// TODO: semantics: should it resize(0), saving type. Now depends on type.
	inline
	void clear(){
		resize(0);
		/*
		if (IS_STRING){
			resize(0);
			data[0] = '\0';
		}
		else
			for (CastableIterator it = begin(); it != end(); ++it){
		 *it = 0;
			};
		 */
	}

	/// Get an element in the array. If the variable is a scalar, get(0) is the only valid call.
	/**
	 */
	template <class T>
	inline
	T get(size_t index) const {
		if (index >= _elementCount)
			throw std::runtime_error("drain::Variable index overflow");

		return Caster::get<T>(&_data[index * byteSize]);
		//return static_cast<>(data[index]);
	}

	/**
	template <class T>
	inline
	void put(size_t index, const T & value){
		_putElement(index, value);
	}
	 */

	/**
	 *
	 *  Semantics: should the type change always?
	 */
	inline
	Variable &operator=(const Variable &v){

		if (IS_STRING || (v.getType() == typeid(std::string))){
			*this = v.toStr();
		}
		else {
			if (!typeIsSet()){
				if (v.typeIsSet())
					setType(v.getType());
				/*
				else {
					std::cerr << "Variable &operator= Variable, warning: unset type in source var " << std::endl;
					//setType(typeid(std::string));
					(*this) = "CHECK";
					return *this;
				}
				*/
			}
			resize(v.getElementCount());

			CastableIterator  it = begin();
			CastableIterator vit = v.begin();
			while (it != end()) {
				*it = *vit;
				++it;
				++vit;
			}
		}

		return *this;
	}

	inline
	Variable &operator=(const Castable &c){

		const std::type_info &t = c.getType();

		//std::cerr << "operator=(const Castable &)" << std::endl;
		/// Notice still that "std::string" has different semantics for Castable and Variable

		if (t == typeid(std::string))
			*this = (const std::string &)c;
		else {
			if (!typeIsSet())
				setType(t);
			//std::cerr << "Kastinen: " << c << "\n";
			resize(1);
			Castable::operator=(c);
			//(Castable &)(*this) = c;
		}

		return *this;
	}


	/// Assignment from char * and std::string splits the input by separator.
	inline
	Variable &operator=(const char *s){

		if (!typeIsSet())
			setType<std::string>();

		resize(0);

		if (IS_STRING){
			const std::string str(s);
			appendString(str); // clumsy?
		}
		else {
			const std::string str(s);
			const size_t n = str.size();
			size_t pos  = 0;
			size_t pos2 = 0;
			while (pos2 < n){
				pos2 = str.find(separator, pos);
				if (pos2 == std::string::npos)
					pos2 = n;
				//std::cerr << "append" << toStr.substr(pos, pos2-pos) << std::endl;
				appendElement(str.substr(pos, pos2-pos));
				pos = pos2+1;
			}
		}
		return *this;

	}

	///
	Variable &operator=(const std::string &s){
		return operator=(s.c_str());
	}


	/// Assignment from basic types and std::string.
	template <class T>
	inline
	Variable &operator=(const T &x){
		resize(0);
		operator<<(x);
		return *this;
	}

	/// Extends the array by one element.
	inline
	Variable &operator<<(const char *s){

		if (!typeIsSet())
			setType<std::string>();

		if (IS_STRING)
			appendString(std::string(s));
			//appendString(s);
		else
			appendElement(s);

		return *this;
	}

	/// Extends the array by one element.
	/*
	 *  Note: if a std::string is given, does not split it like with Variable::operator=(const char *).
	 */
	template <class T>
	inline
	Variable &operator<<(const T &x){

		if (!typeIsSet())
			setType<T>(0);

		if (IS_STRING)
			appendString(x);
		else
			appendElement(x);

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

	/// Conversion to std::string.
	/**
	 *  http://stackoverflow.com/questions/7741531/conversion-operator-template-specialization
	 */
	inline
	operator std::string() const {
		return toStr();
	}

	/// Conversion operator. Works well with basic types. Due to g++ error/feature (?), does not work implicitly for std::string.
	//  Thanks to Mikael Kilpeläinen.
	/**
	 *   Due to c++ compiler error/feature, does not work implicitly for std::string. For example:
	 *   \code
	 *   Variable v;
	 *   v = 1.23;
	 *   std::string s = v;  // OK
	 *   s = v;         // FAILS, for unknown reason
	 *   \endcode
	 *   See http://stackoverflow.com/questions/7741531/conversion-operator-template-specialization .
	 *   The conversion works only if a conversion operator is defined only for std::string (operator std::string()).
	 *   Adding any other conversions (int, double, ...) causes the compiler to fail.
	 */
	template <class T>
	inline
	operator T() const {
		//return Castable::get<T>(& data[0]);
		if (IS_STRING){
			T x;
			std::stringstream sstr;
			//_sstr.toStr("");
			sstr << (const char *)ptr;
			sstr >> x;
			return x;
		}
		else
			return Castable::get<T>(ptr);
	}


	/// Work-around for the bad behaviour of Variable::operator T() for std::string.
	/*
	 *
	 *  Could return directly (const char *)
	 *  but the target would be dynamically changing (be it  _sstr.toStr().c_str() or &data[0]).
	 */
	inline
	std::string toStr() const {  //
		std::stringstream sstr;
		sstr.str("");
		sstr << *this;
		return sstr.str();

	}




	/// Extends the vector to include n elements of the current type.
	inline
	void resize(size_t elementCount){

		_elementCount = elementCount;

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

	}

	/// Returns the length of the array, the number of elements in this Entry.
	/**
	 *   See also getByteSize().
	 *   TODO: rename to size?
	 */
	inline
	size_t getElementCount() const {
		return _elementCount;
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


	inline
	std::ostream & toOStream(std::ostream &ostr) const {

		if (IS_STRING){
			ostr << &_data[0];
		}
		else {
			char separator = 0;
			for (CastableIterator it = begin(); it != end(); ++it){
				if (separator != 0) // Not the first entry.
					ostr << separator;
				(*it).toOStream(ostr); // Castable
				if (this->separator != 0)
					separator = this->separator;
			}
		}
		return ostr;
	}


	/// Export to base type vectors. In case of std::strings, splits with this->separator.
	template <class T>
	void toVector(std::vector<T> & v) const {
		toContainer(v);
	}

	/// Converts to simple iterable container like std::set and std::vector.
	/**
	 *    \tparam T - std::set, std::list or std::vector
	 */
	template <class T>
	void toContainer(T & container) const {

		container.clear(); // NEW 2016

		if (getType() == typeid(std::string)){

			std::list<std::string> l;
			String::split(toStr(), l, std::string(1, this->separator)," \t\n");

			// container.resize(l.size());
			typename std::list<std::string>::iterator sit = l.begin();  // const_iterator more elegant, but Castable is non-const
			// typename T::iterator vit = container.begin();
			Castable c;
			//c.link()
			//c.setType(typeid(T)); // No use... Castable will set type in each link() or setPtr() call.
			while (sit != l.end()) {
				// NEW
				//c.link(sit->c_str());
				c.link(*sit);
				container.insert(container.end(), c);
				/* OLD
				c.link(*vit);
				c = *sit;
				++vit;
				 */
				++sit;
			}

		}
		else {

			// container.resize(getElementCount());
			CastableIterator it = begin();
			// typename T::iterator vit = container.begin();
			while (it != end()) {
				//std::cerr << *vit << "<=" << *it << std::endl;
				container.insert(container.end(), *it);  // NEW
				//*vit = *it; OLD
				// ++vit; OLD
				++it;
			}

		}


	}

	template <class T>
	void fromContainer(const T & container)  {
		for (typename T::const_iterator it = container.begin(); it != container.end(); ++it){
			*this << *it;
		}
	}

	inline
	const void info(std::ostream & ostr) const {
		ostr << "Variable '";
		//ostr << 'D';
		if (getType() == typeid(double)){
			//ostr << 'D';
			ostr.precision(40);
			ostr << (const double &)*this;
		}
		else
			ostr << *this;
		//ostr << "' type=" << getType().name() << "(" << getByteSize() << "bytes), " << getElementCount() << " elements";
		ostr << "' ";
		typeInfo(ostr);
	};

	inline
	const void typeInfo(std::ostream & ostr) const {
		ostr << '[' << drain::Type::getTypeChar(getType());
		//if (size_t n = getElementCount() != 1)
		ostr << getElementCount();
		ostr << '@' << getByteSize() << ']';
	};


	inline
	const void info() const {
		info(std::cout);
		std::cout << std::endl;
	};

	/// Returns the pointer to the first element of the array. Make sure that read lengths match the actual size.
	inline
	const void * getPtr() const {
		return ptr;
	}

	/// Returns the pointer to the first element of the array. Use with care; make sure that read/write lengths match the actual size.
	inline
	void * getPtr() {
		return ptr;
	}



	char separator;

protected:

	std::vector<char> _data;

	bool IS_STRING;

	//mutable std::stringstream _sstr;

	//mutable std::vector<Castable> _dataVector;
	//std::vector<Variable> _dataVector;  //


	/// Number of elements in the array. By default, one.
	size_t _elementCount;

	mutable CastableIterator dataBegin;
	mutable CastableIterator dataEnd;

	//void (Variable::* assignCastable)(const Castable &x);

	/// If the type is not std::string, putElement is applied.
	template <class T>
	inline
	void appendString(const T &x) {
		/// Append std::string
		std::stringstream sstr;
		if ((separator == '\0') ||(_data.size()<=1))
			sstr << toStr() << x;
		else
			sstr << toStr() << separator << x;
		const std::string & s = sstr.str();
		//std::cerr << "Variable &operator<< :" << toStr() << " => "<< s << std::endl;
		resize(s.length() + 1);
		/// Store
		std::vector<char>::iterator  it = _data.begin();
		std::string::const_iterator sit = s.begin();
		while (it != _data.end()){
			*it = *sit;
			++it; ++sit;
		}
	}


	template <class T>
	inline
	void appendElement(const T &x) {
		putElement(_elementCount, x);
	}

	template <class T>
	inline
	void putElement(size_t i, const T &x) {

		if (!typeIsSet())
			Castable::setType<T>();

		// std::cerr << "Put x="<< x << ", e=" << _elementCount << std::endl;
		if (i >= _elementCount)
			resize(i+1);

		Caster::put(& _data[i * byteSize], x);

		// std::cerr << "Now this = " << *this << std::endl;

	}


	inline
	void  putElement(size_t i, const char *s) {

		if (!typeIsSet())
			Castable::setType<std::string>();

		if (IS_STRING){
			throw std::runtime_error("Variable: _putElement called with std::string type content - ? ");
		}

		/// Add scalar element
		const size_t n = _elementCount;
		resize(n + 1);
		// std::cerr << "putElement i=" << i << " s='" << s << "'" << std::endl;
		/// (now elementCount += 1)
		Caster::put(&_data[n * byteSize], s); // Converts to number.

	}





	inline
	void updateIterators()  {

		dataBegin.setType(getType());
		dataBegin = (void *) & _data[0];

		dataEnd.setType(getType());
		dataEnd = (void *) & _data[ getElementCount() * getByteSize() ]; // NOTE (elementCount-1) +1

	}


};


inline
std::ostream & operator<<(std::ostream &ostr, const Variable &x){
	return x.toOStream(ostr);
}

template <>
void Variable::toVector(std::vector<Variable> & v) const;




//std::ostream &operator<<(std::ostream &ostr, const VariableMap &m);

} // drain

#endif /* VARIABLE_H_ */

// Drain
