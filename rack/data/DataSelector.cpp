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
#include <drain/util/RegExp.h>

#include "hi5/Hi5.h"
#include "DataSelector.h"


namespace rack {

using namespace hi5;

DataSelector::DataSelector(const std::string & path, const std::string & quantity,
		unsigned int index, unsigned int count,
		double elangleMin, double elangleMax) : BeanLike(__FUNCTION__) { //, elangle(2) {

	//std::cerr << "DataSelector: " << quantity << " => " << this->quantity << std::endl;
	init();
	this->path = path;
	this->quantity = quantity;
	this->index = index;
	this->count = count;
	this->elangle.min = elangleMin;
	this->elangle.max = elangleMax;

	//std::cerr << 1 << *this << std::endl;
}


DataSelector::DataSelector(const std::string & parameters) : BeanLike(__FUNCTION__){ //, elangle(2) {

	init();
	setParameters(parameters);

}


DataSelector::DataSelector(const DataSelector & selector) : BeanLike(__FUNCTION__){ //, elangle(2) {
	init();
	copy(selector);
}


DataSelector::~DataSelector() {
}

void DataSelector::init() {

	reset();

	parameters.reference("path", path);
	parameters.reference("quantity", quantity);
	parameters.reference("index", index);
	parameters.reference("count", count);

	parameters.reference("elangle", elangle.vect);

	parameters.reference("quantity", quantity);
	// quantity.insert("");
	// reset(){...} ?

	parameters.reference("dataset", dataset.vect);
	parameters["dataset"].fillArray = true;

	parameters.reference("data", data.vect);
	parameters["data"].fillArray = true;

	// Deprecating, use "elangle=min:max" instead
	parameters.reference("elangleMin", elangle.min);
	parameters.reference("elangleMax", elangle.max);
	/*
	 * OLD
	parameters.reference("elangle", elangle);
	parameters.reference("elangleMin", elangle[0]);
	parameters.reference("elangleMax", elangle[1]);
	*/

}

void DataSelector::reset() {

	path = "";
	quantity = "";
	index = 0;
	count = 1000;

	// New
	elangle.min = -90.0;
	elangle.max = +90.0;
	// Old
	//elangle.resize(2);
	//elangle[0] = -90;
	//elangle[1] = +90;

	dataset.min = 0;
	dataset.max = 0xffff;
	data.min = 0;
	data.max = 0xffff;


}

bool DataSelector::isValidPath(const ODIMPath & path) const {

	for (ODIMPath::const_iterator it = path.begin(); it != path.end(); ++it){
		switch (it->group) {
			case ODIM::DATASET:
				if (!dataset.isInside(it->index))
					return false;
				break;
			case ODIM::DATA:
				if (!data.isInside(it->index))
					return false;
				break;
			default:
				break;
		}
	}

	return true;

}

void DataSelector::updatePaths(){

	drain::Logger mout(getName(), __FUNCTION__);

	static drain::RegExp rangeReg("^([+-]?[[:digit:]]+)(:([+-]?[[:digit:]]+))?$");


	static drain::RegExp datasetRE("/?dataset([^/]+)(/.*)?$");

	if (datasetRE.execute(path)==0){
		const std::string & datasetIndex = datasetRE.result[1];
		if (rangeReg.test(datasetIndex)){
			setParameter("dataset", datasetIndex);
		}
		else {
			mout.note() << "deprecating form: " << datasetIndex << mout.endl;
			mout.warn() << "'path' (with regexps) is deprecating, consider explicit parameter 'dataset'" << mout.endl;
		}
	}

	static drain::RegExp dataRE("/?data([^set/]+)(/.*)?$");

	if (dataRE.execute(path)==0){
		const std::string & dataIndex = dataRE.result[1];
		if (rangeReg.test(dataIndex)){
			setParameter("data", dataIndex);
		}
		else {
			mout.note() << "deprecating form: " << dataIndex << mout.endl;
			mout.warn() << "'path' (with regexps) is deprecating, consider explicit parameter 'data'" << mout.endl;
		}
	}


}

bool DataSelector::isValidData(const drain::ReferenceMap & properties) const {

	if (properties.hasKey("where:elangle"))
		if (!elangle.isInside(properties["where:elangle"]))
			return false;

	/*
	if (properties.hasKey("what:quantity")){
		drain::StringTools::split(quantity, quantitySet, ':');
		if (quantitySet.find(properties["what:quantity"]) == quantitySet.end())
			return false;
	}
	*/

	return true;

}


// ------ OLD ------------------
bool DataSelector::getLastOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & basePath, int & index){

	drain::Logger mout("DataSelector", __FUNCTION__);
	//drain::Logger mout(__FILE__, __FUNCTION__);

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
	drain::Variable v(0);
	//vField.setType<int>();
	//vField = 0;  //
	for (std::list<std::string>::iterator it = l.begin(); it != l.end(); ++it){
		/// std::cerr << "???" << *it << std::endl;
		if (r.execute(*it) != REG_NOMATCH ){
			mout.debug(2) << r.result[1] << '|' << r.result[2] << mout.endl;
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

	drain::Logger mout("DataSelector", __FUNCTION__);
	//drain::Logger mout(__FILE__, __FUNCTION__);
	//drain::Logger mout(drain::monitor,"DataSelector::getNextOrdinalPath::filter");

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

	drain::Logger mout(__FILE__, __FUNCTION__);

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
