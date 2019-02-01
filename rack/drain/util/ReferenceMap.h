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

#include "Log.h"
#include "Reference.h"
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
class ReferenceMap : public SmartMap<Referencer> {//public std::map<std::string,Referencer> {

public:

	/// Default constructor
	/**
	 *  \strictness - if true, allow attempts of assigning to non-existing entries; otherways throw exception on attempt.
	 */
	// ReferenceMap(bool ordered, char separator) : SmartMap<Referencer>(ordered, separator){}; //, STRICTLY_CLOSED, keys(orderedKeyList), units(unitMap)
	ReferenceMap(char separator=',') : SmartMap<Referencer>(separator){}; //, STRICTLY_CLOSED, keys(orderedKeyList), units(unitMap)

	/// Copy constructor copies only the separators (doesn't copy the items).
	inline
	ReferenceMap(const ReferenceMap & rmap) : SmartMap<Referencer>(rmap.separator, rmap.arraySeparator){
	};

	/// Associates a map entry with a variable
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */
	template <class F>
	void reference(const std::string & key, F &x, const std::string & unit = std::string()){

		if (find(key) == end()) // not  already referenced
			keyList.push_back(key);

		// Create
		Referencer & r = std::map<std::string,Referencer>::operator[](key);
		r.setSeparator(arraySeparator); // applicable, if array type element
		// Link
		r.link(x);
		unitMap[key] = unit;
	}


	/// Associates a map entry with a variable, adding key in the beginning of key list.
	/**
	 *  \param key - variable name in a map
	 *  \param x   - target variable to be linked
	 */
	template <class F>
	void referenceTop(const std::string & key, F &x, const std::string & unit = std::string()){
		if (find(key) != end()) // already referenced
			keyList.push_front(key);
		std::map<std::string,Referencer>::operator[](key).link(x);
		unitMap[key] = unit;
	}


	/// Adopts the references of r. If replace==false, only new entries are appended.
	void append(ReferenceMap & r, bool replace=true){
		//std::cerr << __FILE__ << " -> " << __FUNCTION__ <<  std::endl;
		const std::list<std::string> & keys = r.getKeyList();
		for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){
			//std::cerr << " -> " << *it <<  std::endl;
			if (replace || !hasKey(*it)){
				reference(*it, r[*it], r.unitMap[*it]);
			}
		}
	}

	// todo: consider NULL? (but leads to problems)

	/// Removes an entry from the map.
	inline
	void dereference(const std::string & key){
		std::map<std::string,Referencer>::erase(key);
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
	virtual inline
	void clear(){
		SmartMap<Referencer>::clear();
		unitMap.clear();
	}

	template <class T>
	inline
	ReferenceMap & operator=(const SmartMap<T> & v){
		importMap(v);
		return *this;
	}

	virtual
	mapped_type & operator[](const std::string &key){

		Logger mout(__FILE__, __FUNCTION__);

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
	virtual
	const mapped_type & operator[](const std::string &key) const {

		Logger mout(__FILE__, __FUNCTION__);

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

	/// Defines the units of each element.
	// Questionable: consider SmartReference etc. with caster and unit info?
	std::map<std::string,std::string> unitMap;


};



}  // namespace drain


#endif

// Drain
