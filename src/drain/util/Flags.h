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

#ifndef DRAIN_FLAGS
#define DRAIN_FLAGS

#include <iostream>
#include <list>
#include <iterator>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

#include <drain/Log.h>
#include <drain/String.h>
#include <drain/Type.h>

#include "Dictionary.h"

namespace drain {

/// Referencing a dictionary of binary values: {"A",1: "B":2, "C": 4, "D": 8, ...} resolves two-way mappings "A,C" <=> 5 .
/** Static utilities.
 *
 */
class FlagResolver {

public:

	// Definite
	typedef std::string  key_t;

	// "Recommended" storage type for both numeric and enumerated-type flags.
	typedef unsigned long int ivalue_t;

	/// "Recommended" dictionary type. All the methods are templates, however.
	typedef drain::Dictionary<key_t,ivalue_t> dict_t;

	static
	const ivalue_t ALL; // = std::numeric_limits<FlagResolver::ivalue_t>::max();


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
	ivalue_t getValue(const drain::Dictionary<key_t,T> & dict, const std::string & key, char separator=',');

	/// Given an integer, retrieves dictionary keys corresponding to each index of set bits.
	template <typename T>
	static
	std::string getKeys(const drain::Dictionary<key_t,T> & dict, ivalue_t, char separator=',');

	/// Write keys in a stream, in numeric order.
	template <typename T>
	static
	std::ostream & keysToStream(const drain::Dictionary<key_t,T> &dict, ivalue_t value, std::ostream & ostr, char separator=',');

	/// Given a bit vector (integer value), extracts separate flag values to a list.
	/**
	 *  Traverses the dictionary and appends matching values in the list.
	 *  The extracted values are of original type (integer or enum).
	 *
	 */
	template <typename T, typename V>
	static
	void valuesToList(ivalue_t value, const drain::Dictionary<key_t,T> &dict, std::list<V> & container);

	/// Add a new entry in the dictionary
	/**
	 *  \param i - if zero, call getFreeBit() to reserve a new, unused bit.
	 */
	template <typename T>
	static
	ivalue_t addEntry(drain::Dictionary<key_t,T> & dict, const key_t & key, ivalue_t i=0);


