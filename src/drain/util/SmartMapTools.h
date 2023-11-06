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

	/// Utility for "guessing" key order. Derived classes will redefine this e.g. for ordered maps.
	/*
	template <class T>
	static
	void getKeyList(const std::map<std::string,T> & src, keylist_t & keys) {
		keys.clear();
		for (const std::pair<std::string,T> & entry : src){
			keys.push_back(entry.first);
		}
	}
	*/

	/// Set value of an element. If not STRICT, set only if ket exists.
	/**
	 *  \tparam M - map type
	 *  \tparam S - source element type
	 */
	template <class M,class V,bool STRICT=true>
	static
	//void setValue(std::map<std::string,T> & dst, const std::string & key, const V & value) {
	void setValue(M & dst, const std::string & key, const V & value) {
		if (STRICT){
			dst[key] = value;  // throws exception in derived classes!
		}
		else {
			//typename std::map<std::string,T>::iterator it = dst.find(key);
			typename M::iterator it = dst.find(key);
			if (it != dst.end()){
				it->second = value;
			}
			else {
				//std::cerr << __FILE__ << ':' << __FUNCTION__ << ": key '" << key << "' not found\n";
			}
		}
	}


	/// Set value only if \c key exists.
	/**
	  *  \tparam M - map type
	  *  \tparam V - value type
	  */
	template <class M,class V>
	static inline
	void updateValue(M & dst, const std::string & key, const V & value) {
		setValue<M,V,false>(dst, key, value);
	}


	/**
	  *  \tparam S - source map type
	  */
	template <class M,class S,bool STRICT=true>
	static
	void setValues(M & dst, const std::map<std::string,S> & srcMap) {
		// std::cerr << __FUNCTION__ << ':' << typeid(M).name() << " <= map<str," << typeid(S).name() << ">\n";
		for (const typename std::map<std::string,S>::value_type & entry: srcMap){
			setValue<M,S,STRICT>(dst, entry.first, entry.second);
		}
	}

	template <class M,class S,bool STRICT=true>
	static
	void setCastableValues(M & dst, const std::map<std::string,S> & srcMap) {
		//std::cerr << __FUNCTION__ << ':' << typeid(M).name() << " <= map<str," << typeid(S).name() << ">\n";
		for (const typename std::map<std::string,S>::value_type & entry: srcMap){
			setValue<M,drain::Castable,STRICT>(dst, entry.first, (const drain::Castable &) entry.second);
		}
		/*
		 for (typename std::map<std::string,S>::const_iterator it = srcMap.begin(); it != srcMap.end(); ++it){
			setValue<M,S,STRICT>(dst, it->first, (const drain::Castable &) it->second);
		}
		*/
	}



	/**
	  *  \tparam M - map type
	  *  \tparam S - source type
	 */
	template <class M,class S>
	static inline
	void updateValues(M & dst, const std::map<std::string,S> & src) {
		 setValues<M,S,false>(dst, src);
	}

	/**
	  *  \tparam M - map type
	  *  \tparam S - source type
	 */
	template <class M,class S>
	static inline
	void updateCastableValues(M & dst, const std::map<std::string,S> & src) {
		 setCastableValues<M,S,false>(dst, src);
	}

	/// Assign values from list, accepting strict "<key>=<value>" format, no positional arguments.
	/**
	 *  \tparam M - map type
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 */
	template <class M, bool STRICT=true>
	static
	void setValues(M & dst, const std::list<std::string> & values, char equalSign='=') {
		for (const std::string & entry: values){
			const size_t i = entry.find(equalSign);
			if (i != std::string::npos){
				if (i == (entry.length()-1)){
					setValue<M,std::string,STRICT>(dst, entry, ""); // todo: touch
				}
				else {
					setValue<M,std::string,STRICT>(dst, entry.substr(0, i), entry.substr(i+1));
				}
			}
			else {
				// TODO: warn dump
				throw std::runtime_error(entry + ": positional args without keys");
			}
		}
	}

	/// Assign values from list, accepting strict "<key>=<value>" format, no positional arguments.
	/**
	 *  \tparam M - map type
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 */
	template <class M>
	static inline
	void updateValues(M & dst, const std::list<std::string> & values, char equals='=') {
		setValues<M,false>(dst, values, equals);
	}

	/// Assign values from list, accepting keyword arguments \c <key>=<value>,<key2>=<value2> and positional arguments \c <value>,<value2>... .
	/**
	 *  \tparam M - map type
	 *  \tparam STRICT - assign always; create a new entry if key does not exist
	 *
	 *  \params keys - ordered key list of writable entries of destination map
	 */
	// Potential for allowed-keys-only policy (hidden/read-only entries)
	template <class M, bool STRICT=true>
	static
	void setValues(M & dst, const std::list<std::string> & keys, const std::list<std::string> & entries, char assignmentSymbol='=') {

		Logger mout(__FILE__, __FUNCTION__);


		const std::string assignmentSymbols(1, assignmentSymbol);

		//const std::list<std::string> & keys = getKeyList();
		std::list<std::string>::const_iterator kit = keys.begin();


		bool acceptOrderedParams = true;

		// mout.warn() << " assignmentSymbol: " <<  assignmentSymbol << mout.endl;
		// mout.warn() << " size: " <<  this->size() << mout.endl;
		for (const std::string & entry: entries){

			//for (std::list<std::string>::const_iterator pit = p.begin(); pit != p.end(); ++pit){
			//mout.warn() << " entry: " << *pit << mout.endl;

			// Check specific assignment, ie. check if the key=value is given explicitly.
			if (assignmentSymbol){ // typically '='

				std::string key, value;

				if (StringTools::split2(entry, key, value, assignmentSymbols)){

					// mout.warn() << " specified " <<  key << "=\t" << value << mout.endl;

					if (dst.size()==1){
						/*
						 * This special handling is for single-element maps
						 * "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"
						 */
						typename M::iterator it = dst.begin();
						if (key == it->first)
							it->second = value;
						else {
							it->second = entry;
							// mout.warn() << "is this in use?" << it->first << " <= '" << entry << "'" << mout;
						}
						return;
					}

					setValue<M,std::string,STRICT>(dst, key, value); //, criticality);
					acceptOrderedParams = false;
					continue;
				}
				else {
					// mout.warn() << " could not split: " << *pit << mout.endl;
				}
			}

			// Key and assignment symbol not given.

			if (kit != keys.end()){
				// Assignment-by-order
				if (!acceptOrderedParams){
					mout.warn() << "positional arg '" << entry << "' for ["<< *kit << "] given after explicit args" << mout.endl;
				}
				//mout.warn() << " ordered  " <<   << mout.endl;
				dst[*kit] = entry;  // does not need to call import() because *kit exists.
				++kit; // NUEVO
			}
			else {
				//mout.log(criticality)
				// << "too many (over "<< this->size() << ") params, run out of keys with entry=" << *pit << mout.endl;
				if (STRICT){
					mout.error() << "too many (over "<< dst.size() << ") params, run out of keys with entry=" << entry << mout.endl;
				}

			}


		}

		/* OLTTI
		bool acceptOrderedParams = true;
		std::list<std::string>::const_iterator kit = keys.begin();

		for (const std::string & entry: entries){
			size_t i=entry.find(equals);
			if (i != std::string::npos){
				setValue<std::map<std::string,T>, std::string, STRICT>(dst, entry.substr(0, i), entry.substr(i+1));
				acceptOrderedParams = false;
			}
			else if (kit != keys.end()){
				// Assignment-by-order
				if (!acceptOrderedParams){
					mout.warn() << "positional arg '" << entry << "' for ["<< *kit << "] given after explicit args" << mout.endl;
				}
				setValue<std::map<std::string,T>, std::string, STRICT>(dst, *kit, entry);
				++kit;
			}
			else {
				//throw std::runtime_error(values + ": run out of positional args at '" +  entry + "'");
			}
		}
		*/

	}

	/**
	 *  \tparam M - map type
	 *  \tparam STRICT - assign always; create a new entry if key does not exist
	 *
	 *  \params keys - ordered key list of writable entries of destination map
	 */
	template <class M>
	static inline
	void updateValues(M & dst, const std::list<std::string> & keys, const std::list<std::string> & entries, char equals='=') {
		setValues<false>(dst, keys, entries, equals);
	}


	/// Assign values from string, assuming strict "<key>=<value>" format, no positional arguments.
	/**
	 *  \tparam S - source type
	 *  \tparam T - destination type
	 */
	/**
	 *  \tparam M - map type
	 *  \tparam STRICT - assign always; create a new entry if key does not exist
	 *
	 *  \params keys - ordered key list of writable entries of destination map
	 */
	template <class M, bool STRICT=true>
	static
	void setValues(M & dst, const std::string & values, char split=',', char equals='=') {
		std::list<std::string> l;
		drain::StringTools::split(values, l, split);
		setValues(dst, l, equals);
	}

	/**
	 *  \tparam M - map type
	 *  \tparam STRICT - assign always; create a new entry if key does not exist
	 *
	 *  \params keys - ordered key list of writable entries of destination map
	 */
	template <class M>
	static inline
	void updateValues(M & dst, const std::string & values, char split=',', char equals='=') {
		setValues<false>(dst, values, split, equals);
	}

	/// Assign values from string, accepting keyword arguments \c <key>=<value>,<key2>=<value2> and positional arguments \c <value>,<value2>... .
	/**
	 *  \tparam M - map type
	 *  \tparam STRICT - assign always; create a new entry if key does not exist
	 *
	 *  \params keys - ordered key list of writable entries of destination map
	 *
	 */
	template <class M, bool STRICT=true>
	static
	void setValues(M & dst, const std::list<std::string> & keys, const std::string & values, char split=',', char equals='=') {
		std::list<std::string> l;
		drain::StringTools::split(values, l, split);
		setValues(dst, keys, l, equals);
	}


	/**
	 *  \tparam M - map type
	 *  \tparam V - value type
	 *  \tparam STRICT - assign always; create a new entry if key does not exist
	 *
	 *  \params keys - ordered key list of writable entries of destination map
	 */
	template <class M>
	static inline
	void updateValues(M & dst, const std::list<std::string> & keys, const std::string & values, char split=',', char equals='=') {
		setValues<false>(dst, keys, values, split, equals);
	}

	/// Assign values from string, accepting keyword arguments \c <key>=<value>,<key2>=<value2> and positional arguments \c <value>,<value2>... .
	/**
	 *  \tparam M - map type
	 *  \tparam V - value type
	 *  \tparam STRICT - assign always; create a new entry if key does not exist
	 *
	 *  \params keys - ordered key list of the already existing entries
	 *
	 */
	template <class M, class V, bool STRICT=true>
	static
	void setValues(M & dst, const std::list<std::string> & keys, std::initializer_list<V> values) {

		std::list<std::string>::const_iterator kit = keys.begin();

		for (const V & value: values){
			if (kit == keys.end()){
				throw std::runtime_error(std::string(__FILE__) + " run out of keys");
			}
			setValue(dst, *kit, value);
			++kit;
		}
	}

	/**
	 *  \tparam M - map type
	 *  \tparam V - value type
	 *
	 *  \params keys - ordered key list of writable entries of destination map
	 */
	template <class M, class V>
	static
	void setValues(M & dst, std::initializer_list<V> values) {

		//typename std::map<std::string,T>::const_iterator it = dst.begin();
		typename M::const_iterator it = dst.begin();

		for (const V & value: values){
			if (it == dst.end()){
				throw std::runtime_error(std::string(__FILE__) + " run out of keys");
			}
			setValue(dst, it->first, value);
			++it;
		}
	}

	/*
	template <class T, class T2>
	static inline
	void updateValues(std::map<std::string,T> & dst, const std::list<std::string> & keys, std::initializer_list<T2> values) {
		setValues<false>(dst, keys, values);
	}
	*/


};




} // drain


#endif // Drain
