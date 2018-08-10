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
#ifndef BASE_ODIM_STRUCT
#define BASE_ODIM_STRUCT

#include <ostream>
//#include <cmath>
#include <string>
#include <string.h>
#include <map>
//#include <algorithm>
#include <drain/util/Path.h>


namespace rack {


/**
 *  Within each ODIM class, each variable should have
 *
 *  -# name
 *  -# type
 *  -# group/name
 *  -# dataset/group/name
 *
 *  The scope for variables is "down to the data array". Ie.
 *
 *  Common interface:
 *
 *  -# operator[](toStr)
 *  -#
 *
 *  Class specific members:
 *
 *  -# native variables (double, long int, std::string)
 *
 *  Typically used in creating and writing a product.
 *  See also: LinearScaling (could be used as base class?)
 */
class BaseODIM  {

public:

	/// In H5, "groups" containers of data corresponding to "directories" or "folders" in Unix and Windows.
	typedef unsigned int group_t;

	/// None (undefined)
	static const group_t NONE = 0;

	/// Root of a path
	/// Zeroth level group, identified with empty string "", or "/" which is the separator prefixed with empty string.
	static const group_t ROOT = 1;

	/// Metadata group "what", at any depth
	static const group_t WHAT  = 2;

	/// User defined group, name stored as a separate string.
	// static const group_t UNUSED = 3;

	/// Metadata group "where", at any depth
	static const group_t WHERE = 4;

	/// Metadata group "how", at any depth
	static const group_t HOW   = 6;

	/// Flag, set if this path element requires an index (like dataset2, or data5)
	// static const group_t IS_INDEXED = 8;

	/// First level group.
	static const group_t DATASET = 8;  // one bit (16) must be unique

	/// Second level group.
	static const group_t DATA    = 16;


	/// Second or third level group containing quality data associated with data at the same level or below.
	static const group_t IS_QUALITY = 32;

	static const group_t QUALITY = DATA | IS_QUALITY;

	/// Data group "data", at deepest level, like /dataset4/data2/quality1/data
	static const group_t ARRAY   = 64;

	/// Flag test for writing attributes
	//static const group_t UPDATED = DATASET|DATA|QUALITY; //

	/// User defined group, name stored as a separate string. Index allowed, but only catenated in the string.
	static const group_t OTHER   = 128;

	// static const group_t OTHER_INDEXED = (OTHER | IS_INDEXED); // tmp, user defined, etc.
	static const group_t ALL = (ROOT | DATASET | DATA | QUALITY);

	static const group_t IS_INDEXED = (DATASET | DATA);

	static inline
	bool isIndexed(group_t group){
		return (group & IS_INDEXED) > 0;
	}

	static inline
	bool isQuality(group_t group){
		return (group & IS_QUALITY) == IS_QUALITY;
	}

	typedef std::map<group_t, std::string> dict_t;

	static
	const dict_t & getDictionary();

	/// User defined group, name stored as a separate string.
	// ?


};



class ODIMPathElem  {

public: // from ODIM.h

	BaseODIM::group_t group;
	typedef int index_t;
	index_t index;

	inline
	ODIMPathElem(BaseODIM::group_t group=BaseODIM::NONE, index_t index = 0) : group(group), index(index) {
	}

	inline
	ODIMPathElem(const std::string &s){
		set(s);
	}

	inline
	ODIMPathElem(const ODIMPathElem &e) : group(e.group), index(e.index) {
		if (e.group == BaseODIM::OTHER)
			other = e.other;
	}

	inline
	ODIMPathElem(const char *s){
		set(s);
	}


	/// Redirects to set(const std::string &) .
	/**
	 *  \tparam T - something castable to std::string.
	 */
	template <class T>
	inline
	ODIMPathElem & operator=(const T &v){
		set(v);
		return *this;
	}

	inline
	ODIMPathElem & operator=(const char *s){
		set(s);
		return *this;
	}

	inline
	void set(BaseODIM::group_t g, index_t i = 0){
		group = g;
		index = i;
		if ((i>0) && !BaseODIM::isIndexed(g)){
			// warning
		}
	}

	/// Assign a path string, like "dataset4/data5/quality1/data".
	void set(const std::string &s);

	/// Abbreviation of (group == BaseODIM::NONE)
	inline
	bool is(BaseODIM::group_t g) const {
		return (group == g);
	}

	/// Abbreviation of (group == BaseODIM::ROOT)
	inline
	bool isRoot() const {
		return (group == BaseODIM::ROOT);
	}

	/// Abbreviation of (group == BaseODIM::NONE)
	inline
	bool isUnset() const {
		return (group == BaseODIM::NONE);
	}

	/// Abbreviation of (group == BaseODIM::NONE)
	inline
	bool isIndexed() const {
		return BaseODIM::isIndexed(this->group);
	}

	inline
	BaseODIM::group_t getType() const {
		return this->group;
	}

	inline
	index_t getIndex() const {
		return index;
	}

	/// Returns the name without the index.
	const std::string & getPrefix() const;

	char getCharCode() const;

	/// Writes the name, including the index, to output stream.
	std::ostream & toOStr(std::ostream & sstr) const {

		/// Step 1: prefix (by group type)
		sstr << getPrefix();

		/// Step 2: index
		if (BaseODIM::isIndexed(this->group))
			sstr << this->index;
		//sstr << '{' << this->index << '}';

		return sstr;
	};

	operator const std::string &() const {
		if (this->group != BaseODIM::OTHER){
			std::stringstream sstr;
			toOStr(sstr);
			other = sstr.str();
		}
		return other;
	}

	// TODO: Returns name
	/**
	 *   Uses "other" field as a temp storage.
	 */
	//const std::string & getStr() const;

protected:

	/// Returns standard name. Does not check if type is OTHER.
	const std::string & getKey() const;

	mutable
	std::string other;


};

bool operator<(const ODIMPathElem & e1, const ODIMPathElem & e2);

bool operator==(const ODIMPathElem & e1, const ODIMPathElem & e2);

inline
bool operator!=(const ODIMPathElem & e1, const ODIMPathElem & e2){
	return ! operator==(e1, e2);
}


inline
std::ostream & operator<<(std::ostream & ostr, const ODIMPathElem & p) {
	return p.toOStr(ostr);
}

inline
std::istream & operator>>(std::istream & istr, ODIMPathElem & p) {
	std::string s;
	drain::StringTools::read<512>(istr, s);
	//istr >> s;
	p.set(s);
	return istr;
}


typedef drain::Path<ODIMPathElem> ODIMPath;


}  // namespace rack


#endif

// Rack
