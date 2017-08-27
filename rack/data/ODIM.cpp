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

#include <drain/util/Debug.h>

#include "ODIM.h"

namespace rack {


const std::string & SourceODIM::getSourceCode() const {

	//sourceMap.setValues(source, ':');  // 2nd par: equal-sign

    #define RETURN_ODIM_SRC(s) if (!s.empty()) return s
	RETURN_ODIM_SRC(NOD); // TODO: add options for desired order
	RETURN_ODIM_SRC(RAD);
	RETURN_ODIM_SRC(WMO);
	RETURN_ODIM_SRC(ORG);
	RETURN_ODIM_SRC(CTY);
	RETURN_ODIM_SRC(PLC);

	static std::string empty;
	return empty;
}


void SourceODIM::init(){
	// Considered prefix "where:source", but gets complicated for default constructor.  (?)
	reference("WMO", WMO);
	reference("RAD", RAD);
	reference("NOD", NOD);
	reference("PLC", PLC);
	reference("ORG", ORG);
	reference("CTY", CTY);
	reference("CMT", CMT);
}

/// Tries to resolve NOD code from partial or deprecated metadata
/**
 *  Assigns NOD if empty, and CMT
 */
void SourceODIM::setNOD(){

	// drain::MonitorSource mout("SourceODIM", __FUNCTION__);
	if (NOD.empty()){
		switch (get("WMO", 0)){
		case 26422:
			NOD = "lvrix";
			break;
		default:
			static drain::RegExp nodRegExp("^[a-z]{5}$");
			if (nodRegExp.test(CMT)){
				NOD = CMT;
			}
			else {
				drain::MonitorSource mout("SourceODIM", __FUNCTION__);
				//NOD = getSourceCode();
				//mout.info() << "Site code 'NOD' not found, substituting with '" << NOD << "'" << mout.endl;
				mout.info() << "Site code 'NOD' not found, using '" << getSourceCode() << "' as node indicator " << mout.endl;
			}
		}
	}

}


const std::set<std::string> & EncodingODIM::attributeGroups(createAttributeGroups());

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

	char typecode = type.at(0);

	if (isnan(offset))
		offset = -gain;

	return setScaling(gain, offset, drain::Type::getMin<double>(typecode), drain::Type::getMax<double>(typecode));

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


void EncodingODIM::init(){

	//std::set<std::string> & rootAttributes = getRootAttributes();
	//std::set<std::string> & datasetAttributes = getDatasetAttributes();
	std::set<std::string> & dataAttributes = getDataAttributes();

	declare(dataAttributes, "what:type", type = "C");

	declare(dataAttributes, "what:gain",   gain = 0.0);
	declare(dataAttributes, "what:offset", offset = 0.0);
	declare(dataAttributes, "what:undetect", undetect = 0.0);
	declare(dataAttributes, "what:nodata", nodata = 0.0);

}

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
	//declareMap::set(data.properties);  FAILS! double-double cast goes via sstream, dropping precision!?!
	const drain::VariableMap & m = data.properties;
	for (drain::VariableMap::const_iterator it = m.begin(); it != m.end(); ++it){

		const iterator oit = find(it->first);

		if (oit != end()){
			drain::Castable & myValue = oit->second;
			const drain::Variable & value = it->second;
			const std::type_info & t = value.getType();
			// std::cerr << key << " type=" << t.name() << '\n';
			if (t == typeid(double)){
				//(*this)[key] = (double)value;
				myValue = static_cast<double>(value);
				//std::cerr << "DOUBLE" << key << "\n\t";
				/*
					std::cerr.precision(20);
					std::cerr << (double)value << "\n\t";
					std::cerr << undetect << "\n\t";
					//std::cerr << (const double &)value << std::endl;
					std::cerr << (const double &)(*this)[key] << std::endl;
				 */
			}
			else if (t == typeid(float))
				myValue = static_cast<float>(value);
			else
				myValue = value;
		}

	}
	type = std::string("") + drain::Type::getTypeChar(data.getType());
}


void EncodingODIM::setRange(double min, double max) {

		if (type.empty())
			type = "C";

		const drain::Type t(type.at(0));

		const double minData = drain::Type::getMin<double>(t);
		const double maxData = drain::Type::getMax<double>(t);

		undetect = minData;
		nodata   = maxData;

		if (drain::Type::isIntegralType(t)){
			gain = (max-min) / static_cast<double>((maxData-1) - (minData+1));
			offset = min - gain*(minData+1);
			//drain::MonitorSource mout("QuantityMap", __FUNCTION__);
		}
		else {
			gain = 1.0;
			offset = 0.0;
		}

		//if (!defaultType)			defaultType = typecode;

}



void EncodingODIM::copyTo(const std::set<std::string> & keys, HI5TREE & dst) const {

	drain::MonitorSource mout("BaseODIM", __FUNCTION__);

	for (std::set<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){
	//for (ReferenceMap::const_iterator it = begin(); it != end(); ++it){

		const std::string & key = *it;

		if (hasKey(key)){

			const size_t i  = key.find(':');

			/// Determine if it is ATTRIBUTE at root or PATH:ATTRIBUTE
			//drain::Data & attribute = (i==std::string::npos) ? dst.data.attributes[key] : dst[key.substr(0,i)].data.attributes[key.substr(i+1)];
			//drain::Variable & attribute = (i==std::string::npos) ? dst.data.attributes[key] : dst[key.substr(0,i)].data.attributes[key.substr(i+1)];
			if (i==std::string::npos){
				throw std::runtime_error(key + ": key contains no semicolon ':' ?");
			}


			drain::Variable & attribute = dst[key.substr(0,i)].data.attributes[key.substr(i+1)];

			const drain::Castable & v = (*this)[key];
			const std::type_info & t = v.getType();

			if (t == typeid(void)){
				mout.warn() << "no type info, skipping key=" << *it << mout.endl;
				continue;
			}

			if (v.getByteSize() == 0){
				mout.warn() << "empty source, skipping key=" << *it << mout.endl;
				continue;
			}


			if ((t == typeid(int)) || (t == typeid(long))){
				attribute.setType<long>();
			}
			else if ((t == typeid(float)) || (t == typeid(double))){
				attribute.setType<double>();
			}
			else {

				attribute.setType<std::string>();
			}

			//std::cerr << "EncodingODIM::" << __FUNCTION__ << ": " << key << " = " << v << '\n';

			attribute = v;

		}

		//attribute.info(std::cerr); std::cerr << '\n';
	}

}

