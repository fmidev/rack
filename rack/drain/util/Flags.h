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

#include "Dictionary.h"


namespace drain {

/// Referencing a dictionary of binary values: {"A",1: "B":2, "C": 4, "D": 8, ...} resolves two-way mappings "A,C" <=> 5 .
class FlagResolver {

public:

	typedef std::string  key_t;
	typedef unsigned int value_t;

	typedef drain::Dictionary2<key_t,value_t> dict_t;

	/*
	inline
	FlagResolver(const dict_t & dict, char separator = 0) : separator(separator?separator:dict.separator), dictionaryRef(dict) {
	}

	inline
	FlagResolver(const FlagResolver & flags) : separator(flags.separator), dictionaryRef(flags.dictionaryRef){
	};
	*/

	/// Computes bitwise OR function on the numeric or alphabetic value(s) presented by a string.
	/**
	 *   \param key - String containing keys ("note,error") or values ("4,8,32") or a mixture of them.
	 *   \param lenient - If true, bypassed unknown keys silently, otherwise throws exception
	 */
	static
	value_t getValue(const dict_t & dict, const key_t & key, char separator=',');

	/// Given an integer, retrieves dictionary keys corresponding to each index of set bits.
	static
	std::string getKeys(const dict_t & dict, value_t, char separator=',');

	//std::string FlagResolver::keysToStr(const dict_t &dict, value_t v, char separator){

	static
	std::ostream & keysToStream(const dict_t &dict, value_t value, std::ostream & ostr, char separator=',');


	/// Return an interger (bit vector) with a new, previously unused value.
	/**
	 *   Traverses the values in dict, and determines a new bit corresponding to a new value.
	 */
	static
	value_t getFreeBit(const dict_t & dict);

	/// Add a new entry in the dictionary
	/**
	 *  \param i - if zero, call getFreeBit() to reserve a new, unused bit.
	 */
	static
	value_t add(dict_t & dict, const dict_t::key_t & key, value_t i=0);





};

/// Bit vector accessible also with integer values associated with strings.
class Flagger :public FlagResolver { // for typedefs

public:

	//typedef FlagResolver:: key_t;
	//typedef FlagResolver:: value_t;
	// typedef drain::Dictionary2<key_t,value_t> dict_t;


	const dict_t & dictionary; // todo open (keep own secret, later)

public:

	virtual inline
	const dict_t & getDict() const{
		return dictionary;
	}



	value_t & value;
	char separator;  // needed?


	inline
	Flagger(value_t & value, const dict_t & dict, char separator = 0) :
		dictionary(dict),
		value(value),
		separator(separator?separator:dict.separator){
	}

	inline
	Flagger(const Flagger & flags) : //FlagResolver(flags.dictionary, flags.separator), value(flags.value){
		dictionary(flags.dictionary),
		value(flags.value),
		separator(flags.separator){
	};



	/// Set the desired flag(s). If value==false, unset the flag(s).
	inline
	Flagger & set(value_t x, bool newValue=true){
		if (newValue)
			this->value |= x;
		else
			this->value &= ~x; //(value & ~x);
		return *this;
	};


	/// Set desired flags. Does not reset any flag.
	inline
	Flagger & set(const key_t & key, bool newValue=true){
		return set(getValue(key, separator), newValue);
	};


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

	/// Unset desired flags. Does not set any flag.
	/*
	inline
	void unset(value_t x, value_t & dst){
		dst = (dst & ~x);
	};
	*/


	/// Reset all the flags.
	inline
	Flagger & reset(){
		value = 0;
		return *this;
	}

	/// Unset desired flags. Does not set any flag.
	inline
	bool isSet(value_t x){
		return (value & x) != 0;
	};

	inline
	bool isSet(const key_t & key){
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

	/// Sets value, ie. set or unsets all the flags.
	/**
	 *   \param x - bit values 1, 2, 4, 8 ... etc combined with \c OR function.
	 */
	inline
    void assign(value_t x){
		value = x;
	}

	/// Copies flags as an integer \c value. Same dictionary not checked.
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
	operator std::string() const {
		std::stringstream sstr;
		keysToStream(sstr);
		//toStream(sstr);
		return sstr.str();
	}

	inline
	operator value_t() const {
		return value;
	}


	/// Computes bitwise OR function on the numeric or alphabetic value(s) presented by a string.
	/**
	 *   \param key - String containing keys ("note,error") or values ("4,8,32") or a mixture of them.
	 *   \param lenient - If true, bypassed unknown keys silently, otherwise throws exception
	 */
	inline
	value_t getValue(const key_t & key, char separator=0) const {
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

	/// Bub risky: value_t ~char !
	inline
	std::ostream & keysToStream(std::ostream & ostr=std::cout, char separator=0) const {
		return FlagResolver::keysToStream(dictionary, value, ostr, separator ? separator : this->separator);
	}

	/// Display current value as key values of which the value is composed with \c OR function.
	/**
	 *   \param ostr
	 *   \param separator - character appended between keys, if not own separator
	 */
	//std::ostream & toStream(std::ostream & ostr=std::cout, char separator=',') const; // PathMatcher '|' !
	//std::ostream & toStream(std::ostream & ostr=std::cout, char separator=0) const; // PathMatcher '|' !


	template <class T>
	void exportStatus(std::map<std::string, T> & statusMap) const {
		for (dict_t::const_iterator it = dictionary.begin(); it != dictionary.end(); ++it){
			if ((it->second > 0) && ((it->second & value)) == it->second){ // fully covered in value
				statusMap[it->first] = 1;
			}
			else {
				statusMap[it->first] = 0;
			}
		}
	}




};


inline
std::ostream & operator<<(std::ostream & ostr, const drain::Flagger & flags) {
	return flags.keysToStream(ostr);
}


class Flags : public Flagger {

protected:

	value_t ownValue;
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


	Flags(char separator = ',') : Flagger(ownValue, ownDictionary, separator){
	}

	//Flags(dict_t & d, char separator = ',') : Flagger(ownValue, d, separator){	}

	Flags(const Flags & flags) : Flagger(ownValue=flags.ownValue, ownDictionary, flags.separator){
		// Copy dict?
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
	value_t add(const dict_t::key_t & key, value_t i=0){
		return Flagger::add(ownDictionary, key, i);
	}

protected:

	/*
	bool usesOwnDict() const { //??
		return (&dictionary == &dictionary);
	}
	*/
};


} // drain::


#endif