	/// Return an interger (bit vector) with a new, previously unused value.
	/**
	 *   Traverses the values in dict, and determines a new bit corresponding to a new value.
	 */
	template <typename T>
	static
	ivalue_t getFreeBit(const drain::Dictionary<key_t,T> & dict);


};


template <typename T>
typename FlagResolver::ivalue_t FlagResolver::getFreeBit(const drain::Dictionary<key_t,T> & dict){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// Bit mask containing all the values.
	ivalue_t currentMask = 0;
	for (const auto & entry: dict){
		currentMask = currentMask | entry.second;
	}

	mout.debug2("sum:" , currentMask );

	ivalue_t i = 1;
	while ((i&currentMask) > 0){
		mout.debug3("checking bit:" , i , " vs.\t" , currentMask );
		//full = (full>>1);
		i = (i<<1);
	}
	/*
	while (full>0){
		mout.debug2("checking bit:" , i , " vs.\t" , full );
		full = (full>>1);
		i = (i<<1);
	}
	*/

	mout.debug("released: " , i );

	return i;

}

/*
template <typename T>
typename FlagResolver::ivalue_t FlagResolver::addEntry(drain::Dictionary<key_t,T> &dict, const typename dict_t::key_t & key){

	drain::Logger mout(__FILE__, __FUNCTION__);
	if (dict.hasKey(key)){
		mout.info(key , " already in dict: " , dict );
		return dict.getValue(key);
	}

	return addEntry(dict, key, )
}
*/

template <typename T>
typename FlagResolver::ivalue_t FlagResolver::addEntry(drain::Dictionary<key_t,T> &dict, const typename dict_t::key_t & key, ivalue_t i){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (dict.hasKey(key)){
		mout.info(key , " already in dict: " , dict );
		return dict.getValue(key);
	}

	if (i==0)
		i = getFreeBit(dict);

	if (i>0){
		dict.add(key, i);
	}
	else {
		mout.warn(key , " could not get a valid (non-zero) bit flag for dict: " , dict );
	}

	return i;
};


template <typename T>
typename FlagResolver::ivalue_t FlagResolver::getValue(const drain::Dictionary<key_t,T> & dict, const std::string & args, char separator){

	drain::Logger mout(__FILE__, __FUNCTION__);

	FlagResolver::ivalue_t v = 0;

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
		// mout.warn(" '" , *it , "'" );

		//typename dict_t::const_iterator dit = dict.findByKey(key);
		typename drain::Dictionary<key_t,T>::const_iterator dit = dict.findByKey(key);

		if (dit != dict.end()){ // String key match,
			// Numeric value for an alphabetic key was found
			v = (v | static_cast<FlagResolver::ivalue_t>(dit->second) );
		}
		else {
			// Numeric value
			FlagResolver::ivalue_t x = 0;
			std::stringstream sstr(key);
			sstr >> x; // FIX if empty
			if (x == 0){
				// String "0" handled already above
				// Consider: could advice keys only: sprinter(dict.getKeys()
				// Or key-value pairs: sprinter(dict)
				// static const SprinterLayout cmdArgLayout = {",", "?", "=", ""};
				// mout.error("key '", key, "' not found in: ", sprinter(dict, Sprinter::cmdLineLayout));
				// mout.error("key '", key, "' not föund in: ", sprinter(dict.getContainer(), cmdArgLayout) );
				mout.error("key '", key, "' not found in: ", dict);
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
std::string FlagResolver::getKeys(const drain::Dictionary<key_t,T> &dict, ivalue_t v, char separator){
	std::stringstream sstr;
	keysToStream(dict, v, sstr, separator);
	return sstr.str();
}



//

/// List keys in their numeric order.
template <typename T>
std::ostream & FlagResolver::keysToStream(const drain::Dictionary<key_t,T> &dict, ivalue_t value, std::ostream & ostr, char separator) {

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




template <typename T, typename V>
void FlagResolver::valuesToList(ivalue_t value, const drain::Dictionary<key_t,T> &dict, std::list<V> & container){
	for (const auto & entry: dict){
		if ((entry.second > 0) && ((entry.second & value) == entry.second)){ // fully covered in value
			// ostr << entry.first;
			container.push_back(static_cast<V>(entry.second));
			// container.push_back(entry.first);
		}
	}
}



// NEW 2022/2023


// typedef drain::Dictionary<std::string,unsigned long> FlaggerDict;

// #define FlaggerDict FlagResolver::dict_t

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

	//static
	//const SprinterLayout dictLayout = {",", "?", "=", ""};

	// Manifested numeric type (enum or unsigned integer)
	typedef E value_t;

	typedef FlagResolver::dict_t dict_t;
	typedef typename dict_t::key_t key_t; // ~string

	// Practical "storage" value
	typedef typename dict_t::value_t dict_value_t;

	// Rember to add an initialized unit: template<> SingleFlagger<...>::dict = {{...,...}, ...}
	// static const dict_t dict;

	/**
	 *
	 */
	inline
	FlaggerBase(): value(ownValue), separator(','), ownValue((dict_value_t)0){
	}

	/// Own value will be unused (and invisible).
	/**
	 *   Reconsider design. Should the value ever be referenced?
	 */
	inline
	FlaggerBase(dict_value_t & v): value(v), separator(','){
	}

	/*  Risky? (Ambiguous)
	inline
	FlaggerBase(const dict_value_t & v): value(ownValue), ownValue(v) {
	}
	*/

	virtual inline
	~FlaggerBase(){
	}

	virtual
	const dict_t & getDict() const = 0;

	void reset(){
		this->value = dict_value_t(0);
	};

	virtual
	inline
	void set(const key_t & key){

		if (key.empty())
			return;

		if (this->getDict().hasKey(key)){
			this->value = (dict_value_t)this->getDict().getValue(key); // why cast? dvalue_t -> dict_value_t
		}
		else {
			this->value = FlagResolver::getValue(this->getDict(), key);
			//Logger mout(__FILE__, __FUNCTION__);
			// mout.error(TypeName<E>::str(), ": no such key: '"+ key, "', keys=", sprinter(this->getDict().getKeys()));
			// throw std::runtime_error(std::string("Dictionary[") + typeid(dict_value_t).name()+ "]: no such key: "+ key);
		}
	}

	virtual inline
	void set(const dict_value_t & value){
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
	bool isSet(dict_value_t x) const {
		return (value & x) != 0;
	};

	inline
	bool isSet(const key_t & key) const {
		return isSet(getDict().getValue(key));
	};

	inline
	const dict_value_t & getValue() const {
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
	operator const dict_value_t & () const {
		return value;
	}

	inline
	operator dict_value_t & () {
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
	dict_value_t & value;

	char separator;

	void debug(std::ostream & ostr) const;

private:

	// Own value, discarded if external value referenced.
	dict_value_t ownValue = 0;


};

template <typename E>
void drain::FlaggerBase<E>::debug(std::ostream & ostr) const {
	//ostr << typeid(drain::FlaggerBase<E>).name() << ": value=" << getValue() << ", ";
	ostr << drain::TypeName<E>::str() << ": value=" << getValue() << ", ";
	FlagResolver::keysToStream(getDict(), getValue(), ostr) << ", ";
	ostr <<  " dict: " << getDict();
}

/*
template <typename E>
const SprinterLayout drain::FlaggerBase<E>::dictLayout = {",", "?", "=", ""};
*/


template <typename E>
inline
std::ostream & operator<<(std::ostream & ostr, const drain::FlaggerBase<E> & flagger) {
	return ostr << flagger.str(); // flags.keysToStream(ostr);
}

template <typename E>
class SingleFlagger : public FlaggerBase<E> {

public:

	//typedef typename FlaggerBase<E>::dict_value_t dict_value_t;
	typedef E ivalue_t;

	typedef FlagResolver::dict_t dict_t;
	typedef typename dict_t::key_t key_t; // ~string
	typedef typename dict_t::value_t dvalue_t;

	inline
	SingleFlagger(){
	};

	inline
	SingleFlagger(ivalue_t v){
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
	typedef typename FlaggerBase<E>::dict_value_t dict_value_t;

	typedef FlagResolver::dict_t dict_t;
	/// Fundamental type of the bitvector - an integral type.
	typedef typename dict_t::key_t key_t; // ~string
	/// Fundamental type of the bitvector - an integral type.
	typedef typename dict_t::value_t dvalue_t;

	inline
	MultiFlagger(){
	};

	/*
	inline
	MultiFlagger(value_t v){
		this->value = v;
	};
	*/

	template <typename ... T>
	inline
	MultiFlagger(const T &... args){
		set(args...);
		// this->value = v;
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
		this->value = (dict_value_t)FlagResolver::getValue(dict, s, this->separator); // uses dict.separator if needed
	}


	/// Add bit values
	template <typename T, typename ... TT>
	inline
	void add(const T & arg, const TT &... args) {
		//std::cerr << __FUNCTION__ << ':' << arg << std::endl;
		addOne(arg);
		add(args...);
	}



	/// For exporting values.
	virtual
	const key_t & str() const {
		const dict_t & dict = this->getDict();
		currentStr = FlagResolver::getKeys(dict, this->value, this->separator);
		return currentStr;
	}

	/// Given only a numeric/enum value,
	/*
	virtual
	std::string str(const dict_value_t & value){
		return FlagResolver::getKeys(this->getDict(), value, this->separator);
	}
	*/

	/// For importing values. After assignment, update() should be called. Experimental
	virtual
	std::string & str(){
		return currentStr;
	}

	/// This should be called after assigning a string to & str() .
	/*
	inline
	void update(){
		assign(currentStr);
	}
	*/



protected:

	inline
	void add(){};

	virtual inline
	void addOne(const dict_value_t & value){
		// why OR op in dvalue
		// this->value = static_cast<dict_value_t>((dvalue_t)this->value | (dvalue_t)value);
		this->value |= static_cast<dict_value_t>(value);
	}

	inline
	void addOne(const key_t & key){

		// Resolves also "" and numeric keys.
		this->value |= FlagResolver::getValue(this->getDict(), key, this->separator);

		/*
		if (key.empty())
			return;

		//dict_value_t v = ::atoi(key);

		const dict_t & dict = this->getDict();
		if (dict.hasKey(key)){
			//this->value |= (dict_value_t)dict.getValue(key); // why cast? dvalue_t -> value_t
			addOne((dict_value_t)dict.getValue(key));
		}
		else {
			/// XXX
			Logger mout(__FILE__, __FUNCTION__);
			mout.error(TypeName<E>::get(), ": no such key: '"+ key, "', keys=", sprinter(this->getDict().getKeys()));
			// throw std::runtime_error(std::string("Dictionary/") + typeid(dict_value_t).name()+ ": no such key: "+ key);
		}
		*/
	}


	mutable
	std::string currentStr;
	// key_t currentStr;


};



template <typename E>
inline
std::ostream & operator<<(std::ostream & ostr, const drain::MultiFlagger<E> & flagger) {
	return FlagResolver::keysToStream(flagger.getDict(), flagger.getValue(), ostr);
}



template <class E>
struct EnumDict {

	typedef FlagResolver::dict_t dict_t;

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
	typedef FlagResolver::ivalue_t ivalue_t;

	inline
	EnumFlagger(){
	}

	/*
	inline
	EnumFlagger(value_t v): fbase_t(v) {
	}
	*/

	template <typename ... T>
	inline
	EnumFlagger(const T & ... arg): fbase_t(arg...) { // designed for MultiFlagger
	}


	/// Returns the static dictionary created for this \c value_t .
	/**
	 *  Notice that this function is non-static but returns a reference to a static dictionary.
	 *  In C++ virtual functions must be non-static.
	 *  This is nevertheless handy in templated design, for example.
	 */
	virtual
	const typename FlaggerBase<value_t>::dict_t & getDict() const {
		return EnumDict<value_t>::dict;
	};

	// risen ... void getValues(std::list<EnumFlagger::value_t> & container);


	/// Return the integer value corresponding to a key.
	/**
	 *  Retrieves the value directly from the dictionary.
	 *  Dictionary throws except if key now found.
	 */
	static
	ivalue_t getValueNEW(const std::string & key){
		return (ivalue_t)EnumDict<value_t>::dict.getValue(key);
	};

	static inline
	std::string getKeysNEW2(const value_t & value, char separator = ','){
		return FlagResolver::getKeys(EnumDict<value_t>::dict, value, separator);
	}

	/**
	 *  Dictionary throws except if key now found.
	static
	value_t getKeys(const value_t & value){
		return (value_t)EnumDict<value_t>::dict.getValue(key);
	};
    */


	template <class T>
	inline
	EnumFlagger<F> & operator=(const T & v){
		this->set(v);
		return *this;
	}

	inline
	operator value_t() const {
		return static_cast<value_t>(this->value);
	}

	inline
	operator bool() const {
		return static_cast<ivalue_t>(this->value) != 0;
	}

};



/// Flagger with referenced/external dictionary accepting values of (integer) type T
/**
 *  Designed for local, instantaneous use.
 *  Should several flaggers use the same dictionary, use Flagger2<F> instead.
 *
 *  \tparam F – SingleFlagger<T> or MultiFlagger<T>, integer type T
 *
 */
template <class F>
class Flagger2 : public F {  // Consider renaming to IntFlagger !

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
