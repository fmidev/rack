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

#ifndef DRAIN_FLAGS_H_
#define DRAIN_FLAGS_H_

#include <stdexcept>
#include <iostream>
#include <list>
#include <iterator>
#include <iostream>
#include <sstream>
#include <list>

#include <typeinfo>

#include "Log.h"
#include "Dictionary.h"
#include "String.h"

namespace drain {

/// Referencing a dictionary of binary values: {"A",1: "B":2, "C": 4, "D": 8, ...} resolves two-way mappings "A,C" <=> 5 .
/** Static utilities.
 *
 */
class FlagResolver {

public:

	// Definite
	typedef std::string  key_t;

	// "Recommended"
	typedef long unsigned int value_t;

	// "Recommended"
	typedef drain::Dictionary<key_t,value_t> dict_t;

	static
	const value_t ALL; // = std::numeric_limits<FlagResolver::value_t>::max();


	/// Computes bitwise OR function on the numeric or alphabetic value(s) presented by a string.
	/**
	 *   \param key – A string containing keys ("note,error") or values ("4,8,32") or a mixture of them.
	 *   \param separator – if given, splits string to several keys
	 *   \return – A numeric value
	 */
	// Consider: lenient: if true, bypasses unknown keys silently, otherwise throws exception
	// Consider: FLAG for keys only, and for numerics throw exception
	template <typename T>
	static
	value_t getValue(const drain::Dictionary<key_t,T> & dict, const std::string & key, char separator=',');

	/// Given an integer, retrieves dictionary keys corresponding to each index of set bits.
	template <typename T>
	static
	std::string getKeys(const drain::Dictionary<key_t,T> & dict, value_t, char separator=',');

	//std::string FlagResolver::keysToStr(const dict_t &dict, value_t v, char separator){
	template <typename T>
	static
	std::ostream & keysToStream(const drain::Dictionary<key_t,T> &dict, value_t value, std::ostream & ostr, char separator=',');

	/// Add a new entry in the dictionary
	/**
	 *  \param i - if zero, call getFreeBit() to reserve a new, unused bit.
	 */
	template <typename T>
	static
	value_t addEntry(drain::Dictionary<key_t,T> & dict, const key_t & key, value_t i=0);


