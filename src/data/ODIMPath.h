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
#ifndef ODIM_PATH
#define ODIM_PATH


#include <ostream>
//#include <cmath>
#include <string>
#include <string.h>
#include <map>
//#include <algorithm>

#include "drain/util/Log.h"

#include "drain/util/Path.h"
#include "drain/util/Flags.h"
//#include "drain/util/GlobalFlags.h"


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

//struct odim_id {};
//typedef drain::GlobalFlags<odim_id> odim_flags;

class ODIMPathElem;

std::ostream & operator<<(std::ostream & ostr, const ODIMPathElem & p);



class ODIMPathElem  {

public:

	typedef drain::Flagger flag_t;
	typedef drain::Flagger::dict_t dict_t;

	/// In H5, "groups" correspond to "directories" or "folders" in Unix and Windows.

	typedef drain::Flagger::value_t group_t;
	//typedef odim_flags::value_t group_t;

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




	/// Metadata group \c /what , at any depth
	static const group_t WHAT  = 32;

	/// Metadata group \c /where , at any depth
	static const group_t WHERE = 64;

	/// Metadata group \c /how , at any depth
	static const group_t HOW   = 128; // ylennysmerkki

	/// Group index mask for groups that contain only meta data.
	static const group_t ATTRIBUTE_GROUPS = WHAT|WHERE|HOW;


	/// User defined group, name stored as a separate string. Index allowed, but only catenated in the string.
	static const group_t ALL_GROUPS = 255; // 511!

	/// Palette data (to be linked). EXPERIMENTAL
	static const group_t PALETTE = 256;

	/// Palette data (to be linked). EXPERIMENTAL
	static const group_t LEGEND = 512;

	/// User defined group, name stored as a separate string. The string may still contain numbers, but no indices will be extracted.
	/**
	 *   Not indexed.
	 */
	//static const group_t OTHER   = 255 ^ IS_INDEXED; // OTHER is not indexed...
	static const group_t OTHER   = 1024; // OTHER is not indexed...

	//static const group_t _SCAN   = 2048; // Extension

	//static const group_t _MOMENT = 4096; // Extension


	/// Group index mask for groups that have an index.
	static const group_t IS_INDEXED = (DATASET | DATA | QUALITY ); // | _SCAN | _MOMENT);


	static inline
	bool isIndexed(group_t group){
		return (group & IS_INDEXED) > 0;
	}

	static inline
	bool isQuality(group_t group){
		return (group == QUALITY);
	}


	static
	const dict_t & getDictionary();

	group_t group;
	typedef int index_t;

	// Running index of the element. Applied also as a lower limit in path matching.
	index_t index;


	inline
	ODIMPathElem(group_t group = ROOT, index_t index = 0) : group(group), index(index){ //, indexMax(index) {  // root=NONE
	}

	inline
	ODIMPathElem(const ODIMPathElem &e) : group(e.group), index(e.index){ // , indexMax(e.indexMax) {
		if (e.group == OTHER)
			str = e.str;
	}

	inline
	ODIMPathElem(const std::string &s) : group(ROOT), index(0){ //, indexMax(0){
		set(s);
	}

	inline
	ODIMPathElem(const char *s): group(ROOT), index(0){ //, indexMax(0) {
		set(s);
	}

	inline
	~ODIMPathElem(){};

	/// Redirects to set(const std::string &) .
	/**
	 *  \tparam T - something castable to std::string.
	 *
	 *  \param  s - std::string, or something directly castable to it.
	 */
	template <class T>
	inline
	ODIMPathElem & operator=(const T &s){
		set(s);
		return *this;
	}


	/// Calls set(const std::string &) .
	/**
	 *  \param  s - string corresponding a path element.
	 */
	inline
	ODIMPathElem & operator=(const char *s){
		set(s);
		return *this;
	}

	/// The fundamental assignment operator
	virtual inline
	bool set(group_t g, index_t i = 0){
		group = g;
		index = i;
		//indexMax = i; // not relevant, or check
		if ((i>0) && !isIndexed(g)){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.note("index (" , i , ") given for non-indexed element:" , *this );
			return false;
		}
		return (g != ODIMPathElem::OTHER);
	}

	/// Assign a string to this path element.
	/**
	 *   \param s - path element as a string,  "dataset4" for example
	 *
	 *   Note that path separator is not recognized; instead, it will be assigned as a part of the string.
	 *
	 *   \return - true if a valid ODIM path element was created
	 */
	virtual
	bool set(const std::string &s);

	/// Abbreviation of (group == NONE)
	inline
	bool is(group_t g) const {
		return (group == g);
	}

