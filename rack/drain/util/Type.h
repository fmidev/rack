/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

 * Type.h
 *
 *  Created on: Nov 25, 2010
 *      Author: mpeura
 */

#ifndef Drain_Type2_H
#define Drain_Type2_H

#include <typeinfo>
//#include <type_traits>
#include <limits>

#include "Log.h"

namespace drain {

// // // using namespace std;






/**
 *  \tparam T - return type
class valueMax {

public:

	 *  \tparam S - type to be analyzed
	 *  \tparam D - destination type
	template <class S, class D>
	static inline
	D callback(){
		return static_cast<D>(getValue<S>());
	}

protected:

	template <class S>
	static inline
	double getValue(){
		return static_cast<double>(std::numeric_limits<S>::max());
	}

};

template <>
inline
double valueMax::getValue<void>(){
	return 0;
}

template <>
inline
double valueMax::getValue<std::string>(){
	return 0;
}
*/




/// Utilities related to stl::type_info.
/**
 *
 */
class Type {
public:

	inline
	Type() : currentType(&typeid(void)) { //, typeMin(0), typeMax(0) {
	}

	inline
	Type(const Type & t) : currentType(t.currentType) { //, typeMin(t.typeMin), typeMax(t.typeMax) {  // FIX 0 0
		//std::cerr << __FUNCTION__ << "(const &)" << std::endl;
	}

	template <class T>
	Type(const T & t) : currentType(&typeid(void)){
		setType(t);
	}


	template <class T>
	inline
	void setType(){
		//std::cerr << __FUNCTION__ << "<T>()" << std::endl;
		currentType = & typeid(T);
	}


	/// Set current type to t.
	/**
	 *  \tparam  T - type code: type_info, char, string or drain::Type.
	 */
	template <class T>
	inline
	void setType(const T & t){
		call<typeSetter>(*this, t);
	}

	/*  // actually "TypeGetter" ?
	struct typeGetter {
		template <class S, class T>
		static
		inline
		T callback(){
			static Type t(typeid(T));
			return t;
		}
	};
	 */




	//    TODO, for std::strings.
	template <class T>
	inline
	Type & operator=(const T & t){
		setType(t);
		return *this;
	}

	inline
	operator const std::type_info & () const {return (*currentType); }

	inline
	operator char () const {return getTypeChar(*currentType); }

	inline
	operator std::string () const {return std::string(1, getTypeChar(*currentType)); }


	/// Deprecated! Use cast (above) instead?
	inline
	const std::type_info & getType() const {
		return *currentType;
	}


	/// Returns the base type associated with a character key.
	/**
	 *  \par c - character for base types:
	 *
	 *  - c - char
	 *  - C - unsigned char
	 *  - s - short
	 *  - S - unsigned short
	 *  - i - integer
	 *  - I - unsigned integer
	 *  - l - long int
	 *  - L - unsigned long int
	 *  - f - float
	 *  - d - double
	 *  - B - boolean
	 *  - # - std::string ($)
	 *  - * - void (identified with undefined type)
	 *
	 *  Notice that for integral types the signed and unsigned are denoted with uppercase and lowercase letters, respectivelty.
	 */
	static
	inline
	const std::type_info & getTypeInfo(char t){

		switch (t) {
		case 'c':
			return typeid(char);
		case 'C':
			return typeid(unsigned char);
		case 's':
			return typeid(short);
		case 'S':
			return typeid(unsigned short);
		case 'i':
			return typeid(int);
		case 'I':
			return typeid(unsigned int);
		case 'l':
			return typeid(long);
		case 'L':
			return typeid(unsigned long);
		case 'f':
			return typeid(float);
		case 'd':
			return typeid(double);
		case 'B':
			return typeid(bool);
		case '$': // deprecated?
		case '#':
			return typeid(std::string);
		case '*':
			return typeid(void);
		default:
			Logger mout("Type", std::string(__FUNCTION__)+"(char c)");
			mout.error() << " undefined type: '" << t << "'=" << (int)t << mout.endl;
			return typeid(void);
		}

	}

