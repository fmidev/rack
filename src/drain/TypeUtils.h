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

#ifndef DRAIN_TYPE_UTILS
#define DRAIN_TYPE_UTILS

#include <cmath>
#include <limits>
#include <set>
#include <typeinfo>


#include "Type.h"
#include "RegExp.h"

namespace drain {

///
/**
 *
 */
template <class S>
struct typeLimits {

	static inline
	long int getMinI(){
		return static_cast<long int>(std::numeric_limits<S>::min());
	}

	static inline
	unsigned long int getMaxI(){
		return static_cast<unsigned long int>(std::numeric_limits<S>::max());
	}

	static inline
	double getMaxF(){
		return static_cast<double>(std::numeric_limits<S>::max());
	}

};


template <> inline long int typeLimits<void>::getMinI(){ return 0l; }
template <> inline unsigned long int typeLimits<void>::getMaxI(){ return 0l; }
template <> inline   double typeLimits<void>::getMaxF(){ return 0.0; }

template <> inline long int typeLimits<std::string>::getMinI(){ return 0l; }
template <> inline unsigned long int typeLimits<std::string>::getMaxI(){ return 0l; }
template <> inline   double typeLimits<std::string>::getMaxF(){ return 0.0; }

/// Class for ensuring that variable of type D remains within limits of type S.
/**
 *  Returns a function pointer of type typeLimiter::value_t .
 *  \code
     typedef drain::typeLimiter<double> Limiter;
     Limiter::value_t limit = Type::call<typeLimiter>(t);  // t is type_info, char or std::string.
	 double d = limit(12345.6789);
    \endcode

    With images of type drain::image::Image, use convenient function:
    \code
    Limiter::value_t limit = img.getEncoding().getLimiter<double>();
	\endcode
 *
 */
// THIS WORKS WELL!
template <class D>
class typeLimiter  {

public:

	/// Definition that simplifies
	typedef D (*value_t)(D);

	/**
	 *  \tparam S - type selector
	 *  \tparam T - destination type
	 */
	template <class S, class T>
	static inline
	T callback(){
		if (std::numeric_limits<S>::is_integer)
			return &typeLimiter<D>::limitInteger<S,D>;
		else
			return &typeLimiter<D>::limitFloat<S,D>;
	}


protected:

	///
	/**
	 *  \tparam S - type selector
	 *  \tparam T - target value type (integer type)
	 *
	 *  Note: expecting T > S, ie. S is a narrower basetype.
	 */
	template <class S, class T>
	static
	T limitInteger(T x){

		static const T minValue = static_cast<T>(typeLimits<S>::getMinI());
		static const T maxValue = static_cast<T>(typeLimits<S>::getMaxI());
		if (x < minValue)
			return minValue;
		else if (x > maxValue)
			return maxValue;

		return x;
	}

	///
	/**
	 *  \tparam S - type selector
	 *  \tparam T - target value type (float or double)
	 */
	template <class S, class T>
	static
	T limitFloat(T x){

		static const T maxValue = static_cast<T>(typeLimits<S>::getMaxF());
		// notice minus sign
		if (x < -maxValue)
			return -maxValue;
		else if (x > maxValue)
			return maxValue;

		return x;
	}



};

struct TypeUtils {


	/// Given a string, check if it could be stored as \c int or \c double instead of \c std::string .
	/**
	 *  \return - std::type_info in priority order: int, double, std::string.
	 */
	static
	const std::type_info & guessType(const std::string & value);

	/// Returns the (loosely) minimal type that could store the value without precision loss.
	/**
	 *
	 */
	static
	const std::type_info & guessType(double d, const std::type_info & type = typeid(unsigned char));



	/// Given a vector or list of strings, suggest a matching storage type (int, double, std::string).
	/**
	 *   \tparam C - contrainer, esp. std::list or std::vector
	 */
	template <class C>
	static
	const std::type_info & guessArrayType(const C & container);


	/**
	 *   \tparam T - range-tested type
	 *   \tparam S - source type
	 */
	template <typename T, typename S>
	static inline
	bool isWithinRange(const S & x){
		// Call limit(), eg. check if 352 stays within [0,256[
		return (Type::call<typeLimiter<S> >(typeid(T))(x) == x);
	}

