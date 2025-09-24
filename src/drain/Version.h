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



#ifndef DRAIN_VERSION_T
#define DRAIN_VERSION_T

#include <sstream>
#include <vector>

namespace drain {

template<typename V=void, char SEP='.'>
struct Version {

	typedef unsigned short index_t;
	// typedef int index_t;

	template<typename ... TT>
	Version(const TT &... args){
		append(args...);
		updateStr();
	}

	inline
	Version(const std::initializer_list<index_t> & args) : id(args){
		updateStr();
	}

	inline
	operator const std::string &() const {
		return idStr;
	}

	inline
	operator const std::vector<index_t> &() const {
		return id;
	}

	inline
	const std::string & str() const {
		return idStr;
	}

	inline
	std::ostream & toStream(std::ostream & ostr) const {
		ostr << idStr;
		return ostr;
	}

private:

	inline
	void updateStr(){
		std::stringstream sstr;
		char sep=0;
		for (index_t i: id){
			if (sep){
				sstr.put(sep);
			}
			else {
				sep = SEP;
			}
			sstr << i;
		}
		idStr = sstr.str();
	};

	template<typename ... TT>
	inline
	void append(const index_t & i, TT... args){
		id.push_back(i);
		append(args...);
	};

	inline
	void append(){
	};

	std::vector<index_t> id;

	std::string idStr;

};

template<typename V>
std::ostream & operator<<(std::ostream & ostr, const Version<V> & version){
	version.toStream(ostr);
	return ostr;
}

} // drain

#endif
