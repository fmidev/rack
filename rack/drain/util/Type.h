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
*//**

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

#include "Debug.h"

namespace drain {

// // // using namespace std;


struct sizeGetter {

	template <class S, class T>
	static
	inline
	T call(){
		return static_cast<T>(getSize<S>());
	}

	template <class S>
	static
	inline
	size_t getSize(){
		return sizeof(S);
	}

};

template <>
inline
size_t sizeGetter::getSize<void>(){
	return 0;
}






/**
 *  \tparam T - return type
 */
struct valueMin {

	/**
	 *  \tparam S - type to be analyzed
	 *  \tparam D - destination type
	 */
	template <class S, class D>
	static
	inline
	D call(){
		return static_cast<D>(getValue<S>());
	}

	template <class S>
	static
	inline
	double getValue(){
		if (std::numeric_limits<S>::is_integer)
			return static_cast<double>( std::numeric_limits<S>::min());
		else
			return static_cast<double>(-std::numeric_limits<S>::max());
	}


};

template <>
inline
double valueMin::getValue<void>(){
	return 0;
}

template <>
inline
double valueMin::getValue<std::string>(){
	return 0;
}


//template <class T>
/*
template <>
double valueMin::call<void>(){
	return 0.0;
	//return static_cast<T>(0.0);
}
 */


/**
 *  \tparam T - return type
 */
struct valueMax {

	/**
	 *  \tparam S - type to be analyzed
	 *  \tparam D - destination type
	 */
	template <class S, class D>
	static
	inline
	D call(){
		return static_cast<D>(getValue<S>());
	}

	template <class S>
	static
	inline
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




/// Utilities related to stl::type_info.
/**
 *
 */
class Type {
public:

	inline
	Type() : currentType(&typeid(void)), typeMin(0), typeMax(0) {
	}

	inline
	Type(const Type & t) : currentType(t.currentType), typeMin(t.typeMin), typeMax(t.typeMax) {  // FIX 0 0
		//std::cerr << __FUNCTION__ << "(const &)" << std::endl;
	}

	template <class T>
	Type(const T & t){
		setType(t);
	}


	template <class T>
	inline
	void setType(){
		//std::cerr << __FUNCTION__ << "<T>()" << std::endl;
		currentType = & typeid(T);
		/*
		typeMax = static_cast<double>(std::numeric_limits<T>::max());
		if (std::numeric_limits<T>::is_integer)
			this->typeMin = static_cast<double>( std::numeric_limits<T>::min());
		else
			this->typeMax = static_cast<double>(-std::numeric_limits<T>::max());
		 */
	}


	/**
	 *  \tparam C - base type
	 *  \param t - destination Type object
	 */
	struct typeSetter {

		template <class T>
		static
		inline
		void call(Type & target){
			//MonitorSource mout(monitor, "Type", __FUNCTION__);
			// mout.warn() << " set... " << mout.endl;
			target.setType<T>();
		}

	};

	template <class T>
	inline
	void setType(const T & x){
		callFunc<typeSetter>(*this, x);
	}


	struct typeGetter {
		template <class S, class T>
		static
		inline
		T call(){
			static Type t(typeid(T));
			return t;
		}
	};


	template <class T>
	static
	inline
	const Type & getType(const T & type){
		static const Type t(type);
		return t;
		//return callFunc<typeGetter, T>(t);
	}


	//    TODO, for std::strings.
	template <class T>
	inline
	Type & operator=(const T & t){
		setType(t);
		return *this;
	}
	//





	inline
	operator const std::type_info & () const {return (*currentType); }

	inline
	operator char () const {return getTypeChar(*currentType); }

	inline
	operator std::string () const {return std::string(1, getTypeChar(*currentType)); }


	/// Deprecated! cast instead?
	inline
	const std::type_info & getType() const {
		return *currentType;
	}


	struct typeIsInteger {
		template <class S, class T>
		static
		inline
		T call(){ return std::numeric_limits<S>::is_integer; }
	};

	///
	template <class T>
	static
	inline
	bool isIntegralType(const T & typeInfo) {
		return callFunc<typeIsInteger, bool>(typeInfo);
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
			//MonitorSource mout(monitor,"Type::getTypeChar(t)");
			//mout.error() << " undefined type: '" << t.name() << "'" << mout.endl;
			return '*';
		}
	}

