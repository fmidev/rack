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

} // drain::


#endif

// Drain
