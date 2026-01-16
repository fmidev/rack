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

#include <drain/Log.h>
#include <drain/Reference.h>
#include <iostream>
#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>
//#include <stdlib.h>
#include <sys/syslog.h>
#include <syslog.h>

#include "Castable.h"
#include "Reference.h"
#include "Sprinter.h"
#include "StringTools.h"
#include "Variable.h"

#include "MapTools.h"


namespace drain {

/// General purpose tools for  handling values and keys of std::map<>
class SmartMapTools : public MapTools {

public:

	/// If the key is found, the value is returned as a reference.
	/**
	 *  \param m - map in which keys are searched
	 *
	 *  \tparam M - map type
	 *  \tparam F - target value type
	template <class M>
	static
	const typename M::mapped_type & get(const M & m, const typename M::key_type & key){
		typename M::const_iterator it = m.find(key);
		if (it != m.end()){
			return it->second;
		}
		else {
			static const typename M::mapped_type empty;
			return empty;
		}
	}
	 */


	/// If the key is found, the value is assigned.
	/**
	 *  \param m - map in which keys are searched
	 *
	 *  \tparam M - map type
	 *  \tparam F - target value type
	 */
	template <class M,class F>
	static
	void get(const M & m, const typename M::key_type & key, F & value){
		typename M::const_iterator it = m.find(key);
		if (it != m.end()){
			drain::Reference r(value);
			r = it->second;
			return; // false; // it->second;
		}
		else { //
			return; //  true; // defaultValue;
		}
	}


	typedef std::list<std::string> keylist_t;


	template <class M,class S,bool STRICT=true>
	static
	void setCastableValues(M & dst, const std::map<std::string,S> & srcMap) {
		//std::cerr << __FUNCTION__ << ':' << typeid(M).name() << " <= map<str," << typeid(S).name() << ">\n";
		for (const typename std::map<std::string,S>::value_type & entry: srcMap){
			MapTools::setValue<M,drain::Castable,STRICT>(dst, entry.first, (const drain::Castable &) entry.second);
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
	void updateCastableValues(M & dst, const std::map<std::string,S> & src) {
		 setCastableValues<M,S,false>(dst, src);
	}


	// Must be renamed to prevent hiding?
	template <class M, bool STRICT=true>
	static
	void setValues(M & dst, const std::initializer_list<Variable::init_pair_t > &l){
		for (const auto & entry: l){
			drain::MapTools::setValue<M,Variable,STRICT>(dst, entry.first, entry.second);
		}
	}




};




} // drain


#endif // Drain
