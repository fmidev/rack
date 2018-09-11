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

#include "ODIM.h"

namespace rack {



void ODIM::copyTo(const std::list<std::string> & keys, HI5TREE & dst) const {

	drain::Logger mout("ODIM", __FUNCTION__);

	for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){
	//for (ReferenceMap::const_iterator it = begin(); it != end(); ++it){

		const std::string & key = *it;

		if (hasKey(key)){

			const drain::Castable & v = (*this)[key];
			const std::type_info & t = v.getType();

			// Mainly debugging
			if (t == typeid(void)){
				mout.warn() << "no type info, skipping key=" << *it << mout.endl;
				continue;
			}
			if (v.getByteSize() == 0){
				mout.warn() << "empty source, skipping key=" << *it << mout.endl;
				continue;
			}

			// Target
			// Split to PATH:ATTRIBUTE
			const size_t i  = key.find(':');
			if (i==std::string::npos){
				throw std::runtime_error(key + ": key contains no semicolon ':' ?");
			}
			const std::string group = key.substr(0,i);  // groupName
			const std::string name  = key.substr(i+1);  // attributeName
			drain::Variable & attribute = dst[group].data.attributes[name];
			//drain::Variable & attribute = dst[key.substr(0,i)].data.attributes[key.substr(i+1)];

			// DEPRECATED? ODIM contains correct types (but for some attributes only?)
			if ((t == typeid(int)) || (t == typeid(long))){
				attribute.setType(typeid(long));
			}
			else if ((t == typeid(float)) || (t == typeid(double))){
				attribute.setType(typeid(double));
			}
			else {
				//std::cerr << "setType: std::string" << std::endl;
				attribute.setType(typeid(std::string));
			}
			//std::cerr << "EncodingODIM::" << __FUNCTION__ << ": " << key << " = " << vField << '\n';
			attribute = v;

			/*
			if (v.getType() == typeid(std::string)){

				mout.note() << "here v:" << mout.endl;
				v.toJSON();
				std::cout << std::endl;

				mout.warn() << "here a:" << mout.endl;
				attribute.toJSON();
				std::cout << std::endl;

			}*/
			//mout << mout.endl;

			// mout.warn() << "writing:" << key << " = " << attribute << mout.endl;
		}
		else {
			mout.note() << "no key: " << key << mout.endl;
		}
		//attribute.toOStr(std::cerr); std::cerr << '\n';
	}

}




const std::string ODIM::dateformat("%Y%m%d");
const std::string ODIM::timeformat("%H%M%S");



void ODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	if (initialize & ROOT){
		reference("what:object", object = "");
		reference("what:version", version = "H5rad 2.2");
		reference("what:date", date = "");
		reference("what:time", time = "");
		reference("what:source", source = "");
		reference("how:ACCnum", ACCnum = 1); // for polar (non-ODIM-standard) and Cartesian
	}

	if (initialize & DATASET){
		reference("what:product", product = "");
		reference("what:prodpar", prodpar = "");
		reference("what:starttime", starttime = "");
		reference("what:startdate", startdate = "");
		reference("what:endtime", endtime = "");
		reference("what:enddate", enddate = "");
		reference("how:NI", NI = 0);
	}


	if (initialize & DATA){
		reference("what:quantity", quantity = "");
	}

}







bool ODIM::getTime(drain::Time & t) const {


	try {
		t.setTime(date, ODIM::dateformat);
		t.setTime(time, ODIM::timeformat);
	} catch (std::exception &e) {
		return false;
	}
	return true;
}

bool ODIM::getStartTime(drain::Time & t) const {

	try {
		t.setTime(startdate, ODIM::dateformat);
		t.setTime(starttime, ODIM::timeformat);
	} catch (std::exception &e) {
		return false;
	}
	return true;
}

bool ODIM::getEndTime(drain::Time & t) const {

	try {
		t.setTime(enddate, ODIM::dateformat);
		t.setTime(endtime, ODIM::timeformat);
	} catch (std::exception &e) {
		return false;
	}
	return true;
}


void ODIM::update(const ODIM & odim){

	if (object.empty())
		object = odim.object;

	if (quantity.empty())
		quantity = odim.quantity;

	// NEW 2017/03
	if (product.empty()){
		product  = odim.product;
		prodpar  = odim.prodpar;
	}

	//if (date.empty() || time.empty()){
	if (date.empty()){
		date = odim.date;
		if (!odim.time.empty())
			time = odim.time;
		else
			time = "000000";
	}

	if (startdate.empty())
		startdate = "999999";

	if (starttime.empty())
		starttime = "999999";

	if (odim.startdate < startdate){
		startdate = odim.startdate;
		starttime = odim.starttime;
	}
	else if ((odim.startdate == startdate) && (odim.starttime < starttime)){
		starttime = odim.starttime;
	}

	if (odim.enddate > enddate){
		enddate = odim.enddate;
		endtime = odim.endtime;
	}
	else if ((odim.enddate == enddate) && (odim.endtime > endtime)){
		endtime = odim.endtime;
	}

	if (source.empty())
		source = odim.source;

	if (NI == 0.0)
		NI = odim.NI;

}


}  // namespace rack



// Rack