	/// Abbreviation of (group == ROOT)
	inline
	bool isRoot() const {
		return (group == ROOT);
	}

	/// Equivalent to isRoot(). This method is required in recognizing the leading empty string. Consider "/usr/include".
	inline
	bool empty() const {
		return isRoot();
	}


	/// Abbreviation of (group == NONE)
	/*
	inline
	bool isUnset() const {
		return (group == NONE);
	}
	*/

	/// Returns true, if group is DATASET, DATA or QUALITY
	inline
	bool isIndexed() const {
		return isIndexed(this->group);
	}

	/// Checks if the element belongs to any of groups given.
	/*
	 *  Warning: DATA and DATASET "belong" to quality?... (QUALITY = DATASET | DATA )
	 */
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

	/// Debugging/logging. Returns standard name. Does not check if type is OTHER.
	static
	const std::string & getKey(group_t g);

	/// Debugging/logging. Returns standard name. Does not check if type is OTHER.
	inline
	const std::string & getKey() const {
		return getKey(this->group);
	}

	/// Writes the name, including the index, to output stream.
	// virtual	std::ostream & toStream(std::ostream & sstr) const;

	operator const std::string &() const {
		if (this->group != OTHER){ // for OTHER, its already set.
			std::stringstream sstr;
			// toStream(sstr);
			sstr << *this;
			str = sstr.str();
		}
		return str;
	}

	/// Given the non-numeric prefix of a group, like "dataset" or "data", set the group.
	virtual
	bool extractPrefix(const std::string &s, bool indexed);

protected:

	/// Given a string starting with a numeral, try to extract the index.
	//  In ODIMPathMatcher, an index range will be extracted.
	virtual
	void extractIndex(const std::string &s);


	mutable
	std::string str;


};

extern ODIMPathElem odimROOT;
extern ODIMPathElem odimWHERE;
extern ODIMPathElem odimWHAT;
extern ODIMPathElem odimARRAY;

/// Important!
bool operator<(const ODIMPathElem & e1, const ODIMPathElem & e2);

bool operator==(const ODIMPathElem & e1, const ODIMPathElem & e2);

inline
bool operator!=(const ODIMPathElem & e1, const ODIMPathElem & e2){
	return ! operator==(e1, e2);
}


inline
std::ostream & operator<<(std::ostream & ostr, const ODIMPathElem & p) {

	/// Step 1: prefix (by group type)
	ostr << p.getPrefix();

	/// Step 2: index
	if (p.isIndexed())
		ostr << p.getIndex();

	return ostr; // p.toStream(ostr);
}

inline
std::istream & operator>>(std::istream & istr, ODIMPathElem & p) {
	std::string s;
	drain::StringTools::read<512>(istr, s);
	//istr >> s;
	p.set(s);
	return istr;
}


typedef drain::Path<ODIMPathElem,'/',true,false,true> ODIMPath;

typedef std::list<ODIMPath> ODIMPathList;

typedef std::vector<ODIMPathElem> ODIMPathElemSeq;

/**
 *   Applicable for timestamp based sorting of DATASET groups and quantity based DATA/quality groups
 */
typedef std::map<std::string,ODIMPathElem> ODIMPathElemMap;

struct ODIMPathLess {

	// Main function
	inline
	bool operator()(const ODIMPathElem & p1, const ODIMPathElem & p2) const {
		// return (p1<p2);
		return !(p1<p2);
	}

};  // end class


class ODIMPathElem2 : public ODIMPathElem {

public:

	inline
	ODIMPathElem2(): elangle(0.0) {};

	virtual inline
	~ODIMPathElem2(){};

	inline
	ODIMPathElem2(const ODIMPathElem & elem, const double elangle, const std::string & date, const std::string & time):
		ODIMPathElem(elem), elangle(elangle), timestamp(date+time) {
	}

	double elangle;
	std::string timestamp;


};


inline
std::ostream & operator<<(std::ostream & ostr, const ODIMPathElem2 & elem) {
	return ostr << (const ODIMPathElem &)elem << '-' << elem.timestamp << '-' << elem.elangle;
}


struct ODIMPathLessTime {
	inline
	bool operator()(const ODIMPathElem2 & e1, const ODIMPathElem2 & e2) const {
		return e1.timestamp < e2.timestamp;
	}
};

struct ODIMPathLessElangle {
	inline
	bool operator()(const ODIMPathElem2 & e1, const ODIMPathElem2 & e2) const {
		return e1.elangle < e2.elangle;
	}
};


/*
inline
std::ostream & operator<<(std::ostream & ostr, const ODIMPath & p) {
	return p.toStream(ostr);
}
*/


}  // namespace rack


#endif

// Rack
