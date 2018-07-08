/*

    Copyright 2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <drain/util/Type.h>

#include "hi5/Hi5.h"
#include "DataSelector.h"

namespace rack {

using namespace hi5;

DataSelector::DataSelector(const std::string & path, const std::string & quantity,
		unsigned int index, unsigned int count,
		double elangleMin, double elangleMax) : BeanLike(__FUNCTION__), elangle(2) {

	//std::cerr << "DataSelector: " << quantity << " => " << this->quantity << std::endl;
	init();
	this->path = path;
	this->quantity = quantity;
	this->index = index;
	this->count = count;
	this->elangle[0] = elangleMin;
	this->elangle[1] = elangleMax;

	//std::cerr << 1 << *this << std::endl;
}


DataSelector::DataSelector(const std::string & parameters) : BeanLike(__FUNCTION__), elangle(2) {

	init();
	setParameters(parameters);

}


DataSelector::DataSelector(const DataSelector & selector) : BeanLike(__FUNCTION__), elangle(2) {
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
	parameters.reference("elangle", elangle);
	//parameters["elangle"].toJSON(std::cout, '\n');
	parameters.reference("elangleMin", elangle[0]);
	parameters.reference("elangleMax", elangle[1]);
}

void DataSelector::reset() {
	path = "";
	quantity = "";
	index = 0;
	count = 1000;
	elangle.resize(2);
	elangle[0] = -90;
	elangle[1] = +90;
}


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
