/*


    Copyright 2011-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include <drain/util/Log.h>

#include "ODIM.h"

namespace rack {



const std::set<std::string> & EncodingODIM::attributeGroups(createAttributeGroups());

/*
EncodingODIM::EncodingODIM(group_t initialize = ALL){
	init(initialize);
};

EncodingODIM::EncodingODIM(const EncodingODIM & odim){
	init(ALL);
	updateFromMap(odim); // importMap can NOT be used because non-EncodingODIM arg will have a larger map
};
*/


//static
const std::set<std::string> & EncodingODIM::createAttributeGroups(){

	static std::set<std::string> s;
	s.insert("what");
	s.insert("where");
	s.insert("how");
	return s;

}

EncodingODIM & EncodingODIM::setScaling(double gain, double offset){

	if (type.empty())
		type = "C";

	//char typecode = type.at(0);

	if (isnan(offset))
		offset = -gain;

	const std::type_info & t = drain::Type::getTypeInfo(type.at(0));

	return setScaling(gain, offset, drain::Type::call<drain::typeMin, double>(t), drain::Type::call<drain::typeMax, double>(t));
	//return setScaling(gain, offset, drain::Type::getMin<double>(typecode), drain::Type::call<drain::typeMax,double>(typecode));

}

EncodingODIM & EncodingODIM::setScaling(double gain, double offset, double undetect, double nodata) {

	if (type.empty())
		type = "C";

	this->gain = gain;
	this->offset = offset;
	this->undetect = undetect;
	this->nodata = nodata;

	return *this;
}

void EncodingODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	if (initialize & ROOT){
	}

	if (initialize & DATASET){
	}

	if (initialize & DATA){
		reference("what:type", type = "C");
		reference("what:gain",   gain = 0.0);
		reference("what:offset", offset = 0.0);
		reference("what:undetect", undetect = 0.0);
		reference("what:nodata", nodata = 0.0);
	}

}




/*
template <class F>
void declare(std::set<std::string> & keys, const std::string & key, F &x){
	keys.insert(key);
	reference(key, x);
}
*/


void EncodingODIM::clear(){
  for (ReferenceMap::iterator it = begin(); it != end(); ++it)
	  //it->second = 0;
	  it->second.clear();
}

void EncodingODIM::update(const EncodingODIM & odim){

	if (type.empty())
		type = odim.type;

	if (gain == 0.0){
		gain   = odim.gain;
		offset = odim.offset;
		nodata = odim.nodata;
		undetect = odim.undetect;
	}

}

void EncodingODIM::addShortKeys(drain::ReferenceMap & ref) {
	const EncodingODIM::keylist_t & keys = getKeyList();

	//for (EncodingODIM::iterator it = begin(); it != end(); ++it){
	for (EncodingODIM::keylist_t::const_iterator kit = keys.begin(); kit != keys.end(); ++kit){
		//const std::string & key = it->first;
		const size_t i = kit->find(':');  // type?
		if (i != std::string::npos){
			ref.reference(kit->substr(i+1), operator[](*kit));
			//alias(key.substr(i+1), key);
		}
	}
}

void EncodingODIM::copyFrom(const drain::image::Image & data){

	const drain::VariableMap & m = data.properties;

	for (drain::VariableMap::const_iterator it = m.begin(); it != m.end(); ++it){

		const iterator oit = find(it->first);
		if (oit != end()){
			const drain::Variable & srcValue = it->second;
			const std::type_info & t = srcValue.getType();
			drain::Castable & dstValue = oit->second;
			// std::cerr << key << " type=" << t.name() << '\n';
			if (t == typeid(double)){
				//(*this)[key] = (double)srcValue;
				dstValue = static_cast<double>(srcValue);
				//std::cerr << "DOUBLE" << key << "\n\t";
				/*
					std::cerr.precision(20);
					std::cerr << (double)srcValue << "\n\t";
					std::cerr << undetect << "\n\t";
					//std::cerr << (const double &)srcValue << std::endl;
					std::cerr << (const double &)(*this)[key] << std::endl;
				 */
			}
			else if (t == typeid(float))
				dstValue = static_cast<float>(srcValue);
			else
				dstValue = srcValue;
		}

	}
	//type = std::string("") + drain::Type::getTypeChar(data.getType());
	type = drain::Type::getTypeChar(data.getType());
}


void EncodingODIM::setRange(double min, double max) {

		if (type.empty())
			type = "C";

		const drain::Type t(type.at(0));

		const double minData = drain::Type::call<drain::typeMin, double>(t); // getMin<double>(type);
		const double maxData = drain::Type::call<drain::typeMax, double>(t); // drain::Type::call<drain::typeMax,double>(t);

		undetect = minData;
		nodata   = maxData;

		if (drain::Type::call<drain::typeIsInteger>(t)){
			gain = (max-min) / static_cast<double>((maxData-1) - (minData+1));
			offset = min - gain*(minData+1);
			//drain::Logger mout("QuantityMap", __FUNCTION__);
		}
		else {
			gain = 1.0;
			offset = 0.0;
		}

		//if (!defaultType)			defaultType = typecode;

}

void EncodingODIM::checkType(HI5TREE & dst, EncodingODIM & odim){


	for (std::map<std::string,drain::Referencer>::iterator it = odim.begin(); it != odim.end(); ++it){

		const std::string & key = it->first;
		const size_t i  = key.find(':');

		/// Determine if it is ATTRIBUTE at root or PATH:ATTRIBUTE
		//  drain::Variable & attribute = (i==std::string::npos) ? dst.data.attributes[key] : dst[key.substr(0,i)].data.attributes[key.substr(i+1)];
		if (i==std::string::npos){
			std::cerr << "_checkType no ODIM group? " << key << '\n';
		}
		else {

			HI5TREE & dstGroup = dst[key.substr(0,i)];

			std::cerr << "checkType: " << key << '\n';

			std::map<std::string,drain::Variable>::iterator ait = dstGroup.data.attributes.find(key.substr(i+1));
			if (ait == dstGroup.data.attributes.end())
				continue;

			std::cerr << "  found: " << ait->first << '\n';

			//drain::Variable & attribute = dst[key.substr(0,i)].data.attributes[key.substr(i+1)];
			const std::type_info & t = it->second.getType();
			if (ait->second.getType() != t){
				it->second = ait->second;
				ait->second.setType(t);
				ait->second = it->second; // DOES NOT WORK!
			}
		}
	}

	/*
	for (HI5TREE::iterator it = dst.begin(); it != dst.end(); ++it) {
		//checkType(it->second, odim); // TODO skip /what /where?
	}
	*/

}


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




double ODIM::getNyquist() const {

	if (NI != 0.0){
		return NI;
	}
	else {
		drain::Logger mout("ODIM", __FUNCTION__);
		const std::type_info & t = drain::Type::getTypeInfo(type);
		if (drain::Type::call<drain::typeIsSmallInt>(t)){
			const double vMax = drain::Type::call<drain::typeMax, double>(t);
			const double vMin = drain::Type::call<drain::typeMin, double>(t);
			mout.info() << "no NI in metadata, guessing speed range [" << vMin << ',' << vMax << "]" << mout.endl;
			return scaleForward(vMax);
		}
		else {
			mout.warn() << " could not derive Nyquist speed (NI)" << mout.endl;
			return 0.0;
		}
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


