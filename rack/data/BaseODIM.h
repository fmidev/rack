/*


    Copyright 2011-2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

   Created on: Sep 30, 2010

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