void EncodingODIM::checkType(HI5TREE & dst, EncodingODIM & odim){


	for (std::map<std::string,drain::Castable>::iterator it = odim.begin(); it != odim.end(); ++it){

		const std::string & key = it->first;
		const size_t i  = key.find(':');

		/// Determine if it is ATTRIBUTE at root or PATH:ATTRIBUTE
		//drain::Variable & attribute = (i==std::string::npos) ? dst.data.attributes[key] : dst[key.substr(0,i)].data.attributes[key.substr(i+1)];
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


void ODIM::init(){

	std::set<std::string> & rootAttributes =    getRootAttributes();
	std::set<std::string> & datasetAttributes = getDatasetAttributes();
	std::set<std::string> & dataAttributes =    getDataAttributes();


	declare(dataAttributes, "what:quantity", quantity = "");

	declare(datasetAttributes, "what:product", product = "");
	declare(datasetAttributes, "what:prodpar", prodpar = "");

	declare(rootAttributes, "what:object", object = "");
	declare(rootAttributes, "what:version", version = "H5rad 2.2");
	declare(rootAttributes, "what:date", date = "");
	declare(rootAttributes, "what:time", time = "");
	declare(rootAttributes, "what:source", source = "");

	declare(datasetAttributes, "what:starttime", starttime = "");
	declare(datasetAttributes, "what:startdate", startdate = "");
	declare(datasetAttributes, "what:endtime", endtime = "");
	declare(datasetAttributes, "what:enddate", enddate = "");

	declare(datasetAttributes, "how:NI", NI = 0);

}


bool ODIM::getTime(drain::Time & t) const {

	try {
		t.setTime(date, "%Y%m%d");
		t.setTime(time, "%H%M%S");
	} catch (std::exception &e) {
		return false;
	}
	return true;
	/*
	if (!date.empty()){
		t.setTime(date, "%Y%m%d");
		if (time.empty())
			t.setTime(time, "%H%M%S");
		return true;
	}
		return false;
	*/

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



void PolarODIM::init(){


	object = "PVOL";

	std::set<std::string> & rootAttributes    = getRootAttributes();
	std::set<std::string> & datasetAttributes = getDatasetAttributes();
	//std::set<std::string> & dataAttributes = getDataAttributes();

	// new
	declare(datasetAttributes, "where:nbins",  nbins = 0L);
	declare(datasetAttributes, "where:nrays",  nrays = 0L);
	declare(datasetAttributes, "where:rscale", rscale = 0.0);


	declare(rootAttributes, "where:lon", lon = 0.0);
	declare(rootAttributes, "where:lat", lat = 0.0);
	declare(rootAttributes, "where:height", height = 0.0);

	declare(datasetAttributes, "where:elangle", elangle = 0.0);

	declare(datasetAttributes, "where:rstart", rstart = 0.0);
	declare(datasetAttributes, "where:a1gate", a1gate = 0L);

	declare(datasetAttributes, "where:startaz", startaz = 0.0);
	declare(datasetAttributes, "where:stopaz",   stopaz = 0.0);



	// declare(datasetAttributes, "how:NI", NI = 0.0);
	declare(datasetAttributes, "how:wavelength", wavelength = 0.0);
	declare(datasetAttributes, "how:highprf", highprf = 0.0);
	declare(datasetAttributes, "how:lowprf", lowprf = 0.0);

	declare(rootAttributes, "how:freeze", freeze = 10.0);

}



void CartesianODIM::init(){

	object = "COMP";

	std::set<std::string> & rootAttributes = getRootAttributes();
	//std::set<std::string> & datasetAttributes = getDatasetAttributes();

	declare(rootAttributes, "where:projdef", projdef = "");
	declare(rootAttributes, "where:xsize", xsize = 0L);
	declare(rootAttributes, "where:ysize", ysize = 0L);
	declare(rootAttributes, "where:xscale", xscale = 0.0);
	declare(rootAttributes, "where:yscale", yscale = 0.0);
	//
	declare(rootAttributes, "where:UR_lon", UR_lon = 0.0);
	declare(rootAttributes, "where:UR_lat", UR_lat = 0.0);
	declare(rootAttributes, "where:UL_lon", UL_lon = 0.0);
	declare(rootAttributes, "where:UL_lat", UL_lat = 0.0);
	declare(rootAttributes, "where:LR_lon", LR_lon = 0.0);
	declare(rootAttributes, "where:LR_lat", LR_lat = 0.0);
	declare(rootAttributes, "where:LL_lon", LL_lon = 0.0);
	declare(rootAttributes, "where:LL_lat", LL_lat = 0.0);
	declare(rootAttributes, "how:camethod", camethod = "");
	declare(rootAttributes, "how:nodes", nodes = "");
	// declare(rootAttributes, "how:NI", NI = 0.0);

}

}  // namespace rack



// Rack
