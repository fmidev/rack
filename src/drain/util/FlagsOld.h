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

#ifndef DRAIN_FLAGS_OLD_H_
#define DRAIN_FLAGS_OLD_H_

#include "Flags.h"

namespace drain {






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

		drain::Logger mout(__FILE__, __FUNCTION__);
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






} // drain::


#endif
