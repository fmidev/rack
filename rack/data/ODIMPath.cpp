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

#include <drain/util/Log.h>

#include "ODIMPath.h"

namespace rack {


const ODIMPathElem::group_t ODIMPathElem::NONE;
const ODIMPathElem::group_t ODIMPathElem::ROOT;
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
const ODIMPathElem::group_t ODIMPathElem::ALL_LEVELS; // ?


/*
const ODIMPathElem::flag_t & ODIMPathElem::getFlags(){

	static ODIMPathElem::flag_t flags(dict);

	return flags;

}
*/


const ODIMPathElem::dict_t & ODIMPathElem::getDictionary(){

	static ODIMPathElem::dict_t dict;

	if (dict.empty()){
		dict.add("*", NONE);
		dict.add("", ROOT);
		dict.add("dataset", DATASET);

		// NOTE: when searching for "data", DATA will be found (first) and returned
		dict.add("data", DATA);
		// NOTE: when searching for ARRAY, "data" will be found (first) and returned
		dict.add("data", ARRAY);
		dict.add("array", ARRAY);
		//dict.first["array"] = ARRAY;
		//dict.second[ARRAY] = "data";

		//dict.add("data", ARRAY);
		dict.add("quality", QUALITY);
		dict.add("OTHER", OTHER);
		dict.add("what", WHAT);
		dict.add("where", WHERE);
		dict.add("how", HOW);
		dict.add("palette", PALETTE);
		dict.add("legend", LEGEND);
	}

	return dict;

}




bool ODIMPathElem::set(const std::string &s){

	drain::Logger mout(__FUNCTION__, __FILE__);

	this->group = ROOT; // or none?
	this->index = 0;
	bool INDEXED = false; // to separate data and data1
	this->str = "";

	/// Empty string is identified with root (rethink?)
	if (s.empty()){
		this->group = ROOT;
		//std::cout << "root" << '\n';
		return true;
	}
	else if (s == "data"){
		this->group = ARRAY; // ODIM
		return true;
	}

	// Extract prefix (alphabets) and index (digits)
	size_t i = 0;
	while(i<s.length()){
		if ((s.at(i)>='0') && (s.at(i)<='9')){
			std::stringstream sstr(s.substr(i));
			sstr >> this->index;
			INDEXED = true;
			break;
		}
		++i;
	}
	/// The non-numeric prefix
	const std::string prefix(s.substr(0, i));

	//std::cout << "  raw: " << prefix << ':' << this->index << '\t';

	/// Check if matches predefined group types
	const dict_t & d = getDictionary();
	for (dict_t::const_iterator it=d.begin(); it!=d.end(); ++it){
		// it->first  : group id [enum code]
		// it->second : group prefix [string]
		if ((prefix == it->first) && (INDEXED == isIndexed(it->second))) {
			this->group = it->second;
			// std::cout << ", code: " << (int)this->group << '\n';
			return true;
		}
	}

	//std::cout << "OTHER: " << prefix << (int)IS_INDEXED << '\n';
	this->group = OTHER;  //(INDEXED) ? ODIMPathElem::OTHER_INDEXED :
	//this->str = prefix;
	this->str = s;

	mout.note() << "non-standard path element: " << *this << mout.endl;

	return false;

}

const std::string & ODIMPathElem::getKey(group_t group)  {

	static const dict_t & d = getDictionary();

	const dict_t::const_iterator it = d.findByValue(group);
	//const dict_t::const_iterator it = d.find(group); // should be always found, if group != OTHER
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
		return this->str;
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
	/*
	if (ODIMPath::isIndexed(group))
		return drain::StringTools::upperCase(prefix.at(0));
	else
		return prefix.at(0);
	*/
}

bool operator==(const ODIMPathElem & e1, const ODIMPathElem & e2){

	if (e1.group != e2.group){
		return false;
	}
	else { // same group
		//if (ODIMPath::isIndexed(e1.group))
		if (e1.isIndexed())
			return (e1.getIndex() == e2.getIndex());
		else if (e1.group == ODIMPathElem::OTHER)
			return (e1.getPrefix() == e2.getPrefix());
		else
			return false;
	}
}



bool operator<(const ODIMPathElem & e1, const ODIMPathElem & e2){

	if (e1.group != e2.group){
		return (e1.group < e2.group);
	}
	else { //if (e1.group == e2.group){
		if (e1.isIndexed()){
			return (e1.getIndex() < e2.getIndex());
		}
		else if (e1.group == ODIMPathElem::OTHER){
			return (e1.getPrefix() < e2.getPrefix());
			//strcmp(e1.getPrefix().c_str(), e2.getPrefix().c_str());
		}
		else
			return false; // equal?

	}
}

// Experimental naming.
ODIMPathElem odimROOT(ODIMPathElem::ROOT);
ODIMPathElem odimWHERE(ODIMPathElem::WHERE);
ODIMPathElem odimWHAT(ODIMPathElem::WHAT);
ODIMPathElem odimARRAY(ODIMPathElem::ARRAY);

}  // namespace rack