	// 	 *   \tparam S - source type, typically with a large value range, double for example

	/**
	 *   \tparam T - narrower type, like short int.
	 */
	template <typename T=unsigned char>
	static
	const std::type_info & minimizeIntType(double d);

	static
	const drain::RegExp  trueRegExp; // ignore case

	static
	const drain::RegExp falseRegExp; // ignore case

	// TODO: better whitespace
	static
	const drain::RegExp numeralRegExp;

};



/**
 *  Starting from uchar, rising until
 *
 */
template <typename T=unsigned char>
const std::type_info & TypeUtils::minimizeIntType(double d){ // , const std::type_info & type = typeid(unsigned char)){

	// const bool IS_INTEGER = (d == ::trunc(d));

	if (d != ::trunc(d)){
		return typeid(double); // later, also float option?
	}


	if (isWithinRange<T>(d)){
		return typeid(T);
	}

	static const std::type_info & type = typeid(T);

	// Type::call<isSigned>(type)
	// Later, directly "inline" through type specification ?
	if (std::numeric_limits<T>::is_signed){
		// throw std::runtime_error(StringBuilder<' '>(__FUNCTION__, "  requested signed type: ", TypeName<T>::str()));
		if (&type == &typeid(signed char)){
			return minimizeIntType<signed short>(d);
		}
		else if (&type == &typeid(signed short)){
			return minimizeIntType<signed int>(d);
		}
		else if (&type == &typeid(signed int)){
			return minimizeIntType<signed long>(d);
		}
	}
	else {
		if (&type == &typeid(unsigned char)){
			return minimizeIntType<unsigned short>(d);
		}
		else if (&type == &typeid(unsigned short)){
			return minimizeIntType<unsigned int>(d);
		}
		else if (&type == &typeid(unsigned int)){
			return minimizeIntType<unsigned long>(d);
		}
	}

	return typeid(double);

}




/*
 *   \tparam C - contrainer, esp. std::list or std::vector
 */
template <class C>
const std::type_info & TypeUtils::guessArrayType(const C & l){


	typedef std::set<const std::type_info *> typeset;

	typeset s;
	for (typename C::const_iterator it = l.begin(); it != l.end(); ++it) {
		s.insert(& guessType(*it));
	}

	/// Contains at least one string
	if (s.find(& typeid(std::string)) != s.end())
		return typeid(std::string);

	/// Contains at least one decimal value
	if (s.find(& typeid(double)) != s.end())
		return typeid(double);

	if (s.find(& typeid(int)) != s.end())
		return typeid(int);

	/// Contains only \c true and \false values
	if (s.find(& typeid(bool)) != s.end())
		return typeid(bool);

	// General fallback solution
	return typeid(std::string);

}



/// Utility for implementing setType(const std::type_info &t) in classes supporting setType<T>().
class typesetter {
public:

	/**
	 *  \tparam S - target type
	 *  \tparam T - type to be analyzed
	 */
	template <class S, class T>
	static
	void callback(T & target){
		target.template setType<S>();
	}
};


/// Returns the sizeof() of a type. Accepts \c void (and returns size 0), which is not supported by std::numeric_limits.
/**
 *  Usage:
 *  Type::call<drain::sizeGetter>(t)
 *
 *  sizeof() returns size in bytes, so 1 for char, 2 for short int and so on.
 *
 *  Todo: renaming to byteSizeGetter, and adding bitSizeGetter
 */
class sizeGetter {

public:

	typedef std::size_t value_t;

	/**
	 *  \tparam S - type to be analyzed
	 *  \tparam T - destination type  (practically value_t)
	 */
	template <class S, class T>
	static inline
	T callback(){
		return static_cast<T>(getSize<S>()); // static cast unneeded in these ?
	}

protected:

	// Must be here to get template<void> implemented
	template <class S>
	static inline
	size_t getSize(){
		return sizeof(S);
	}

};

template <>
inline
size_t sizeGetter::getSize<void>(){
	return 0;
}
// todo:: std::string?







/// Returns the basic type (integer, float, bool, string, void) as a string.
/**
 *  Usage:
 *  Type::call<drain::compactName>(t)
 */
class compactName {

public:

