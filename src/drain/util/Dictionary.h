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
#include <drain/Sprinter.h>



namespace drain {

/// Two-way mapping between strings and objects of template class T
/**
 *  TODO: hide first and second so that an entry cannot be assigned to either of them
 */

// TODO: consider map<K, V> for faster search and order? But no, if still slow.

/** Simple list based container for small dictionaries. Uses brute-force linear search.
 *
 *   In a way, works like std::map as each entry is a std::pair. However, no less-than relation is needed as the entries are not in
 *   order but appended sequentially.
 *
 *   For handling input and output, dictionary has a separator char which is a comma ',' by default.
 *
 */
template <class K, class V>
class Dictionary : public std::list<std::pair<K, V> > {

public:

	typedef K key_t;
	typedef V value_t;

	typedef std::pair<K, V> entry_t;
	typedef std::list<entry_t> container_t;

	typedef std::list<key_t>   keylist_t;
	typedef std::list<value_t> valuelist_t;


	Dictionary() : separator(','){};

	Dictionary(const Dictionary & d) : separator(d.separator){};

	Dictionary(std::initializer_list<entry_t> d) : std::list<entry_t>(d), separator(','){
	};


	virtual
	~Dictionary(){};

	inline
	const container_t & getContainer() const {
		return *this;
	}

	entry_t & add(const K & key, const V & value){
		this->push_back(entry_t(key, value));
		return this->back();
	}

	/// Replaces existing or adds
	entry_t & set(const K & key, const V & value){
		for (entry_t & entry: *this){
			if (entry.first == key){
				entry.second = value;
				return entry;
			}
		}
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


	inline
	bool hasKey(const K & key) const {
		return (findByKey(key) != this->end());
	}

	/// Given a key, return the first value associated with it.
	inline
	bool hasValue(const V & value) const {
		return (findByValue(value) != this->end());
	}


	/// Given a key, return the first value associated with it.
	const V & getValue(const K & key) const {
		typename container_t::const_iterator it = findByKey(key);
		if (it != this->end())
			return it->second;
		else {
			static V empty;
			return empty;
		}

	}

	/// Given a value, return the first key associated with it.
	const K & getKey(const V & value) const {
		typename container_t::const_iterator it = findByValue(value);
		if (it != this->end())
			return it->first;
		else {
			static K empty;
			return empty;
		}
	}


	const keylist_t & getKeys() const {

		#pragma omp critical
		{
			keyList.clear();
			for (const entry_t & entry: *this){
				keyList.push_back(entry.first);
			}
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
		{
			valueList.clear();
			for (const entry_t & entry: *this){
				valueList.push_back(entry.second);
			}
		}
		return valueList;
	}

	void getValues(keylist_t & l) const {
		for (const entry_t & entry: *this){
			l.push_back(entry.second);
		}
	}

	char separator;

protected:

	mutable
	keylist_t keyList;

	mutable
	valuelist_t valueList;


};

/*
template <class K, class V>
inline
std::ostream & operator<<(std::ostream & ostr, const Dictionary<K,V> & dict) {
	dict.toStream(ostr);
	return ostr;
}
*/

template <class K, class V>
inline
std::ostream & operator<<(std::ostream & ostr, const Dictionary<K,V> & dict) {
	// SprinterLayout(const char *arrayChars="[,]", const char *mapChars="{,}", const char *pairChars="(,)", const char *stringChars=nullptr)
	// static drain::SprinterLayout dict_layout("{,}", "{,}", "{,}", "{,}");
	static const SprinterLayout cmdArgLayout = {",", "?", "=", ""};
	// Note: the following cast is (also) the only way to apply layout on a Dictionary
	//ostr << drain::sprinter(dict.getContainer(), Sprinter::cppLayout);
	ostr << drain::sprinter(dict.getContainer(), cmdArgLayout);
	//ostr << drain::sprinter((const typename Dictionary<K,V>::container_t &)dict, Sprinter::cppLayout);
	return ostr;
}

/*
template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const drain::Variable & x, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &)x, layout);
}
*/



/// Associates type info
template <class K, class V>
class DictionaryPtr : public Dictionary<K, V*> {

public:

	typedef Dictionary<K, V*> parent_t;

	DictionaryPtr(){};

	virtual
	~DictionaryPtr(){};

	//virtual
	void add(const K & key, const V & value){
		Dictionary<K, V*>::add(key, &value);
	}

	typename parent_t::container_t::const_iterator findByValue(const V & value) const {
		return parent_t::findByValue(& value);
	}

	//virtual
	const V & getValue(const K & key) const {
		return *Dictionary<K, V*>::getValue(key);
	}

	//virtual
	const K & getKey(const V & value) const {
		return Dictionary<K, V*>::getKey(&value);
	}


};

} // drain::


#endif

// Drain
