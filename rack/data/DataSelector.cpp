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

#include <drain/util/Type.h>

#include "hi5/Hi5.h"
#include "DataSelector.h"

namespace rack {

using namespace hi5;

DataSelector::DataSelector(const std::string & path, const std::string & quantity,
		unsigned int index, unsigned int count,
		double elangleMin, double elangleMax) : BeanLike(__FUNCTION__) {

	//std::cerr << "DataSelector: " << quantity << " => " << this->quantity << std::endl;

	_init();

	this->path = path;
	this->quantity = quantity;
	this->index = index;
	this->count = count;
	this->elangleMin = elangleMin;
	this->elangleMax = elangleMax;

	//std::cerr << 1 << *this << std::endl;
}


DataSelector::DataSelector(const std::string & parameters) : BeanLike(__FUNCTION__) {
	_init();
	setParameters(parameters);
	//std::cerr << 2 << *this << std::endl;
}


DataSelector::DataSelector(const DataSelector & selector) : BeanLike(__FUNCTION__) {
	_init();
	copy(selector);
}


DataSelector::~DataSelector() {
}

void DataSelector::_init() {
	parameters.reference("path", path);
	parameters.reference("quantity", quantity);
	parameters.reference("index", index = 0);
	parameters.reference("count", count = 1000);
	parameters.reference("elangleMin", elangleMin = -90.0);
	parameters.reference("elangleMax", elangleMax = +90.0);
}

void DataSelector::reset() {
	path = "";
	// pathRegExp.clear();
	quantity = "";
	index = 0;
	count = 1000;
	elangleMin = -90;
	elangleMax =  90;
}

/*
void DataSelector::copy(const DataSelector & selector){
	parameters.importMap(selector.parameters);
}
*/

/*
DataSelector & DataSelector::setParameters(const std::string &parameters, bool allowSpecific){
	parameters.setValues(parameters, allowSpecific ? '=' : '\0');
	return *this;
}
*/


const drain::Variable & DataSelector::getAttribute(const HI5TREE &src, const std::string & path, const std::string & group, const std::string & attributeName){

	static drain::Variable dummy;

	const HI5TREE &g = src(path)[group];

	//if (attributeName == "xscale")
	//	std::cerr << path << '/' << group << ':' << attributeName << '=' << g.data.attributes[attributeName] << std::endl;

	if (g.data.attributes.hasKey(attributeName))
		return g.data.attributes[attributeName];
	else {
		if (path.empty())
			return dummy;
		else {
			size_t i = path.rfind('/');
			//if ((i != path.npos) && (i != 0))
			if (i != path.npos)
				return getAttribute(src, path.substr(0,i), group, attributeName);
			else
				return getAttribute(src, "", group, attributeName);
			//else
			//	return dummy;
		}
	}
}


//void DataSelector::_updateAttributes(HI5TREE & src, const drain::VariableMap & attributes){
void DataSelector::updateAttributes(HI5TREE & src, const drain::VariableMap & attributes){

	//drain::MonitorSource mout(drain::monitor, "DataSelector::updateAttributes");
	drain::MonitorSource mout(__FILE__, __FUNCTION__);


	drain::VariableMap & a = src.data.dataSet.properties;
	a.importMap(attributes); // Copy

	if (src.hasChild("data")){
		const drain::image::Image & img = src["data"].data.dataSet;
		if (img.typeIsSet())
			a["what:type"] = std::string(1u, drain::Type::getTypeChar(img.getType()));
	}


	std::stringstream sstr; // For speed
	for (std::set<std::string>::const_iterator git = EncodingODIM::attributeGroups.begin(); git != EncodingODIM::attributeGroups.end(); ++git){
		if (src.hasChild(*git)){
			const drain::VariableMap  & groupAttributes = src[*git].data.attributes;
			for(drain::VariableMap::const_iterator it = groupAttributes.begin(); it != groupAttributes.end(); it++){
				sstr.str("");
				sstr << *git << ':' << it->first;
				a[sstr.str()] = it->second;
				// if (it->first == "quantity") mout.warn() << "quantity=" << it->second << mout.endl;
			}
		}
	}


	// Traverse children (recursion)
	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it){
		//const std::string & key = it->first;
		//if ((key != "what" ) && (key != "where" ) && (key != "how" ))
		//EncodingODIM::attributeGroups.
		if (EncodingODIM::attributeGroups.find(it->first) == EncodingODIM::attributeGroups.end())
			updateAttributes(it->second, a);
	}

	// std::cerr << "### updateAttributes"
}


/*
bool DataSelector::getQualityPath(const HI5TREE & srcRoot, const std::string & datapath, std::string & qpath) {

}
*/


bool DataSelector::getLastOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & basePath, int & index){

	drain::MonitorSource mout("DataSelector", __FUNCTION__);
	//drain::MonitorSource mout(__FILE__, __FUNCTION__);

	mout.debug(2) << "selector=" << selector << mout.endl;

	std::list<std::string> l;
	getPaths(src, selector, l);

	if (l.empty()){
		//mout.warn() << "No paths with: "  << selector << mout.endl;
		return false;
	}

	//drain::RegExp r("^(.*[^0-9])([0-9]+)([^0-9]*)$");  // 2nd item is the last numeric substd::string
	// skip leading '/'
	drain::RegExp r("^/?([^/].*[^0-9])([0-9]+)([^0-9]*)$");  // 2nd item is the last numeric substd::string
	index = -1;
	drain::Variable v;
	v.setType<int>();
	v = 0;  //
	for (std::list<std::string>::iterator it = l.begin(); it != l.end(); ++it){
		/// std::cerr << "???" << *it << std::endl;
		if (r.execute(*it) != REG_NOMATCH ){
			v = r.result[2];
			if (static_cast<int>(v) > index){
				index = v;
				basePath  = r.result[1];
				/// std::cerr << "xxxx" << *it << std::endl;
			}
		}
	}

	mout.debug(2) << "result: " << basePath << mout.endl;

	return (index != -1);

}

bool DataSelector::getLastOrdinalPath(const HI5TREE &src, const std::string & pathRegExp, std::string & path){

	drain::MonitorSource mout("DataSelector", __FUNCTION__);
	//drain::MonitorSource mout(__FILE__, __FUNCTION__);
	//drain::MonitorSource mout(drain::monitor,"DataSelector::getNextOrdinalPath::filter");

	mout.debug(2) << " selector=" << pathRegExp << mout.endl;

	int index = -1;

	DataSelector::getLastOrdinalPath(src, DataSelector(pathRegExp), path, index);

	if (index == -1)
		return false;
	else {
		std::stringstream sstr;
		sstr << path << index;
		path = sstr.str();
		mout.debug(2) << "result: " << path << mout.endl;
		return true;
	}
}


bool DataSelector::getNextOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & path){

	drain::MonitorSource mout(__FILE__, __FUNCTION__);

	mout.debug(1) << " selector=" << selector << mout.endl;

	int index = -1;

	DataSelector::getLastOrdinalPath(src, selector, path, index);

	if (index == -1)
		return false;
	else {
		std::stringstream sstr;
		sstr << path << ++index;
		path = sstr.str();
		mout.debug(1) << "result: " << path << mout.endl;
		return true;
	}
}


}  // rack::

// Rack