	/// Return an interger (bit vector) with a new, previously unused value.
	/**
	 *   Traverses the values in dict, and determines a new bit corresponding to a new value.
	 */
	template <typename T>
	static
	value_t getFreeBit(const drain::Dictionary<key_t,T> & dict);


};


template <typename T>
typename FlagResolver::value_t FlagResolver::getFreeBit(const drain::Dictionary<key_t,T> & dict){

	drain::Logger mout(__FUNCTION__, __FILE__);

	// Bit mask containing all the values.
	value_t currentMask = 0;
	for (const auto & entry: dict){
		currentMask = currentMask | entry.second;
	}

	mout.debug2() << "sum:" << currentMask << mout.endl;

	value_t i = 1;
	while ((i&currentMask) > 0){
		mout.debug3() << "checking bit:" << i << " vs.\t" << currentMask << mout.endl;
		//full = (full>>1);
		i = (i<<1);
	}
	/*
	while (full>0){
		mout.debug2() << "checking bit:" << i << " vs.\t" << full << mout.endl;
		full = (full>>1);
		i = (i<<1);
	}
	*/

	mout.debug() << "released: " << i << mout.endl;

	return i;

}

/*
template <typename T>
typename FlagResolver::value_t FlagResolver::addEntry(drain::Dictionary<key_t,T> &dict, const typename dict_t::key_t & key){

	drain::Logger mout(__FUNCTION__, __FILE__);
	if (dict.hasKey(key)){
		mout.info() << key << " already in dict: " << dict << mout.endl;
		return dict.getValue(key);
	}

	return addEntry(dict, key, )
}
*/

template <typename T>
typename FlagResolver::value_t FlagResolver::addEntry(drain::Dictionary<key_t,T> &dict, const typename dict_t::key_t & key, value_t i){

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (dict.hasKey(key)){
		mout.info() << key << " already in dict: " << dict << mout.endl;
		return dict.getValue(key);
	}

	if (i==0)
		i = getFreeBit(dict);

	if (i>0){
		dict.add(key, i);
	}
	else {
		mout.warn() << key << " could not get a valid (non-zero) bit flag for dict: " << dict << mout.endl;
	}

	return i;
};


template <typename T>
typename FlagResolver::value_t FlagResolver::getValue(const drain::Dictionary<key_t,T> & dict, const std::string & args, char separator){

	drain::Logger mout(__FUNCTION__, __FILE__);

	value_t v = 0;

	std::list<key_t> keys;

	if (!separator)
		separator = dict.separator;


	drain::StringTools::split(args, keys, separator);

	for (const key_t & key: keys){

		if (key == ""){
			continue;
		}
		else if (key == "0"){
			v = 0; // TODO if resets and sets?
			continue;
		}
		// mout.warn() << " '" << *it << "'" << mout.endl;

		//typename dict_t::const_iterator dit = dict.findByKey(key);
		typename drain::Dictionary<key_t,T>::const_iterator dit = dict.findByKey(key);

		if (dit != dict.end()){ // String key match,
			// Numeric value for an alphabetic key was found
			v = (v | dit->second);
		}
		else {
			// Numeric value
			value_t x;
			std::stringstream sstr(key);
			sstr >> x; // FIX if empty
			if (x == 0){
				// String "0" handled already above
				mout.error("key '", key, "' not found in Flags, dict: ", dict);
				//throw std::runtime_error(key, ": key not found in Flags, dict: ", dict);
			}
			v = v | x;
			// Nice to know
			/*
			dict_t::const_iterator vit = dictionaryRef.findByValue(v);
			if (vit != dictionaryRef.end()){
				std::cout << "(assigned key '" << vit->second << "')\n"; // or vit->first?
			}
			*/
		}
	}

	return v;
}


/// List keys in their numeric order.
template <typename T>
std::string FlagResolver::getKeys(const drain::Dictionary<key_t,T> &dict, value_t v, char separator){
	std::stringstream sstr;
	keysToStream(dict, v, sstr, separator);
	return sstr.str();
}



//

/// List keys in their numeric order.
template <typename T>
std::ostream & FlagResolver::keysToStream(const drain::Dictionary<key_t,T> &dict, value_t value, std::ostream & ostr, char separator) {

	/* note: instead of shifting bits of this->value, traverses the dictionary which can contain
		- combined values
		- repeated values (aliases)
	*/

	if (!separator)
		separator = dict.separator;

	char sep = 0;

	for (const auto & entry: dict){
		if ((entry.second > 0) && ((entry.second & value) == entry.second)){ // fully covered in value
			if (sep)
				ostr <<  sep;// "{" << (int)(sep) << "}" <<
			else
				sep = separator;
			ostr << entry.first;
		}
		else {

		}
	}

	return ostr;
}







/// A bit vector with external Dictionary mapping from strings to bits and vice versa.
class Flagger :public FlagResolver { // for typedefs

public:

	value_t ownValue;
	value_t & value;

	const dict_t & dictionary; // todo open (keep own secret, later)

	/*
	virtual inline
	const dict_t & getDict() const {
		return dictionary;
	}
	*/

	char separator;  // needed?

	inline
	Flagger(const dict_t & dict, char separator = 0) :
		ownValue(0),
		value(ownValue),
		dictionary(dict),
		separator(separator?separator:dict.separator){
	};

	inline
	Flagger(value_t & value, const dict_t & dict, char separator = 0) :
		ownValue(0),
		value(value),
		dictionary(dict),
		separator(separator?separator:dict.separator){
	};

	inline
	Flagger(const Flagger & flags) : //FlagResolver(flags.dictionary, flags.separator), value(flags.value){
		ownValue(flags.value), // OK init
		value(flags.value),  // <-- FIX: this will cause error (non-const referencing const). Should be
		dictionary(flags.dictionary),
		separator(flags.separator){

		drain::Logger mout(__FUNCTION__, __FILE__);
		mout.error("Flagger inits value &= flags.value", flags);
	};


	inline virtual
	~Flagger(){};


	/// Set desired flags. Does not reset any flag (zero value has no effect). \see assign().
	// Variadic-argument member set function.
	// https://en.cppreference.com/w/cpp/language/parameter_pack
	// TODO: should explicit zero reset the value?
	template <typename T, typename ... TT>
	inline
	Flagger & set(const T & arg, const TT &... args) {
		deepSet(arg);
		set(args...);
		return *this;
	}


protected:

	inline
	Flagger & set(){
		return *this;
	}

	// TODO: should explicit zero reset the value?
	inline
	Flagger & deepSet(value_t x){
		this->value = (this->value | x);
		return *this;
	}


