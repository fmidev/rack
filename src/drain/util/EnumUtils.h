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

#ifndef DRAIN_ENUM_UTILS
#define DRAIN_ENUM_UTILS

//#include <drain/util/StringTools.h>

#include "Dictionary.h"

#include "Flags.h"

namespace drain {



/// A container for a static dictionary of enumeration values.
/**
 *  DrainCore
 *
 *  \tparam E - enumeration or integer type
 *  \tparam OWNER - identifier type (separate type, typically a class, can be used if several dictionaries needed for the same enumeration type)
 *
 *  Template is needed to create a unique, shared (static) dict object for each template.
 */
template <class E, class OWNER=E>
struct EnumDict {

	//typedef FlagResolver::dict_t dict_t;
	typedef drain::Dictionary<std::string,E> dict_t;

	static
	const dict_t dict;

	static
	const dict_t & getDict(){
		return dict;
	}



	/// Convenience
	static inline
	const std::string & str(const E & value){
		return dict.getKey(value);
	}


	/// Assign string values to an enumeration type.
	/**
	 *  \tparam E - some enumeration type for which Dictionary \c dict has been defined.
	 */
	static
	bool setValue(const std::string & key, E & value){  // NOTE:  could be more general, without explicit template
		if (drain::EnumDict<E>::dict.hasKey(key)){
			value = drain::EnumDict<E>::dict.getValue(key);
			return true; // assigned
		}
		else {
			return false; // not found
		}
	}


	/// Convenience for object.set(...) like commands.
	/**
	 *   \return enum value of the key
	 *
	 *   Note: cannot return (const E &) as storage type is (long) int.
	 */
	static inline
	E getValue(const E & value, bool lenient=true){
		return value;
	}

	/// Convenience for object.set(...) like commands.
	/**
	 *   \param lenient - when key not found, only return empty, do not throw exception.
	 *   \return enum value of the key
	 */
	static inline
	E getValue(const std::string &key, bool lenient=true){
		return dict.getValue(key, lenient);
	}

	/// Convenience for object.set(...) like commands.
	/**
	 *   \param lenient - when value not found, only return empty, do not throw exception.
	 *   \return enum value of the key
	 */
	static inline
	E getValue(const char *key, bool lenient=true){
		return dict.getValue(key, lenient);
	}

	/*
	static inline
	E getValue(char * const key, bool lenient=true){
		return dict.getValue(key, lenient);
	}
	*/

	/// Convenience for object.set(...) like commands.
	/**
	 *   \return enum value of the key
	 *
	 *   Note: cannot return (const E &) as storage type is (long) int.
	static inline
	E getKey(const E & value, bool lenient=true){
		return value;
	}
	 */

	/// Experimental. Convenience for object.set(...) like commands.
	/**
	 *   \param lenient - when key not found, only return empty, do not throw exception.
	 *   \return enum value of the key
	 */
	static inline // TODO: remove/handle misleading lenient here
	const std::string & getKey(const std::string & s, bool lenient=true){
		return s;
	}

	static inline // TODO: remove/handle misleading lenient here
	const char * getKey(const char * s, bool lenient=true){
		return s;
	}

	static inline // TODO: remove/handle misleading lenient here
	const std::string & getKey(const E & value, bool lenient=true){
		return dict.getKey(value, lenient);
	}

	/*
	static inline
	const std::string & getKey(const char *value, bool lenient=true){
		//return dict.getKey(std::string(value), lenient);
		return dict.getKey(std::string(value), lenient);
	}
	*/

	/// Experimental. Convenience for object.set(...) like commands.
	/**
	 *   \param lenient - when value not found, only return empty, do not throw exception.
	 *   \return enum value of the key

	static inline
	const std::string & getKey(const std::string & value, bool lenient=true){
		return value;
	}
	 */

};

/**
 *   If you want to support assigning several enum sets, use StringWrapper<>
 *   with
 */
template <typename E, bool STRICT=true>
class EnumWrapper : public std::string { // StringWrapper<E> {

public:

	inline
	EnumWrapper(const std::string & s=""){ //  : std::string(s){
		set(s);
	};

	/// All the other constructors, including default constructor.
	inline
	EnumWrapper(const E & x){
		set(x);
	};

	template <typename T>
	void set(const T & value){
		// handle empty with default enum value?
		assign(drain::EnumDict<E>::dict.getKey(value, !STRICT));
	}

	/*
	virtual inline
	void setSpecial(const E & x) override {
		assign(drain::EnumDict<E>::dict.getKey(x, !STRICT));
	};
	*/

};

class MultiEnumWrapper : public std::string { // StringWrapper<E> {

public:

	inline
	MultiEnumWrapper(const std::string & s="") : std::string(s){
	};

	inline
	MultiEnumWrapper(const char *s) : std::string(s){
	};

	/// Default constructor
	// inline
	// MultiEnumWrapper(const MultiEnumWrapper & wrapper) : std::string(wrapper){
	// };

