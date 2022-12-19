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
#ifndef DRAIN_DICTIONARY
#define DRAIN_DICTIONARY "Dictionary v2.0"

//
#include <iostream>
#include <map>
#include <list>
#include <string>

//#include "Log.h"
#include "Sprinter.h"



namespace drain {

/// Two-way mapping between strings and objects of template class T
/**
 *  TODO: hide first and second so that an entry cannot be assigned to either of them
 */

// TODO: consider map<K, V> for faster search and order? But no, if still slow...

/** Simple list based container for small dictionaries. Uses brute-force search.
 *
 *   In a way, works like std::map as each entry is a std::pair. However, no less-than relation is needed as the entries are not order but
 *   appended sequentally.
 *
 *   For handling input and output, dictionary has a separator char which is a comma ',' by default.
 *
 */
template <class K, class V>
class Dictionary2 : public std::list<std::pair<K, V> > {

public:

	typedef K key_t;
	typedef V value_t;

	typedef std::pair<K, V> entry_t;
	typedef std::list<entry_t> container_t;

	typedef std::list<key_t>   keylist_t;
	typedef std::list<value_t> valuelist_t;


	Dictionary2() : separator(','){};

	Dictionary2(const Dictionary2 & d) : separator(d.separator){};

	Dictionary2(std::initializer_list<entry_t> d) : std::list<entry_t>(d), separator(','){
	};


	virtual
	~Dictionary2(){};

	//virtual
	entry_t & add(const K & key, const V & value){
		this->push_back(entry_t(key, value));
		return this->back();
	}


	typename container_t::const_iterator findByKey(const K & key) const {
		for (typename container_t::const_iterator it = this->begin(); it != this->end(); ++it){
			if (it->first == key)
				return it;
		}
		return this->end();
	}

	typename container_t::const_iterator findByValue(const V & value) const {
		for (typename container_t::const_iterator it = this->begin(); it != this->end(); ++it){
			if (it->second == value)
				return it;
		}
		return this->end();
	}


	/*
	typename container_t & findByKeyNew(const K & key) {
		for (auto & entry: *this){
			if (entry.first == key)
				return entry;
		}
		return this->front(); // todo: dummy
	}
	*/


	inline
	bool hasKey(const K & key) const {
		return (findByKey(key) != this->end());
	}

	inline
	bool hasValue(const V & value) const {
		return (findByValue(value) != this->end());
	}

	/*
	typename container_t::iterator findByValue(const V & value){
		for (typename container_t::iterator it = this->begin(); it != this->end(); ++it){
			if (it->second == value)
				return it;
		}
		return this->end();
	}
	*/

	//virtual
	const V & getValue(const K & key) const {
		typename container_t::const_iterator it = findByKey(key);
		if (it != this->end())
			return it->second;
		else {
			static V empty;
			return empty;
		}

	}

	// virtual
	const K & getKey(const V & value) const {
		typename container_t::const_iterator it = findByValue(value);
		if (it != this->end())
			return it->first;
		else {
			static K empty;
			return empty;
		}
	}

	/*
	 * REPLACED with Sprinter
	 *
	void toStreamOLD(std::ostream & ostr = std::cout, char separator=0) const {

		if (!separator)
			separator = this->separator;

		char sep = 0;
		//for (typename container_t::const_iterator it = this->begin(); it != this->end(); ++it){
		for (const auto & entry: *this){

			if (sep)
				ostr << sep;
			else
				sep = separator;

			ostr << entry.first << '=' << entry.second;
			//ostr << it->first << '=' << it->second;
		}
	}

	std::string toStrOLD(char separator=0) const {
		std::stringstream sstr;
		this->toStream(sstr, separator);
		return sstr.str();
	}
	*/


protected:

	mutable
	keylist_t keyList;

	mutable
	valuelist_t valueList;

public:

	const keylist_t & getKeys() const {

		#pragma omp critical
		keyList.clear();
		for (const entry_t & entry: *this){
			keyList.push_back(entry.first);
		}
		return keyList;
	}

	void getKeys(keylist_t & l) const {
		for (const entry_t & entry: *this){
			l.push_back(entry.first);
		}
	}


	const valuelist_t & getValues() const {

		#pragma omp critical
		valueList.clear();
		for (const entry_t & entry: *this){
			valueList.push_back(entry.second);
		}
		return valueList;
	}

	void getValues(keylist_t & l) const {
		for (const entry_t & entry: *this){
			l.push_back(entry.second);
		}
	}

	/*
	void keysToStream(std::ostream & ostr = std::cout, char separator=0) const {


		if (!separator)
			separator = this->separator;

		char sep = 0;
		//for (typename container_t::const_iterator it = this->begin(); it != this->end(); ++it){
		for (const entry_t & entry: *this){

			if (sep)
				ostr << sep;
			else
				sep = separator;

			ostr << entry.first; // << '=' << it->second;
		}
	}

	void valuesToStream(std::ostream & ostr = std::cout, char separator=0) const {

		if (!separator)
			separator = this->separator;

		char sep = 0;
		//for (typename container_t::const_iterator it = this->begin(); it != this->end(); ++it){
		for (const entry_t & entry: *this){
			if (sep)
				ostr << sep;
			else
				sep = separator;

			ostr << entry.second;
			//ostr << it->second;
		}
	}
	*/

	char separator;
};

/*
template <class K, class V>
inline
std::ostream & operator<<(std::ostream & ostr, const Dictionary2<K,V> & dict) {
	dict.toStream(ostr);
	return ostr;
}
*/

template <class K, class V>
inline
std::ostream & operator<<(std::ostream & ostr, const Dictionary2<K,V> & dict) {
	// SprinterLayout(const char *arrayChars="[,]", const char *mapChars="{,}", const char *pairChars="(,)", const char *stringChars=nullptr)
	// static drain::SprinterLayout dict_layout("{,}", "{,}", "{,}", "{,}");
	ostr << drain::sprinter((const typename Dictionary2<K,V>::container_t &)dict, SprinterBase::cppLayout);
	return ostr;
}

/*
template <>
inline
std::ostream & SprinterBase::toStream(std::ostream & ostr, const drain::Variable & x, const SprinterLayout & layout){
	return SprinterBase::toStream(ostr, (const drain::Castable &)x, layout);
}
*/



/// Associates type info
template <class K, class V>
class Dictionary2Ptr : public Dictionary2<K, V*> {

public:

	typedef Dictionary2<K, V*> parent_t;

	Dictionary2Ptr(){};

	virtual
	~Dictionary2Ptr(){};

	//virtual
	void add(const K & key, const V & value){
		Dictionary2<K, V*>::add(key, &value);
	}

	typename parent_t::container_t::const_iterator findByValue(const V & value) const {
		return parent_t::findByValue(& value);
	}

	//virtual
	const V & getValue(const K & key) const {
		return *Dictionary2<K, V*>::getValue(key);
	}

	//virtual
	const K & getKey(const V & value) const {
		return Dictionary2<K, V*>::getKey(&value);
	}


};

} // drain::


#endif

// Drain