	/// Set desired flags. Does not reset any flag. \see assign().
	/**
	 *
	 */
	inline
	Flagger & deepSet(const key_t & key){
		deepSet(getValue(key, separator));
		return *this;
	};



public:


	/// Unset desired flags. Does not set any flag.
	inline
	Flagger & unset(value_t x){
		value = (value & ~x);
		return *this;
	};

	/// Unset desired flags. Does not set any flag.
	inline
	Flagger & unset(const key_t & key){
		//unset(dictionary.getValue(key, separator));
		unset(getValue(key, separator));
		return *this;
	};


	/// Reset all the flags.
	inline
	Flagger & reset(){
		value = 0;
		return *this;
	}

	/// Checks if a given bit, or any of given bits, is set.
	inline
	bool isSet(value_t x) const {
		return (value & x) != 0;
	};

	inline
	bool isSet(const key_t & key) const {
		return isSet(dictionary.getValue(key));
	};


	// For some reason this cannot be templated (see below)
	inline
	Flagger & operator=(const Flagger &x){
		assign(x);
		return *this;
	}

	/// Sets value, ie. set or unsets all the flags.
	/**
	 *   \param x - bit values 1, 2, 4, 8 ... etc combined with \c OR function.
	 */
	template <class T>
	inline
	Flagger & operator=(const T &x){
		assign(x);
		return *this;
	}

	/// Sets value, ie. set or unsets all the flags. \see set().
	/**
	 *   \param x - bit values 1, 2, 4, 8 ... etc combined with \c OR function.
	 */
	inline
    void assign(value_t x){
		value = x;
	}

	/// Copies flags as an integer \c value. Same dictionary not checked. \see set().
	/**
	 *   \param flags -
	 */
	inline
	void assign(const Flagger & flags){
		value = flags.value;
	}

	/// Set flags, like 'FILEOK=1,WARN=0'
	/**
	 *   \param x - comma separated keys or bit values 1, 2, 4, 8
	 */
	void assign(const std::string & params);


	inline
	operator value_t() const {
		return value;
	}

	inline
	operator bool() const {
		return (value > 0);
	}

	inline
	operator std::string() const {
		std::stringstream sstr;
		keysToStream(sstr);
		//toStream(sstr);
		return sstr.str();
	}



	/// Computes bitwise OR function on the numeric or alphabetic value(s) presented by a string.
	/**
	 *   \param key - String containing keys ("note,error") or values ("4,8,32") or a mixture of them.
	 *   \param lenient - If true, bypassed unknown keys silently, otherwise throws exception
	 */
	inline
	value_t getValue(const std::string & key, char separator=0) const {
		// important: if sep undefined, use Flags.sep.
		// Or/consider Flag dict sep => '|'
		return FlagResolver::getValue(dictionary, key, separator ? separator : this->separator);
	}

	/// Given an integer, retrieves dictionary keys corresponding to each index of set bits.
	inline
	std::string getKeys(value_t value, char separator=0) const {
		return FlagResolver::getKeys(dictionary, value, separator ? separator : this->separator);
	}

	/// Given an integer, retrieves dictionary keys corresponding to each index of set bits.
	inline
	std::string getKeys(char separator=0) const {
		return FlagResolver::getKeys(dictionary, value, separator ? separator : this->separator);
	}

	// TODO consider rename
	const dict_t::keylist_t & keys() const;

protected:

	mutable dict_t::keylist_t keyList;

	//mutable dict_t::valuelist_t valueList;

public:


	/// Display current value as key values of which the value is composed with \c OR function.
	/**
	 *   \param ostr
	 *   \param separator - character appended between keys, if not own separator
	 */
	///  risky: value_t ~char !
	inline
	std::ostream & keysToStream(std::ostream & ostr=std::cout, char separator=0) const {
		return FlagResolver::keysToStream(dictionary, value, ostr, separator ? separator : this->separator);
	}

	//std::ostream & toStream(std::ostream & ostr=std::cout, char separator=',') const; // PathMatcher '|' !
	//std::ostream & toStream(std::ostream & ostr=std::cout, char separator=0) const; // PathMatcher '|' !


	template <class T>
	void exportStatus(std::map<std::string, T> & statusMap) const {
		for (dict_t::const_iterator it = dictionary.begin(); it != dictionary.end(); ++it){
			if ((it->second > 0) && ((it->second & value) == it->second)){ // fully covered in value
				statusMap[it->first] = 1;
			}
			else {
				statusMap[it->first] = 0;
			}
		}
	}