	static
	inline
	char getTypeChar(const std::type_info & t){
		if (t == typeid(char))
			return 'c';
		else if (t == typeid(unsigned char))
			return 'C';
		else if (t == typeid(short))
			return 's';
		else if (t == typeid(unsigned short))
			return 'S';
		else if (t == typeid(int))
			return 'i';
		else if (t == typeid(unsigned int))
			return 'I';
		else if (t == typeid(long))
			return 'l';
		else if (t == typeid(unsigned long))
			return 'L';
		else if (t == typeid(float))
			return 'f';
		else if (t == typeid(double))
			return 'd';
		else if (t == typeid(bool))
			return 'B';
		else if (t == typeid(void))
			return '*';
		else if (t == typeid(std::string))
			return '#';
		else if (t == typeid(const char *))
			return '#';
		else if (t == typeid(char *))
			return '#';
		else {
			//Logger mout(monitor,"Type::getTypeChar(t)");
			//mout.error() << " undefined type: '" << t.name() << "'" << mout.endl;
			return '*';
		}
	}

	/// Calls a static function that has no parameters.
	/**
	 *  Maps std::type_info to the corresponding template, calling a desired modifier function.
	 *
	 *  \tparam F - struct with static method T callback<type>(), where \c type is a basetype or std::string.
	 *  \tparam T - return type
	 */
	template <class F, class T>
	static
	T call(const std::type_info & t){  // ORIGINAL

		//std::cout << "Calling call with " << t.name() << " (" << getTypeChar(t) << ")\n";
		if (t == typeid(char)){
			return F::template callback<char,T>();
		}
		else if (t == typeid(unsigned char)) {
			return F::template callback<unsigned char,T>();
		}
		else if (t == typeid(int)) {
			return F::template callback<int,T>();
		}
		else if (t == typeid(unsigned int)) {
			return F::template callback<unsigned int,T>();
		}
		else if (t == typeid(short int)) {
			return F::template callback<short int,T>();
		}
		else if (t == typeid(unsigned short int)) {
			return F::template callback<unsigned short int,T>();
		}
		else if (t == typeid(long int)) {
			return F::template callback<long int,T>();
		}
		else if (t == typeid(unsigned long int)) {
			return F::template callback<unsigned long int,T>();
		}
		else if (t == typeid(float)) {
			return F::template callback<float,T>();
		}
		else if (t == typeid(double)) {
			return F::template callback<double,T>();
		}
		else if (t == typeid(bool)) {
			return F::template callback<bool,T>();
		}
		else if (t == typeid(void)) {
			return F::template callback<void,T>();
		}
		else if (t == typeid(std::string)) {
			return F::template callback<std::string,T>();
		}
		else {
			Logger mout(__FILE__, __FUNCTION__);
			mout.error() << "unimplemented type: ..." << t.name() << mout.endl;
			return T(); //F::template callback<char,T>();
		}

	}

	/// New, preferred implementation: a single if-then structure once and for all.
	/**
	 *   \tparam F - struct implementing: T callback<T>()
	 *   \tparam D - destination type
	 *   \tparam T - selector type (char, std::string, drain::Type)
	 */
	template <class F, class D, class T>
	static 	inline
	D call(const T & typeCode){
		return call<F,D>(getTypeInfo(typeCode));
	}

	/// Experimental
	/**
	 *   \tparam F2 - struct implementing: T callback<T>() AND value_t (replacing destination type S)
	 *   \tparam S  - selector type (char, std::string, drain::Type)
	 */
	template <class F2, class S>
	static 	inline
	typename F2::value_t call(const S & typeCode){
		return call<F2,typename F2::value_t>(getTypeInfo(typeCode));
	}


	/// Static function call with a single parameter.
	/**
	 *   Maps std::type_info to the corresponding template, calling a desired modifier function.
	 *
	 *   \tparam F - struct that implements static callback<type>(T & target), where \c type is basetype or std::string.
	 *   \tparam T - target object class
	 *
	 *
	 */
	template <class F, class T>
	static
	void call(T & target, const std::type_info &t){ // const Type &type){

		if (t == typeid(char))
			F::template callback<char>(target);
		else if (t == typeid(unsigned char))
			F::template callback<unsigned char>(target);
		else if (t == typeid(int))
			F::template callback<int>(target);
		else if (t == typeid(unsigned int))
			F::template callback<unsigned int>(target);
		else if (t == typeid(short))
			F::template callback<short>(target);
		else if (t == typeid(unsigned short))
			F::template callback<unsigned short>(target);
		else if (t == typeid(long))
			F::template callback<long>(target);
		else if (t == typeid(unsigned long))
			F::template callback<unsigned long>(target);
		else if (t == typeid(float))
			F::template callback<float>(target);
		else if (t == typeid(double))
			F::template callback<double>(target);
		else if (t == typeid(bool))
			F::template callback<bool>(target);
		else if (t == typeid(std::string))
			F::template callback<std::string>(target);
		else if (t == typeid(void)) {
			F::template callback<void>(target);
			//c.setType<void>();
			//unsetType();
		}
		else {
			throw std::runtime_error(std::string(": unimplemented type: ") + t.name());
			return;
		}
	}