	typedef const std::string & value_t;

	/**
	 *  \tparam S - type to be analyzed (argument)
	 *  \tparam T - return type  (practically value_t)
	 */
	template <class S, class T>
	static
	T callback(){

		static std::string s;

		if (s.empty()){
			std::stringstream sstr;
			size_t n = sizeGetter::callback<S, std::size_t>();
			if (std::numeric_limits<S>::is_specialized){
				if (std::numeric_limits<S>::is_integer){
					if (!std::numeric_limits<S>::is_signed)
						sstr << 'u';
					// sstr << ' ';
					if (n==1)
						sstr << "char";
					else
						sstr << "int";
				}
				else
					sstr << "float";
			}
			else {
				if (typeid(S) == typeid(bool))
					sstr << "bool";
				else if (n==0)
					sstr << "void";
				else
					sstr << "other"; // pointer, for example?
			}
			//sstr << (8 * n);
			s = sstr.str();
		}

		return s;

	}

};

class complexName {

public:

	//typedef std::string value_t;
	typedef const std::string & value_t;

	/**
	 *  \tparam S - type to be analyzed (argument)
	 *  \tparam T - return type  (practically value_t)
	 */
	template <class S, class T>
	static
	T callback(){
		// typedef std::numeric_limits<S> nlim;
		// return numInfo<nlim::is_specialized, nlim::is_integer, nlim::is_signed>::s;
		static std::string s;

		if (s.empty()){
			std::stringstream sstr;
			size_t n = sizeGetter::callback<S, std::size_t>();
			if (std::numeric_limits<S>::is_specialized){
				if (std::numeric_limits<S>::is_integer){
					if (std::numeric_limits<S>::is_signed)
						sstr << "signed";
					else
						sstr << "unsigned";
					sstr << ' ';
					if (n==1)
						sstr << "char";
					else
						sstr << "integer";
				}
				else
					sstr << "float";
			}
			else {
				if (typeid(S) == typeid(bool))
					sstr << "bool";
				else if (n==0)
					sstr << "void"; // "uninitialized";
				else
					sstr << "non-numeric";
			}
			sstr << " (" << (8 * n) << "b)";
			s = sstr.str();
		}

		return s;

	}



};




/// Returns the compiler specific ie. non-standard name of the type.
class nameGetter {

public:

	typedef const char * value_t;

	template <class S, class T>
	static inline
	T callback(){
		return typeid(S).name();
	}

};

/*
 *  Usage:
 *
 *  Type::call<drain::typeIsFundamental>(t)
 *
 */
class typeIsFundamental {

public:

	typedef bool value_t;

	/**
	 *  \tparam S - selector type
	 *  \tparam T - destination type (practically value_t)
	 */
	template <class S, class T>
	static inline
	T callback(){ return std::is_fundamental<S>::value; }

};

/**
 *  \tparam T - return type
 */
class isSigned {  // F2

public:

	typedef bool value_t;

	/**
	 *  \tparam S - type to be analyzed
	 *  \tparam T - destination type  (practically value_t)
	 */
	template <class S, class T>
	static inline
	T callback(){
		return static_cast<T>( std::numeric_limits<S>::is_signed);
	}

};


/// Checks if type is numeric.
/**
 *  https://en.cppreference.com/w/cpp/types/numeric_limits/is_specialized
 *
 *  Usage:
 *  Type::call<drain::typeIsScalar>(t)
 */
class typeIsScalar {

public:

	typedef bool value_t;

	/**
	 *  \tparam S - selector type
	 *  \tparam T - destination type (practically value_t)
	 */
	template <class S, class T>
	static inline
	T callback(){ return std::numeric_limits<S>::is_specialized; }

};

/**
 *  Usage:
 *  Type::call<drain::typeIsInteger>(t)
 *
 */
class typeIsInteger { // F2

public:

	typedef bool value_t;

