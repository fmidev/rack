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

#include <drain/Log.h>
// #include <drain/util/GlobalFlags.h>

#include "ODIMPath.h"

namespace rack {


const ODIMPathElem::group_t ODIMPathElem::NONE;
const ODIMPathElem::group_t ODIMPathElem::ROOT; // = odim_flags::add("ROOT");
const ODIMPathElem::group_t ODIMPathElem::ARRAY;
const ODIMPathElem::group_t ODIMPathElem::ATTRIBUTE_GROUPS;
const ODIMPathElem::group_t ODIMPathElem::IS_INDEXED;
const ODIMPathElem::group_t ODIMPathElem::DATASET;
const ODIMPathElem::group_t ODIMPathElem::DATA;
const ODIMPathElem::group_t ODIMPathElem::QUALITY;
const ODIMPathElem::group_t ODIMPathElem::WHAT;
const ODIMPathElem::group_t ODIMPathElem::WHERE;
const ODIMPathElem::group_t ODIMPathElem::HOW;
//const (ODIMPathElem::group_t ODIMPathElem::IS_QUALITY;
const ODIMPathElem::group_t ODIMPathElem::OTHER;
const ODIMPathElem::group_t ODIMPathElem::PALETTE; // experimental
const ODIMPathElem::group_t ODIMPathElem::LEGEND;  // experimental
const ODIMPathElem::group_t ODIMPathElem::ALL_LEVELS; // = ODIMPathElem::ROOT | ODIMPathElem::DATASET | ODIMPathElem::DATA;

// const ODIMPathElem::group_t ODIMPathElem::_SCAN;
// const ODIMPathElem::group_t ODIMPathElem::_MOMENT;

const ODIMPathElem::index_t ODIMPathElem::INDEX_MAX = std::numeric_limits<unsigned short int>::max(); // 0xffff: enough for DATA, DATASET, QUANTITY

void ODIMPathElem::reset(){
	group = 0;
	index = 0;
	//prefix = "";
}

// TODO: remove return value!
//bool
void ODIMPathElem::add(const std::string &s){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//mout.attention("setting: ", s);

	/*
	this->group = ROOT; // or none?

	// ?? Require explicit indexing.
	// ?? ODIMPathMatcher uses mixed groups, so "data2|what" should result index=2, not index=0.
	// this->index = 0;
	bool INDEXED = false; // to separate data and data1
	this->str = "";
	*/
	bool INDEXED = false; // to separate data and data1

	// Try removing this...
	/*
	static
	const dict_t & d = ODIMPathElem::getDictionary(); // New here
	*/

	// TEST1: equality
	/*
	dict_t::const_iterator dit = d.findByKey(s);
	if (dit != d.end()){
		this->group = dit->second;
		mout.note("direct: " , dit->first , '~' , dit->second );
		return true;
	}*/


	// Separate prefix (alphabets) and index (digits)
	std::string::const_iterator it = s.begin();
	while (it != s.end()){
		// Testing colon is "forward definition" of index range applied by ODIMPathMatcher
		if ((*it==':') || ((*it>='0') && (*it<='9'))){
			//if ((*it<'a') || (*it>'z')){ // to detect ':' if path elem mathcing
			// index_t i = 0;
			extractIndex(std::string(it, s.end())); // virtual
			// this->index |= i;
			INDEXED = true;
			break;
		}
		++it;
	}

	//mout.attention("setting: ", s, " --> ", std::string(s.begin(), it));

	if (extractPrefix(std::string(s.begin(), it), INDEXED))
		return; // true;

	this->group = OTHER;  //(INDEXED) ? ODIMPathElem::OTHER_INDEXED :
	this->currentStr = s;

	//mout.note("non-standard path element: " , *this );
	mout.warn("non-standard path element: " , *this );

}

template <>
void ODIMPathElem::extractIndex(const std::string &s, ODIMPathElem::index_t & idx){
	std::stringstream sstr(s);
	sstr >> idx;
}


void ODIMPathElem::extractIndex(const std::string &s){
	//std::stringstream sstr(s);
	//sstr >> this->index;
	extractIndex(s, this->index);
}

bool ODIMPathElem::extractPrefix(const std::string & prefix, bool indexed){

	drain::Logger mout(__FILE__, __FUNCTION__);

	static
	const dict_t & d = ODIMPathElem::getDictionary(); // New here

	// plain match (esp. "dataset" and "quality" without indices
	dict_t::const_iterator pit = d.end();

	/// Check first if prefix AND index match.
	for (dict_t::const_iterator it=d.begin(); it!=d.end(); ++it){

		if (prefix == it->first) {

			// ODIM oddity. Test only if indexed-modes match (this actually only is for data vs data1...N)
			if (indexed == isIndexed(it->second)) {
				// it->first is the group prefix [string]
				this->group |= it->second;
				return true;
			}

			// well, prefix matched anyway, so save as a secondary match
			pit = it;
		}

	}

	if (pit != d.end()){
		this->group |= pit->second;
		mout.debug(" -> recognized indexed group " , prefix , '=' , this->group );
		return true;
	}

	return false;

}



const std::string & ODIMPathElem::getKey(group_t group)  {

	static const dict_t & d = getDictionary();

	const dict_t::const_iterator it = d.findByValue(group);
	if (it != d.end()){
		return it->first;
	}
	else {
		static std::string empty("other");
		return empty;
	}
}

const std::string & ODIMPathElem::getPrefix() const {

	//if ((this->group & ODIMPathElem::OTHER) == 0){
	if (this->group != OTHER){
		return getKey();
	}
	else {
		return this->currentStr;
	}

}

char ODIMPathElem::getCharCode() const { // TODO: make faster?

	const std::string & prefix = getPrefix();

	if (prefix.empty())
		return 'E';

	if (group == DATASET){
		return 'D';
	}
	else
		return prefix.at(0);

}

// Note; ODIMPathElemMatcher redefines
/*
std::ostream & ODIMPathElem::toStream(std::ostream & sstr) const {

	/// Step 1: prefix (by group type)
	sstr << getPrefix();

	/// Step 2: index
	if (isIndexed(this->group))
		sstr << this->index;
	//sstr << '{' << this->index << '}';

	return sstr;
}
*/


bool operator==(const ODIMPathElem & e1, const ODIMPathElem & e2){

	if (e1.group != e2.group){
		return false;
	}
	else { // same group
		if (e1.isIndexed())
			return (e1.getIndex() == e2.getIndex());
		else if (e1.group == ODIMPathElem::OTHER)
			return (e1.getPrefix() == e2.getPrefix());
		else
			return (e1.group == e2.group);
	}
}



bool operator<(const ODIMPathElem & e1, const ODIMPathElem & e2){


	if (e1.group < e2.group){
		return true;
	}
	else if (e1.group > e2.group){
		return false;
	}
	else { // e1.group == e2.group
		if (e1.isIndexed()){
			return (e1.getIndex() < e2.getIndex());
		}
		else if (e1.group == ODIMPathElem::OTHER){ // e1.is(ODIMPathElem::OTHER)
			//return e1.str < e1.str;
			return (const std::string &)e1 < (const std::string &)e2 ;
		}
	}
	// e.g. WHAT == WHAT
	return false;

}

const ODIMPathElem::dict_t & ODIMPathElem::getDictionary(){

	static ODIMPathElem::dict_t dict = {
			{"*", NONE},
			{"", ROOT},
			{"dataset", DATASET},
			// NOTE: when searching by key  "data", ARRAY  will be found (first) and returned
			// NOTE: when searching by value ARRAY, "data" will be found (first) and returned
			{"data",    ARRAY},
			{"data",    DATA},
			{"quality", QUALITY},
			{"OTHER",   OTHER},
			{"what",    WHAT},
			{"where",   WHERE},
			{"how",     HOW},
			{"palette", PALETTE},
			{"legend",  LEGEND}
	//		{"scan",     _SCAN},
	//		{"moment_",  _MOMENT}
	};

	return dict;

}

/*
ODIMPathElem::operator const std::string &() const {
	if (this->group != OTHER){ // for OTHER, its already set.
		std::stringstream sstr;
		toStream(sstr);
		//sstr << *this;
		str = sstr.str();
	}
	return str;
}
*/


std::ostream & ODIMPathElem::toStream(std::ostream & ostr) const {

	/// Step 1: prefix (by group type)
	ostr << getPrefix();

	/// Step 2: index
	if (isIndexed())
		ostr << getIndex();

	return ostr; // p.toStream(ost
}


// Experimental naming.
ODIMPathElem odimROOT(ODIMPathElem::ROOT);
ODIMPathElem odimWHERE(ODIMPathElem::WHERE);
ODIMPathElem odimWHAT(ODIMPathElem::WHAT);
ODIMPathElem odimARRAY(ODIMPathElem::ARRAY);

}  // namespace rack

