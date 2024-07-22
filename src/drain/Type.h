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
#ifndef DRAIN_TYPE
#define DRAIN_TYPE

#include <typeinfo>
//#include <type_traits>
//#include <limits>
#include <list>
#include <set>
#include <stdexcept>

//#include "Log.h"
#include "RegExp.h"

namespace drain {


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

	//  TODO for std::strings.
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
			//Logger mout("Type", std::string(__FUNCTION__)+"(char c)");
			//Logger mout(__FILE__, __FUNCTION__);
			//mout.error(" undefined type: '" , t , "'=" , (int)t );
			std::cerr << __FILE__ << ' ' << __FUNCTION__ << " undefined type: '" << t << "'=" << (int)t << std::endl;
			//mout.error(" undefined type: '" , t , "'=" , (int)t );
			throw std::runtime_error("undefined type");
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
			return 'x'; // = "extension"
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
		// NOTE: STL uses string comparison!!!
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
		/*
		else if (Type::call<isEnum>(t)){ // NEW 2023
			// Problem: looping if not enum...
			return F::template callback<int,T>();
		}
		*/
		else {
			std::cerr << __FILE__ << ' ' << __FUNCTION__ << "unimplemented type: ..." << t.name() << std::endl;
			//mout.error(" undefined type: '" , t , "'=" , (int)t );
			throw std::runtime_error("unimplemented type: ...");
			//Logger mout(__FILE__, __FUNCTION__);
			// mout.error("unimplemented type: ...", t.name(), " NOTE: enums suppressed");
			//return T(); //F::template callback<char,T>();
			// Problem with ref types
			return F::template callback<char,T>();
		}

	}

	/// New, preferred implementation: a single if-then structure once and for all.
	/**
	 *   \tparam F - struct implementing: T callback<S>()
	 *   \tparam D - destination type
	 *   \tparam S - selector type (char, std::string, drain::Type)
	 */
	template <class F, class D, class S>
	static 	inline
	D call(const S & typeCode){
		return call<F,D>(getTypeInfo(typeCode));
	}

	/// A shorthand for functors with a fixed return type, hence D = F::value_t
	/**
	 *   Simpler template with two arguments
	 *
	 *   \tparam F - struct implementing: T::callback<S>() AND value_t (replacing destination type S)
	 *   \tparam S - selector type (char, std::string, drain::Type)
	 */
	template <class F, class S>
	static 	inline
	typename F::value_t call(const S & typeCode){
		return call<F,typename F::value_t>(getTypeInfo(typeCode));
	}


	/// Static function call without parameters on a single target of type T.
	/**
	 *   Maps std::type_info to the corresponding template, calling a desired modifier function.
	 *
	 *   \tparam F - struct that implements static void callback<type>(T & target), where \c <type> is basetype or std::string.
	 *   \tparam T - target object class
	 *
	 *  Example struct F:
	 *  \code

	    class typesetter {
        public:
  	      // param S - target type
	      // param T - type to be analyzed
	      template <class S, class T>
	      static
	      void callback(T & target){
		    target.template setType<S>();
          }
        };

	    Type::call<typesetter>(obj, typeid(double));

	    \endcode

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
		}
		/*
		else if (Type::call<isEnum>(t)){
			F::template callback<int>(target);
		}
		*/
		else {
			std::cerr << __FILE__ << ' ' << __FUNCTION__ << "unimplemented type: ..." << t.name() << std::endl;
			//mout.error(" undefined type: '" , t , "'=" , (int)t );
			throw std::runtime_error("unimplemented type: ...");

			//Logger mout(__FILE__, __FUNCTION__);
			//mout.error("unimplemented type: ...", t.name(), " NOTE: enums suppressed");
			// throw std::runtime_error(std::string(": unimplemented type: ") + t.name() + " NOTE: enums suppressed");
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

	/*
	static
	const drain::RegExp trueRegExp;

	static
	const drain::RegExp falseRegExp;
	*/

	/// Given a string, check if it could be stored as \c int or \c double instead of \c std::string .
	/**
	 *  \return - std::type_info in priority order: int, double, std::string.
	 */
	/*
	static
	const std::type_info & guessType(const std::string & value);

	/// Given a vector or list of strings, suggest a matching storage type (int, double, std::string).
	template <class C>
	static
	const std::type_info & guessArrayType(const C & container);
	*/

	static inline
	std::ostream & toStream(std::ostream & ostr, const Type &t){
		const std::type_info & type = t.getType();
		ostr << Type::getTypeChar(type) << '@'; // << Type::call<drain::sizeGetter>(type);
		return ostr;
	}

	static inline
	std::ostream & toStreamFull(std::ostream & ostr, const Type &t){
		toStream(ostr, t);
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

	struct isEnum {

		typedef bool value_t;

		/**
		 *  \tparam S - type to be analyzed (argument)
		 *  \tparam T - return type  (practically value_t)
		 */
		template <class S, class T>
		static
		T callback(){
			return std::is_enum<S>::value;
		}


	};

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

};

/**
 *   \tparam C - contrainer, esp. std::list or std::vector
 */
/*
template <class C>
const std::type_info & Type::guessArrayType(const C & l){


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
*/

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
	Type::toStream(ostr, t);
	return ostr;
}

}

#endif /* Type2_H_ */