	/// All the other constructors.
	template <typename T>
	inline
	MultiEnumWrapper(const T & arg){
		set(arg);
	};


	void set(const std::string &arg){
		assign(arg);
	}

	void set(const char * arg){
		assign(arg);
	}

	void set(const MultiEnumWrapper &arg){
		assign(arg);
	}

	template <typename T>
	void set(const T & value){
		assign(drain::EnumDict<T>::dict.getKey(value, false));
	}

};



/* Perhaps useful!
template <class E>
class EnumKey {

public:

	inline
	EnumKey(const E & value) : key(EnumDict<E>::dict.getKey(value)){
	};

	inline
	operator const std::string & () const {
		return key;
	}

protected:

	const std::string & key;
};
*/


/// Flagger accepting values of enum type E.
/**
 *  \tparam F – SingleFlagger<E> or MultiFlagger<E>
 *
 *  Enum dictionary should be defined as follows:
 *
 *  template <>
 *  const drain::EnumDict<...>::dict_t  drain::EnumDict<...>::dict = {{"A", A}, {"B", B}, {"C", C}};
 *
 */
template <class F>  // F =SingleFlagger<E>
class EnumFlagger : public F {

public:

	typedef F fbase_t;
	typedef typename F::value_t value_t;
	typedef typename F::storage_t storage_t;
	typedef typename F::dict_t  dict_t;
	typedef FlagResolver::ivalue_t ivalue_t;

	/// Default constructor
	inline
	EnumFlagger(): fbase_t(EnumDict<value_t>::dict){
	}

	/// Constructor with initial value
	inline
	EnumFlagger(const storage_t & v): fbase_t(EnumDict<value_t>::dict) {
		this->value = v;
	}

	/*
	inline
	EnumFlagger(value_t v): fbase_t(v) {
	}
	*/

	///
	template <typename ... T>
	inline
	EnumFlagger(const T & ... arg): fbase_t(EnumDict<value_t>::dict, arg...) { // designed for MultiFlagger
	}

	/*  // keep
	virtual void reset() override {
		this->value = EnumDict<value_t>::defaultValue; // ALERT! enums need neutral value.
	};
	*/

	/// Returns the static dictionary created for this \c value_t .
	/**
	 *  Notice that this function is non-static but returns a reference to a static dictionary.
	 *  In C++ virtual functions must be non-static.
	 *  This is nevertheless handy in templated design, for example.
	virtual
	const dict_t & getDict() const override {
		return EnumDict<value_t>::dict;
	};
	 */

	/// Return the integer value corresponding to a key.
	/**
	 *  Retrieves the value directly from the dictionary.
	 *  Dictionary throws except if key now found.
	 */
	static
	ivalue_t getValueNEW(const std::string & key){
		return (ivalue_t)EnumDict<value_t>::dict.getValue(key);
	};

	// Raise?
	/*
	static inline
	std::string getKeysNEW2(const storage_t & value, char separator = ','){
		// currentStr = FlagResolver::getKeys(dict, this->value, this->separator);
		return FlagResolver::getKeys(EnumDict<value_t>::dict, value, separator);
	}
	*/

	/**
	 *  Dictionary throws except if key now found.
	static
	value_t getKeys(const value_t & value){
		return (value_t)EnumDict<value_t>::dict.getValue(key);
	};
    */

	/**
	 *  Consider handler through this->set(Flagger) ? Needs protected/private _set() (or assign()).
	 */
	inline
	EnumFlagger<F> & operator=(const EnumFlagger<F> & flagger){
		this->set(flagger.value);
		return *this;
	}

	template <class T>
	inline
	EnumFlagger<F> & operator=(const T & v){
		this->set(v);
		return *this;
	}

};


} // drain::

#define DRAIN_ENUM_DICT(enumtype)        template <> const drain::EnumDict<enumtype>::dict_t drain::EnumDict<enumtype>::dict
#define DRAIN_ENUM_DICT2(enumtype,owner) template <> const drain::EnumDict<enumtype,owner>::dict_t drain::EnumDict<enumtype,owner>::dict

#define DRAIN_ENUM_ENTRY(nspace, key) {#key, nspace::key}

#define DRAIN_ENUM_OSTREAM(enumtype) inline std::ostream & operator<<(std::ostream &ostr, const enumtype & e){return ostr << drain::EnumDict<enumtype>::dict.getKey(e);}

/**
 * #include <drain/Converter.h>
 */
#define DRAIN_ENUM_CONV(enumtype)  \
		template <> inline void Converter<enumtype>::convert(const enumtype & value, std::string &s){s.assign(drain::EnumDict<enumtype>::dict.getKey(value));};
//		template <> inline void Converter<enumtype>::convert(const std::string &key, enumtype & value){value = drain::EnumDict<enumtype>::dict.getValue(key));};

#endif