	static
	const drain::SprinterLayout flagDictLayout;


};


inline
std::ostream & operator<<(std::ostream & ostr, const drain::Flagger & flags) {
	return flags.keysToStream(ostr);
}


/// Self-contained Flagger – with an own dictionary and value.
class Flags : public Flagger {

protected:

	dict_t  ownDictionary;

public:

	virtual inline
	const dict_t & getDict() const{
		return ownDictionary;
	}


	virtual inline
	dict_t & getDict(){
		return ownDictionary;
	}


	Flags(char separator = ',') : Flagger(ownDictionary, separator){
	}

	//Flags(dict_t & d, char separator = ',') : Flagger(ownValue, d, separator){	}

	Flags(const Flags & flags) : Flagger(ownValue = flags.ownValue, ownDictionary, flags.separator){
		// Copy dict?
	}

	/*
	 * std::initializer_list<entry_t> d
	 */
	/// Initialize with {{"first", 123}, {"second", 456}}
	Flags(std::initializer_list<dict_t::entry_t> d, char separator = ',') : Flagger(ownValue, ownDictionary, separator) { //, ownDictionary(d){
		for (const dict_t::entry_t & entry: d){
			addEntry(entry.first, entry.second);
		}
	}



	/// Sets value, ie. set or unsets all the flags.
	template <class T>
	inline
	Flags & operator=(const T &x){
		assign(x);
		return *this;
	}

	/// Add a new entry in the dictionary
	/**
	 *  \param i - if zero, call getFreeBit() to reserve a new, unused bit.
	 */
	inline
	value_t addEntry(const dict_t::key_t & key, value_t i=0){
		return Flagger::addEntry(ownDictionary, key, i);
	}

	/*
	inline
	value_t add(const dict_t::entry_t & entry){
		return Flagger::add(ownDictionary, entry.first,  entry.second);
	}
	*/

protected:

	/*
	bool usesOwnDict() const { //??
		return (&dictionary == &dictionary);
	}
	*/
};




// NEW 2022/2023


typedef drain::Dictionary<std::string,unsigned long> FlaggerDict;

/** A Flag mapper designed for global use, also supporting ENUM types. @See
 *  Perhaps replacing GlobalFlagger later.
 *
 *  Example:
   \code
   enum Crit {DATA, ELANGLE, TIME};
    template <>
    const SingleFlagger<Crit>::dict_t SingleFlagger<Crit>::dict = {{"DATA", DATA}, {"ELANGLE", ELANGLE}, {"TIME", TIME}};
   \endcode

    Inside a class:
   \code
    class Reader {
      public:
	  enum Mode {ATTRIBUTES=1, DATASETS=2};
    }
    // In the same or another header file:
    template <>
    const drain::SingleFlagger<Reader::Mode>::dict_t drain::SingleFlagger<Reader::Mode>::dict;
    // In the definition file:
    template <>
    const drain::SingleFlagger<Reader::Mode>::dict_t drain::SingleFlagger<Reader::Mode>::dict =
          {{"ATTRIBUTES", Reader::ATTRIBUTES}, {"DATASETS", Reader::DATASETS}};
   \endcode
 *
 * \tparam E - enum or integer
 * \tparam D - dictionary, possibly const
 */
template <typename E=std::size_t> //, typename D=drain::Dictionary<std::string,unsigned long> >
class  FlaggerBase {

public:

	// Manifested numeric type (enum or unsigned integer)
	typedef E value_t;

	typedef FlaggerDict dict_t;
	typedef typename dict_t::key_t key_t; // ~string

	// Practical "storage" value
	typedef typename dict_t::value_t dvalue_t;

	// Rember to add an initialized unit: template<> SingleFlagger<...>::dict = {{...,...}, ...}
	// static const dict_t dict;

	/**
	 *
	 */
	inline
	FlaggerBase(): value(ownValue), separator(','), ownValue((value_t)0){
	}

	/// Own value will be unused (and invisible).
	/**
	 *   Reconsider design. Should the value ever be referenced?
	 */
	inline
	FlaggerBase(value_t & v): value(v), separator(','){
	}

	/*  Risky? (Ambiguous)
	inline
	FlaggerBase(const value_t & v): value(ownValue), ownValue(v) {
	}
	*/

