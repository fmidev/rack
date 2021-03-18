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
#ifndef DRAIN_SMARTMAP_TOOLS_H
#define DRAIN_SMARTMAP_TOOLS_H

#include <iostream>
#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>
//#include <stdlib.h>
#include <sys/syslog.h>
#include <syslog.h>

#include "Log.h"
#include "Castable.h"
#include "String.h"

#include "Sprinter.h"


namespace drain {

/// A base class for smart maps providing methods for importing and exporting values, among others
// Note: some update()'s may still be buggy
class SmartMapTools {

public:

	typedef std::list<std::string> keylist_t;

	/**
	 *  \tparam S - source element type
	 *  \tparam T - target map type
	 */
	template <class T,class S,bool STRICT=true>
	static
	void setValue(T & dstMap, const std::string & key, const S & value) {

		if (STRICT){
			dstMap[key] = value;  // throws exception in derived classes!
		}
		else {
			//typename std::map<std::string, T>::iterator it = dstMap.find(key);
			typename T::iterator it = dstMap.find(key);
			if (it != dstMap.end()){
				it->second = value;
			}
			else {
				std::cerr << "EO\n";
			}
		}
	}



	/**
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 */
	template <class T,class S>
	static inline
	void updateValue(std::map<std::string, T> & dstMap, const std::string & key, const S & value) {
		setValue<T,S,false>(dstMap, key, value);
	}


	/**
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 */
	template <class T,class S,bool STRICT=true>
	static
	//void setValues(std::map<std::string,T> & dstMap, const std::map<std::string,S> & srcMap) {
	void setValues(T & dstMap, const std::map<std::string,S> & srcMap) {
		for (const typename std::map<std::string,S>::value_type & entry: srcMap){
			setValue<T,S,STRICT>(dstMap, entry.first, entry.second);
		}
	}

	/**
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 */
	template <class T,class S,bool STRICT=true>
	static inline
	void updateValues(std::map<std::string,T> & dstMap, const std::map<std::string,S> & srcMap) {
		 setValues<T,S,false>(dstMap, srcMap);
	}

	/// Assign values from list, accepting strict "<key>=<value>" format, no positional arguments.
	/**
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 */
	template <class T, bool STRICT=true>
	static
	void setValues(std::map<std::string,T> & dstMap, const std::list<std::string> & values, char equals='=') {
		typedef std::map<std::string,T> map_t;
		for (const std::string & entry: values){
			size_t i=entry.find(equals);
			if (i != std::string::npos){
				setValue<map_t,std::string,STRICT>(dstMap, entry.substr(0, i), entry.substr(i+1));
			}
			else {
				throw std::runtime_error(entry + "positional args without key list");
			}
		}
	}

	/// Assign values from list, accepting strict "<key>=<value>" format, no positional arguments.
	/**
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 */
	template <class T>
	static inline
	void updateValues(std::map<std::string,T> & dstMap, const std::list<std::string> & values, char equals='=') {
		setValues<T,false>(dstMap, values, equals);
	}

	/// Assign values from list, accepting keyword arguments \c <key>=<value>,<key2>=<value2> and positional arguments \c <value>,<value2>... .
	/**
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 *
	 *  \params keys - ordered key list of writable entries of destination map
	 */
	// Potential for allowed-keys-only policy (hidden/read-only entries)
	template <class T, bool STRICT=true>
	static
	void setValues(std::map<std::string,T> & dstMap, const std::list<std::string> & keys, const std::list<std::string> & entries, char equals='=') {

		bool acceptOrderedParams = true;
		std::list<std::string>::const_iterator kit = keys.begin();

		for (const std::string & entry: entries){
			size_t i=entry.find(equals);
			if (i != std::string::npos){
				setValue<STRICT>(dstMap, entry.substr(0, i), entry.substr(i+1));
				acceptOrderedParams = false;
			}
			else if (kit != keys.end()){
				setValue<STRICT>(dstMap, *kit, entry);
				++kit;
			}
			else {
				//throw std::runtime_error(values + ": run out of positional args at '" +  entry + "'");
			}
		}

	}

	template <class T>
	static inline
	void updateValues(std::map<std::string,T> & dstMap, const std::list<std::string> & keys, const std::list<std::string> & entries, char equals='=') {
		setValues<false>(dstMap, keys, entries, equals);
	}


	/// Assign values from string, assuming strict "<key>=<value>" format, no positional arguments.
	/**
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 */
	template <class T, bool STRICT=true>
	static
	void setValues(std::map<std::string,T> & dstMap, const std::string & values, char split=',', char equals='=') {
		std::list<std::string> l;
		drain::StringTools::split(values, l, split);
		setValues(dstMap, l, equals);
	}

	template <class T>
	static inline
	void updateValues(std::map<std::string,T> & dstMap, const std::string & values, char split=',', char equals='=') {
		setValues<false>(dstMap, values, split, equals);
	}

	/// Assign values from string, accepting keyword arguments \c <key>=<value>,<key2>=<value2> and positional arguments \c <value>,<value2>... .
	/**
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 *
	 *  \params keys - ordered key list of writable entries of destination map
	 *
	 */
	template <class T, bool STRICT=true>
	static
	void setValues(std::map<std::string,T> & dstMap, const std::list<std::string> & keys, const std::string & values, char split=',', char equals='=') {
		std::list<std::string> l;
		drain::StringTools::split(values, l, split);
		setValues(dstMap, keys, l, equals);
	}


	template <class T>
	static inline
	void updateValues(std::map<std::string,T> & dstMap, const std::list<std::string> & keys, const std::string & values, char split=',', char equals='=') {
		setValues<false>(dstMap, keys, values, split, equals);
	}


};


/**
 *  Unlike with std::map<>, operator[] is defined as const, too, returning reference to a static empty instance.
 *
 *  SmartMap maintains an ordered list of keys, accessible with getKeys(). The items in the key list are
 *  appended in the order they have been created. This is order is generally different from the (alphabetical) order
 *  appearing in std::map iteration.
 *
 *  Comma-separated values can be assigned simultaneously with setValues() and updateValues()
 */


} // drain


#endif // Drain