	/**
	 *  \tparam S - selector type
	 *  \tparam T - destination type (practically value_t)
	 */
	template <class S, class T>
	static inline
	T callback(){ return std::numeric_limits<S>::is_integer; }

};

/**
 *  Usage:
 *  Type::call<drain::typeIsFloat>(t)
 *
 */
class typeIsFloat { // F2

public:

	typedef bool value_t;

	/**
	 *  \tparam S - selector type
	 *  \tparam T - destination type (practically value_t)
	 */
	template <class S, class T>
	static inline
	T callback(){
		return (typeid(S)==typeid(float)) || (typeid(S)==typeid(double));
		//return std::numeric_limits<S>::is_float;
	}
	// { return (typeid(S)==typeid(float)) || (typeid(S)==typeid(double)); }

};


/// The maximum value of a type given as type_info, char or std::string.
/**
 *  Usage:
 *  \code
 *    Type::call<drain::typeIsSmallInt>(t)
 *  \endcode
 */
class typeIsSmallInt {

public:

	typedef bool value_t;

	/**
	 *  \tparam S - type selector
	 *  \tparam D - destination type (practically value_t)
	 */
	template <class S, class D>
	static inline
	D callback(){
		return (typeid(S) == typeid(char)) || (typeid(S) == typeid(unsigned char)) || (typeid(S) == typeid(short)) || (typeid(S) == typeid(unsigned short));
	}
};




/// The minimum value of a type given as type_info, char or std::string.
/**
 *  Usage:
 *  \code
 *    double d = Type::call<typeMin, double>(t);
 *  \endcode
 */
class typeMin {

public:

	//typedef T value_t;

	/**
	 *  \tparam S - type selector
	 *  \tparam D - destination type
	 */
	template <class S, class D>
	static inline
	D callback(){
		if (std::numeric_limits<S>::is_integer)
			return static_cast<D>(+typeLimits<S>::getMinI());
		else
			return static_cast<D>(-typeLimits<S>::getMaxF());
	}
};


/// The maximum value of a type given as type_info, char or std::string.
/**
 *  Usage:
 *  \code
 *    double d = Type::call<typeMax, double>(t);
 *  \endcode
 */
class typeMax {

public:

	//typedef T value_t;

	/**
	 *  \tparam S - type selector
	 *  \tparam D - destination type
	 */
	template <class S, class D>
	static inline
	D callback(){
		if (std::numeric_limits<S>::is_integer)
			return static_cast<D>(+typeLimits<S>::getMaxI());
		else
			return static_cast<D>(+typeLimits<S>::getMaxF());
	}
};

/// The maximum value of a
/**
 *  Type can be given as type_info, char or std::string.
 *
 *  Usage:
 *  \code
 *    double d = Type::call<typeNaturalMax>(t);
 *  \endcode
 */
class typeNaturalMax {

public:

	typedef double value_t;

	/**
	 *  \tparam S - type selector
	 *  \tparam D - destination type
	 */
	template <class S, class D>
	static inline
	D callback(){
		if (typeIsSmallInt::callback<S,bool>())
			return static_cast<D>(typeLimits<S>::getMaxI());
		else
			return static_cast<D>(1.0);
	}
};



// Experimental template exercise... (works ok)
/**
 *  \tparam N - is numeric
 *  \tparam T - is integer
 *  \tparam S - is signed
template <bool N, bool T, bool S> //, std::string STR = "">
struct numInfo { //: protected numInfoBase {
	static std::string s;
};

// Default: non-numeric (N==false)
template <bool N, bool T, bool S>
std::string complexName::numInfo<N,T,S>::s("non-numeric");

// Numeric, float
template <bool S>
struct complexName::numInfo<true,false,S> {
	static std::string s;
};

template <bool S>
std::string complexName::numInfo<true,false,S>::s("float");

// Numeric, integer
template <bool S>
struct complexName::numInfo<true,true,S> {
	static std::string s; //("non-numeric");
};

template <>
std::string complexName::numInfo<true,true,true>::s("signed integer");

template <>
std::string complexName::numInfo<true,true,false>::s("unsigned integer");
 */


} // drain::

#endif /* TypeUTILS_H_ */