/*

/// Like {ODIMPathElem::WHAT, "product"}
class ODIMvariable : public std::pair<ODIMPathElem::group_t, std::string> {
public:

	typedef std::pair<ODIMPathElem::group_t, std::string> pair_t;

	// Default constructor
	inline
	ODIMvariable(ODIMPathElem::group_t g=ODIMPathElem::NONE, const std::string & s="") : pair_t(g,s){};

	// Nearby default constructor
	inline
	ODIMvariable(ODIMPathElem::group_t g, const char *s) : pair_t(g,s){};


	// Copy constructor
	inline
	ODIMvariable(const ODIMvariable & v) : pair_t(v){};

	// Nearby copy constructor
	inline
	ODIMvariable(const pair_t & v) : pair_t(v){};

	inline
	ODIMvariable & operator=(const pair_t & v) {
		first = v.first;
		second = v.second;
		return *this;
	};


	inline
	operator std::string() const {
		return ODIMPathElem::getDictionary().getKey(first)+':'+second;
	}

};

ODIMvariable ovar = {ODIMPathElem::WHAT, "prodpar"};
const std::list<ODIMvariable> l = {
		{ODIMPathElem::WHAT, "product"},
		{ODIMPathElem::WHAT, "prodpar"}
		// ODIMvariable({ODIMPathElem::WHAT, "product"}),
		// ODIMvariable({ODIMPathElem::WHAT, "prodpar"})
};

for (const ODIMvariable & odimvar: l){
	if (this->odim.hasKey(odimvar)){
		const drain::Reference & value = this->odim[odimvar];
		dst[dataSetPath][odimvar.first].data.attributes[odimvar.second] = value;
	}
}
*/