	virtual inline
	~FlaggerBase(){
	}

	virtual
	const dict_t & getDict() const = 0;

	void reset(){
		this->value = value_t(0);
	};

	virtual
	inline
	void set(const key_t & key){

		if (key.empty())
			return;

		if (this->getDict().hasKey(key)){
			this->value = (value_t)this->getDict().getValue(key); // why cast? dvalue_t -> value_t
		}
		else {
			throw std::runtime_error(std::string("Dictionary[") + typeid(value_t).name()+ "]: no such key: "+ key);
		}
	}

	virtual inline
	void set(const value_t & value){
		this->value = value;
	};

	virtual inline
	void set(const FlaggerBase<E> & flagger){
		this->value = flagger.value;
	}


	virtual
	void assign(const std::string & s) = 0;

	/// Checks if a given bit, or any of given bits, is set.
	inline
	bool isSet(value_t x) const {
		return (value & x) != 0;
	};

	inline
	bool isSet(const key_t & key) const {
		return isSet(getDict().getValue(key));
	};

	inline
	const value_t & getValue() const {
		return value;
	}

	/// Given an integer, retrieves dictionary keys corresponding to each index of set bits.
	/*
	inline
	std::string getKeys(char separator=0) const {
		return FlagResolver::getKeys(dictionary, value, separator ? separator : this->separator);
	}
	*/


	///
	inline
	operator const value_t & () const {
		return value;
	}

	inline
	operator value_t & () {
		return value;
	}

	/// String corresponding the current value. Returns empty, if not found.
	virtual
	const key_t & str() const = 0;

	// String
	operator const key_t & () const {
		return str();
	}

	// Own or external value.
	value_t & value;

	char separator;

	void debug(std::ostream & ostr) const;

private:

	// Own value, discarded if external value referenced.
	value_t ownValue;


};

template <typename E>
void drain::FlaggerBase<E>::debug(std::ostream & ostr) const {
	ostr << typeid(drain::FlaggerBase<E>).name() << ": value=" << getValue() << ", ";
	FlagResolver::keysToStream(getDict(), getValue(), ostr) << ", ";
	ostr <<  " dict: " << getDict();
}



template <typename E>
class SingleFlagger : public FlaggerBase<E> {

public:

	typedef typename FlaggerBase<E>::value_t value_t;

	typedef FlaggerDict dict_t;
	typedef typename dict_t::key_t key_t; // ~string
	typedef typename dict_t::value_t dvalue_t;

	inline
	SingleFlagger(){
	};

	inline
	SingleFlagger(value_t v){
		this->value = v;
	};

	virtual inline
	~SingleFlagger(){};

	virtual
	void assign(const std::string & s){
		this->set(s);
	};

	template <class T>
	inline
	SingleFlagger<E> & operator=(const T & v){
		this->set(v);
		return *this;
	}

	/// String corresponding the current value. Returns empty, if not found.
	virtual inline
	const key_t & str() const {
		//static const key_t SINGLE="SINGLE";
		//return SINGLE;
		return this->getDict().getKey(this->value);
	}



};

template <typename E>
inline
std::ostream & operator<<(std::ostream & ostr, const drain::SingleFlagger<E> & flagger) {
	ostr << flagger.str();
	return ostr;
}


template <typename E>
class MultiFlagger : public FlaggerBase<E> {

public:

	/// Fundamental type – in this case an enum.
	typedef typename FlaggerBase<E>::value_t value_t;

	typedef FlaggerDict dict_t;
	/// Fundamental type of the bitvector - an integral type.
	typedef typename dict_t::key_t key_t; // ~string
	/// Fundamental type of the bitvector - an integral type.
	typedef typename dict_t::value_t dvalue_t;

	inline
	MultiFlagger(){
	};

	inline
	MultiFlagger(value_t v){
		this->value = v;
	};

	virtual inline
	~MultiFlagger(){};


	/// Set bits, accepting numeric values or keys.
	/**
	 *  Starts with resetting the flags.
	 *
	 *  \tparam T – numeric values or strings
	 *
	 */
	template <typename ... T>
	inline
	void set(const T &... args) {
		this->reset();
		//this->value = 0;
		add(args...);
	}

	/// Set bits, accepting keys only.
	/**
	 *  Starts with resetting the flags.
	 *
	 *  \tparam T – numeric values or strings
	 *
	inline
	void setKeys(const std::string & keys) {
		this->reset();
	}
	 */

