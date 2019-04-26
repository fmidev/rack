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
#include <drain/util/Log.h>


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


class ODIMPathElem  {

public: // from ODIM.h


	/// In H5, "groups" containers of data corresponding to "directories" or "folders" in Unix and Windows.
	typedef unsigned int group_t;

	/// None (undefined)
	static const group_t NONE = 0;

	/// Root of a path
	/// Zeroth level group, identified with empty string "", or "/" which is the separator prefixed with empty string.
	static const group_t ROOT = 1;

	/// First level group, \c /dataset + \e digit .
	static const group_t DATASET = 2; // 8;  // one bit (16) must be unique

	/// Second level group, \c /data + \e digit .
	static const group_t DATA    = 4; // 16;

	/// Data group "data", at deepest level, like /dataset4/data2/quality1/data
	static const group_t ARRAY   = 8; // low index, to appear before QUALITY (below)

	/// Special group on first or second level, \c /quality + \e digit , used for storing quality data.
	static const group_t QUALITY = 16;

	/// Group index mask for groups under which data arrays (ARRAY type) are found
	static const group_t DATA_GROUPS = DATASET | DATA | QUALITY;

	/// Abbreviation for linking (referencing) attributes at different levels (tree depths).
	static const group_t ALL_LEVELS = (ROOT | DATASET | DATA); //  | QUALITY

	/// Group index mask for groups that have an index.
	static const group_t IS_INDEXED = (DATASET | DATA | QUALITY);



	/// Metadata group \c /what , at any depth
	static const group_t WHAT  = 32;

	/// Metadata group \c /where , at any depth
	static const group_t WHERE = 64;

	/// Metadata group \c /how , at any depth
	static const group_t HOW   = 128; // ylennysmerkki

	/// Group index mask for groups that contain only meta data.
	static const group_t ATTRIBUTE_GROUPS = WHAT|WHERE|HOW;


	/// User defined group, name stored as a separate string. The string may still contain numbers, but no indices will be extracted.
	/**
	 *   Not indexed.
	 */
	static const group_t OTHER   = 255;

	/// User defined group, name stored as a separate string. Index allowed, but only catenated in the string.
	static const group_t ALL_GROUPS = 255;

	static inline
	bool isIndexed(group_t group){
		return (group & IS_INDEXED) > 0;
	}

	static inline
	bool isQuality(group_t group){
		return (group == QUALITY);
		//return (group & IS_QUALITY) == IS_QUALITY;
	}

	typedef std::map<group_t, std::string> dict_t;

	static
	const dict_t & getDictionary();

	/// User defined group, name stored as a separate string.
	// ?

	//ODIMPath::
	group_t group;
	typedef int index_t;
	index_t index;

	inline
	ODIMPathElem(group_t group = ROOT, index_t index = 0) : group(group), index(index) {  // root=NONE
	}

	inline
	ODIMPathElem(const std::string &s){
		set(s);
	}

	inline
	ODIMPathElem(const ODIMPathElem &e) : group(e.group), index(e.index) {
		if (e.group == OTHER)
			str = e.str;
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
	void set(group_t g, index_t i = 0){
		group = g;
		index = i;
		if ((i>0) && !isIndexed(g)){
			drain::Logger mout("ODIMPath", __FUNCTION__);
			mout.note() << "index (" << i << ") given for non-indexed element:" << *this << mout.endl;
		}
	}

	/// Assign a path string, like "dataset4/data5/quality1/data".
	void set(const std::string &s);

	/// Abbreviation of (group == NONE)
	inline
	bool is(group_t g) const {
		return (group == g);
	}

	/// Abbreviation of (group == ROOT)
	inline
	bool empty() const {
		return (group == ROOT);
	}

	/// Abbreviation of (group == ROOT)
	inline
	bool isRoot() const {
		return (group == ROOT);
	}

	/// Abbreviation of (group == NONE)
	/*
	inline
	bool isUnset() const {
		return (group == NONE);
	}
	*/

	/// Abbreviation of (group == NONE)
	inline
	bool isIndexed() const {
		return isIndexed(this->group);
	}

	/// Checks if the element belongs to any of groups given.
	inline
	bool belongsTo(int groupFilter) const {
		// Notice: filter must fully "cover" group bits (especially because QUALITY = DATASET | DATA )
		return ((this->group & groupFilter) == this->group);
		//return ((this->group & groupFilter) != 0);
	}

	inline
	group_t getType() const {
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
		if (isIndexed(this->group))
			sstr << this->index;
		//sstr << '{' << this->index << '}';

		return sstr;
	};

	operator const std::string &() const {
		if (this->group != OTHER){
			std::stringstream sstr;
			toOStr(sstr);
			str = sstr.str();
		}
		return str;
	}

	// TODO: Returns name
	/**
	 *   Uses "str" field as a temp storage.
	 */
	//const std::string & getStr() const;

	/// Returns standard name. Does not check if type is OTHER.
	static
	const std::string & getKey(group_t g);

	/// Returns standard name. Does not check if type is OTHER.
	inline
	const std::string & getKey() const {
		return getKey(this->group);
	}

protected:


	mutable
	std::string str;


};


extern ODIMPathElem odimWHERE;
extern ODIMPathElem odimWHAT;
extern ODIMPathElem odimARRAY;

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

typedef std::list<ODIMPath> ODIMPathList;


struct ODIMPathLess {


	/*
	bool kompate(const ODIMPath & p1, const ODIMPath & p2) const {

		const_iterator it1 = p1.begin();
		const_iterator it2 = p2.begin();

		while (true){

			if (it1 == p1.end())
				return true;

			if (it2 == p2.end())
				return false;

			++it1;
				++it2;

		}

	}
	*/

	// Main function
	bool operator()(const ODIMPathElem & p1, const ODIMPathElem & p2) const {

		if (p1.group < p2.group){
			return true;
		}
		else if (p1.group > p2.group){
			return false;
		}
		else { // p1.group == p2.group
			if (p1.isIndexed()){
				return (p1.getIndex() < p2.getIndex());
			}
			else if (p1.is(ODIMPathElem::OTHER)){
				return (const std::string &)p1 < (const std::string &)p2 ;
			}
		}
		// e.g. WHAT == WHAT
		return false;
	}

};  // end class





}  // namespace rack


#endif

// Rack