	/// Static function call with a single parameter.
	/**
	 *
	 *   Maps std::type_info to the corresponding template, calling a desired modifier function.
	 *
	 *   \tparam F  - struct that implements static callback<type>(T & target), where \c type is basetype or std::string.
	 *   \tparam T  - target object class
	 *   \tparam T2 - selector type (Type or typecode as char or string)
	 *
	 */
	template <class F, class T, class T2>
	static
	void call(T & target, const T2 &t){
		call<F>(target, getTypeInfo(t));
	}



	/// Returns the type corresponding to a single-character string.
	/**
	 *  Checks that a single letter is given, and calls call(char);
	 */
	static
	inline
	const std::type_info & getTypeInfo(const char *typeCode){
		if ((typeCode != NULL) && (*typeCode != '\0'))
			return getTypeInfo(*typeCode);
		else if (typeCode == NULL)
			return getTypeInfo('*');
		else
			throw std::runtime_error(std::string("Type::operator=, not unit-length std::string: ") + typeCode);
	}

	/// Returns the type corresponding to a single-character string.
	/**
	 *  Checks that a single letter is given, and calls call(char);
	 */
	static
	inline
	const std::type_info & getTypeInfo(const std::string & typeCode){
		if (typeCode.length() == 1)
			return getTypeInfo(typeCode.at(0));
		else if (typeCode.empty())
			return getTypeInfo('*');
		else
			throw std::runtime_error(std::string("Type::operator=, not unit-length std::string: ") + typeCode);
	}

	/// Convenience. Purpose is to have a full set of getTypeInfo commands for templated constructs.
	static
	inline
	const std::type_info & getTypeInfo(const Type & t){
		return t;
	}




	///
	/*
	template <class T>
	static inline
	bool isIntegralType(const T & typeInfo) {
		return call<typeIsInteger>(typeInfo);
	}
	*/



	/*
	template <class T>
	static inline
	bool isSigned(const T & typeInfo) {
		return call<typeIsSigned>(typeInfo);
	}
	*/








	/// Returns the minimum possible value of type S, casted to type T.
	/**  Calls std::numeric_limits and casts the result.
	 *
	template <class S,class T>
	static inline
	T getMax(){
		return call<valueMax, T>(typeid(S));
	}

	template <class D, class T>
	static inline
	D getMax(const T & type){
		return call<valueMax, D>(type);
	}
	 */




	/*
	template <class T>
	static inline
	nameGetter::value_t getName(const T & typeCode){
		return call<nameGetter>(typeCode);
	}
	 */


	/*
	template <class T>
	static inline
	size_t getSize(const T & typeCode){
		return call<sizeGetter, size_t>(typeCode);
	}
	*/

	static inline
	std::ostream & toOStr(std::ostream & ostr, const Type &t){
		const std::type_info & type = t.getType();
		ostr << Type::getTypeChar(type) << '@'; // << Type::call<drain::sizeGetter>(type);
		return ostr;
	}

	static inline
	std::ostream & toOStrFull(std::ostream & ostr, const Type &t){
		toOStr(ostr, t);
		//const std::type_info & type = t.getType();
		// ostr << ' '<< '(' << Type::getMin<double>(type) << ',' << Type::call<drain::typeMax,double>(type) << ')';
		/*
		if (isSigned(type)){
			//if (true){
			ostr << " +/-";
		}
		*/
		return ostr;
	}

protected:

	/**
	 *  \tparam T - base type
	 *  \param  t - destination Type object
	 */
	struct typeSetter {

		/**
		 *  \tparam T - base type
		 *  \param  t - destination Type object
		 */
		template <class T>
		static inline
		void callback(Type & target){
			//Logger mout(monitor, "Type", __FUNCTION__);
			target.setType<T>();
		}

	};

	const std::type_info *currentType;
	// double typeMin;
	// double typeMax;

};


/*
template <>
inline
void Type::setType<void>(){
	currentType = & typeid(void);
	// typeMax = 0.0;
	// typeMin = 0.0;
}
*/


/// Dumps a node.
inline
std::ostream & operator<<(std::ostream & ostr, const Type &t){
	Type::toOStr(ostr, t);
	return ostr;
}

}

#endif /* Type2_H_ */
