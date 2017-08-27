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
//#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>

#include <map>
#include <list>

#include "Debug.h"
#include "Castable.h"
#include "String.h"
#include "SmartMap.h"

#ifndef REFERENCE_MAP
#define REFERENCE_MAP

// // // using namespace std;

namespace drain {


/// Map in which each entry references a base type variable or a std::string; changing values in either are equivalent operations.
/** A class designed for objects returned by CastableIterator. Contains a pointer to external object.
 *
 *  \example ReferenceMap-example.cpp
 */
// TODO: consider clear() (shallow op)
class ReferenceMap : public SmartMap<Castable> {//public std::map<std::string,Castable> {

public:

	/**
	 *  \strictness - if true, allow attempts of assigning to non-existing entries; otherways throw exception on attempt.
	 */
	ReferenceMap(bool ordered=true, char separator=',') : SmartMap<Castable>(ordered, separator){}; //, STRICTLY_CLOSED, keys(orderedKeyList), units(unitMap)
	//ReferenceMap() : lenient(false), separators(",") {}; //, keys(orderedKeyList), units(unitMap)

	ReferenceMap(const ReferenceMap & rmap) : SmartMap<Castable>(rmap.ORDERED, rmap.separator){};  // , STRICTLY_CLOSED,


	/// Associates a map entry with a variable
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */
	template <class F>
	void reference(const std::string & key, F &x, const std::string & unit = std::string()){
		if (ORDERED){
			if (find(key) == end()) // already referenced
				keyList.push_back(key);
		}
		std::map<std::string,Castable>::operator[](key).link(x);
		unitMap[key] = unit;
	}

	/// Associates a map entry with a variable, adding key in the beginning of key list.
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */
	template <class F>
	void referenceTop(const std::string & key, F &x, const std::string & unit = std::string()){
		if (ORDERED){
			if (find(key) != end()) // already referenced
				keyList.push_front(key);
		}
		std::map<std::string,Castable>::operator[](key).link(x);
		unitMap[key] = unit;
	}


	/// Re-references all the entries of a given reference map.
	void append(ReferenceMap & r){
		//std::cerr << __FILE__ << " -> " << __FUNCTION__ <<  std::endl;
		const std::list<std::string> & keys = r.getKeyList();
		for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){
			// std::cerr << " -> " << *it <<  std::endl;
			reference(*it, r[*it]);
		}
	}


	/// Add a reference to an existing reference.
	/*
	void aliasOLD(const std::string & keyNew, const std::string & key){
		std::map<std::string,Castable>::insert(std::pair<std::string,Castable>(keyNew, std::map<std::string,Castable>::operator[](key)));
	}
	*/

	// todo: consider NULL? (but leads to problems)
	inline
	void dereference(const std::string & key){
		std::map<std::string,Castable>::erase(key);
		for (std::list<std::string>::iterator it = keyList.begin(); it != keyList.end(); ++it)
			if (*it == key){
				keyList.erase(it);
				break;
			}
		unitMap.erase(key);
	}

	/// Removes all the elements of the map.
	/**
	 *  Clears the map and its ordered keylist (and units).
	 */
	inline
	void clear(){
		SmartMap<Castable>::clear();
		unitMap.clear();
		keyList.clear();
	}

	template <class T>
	inline
	ReferenceMap & operator=(const SmartMap<T> & v){
		importMap(v);
		return *this;
	}

	inline
	virtual
	mapped_type & operator[](const std::string &key){

		MonitorSource mout(__FILE__, __FUNCTION__);

		iterator it = this->find(key);
		if (it != this->end()) {
			return it->second;
		}
		else {
			mout.error() << "key '" << key <<"' not declared (referenced)" << mout.endl;
			//throw std::runtime_error(key + ": ReferenceMap & operator[] : key not found");
			static mapped_type empty;
			return empty;
		}

	}

	///  Calling unreferenced key throws an exception.
	inline
	virtual
	const mapped_type & operator[](const std::string &key) const {

		MonitorSource mout(__FILE__, __FUNCTION__);

		const_iterator it = this->find(key);
		if (it != this->end()) {
			return it->second;
		}
		else {
			mout.error() << "key '" << key <<"' not declared (referenced)" << mout.endl;
			//throw std::runtime_error(key + ": ReferenceMap & operator[] : key not found");
			static const mapped_type empty;
			return empty;
		}
	}


	/// Returns measurement unit information of the actual map entries.
	inline
	const std::map<std::string,std::string> & getUnitMap() const { return unitMap; };


protected:

	/// Defines the _units of each element. NEW (incomplete)
	std::map<std::string,std::string> unitMap;


};


}  // namespace drain


#endif

// Drain
