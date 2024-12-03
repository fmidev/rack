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

#ifndef DRAIN_FLAG_BASE
#define DRAIN_FLAG_BASE

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
 *  More for MultiFlaggers?
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
	ivalue_t getIntValue(const drain::Dictionary<key_t,T> & dict, const std::string & keys, char separator=',');

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
typename FlagResolver::ivalue_t FlagResolver::getIntValue(const drain::Dictionary<key_t,T> & dict, const std::string & args, char separator){

	drain::Logger mout(__FILE__, __FUNCTION__);

	FlagResolver::ivalue_t v = 0;

	std::list<key_t> keys;

	if (!separator){
		separator = dict.separator;
	}

	if (separator){
		drain::StringTools::split(args, keys, separator);
	}


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
std::ostream & FlagResolver::keysToStream(const drain::Dictionary<FlagResolver::key_t,T> &dict, ivalue_t value, std::ostream & ostr, char separator) {

	/* note: instead of shifting bits of this->value, traverses the dictionary which can contain
		- combined values
		- repeated values (aliases)
	*/

	if (!separator)
		separator = dict.separator;

	char sep = 0;

	for (const auto & entry: dict){
		const ivalue_t v = (entry.second); // static_cast<ivalue_t>
		//if ((entry.second > 0) && ((entry.second & value) == entry.second)){ // fully covered in value
		//if ((entry.second == 0) && ((entry.second & value) == entry.second)){ // fully covered in value
		if ((v > 0) && ((v & value) == v)){ // fully covered in value
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
 * \tparam E - single-value type: enum or integer
 * \tparam T - storage type (bit vector, "state"). T should not be enum, if E is numeric.
 */
template <typename E=std::size_t, typename T=E> //, typename D=drain::Dictionary<std::string,unsigned long> >
class  FlaggerBase {

public:

	//static
	//const SprinterLayout dictLayout = {",", "?", "=", ""};

	// Manifested numeric type (enum or unsigned integer)
	typedef E value_t;
	typedef T storage_t;
	typedef Dictionary<std::string,E> dict_t;
	typedef typename dict_t::key_t key_t; // ~string

	// Practical "storage" value
	// typedef typename dict_t::value_t dict_value_t;  // TODO: xould be same as value_t ?

	// Rember to add an initialized unit: template<> SingleFlagger<...>::dict = {{...,...}, ...}
	// static const dict_t dict;

	// Default constructor. Local value \c ownValue will be used.
	/**
	 *
	 */
	inline
	FlaggerBase(char separator=','): value(ownValue), separator(','), ownValue((storage_t)0){ // drain::NEUTRAL VALUE!
	}

	/// Value-referencing constructor.
	/**
	 *  An external value will be used, leaving protected \c ownValue unused .
	 *  Reconsider design. Should the value ever be referenced?
	 */
	inline
	FlaggerBase(storage_t & v, char separator=','): value(v), separator(','){
	}

	/*  Risky? (Ambiguous)
	inline
	FlaggerBase(const dict_value_t & v): value(ownValue), ownValue(v) {
	}
	*/

	virtual inline
	~FlaggerBase(){
	}

	/// Returns the static dictionary created for this \c value_t .
	/**
	 *  Notice that this function is non-static but returns a reference to a static dictionary.
	 *  In C++ virtual functions must be non-static.
	 *  This is nevertheless handy in templated design, for example.
	 */
	virtual
	const dict_t & getDict() const = 0;

	void reset(){
		this->value = storage_t(0); // ALERT! enums need neutral value.
	};

	/// Sets one or several flags.
	virtual inline
	void set(const key_t & key){
		if (key.empty())
			return; // needed?
		assign(key);
	}
	/*

		if (key.empty())
			return;

		if ((key.find(',')!= std::string::npos)||(key.find('|')!= std::string::npos)){
			Logger mout(__FILE__, __FUNCTION__);
			mout.unimplemented<LOG_ERR>(__FILE__, ':', TypeName<E>::str(), ": multi-key arg: '"+ key, "', for single-flagger: ", sprinter(this->getDict().getKeys()));
		}

		if (this->getDict().hasKey(key)){
			this->value = static_cast<storage_t>(this->getDict().getValue(key)); // Cast needed (only) if MultiFlagger converts enum value_t -> storage_t
		}
		else {
			// this->value = FlagResolver::getIntValue(this->getDict(), key);  // SingleFlagger does not like this.
			Logger mout(__FILE__, __FUNCTION__);
			mout.error(__FILE__, ':', TypeName<E>::str(), ": no such key: '"+ key, "', keys=", sprinter(this->getDict().getKeys()));
			// throw std::runtime_error(std::string("Dictionary[") + typeid(dict_value_t).name()+ "]: no such key: "+ key);
		}
	}
	*/

	/// Set a single flag.
	virtual inline
	void set(const value_t & value){
		this->value = value;
	};

	virtual inline
	void set(const FlaggerBase<E> & flagger){
		this->value = flagger.value;
	}

	/// Checks if a given bit, or any of given bits, is set.
	/**
	 *  SingleFlagger: exact match
	 *  MultiFlagger:  bitwise OR > 0
	 */
	virtual
	bool isSet(const storage_t & x) const = 0;

	/*
	inline
	bool isSet(const value_t & x) const {
		return (value & x) != 0; // enums...
	};
	*/

	inline
	bool isSet(const key_t & key) const {
		return isSet(getDict().getValue(key));
	};

	inline
	const storage_t & getValue() const {
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
	operator const storage_t & () const {  // should be ivalue ?
		return this->value;
	}

	inline
	operator storage_t & () { // should be ivalue ?
		return this->value;
	}

	inline
	operator bool() const {
		return static_cast<FlagResolver::ivalue_t>(this->value) != 0;
	}


	/// String corresponding the current value. Returns empty, if not found.
	virtual
	const key_t & str() const = 0;

	// String
	operator const key_t & () const {
		return str();
	}

	// Own or external value.
	storage_t & value;

	char separator;

	void debug(std::ostream & ostr) const;

private:

	// protect?
	virtual
	void assign(const std::string & s) = 0;


	// Own value, discarded if external value referenced.
	storage_t ownValue = 0;  // NEUTRAL

};


template <typename E,typename T>
void drain::FlaggerBase<E,T>::debug(std::ostream & ostr) const {
	//ostr << typeid(drain::FlaggerBase<E>).name() << ": value=" << getValue() << ", ";
	ostr << drain::TypeName<E>::str() << ": value=" << getValue() << ", ";
	FlagResolver::keysToStream(getDict(), getValue(), ostr) << ", ";
	ostr <<  " dict: " << getDict();
}

template <typename E,typename T>
inline
std::ostream & operator<<(std::ostream & ostr, const drain::FlaggerBase<E,T> & flagger) {
	return ostr << flagger.str(); // flags.keysToStream(ostr);
}

/*
template <typename E>
const SprinterLayout drain::FlaggerBase<E>::dictLayout = {",", "?", "=", ""};
*/




} // drain::


#endif