	// std::type_info => <T> => function<T>

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
		case '$':
		case '#':
			return typeid(std::string);
		case '*':
			return typeid(void);
		default:
			MonitorSource mout(monitor, "Type", std::string(__FUNCTION__)+"(char c)");
			mout.error() << " undefined type: '" << t << "'=" << (int)t << mout.endl;
			return typeid(void);
		}

	}

	/// For type given as std::string. Checks that a single letter is given, and calls callFunc(char c);
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


	template <class T>
	void limit(T &x){
		if (x < typeMin)
			x = static_cast<T>(typeMin);
		else if (x > typeMax)
			x = static_cast<T>(typeMax);
	}




	/// Returns the maximum possible value of type S, casted to type T.
	/** Calls std::numeric_limits and casts the result.
	 *
	 */
	template <class S,class T>
	static
	inline
	T getMin(){
		return callFunc<valueMin, T>(typeid(S));
	}

	template <class D, class T>
	static inline
	D getMin(const T & type){
		return callFunc<valueMin, D>(type);
	}

	/// Returns the minimum possible value of type S, casted to type T.
	/**  Calls std::numeric_limits and casts the result.
	 *
	 */
	template <class S,class T>
	static inline
	T getMax(){
		return callFunc<valueMax, T>(typeid(S));
	}

	template <class D, class T>
	static inline
	D getMax(const T & type){
		return callFunc<valueMax, D>(type);
	}



	struct nameGetter {
		template <class S, class T>
		static
		inline
		T call(){
			return typeid(S).name();
		}
	};

	template <class T>
	static inline
	const char * getName(const T & typeCode){
		return callFunc<nameGetter, const char *>(typeCode);
	}





	template <class T>
	static inline
	size_t getSize(const T & typeCode){
		return callFunc<sizeGetter, size_t>(typeCode);
	}


	/// New, preferred implementation: a single if-then structure once and for all.
	/**
	 *   \tparam F - struct implementing: T call<T>()
	 *   \tparam T
	 */
	template <class F, class T>
	static
	T callFunc(const std::type_info & t){  // ORIGINAL

		//std::cout << "Calling callFunc with " << t.name() << " (" << getTypeChar(t) << ")\n";
		if (t == typeid(char)){
			return F::template call<char,T>();
		}
		else if (t == typeid(unsigned char)) {
			return F::template call<unsigned char,T>();
		}
		else if (t == typeid(int)) {
			return F::template call<int,T>();
		}
		else if (t == typeid(unsigned int)) {
			return F::template call<unsigned int,T>();
		}
		else if (t == typeid(short int)) {
			return F::template call<short int,T>();
		}
		else if (t == typeid(unsigned short int)) {
			return F::template call<unsigned short int,T>();
		}
		else if (t == typeid(long int)) {
			return F::template call<long int,T>();
		}
		else if (t == typeid(unsigned long int)) {
			return F::template call<unsigned long int,T>();
		}
		else if (t == typeid(float)) {
			return F::template call<float,T>();
		}
		else if (t == typeid(double)) {
			return F::template call<double,T>();
		}
		else if (t == typeid(bool)) {
			return F::template call<bool,T>();
		}
		else if (t == typeid(void)) {
			return F::template call<void,T>();
		}
		else if (t == typeid(std::string)) {
			return F::template call<std::string,T>();
		}
		else {
			MonitorSource mout(__FILE__, __FUNCTION__);
			mout.error() << "unimplemented type: ..." << t.name() << mout.endl;
			return T(); //F::template call<char,T>();
		}

	}

	/// New, preferred implementation: a single if-then structure once and for all.
	/**
	 *   \tparam F - struct implementing: T call<T>()
	 *   \tparam T
	 */
	template <class F, class D, class T>
	static
	inline
	D callFunc(const T & typeCode){
		return callFunc<F,D>(getTypeInfo(typeCode));
	}

	/// Static function call
	/**
	 *   "Inverts" std::type_info back to template, calling a desired modifier function.
	 *
	 *   \tparam F - struct that implements static call<type>(T & target), where \c type is basetype or std::string.
	 *   \tparam T - target object class
	 *
	 *
	 */
	template <class F, class T>
	static
	void callFunc(T & target, const std::type_info &t){ // const Type &type){

		if (t == typeid(char))
			F::template call<char>(target);
		else if (t == typeid(unsigned char))
			F::template call<unsigned char>(target);
		else if (t == typeid(int))
			F::template call<int>(target);
		else if (t == typeid(unsigned int))
			F::template call<unsigned int>(target);
		else if (t == typeid(short))
			F::template call<short>(target);
		else if (t == typeid(unsigned short))
			F::template call<unsigned short>(target);
		else if (t == typeid(long))
			F::template call<long>(target);
		else if (t == typeid(unsigned long))
			F::template call<unsigned long>(target);
		else if (t == typeid(float))
			F::template call<float>(target);
		else if (t == typeid(double))
			F::template call<double>(target);
		else if (t == typeid(bool))
			F::template call<bool>(target);
		else if (t == typeid(std::string))
			F::template call<std::string>(target);
		else if (t == typeid(void)) {
			F::template call<void>(target);
			//c.setType<void>();
			//unsetType();
		}
		else {
			throw std::runtime_error(std::string(": unimplemented type: ") + t.name());
			return;
		}
	}

	template <class F, class T, class T2>
	static
	void callFunc(T & target, const T2 &t){
		callFunc<F>(target, getTypeInfo(t));
	}

	/// For type given as std::string. Checks that a single letter is given, and calls callFunc(char c);
	/**
	 *   \param typeCode - single-character std::string referring to a base type
	template <class F, class T>
	static
	void callFunc(T & x, const char *typeCode){
		if ((typeCode != NULL) && (*typeCode != '\0'))
			callFunc<F>(x, *typeCode);
		else
			throw std::runtime_error(std::string("Type::operator=, not unit-length std::string: ") + typeCode);
	}
    */





protected:


	const std::type_info *currentType;
	double typeMin;
	double typeMax;



	/*
	template <class F, class T>
	static
	T callFunc(char t){
		return callFunc<F,T>(getType(t));
	}
	 */


};


template <>
inline
void Type::setType<void>(){
	currentType = & typeid(void);
	typeMax = 0.0;
	typeMin = 0.0;
}

/// Dumps a node.
inline
std::ostream & operator<<(std::ostream & ostr, const Type &t){
	const std::type_info & type = t.getType();
	// ostr << Type::getTypeChar(type) << '(' << Type::getSize(t) << 'B'<< ')' << ':' << Type::getMin<double>(t);
	ostr << Type::getTypeChar(type) << '(' << Type::getSize(type) << 'B'<<':' << Type::getMin<double>(t) << '-' << Type::getMax<double>(t) << ')';
	return ostr;
}

}

#endif /* Type2_H_ */
