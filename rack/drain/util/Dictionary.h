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
#define DRAIN_DICTIONARY "Dictionary v1.0"

//
#include <iostream>
#include <map>
#include <list>
#include <string>

#include "Log.h"



namespace drain {

/// Two-way mapping between strings and objects of template class T
/**
 *  TODO: hide first and second so that an entry cannot be assigned to either of them
 */
/*
template <class T>
class Dictionary : public std::pair<std::map<std::string,T>, std::map<T,std::string> > {

public:

	typedef T value_t;
	typedef Dictionary<T> dict_t;


	inline
	Dictionary() : separator(',') {
	}

	inline
	bool contains(const std::string & entry) const {
		return (this->first.find(entry) != this->first.end());
	}

	template <class T2>
	inline
	bool contains(const T2 & entry) const {
		return (this->second.find(entry) != this->second.end());
	}

	inline
	void addEntry(const std::string & key, const T &x){
		this->first[key] = x;
		this->second[x]  = key;
	}

	void toOstr(std::ostream & ostr, char separator=0){

		if (!separator)
			separator = this->separator;

		char sep = 0;
		for (typename dict_t::first_type::const_iterator it = this->first.begin(); it != this->first.end(); ++it){
			if (sep)
				ostr << sep;
			else
				sep = separator;
			ostr << it->first << '=' << it->second;
		}
	}

	char separator;

};




template <class T>
inline
std::ostream & operator<<(std::ostream & ostr, const Dictionary<T> & d) {
	return d.toOStr(ostr);
}
*/

/***
 *   In a way, works like std::map as each entry is a std::pair. However, no less-than relation is needed as the entries are not order but
 *   appended sequentally.
 */
template <class K, class V>
class Dictionary2 : public std::list<std::pair<K, V> > {

public:

	typedef K key_t;
	typedef V value_t;

	typedef std::pair<K, V> entry_t;
	typedef std::list<entry_t> container_t;

	Dictionary2() : separator(','){};

	Dictionary2(const Dictionary2 & d) : separator(d.separator){};

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

	//static	const K defaultKey = K();
	//static	const V defaultValue = V();
	void toOStr(std::ostream & ostr = std::cout, char separator=0) const {

		if (!separator)
			separator = this->separator;

		char sep = 0;
		for (typename container_t::const_iterator it = this->begin(); it != this->end(); ++it){
			if (sep)
				ostr << sep;
			else
				sep = separator;
			ostr << it->first << '=' << it->second;
		}
	}

	std::string toStr(char separator=0) const {
		std::stringstream sstr;
		this->toOStr(sstr, separator);
		return sstr.str();
	}

	char separator;
};

template <class K, class V>
inline
std::ostream & operator<<(std::ostream & ostr, const Dictionary2<K,V> & dict) {
	dict.toOStr(ostr);
	return ostr;
}


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