	/// Split a string to separate keys and/or values and set them.
	virtual
	void assign(const std::string & s) {
		const dict_t & dict = this->getDict();
		this->value = (value_t)FlagResolver::getValue(dict, s, this->separator); // uses dict.separator if needed
	}


	/// Add bit values
	template <typename T, typename ... TT>
	inline
	void add(const T & arg, const TT &... args) {
		//std::cerr << __FUNCTION__ << ':' << arg << std::endl;
		addOne(arg);
		add(args...);
	}


	virtual
	const key_t & str() const {
		const dict_t & dict = this->getDict();
		currentStr = FlagResolver::getKeys(dict, this->value, this->separator);
		return currentStr;
	}

protected:

	inline
	void add(){};

	virtual inline
	void addOne(const value_t & value){
		this->value = (value_t)((dvalue_t)this->value | (dvalue_t)value);
	}

	inline
	void addOne(const key_t & key){

		if (key.empty())
			return;

		const dict_t & dict = this->getDict();
		if (dict.hasKey(key)){
			//this->value |= (value_t)dict.getValue(key); // why cast? dvalue_t -> value_t
			addOne((value_t)dict.getValue(key));
		}
		else {
			throw std::runtime_error(std::string("Dictionary/") + typeid(value_t).name()+ ": no such key: "+ key);
		}
	}


	mutable
	key_t currentStr;

};



template <typename E>
inline
std::ostream & operator<<(std::ostream & ostr, const drain::MultiFlagger<E> & flagger) {
	return FlagResolver::keysToStream(flagger.getDict(), flagger.getValue(), ostr);
}



template <class E>
struct EnumDict {

	typedef FlaggerDict dict_t;

	static
	const dict_t dict;
};

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
	typedef typename fbase_t::value_t value_t;

	inline
	EnumFlagger(){
	}

	inline
	EnumFlagger(value_t v): fbase_t(v) {
	}


	virtual
	const typename FlaggerBase<value_t>::dict_t & getDict() const {
		return EnumDict<value_t>::dict;
	};


	template <class T>
	inline
	EnumFlagger<F> & operator=(const T & v){
		this->set(v);
		return *this;
	}

};

/// Flagger accepting values of (integer) type T
/**
 *  Designed for local, instantaneous use.
 *  Should several flaggers use the same dictionary, use Flagger2<F> instead.
 *
 *  \tparam F – SingleFlagger<T> or MultiFlagger<T>
 *
 */
template <class F>  // F =SingleFlagger<E>
class Flagger2 : public F {

public:

	typedef F fbase_t;
	typedef typename fbase_t::value_t value_t;
	typedef typename fbase_t::dict_t   dict_t;

	virtual
	const dict_t & getDict() const {
		return dict;
	};

	inline
	Flagger2(const dict_t & dict) : dict(dict){
	}

	inline
	Flagger2(const Flagger2<F> & flagger) : dict(flagger.dict){
	}

	template <class T>
	inline
	Flagger2<F> & operator=(const T & v){
		this->set(v);
		return *this;
	}

protected:

	const dict_t & dict;


};

/// Flagger with own dictionary, and accepting values of (integer) type T
/**
 *  Designed for local, instantaneous use.
 *  Should several flaggers use the same dictionary, use Flagger2<F> instead.
 *
 *  \tparam F – SingleFlagger<T> or MultiFlagger<T>
 *
 */
template <class F>
class Flags2 : public F {

public:

	typedef F fbase_t;
	typedef typename fbase_t::value_t value_t;
	typedef typename fbase_t::dict_t   dict_t;


	virtual
	const dict_t & getDict() const {
		return dict;
	};

	virtual
	dict_t & getDict(){
		return dict;
	};


	///
	/** If the dictionary is small, consider short init:
	 *  Flags2({"A",1}, {"B",2}}
	 *
	 */
	inline
	Flags2(){
	};

	/// Constructor for relatively short dictionaries
	/**
	 *  \code
	 *  Flags2 flags({"A",1}, {"B",2}});
	 *  \endcode
	 */
	inline
	Flags2(std::initializer_list<typename dict_t::entry_t> list) : dict(list){
	};

	inline
	Flags2(const Flags2<F> & flags) : dict(flags.dict){
	}

	template <class T>
	inline
	Flags2<F> & operator=(const T & v){
		this->set(v);
		return *this;
	}

protected:

	dict_t dict;


};



} // drain::


#endif
