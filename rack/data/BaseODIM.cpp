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

#include "BaseODIM.h"

namespace rack {

/*
const BaseODIM::group_t BaseODIM::ROOT = 0;
const BaseODIM::group_t BaseODIM::IS_INDEXED = 1;
const BaseODIM::group_t BaseODIM::DATASET = 2 | BaseODIM::IS_INDEXED;
const BaseODIM::group_t BaseODIM::DATA    = 4 | BaseODIM::IS_INDEXED; // or quality
const BaseODIM::group_t BaseODIM::QUALITY = 8 | BaseODIM::DATA | BaseODIM::IS_INDEXED; // or quality
const BaseODIM::group_t BaseODIM::UPDATED = BaseODIM::DATASET|BaseODIM::DATA|BaseODIM::QUALITY; //
const BaseODIM::group_t BaseODIM::ARRAY = 16;
const BaseODIM::group_t BaseODIM::OTHER = 32; // tmp, user defined, etc.
const BaseODIM::group_t BaseODIM::OTHER_INDEXED = (BaseODIM::OTHER | BaseODIM::IS_INDEXED); // tmp, user defined, etc.
const BaseODIM::group_t BaseODIM::ALL = (BaseODIM::ROOT | BaseODIM::DATASET | BaseODIM::DATA | BaseODIM::QUALITY);
//const BaseODIM::group_t BaseODIM::NONE = 128;
*/

const BaseODIM::group_t BaseODIM::NONE;
const BaseODIM::group_t BaseODIM::ROOT;
const BaseODIM::group_t BaseODIM::WHAT;
const BaseODIM::group_t BaseODIM::WHERE;
const BaseODIM::group_t BaseODIM::HOW;
const BaseODIM::group_t BaseODIM::ARRAY;
const BaseODIM::group_t BaseODIM::IS_INDEXED;
const BaseODIM::group_t BaseODIM::DATASET;
const BaseODIM::group_t BaseODIM::DATA;
const BaseODIM::group_t BaseODIM::QUALITY;
const BaseODIM::group_t BaseODIM::IS_QUALITY;
const BaseODIM::group_t BaseODIM::OTHER;
//const BaseODIM::group_t BaseODIM::OTHER_INDEXED;
const BaseODIM::group_t BaseODIM::ALL;

//const std::set<std::string> & ODIM::attributeGroups(createAttributeGroups());

const BaseODIM::dict_t & BaseODIM::getDictionary(){

	static BaseODIM::dict_t map;

	if (map.empty()){
		map[NONE]   = "*";
		map[ROOT]   = "";
		map[DATASET] = "dataset";
		map[DATA]   = "data";
		map[ARRAY]  = "data";
		map[QUALITY] = "quality";
		map[OTHER]  = "OTHER";
		map[WHAT]   = "what";
		map[WHERE]  = "where";
		map[HOW]    = "how";
		//map[OTHER_INDEXED] = "OTHER_INDEXED";
	}
	return map;
}




void ODIMPathElem::set(const std::string &s){

	// std::cout << "setting: " << s << '\n';
	this->group = BaseODIM::NONE;
	this->index = 0;
	bool INDEXED = false; // to separate data and data1
	this->other = ""; //.clear();

	/// Empty string is identified with root
	if (s.empty()){
		this->group = BaseODIM::ROOT;
		//std::cout << "root" << '\n';
		return;
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
	const std::string prefix(s.substr(0, i)); // +1?

	//std::cout << "  raw: " << prefix << ':' << this->index << '\t';

	/// Check if matches predefined group types
	const BaseODIM::dict_t & d = BaseODIM::getDictionary();
	for (BaseODIM::dict_t::const_iterator it=d.begin(); it!=d.end(); ++it){
		// it->first  : group id [enum code]
		// it->second : group prefix [string]
		if ((prefix == it->second) && (INDEXED == BaseODIM::isIndexed(it->first))) {
			this->group = it->first;
			// std::cout << ", code: " << (int)this->group << '\n';
			return;
		}
	}

	//std::cout << "OTHER: " << prefix << (int)IS_INDEXED << '\n';
	this->group = BaseODIM::OTHER;  //(INDEXED) ? BaseODIM::OTHER_INDEXED :
	this->other = prefix;
	//std::cout << ", OTHER: " << (int)this->group << '\n';

	return;

}

const std::string & ODIMPathElem::getKey() const {
	const BaseODIM::dict_t & d = BaseODIM::getDictionary();
	const BaseODIM::dict_t::const_iterator it= d.find(this->group); // should be always found, if group != OTHER
	if (it != d.end()){
		return it->second;
	}
	else {
		static std::string empty("error");
		return empty;
	}
}

const std::string & ODIMPathElem::getPrefix() const {

	//if ((this->group & BaseODIM::OTHER) == 0){
	if (this->group != BaseODIM::OTHER){
		return getKey();
	}
	else {
		return this->other;
	}

}

char ODIMPathElem::getCharCode() const {

	const std::string & prefix = getPrefix();

	if (prefix.empty())
		return 'E';

	if (group == BaseODIM::DATASET){
		return 'D';
	}
	else
		return prefix.at(0);
	/*
	if (BaseODIM::isIndexed(group))
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
		if (BaseODIM::isIndexed(e1.group))
			return (e1.getIndex() == e2.getIndex());
		else if (e1.group == BaseODIM::OTHER)
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
		if (BaseODIM::isIndexed(e1.group)){
			return (e1.getIndex() < e2.getIndex());
		}
		else if (e1.group == BaseODIM::OTHER){
			return (e1.getPrefix() < e2.getPrefix());
			//strcmp(e1.getPrefix().c_str(), e2.getPrefix().c_str());
		}
		else
			return false; // equal?

	}
}


}  // namespace rack



// Rack
