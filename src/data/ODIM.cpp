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
#include <drain/util/Flags.h>

#include "ODIM.h"

template <>
const drain::EnumDict<rack::ODIM::Version>::dict_t drain::EnumDict<rack::ODIM::Version>::dict = {
		{"2.2", rack::ODIM::ODIM_2_2 },
		{"2.3", rack::ODIM::ODIM_2_3 },
		{"2.4", rack::ODIM::ODIM_2_4 },
		{"KILOMETRES", rack::ODIM::KILOMETRES },
};


namespace rack {

ODIM::VersionFlagger ODIM::versionFlagger(rack::ODIM::ODIM_2_4, rack::ODIM::RACK_EXTENSIONS);


const std::string ODIM::dateformat("%Y%m%d");
const std::string ODIM::timeformat("%H%M%S");

const ODIM::nameSet ODIM::timeKeys = {"what:time", "what:starttime", "what:endtime"}; // , "time", "starttime", "endtime"
const ODIM::nameSet ODIM::dateKeys = {"what:date", "what:startdate", "what:enddate"};
const ODIM::nameSet ODIM::locationKeys = {"where:site", "where:src", "where:lat", "where:lon", "PLC", "NOD", "WMO"}; // , "site", "src", "lat", "lon"


void ODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	ACCnum = 0;

	// TODO: consider attribs under ODIMPathElem::WHAT, ODIMPathElem::WHERE, ODIMPathElem::HOW ?
	if (initialize & ODIMPathElem::ROOT){
		link("what:object", object = "");
		link("what:version", version = "H5rad 2.2");
		link("what:date", date = "");
		link("what:time", time = "");
		link("what:source", source = "");
		link("how:ACCnum", ACCnum = 1); // for polar (non-ODIM-standard) and Cartesian
	}

	if (initialize & ODIMPathElem::DATASET){
		link("what:product", product = "");
		link("what:prodpar", prodpar = "");
		link("what:starttime", starttime = "");
		link("what:startdate", startdate = "");
		link("what:endtime", endtime = "");
		link("what:enddate", enddate = "");
		link("how:NI", NI = 0);

		// Almost-ODIM: in ODIM, this is for Cartesians only
		// angles.reserve(128); // not enough for RHI...
		// NOTE: linking std::vector has been removed, as ptr target (address of 1st elem) to may be changed
		// link("how:angles", angles);

		// Non-ODIM:
		link("how:resolution", resolution.tuple());
	}


	if (initialize & ODIMPathElem::DATA){
		link("what:quantity", quantity = "");
	}

}

bool ODIM::distinguishNodata(const std::string & quantityPrefix){

	// Accept anything starting with quantity. So, if 'VRAD', hence 'VRADH' or 'VRADV'.
	//if (quantity.empty() || (this->quantity.find(quantityPrefix)==0)){  // Fix Vaisala IRIS bug
	if (this->quantity.find(quantityPrefix)==0){  // Fix Vaisala IRIS bug
		//std::cerr << "setNodata" << quantity << '\n';
		if (nodata == undetect){
			drain::Logger mout(__FILE__, __FUNCTION__);
			nodata = drain::Type::call<drain::typeMax,double>(type);
			mout.special("setting [", quantity ,"] nodata=", nodata, " to distinguish undetect=", undetect);
			return true;
		}
		//std::cerr << "nodata: " << nodata << '\n';
	}
	return false;
}



void ODIM::copyTo(const std::list<std::string> & keys, Hi5Tree & dst) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	//for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){
	//for (ReferenceMap::const_iterator it = begin(); it != end(); ++it){
	for (const std::string & key: keys){

		// const std::string & key = *it;

		if (hasKey(key)){

			const drain::Castable & v = (*this)[key];
			const std::type_info & t = v.getType();

			// Mainly debugging
			if (t == typeid(void)){
				mout.warn("no type info, skipping key=", key);
				continue;
			}
			if (v.getElementSize() == 0){
				mout.warn("empty source, skipping key=", key);
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
			/*
			if ((t == typeid(int)) || (t == typeid(long))){
				attribute.setType(typeid(long));
			}
			else if ((t == typeid(float)) || (t == typeid(double))){
				attribute.setType(typeid(double));
			}
			*/
			//if (drain::Type::call<drain::typeIsScalar>(t)){
			if (drain::Type::call<drain::typeIsInteger>(t))
				attribute.setType(typeid(long));
			else if (drain::Type::call<drain::typeIsFloat>(t))
				attribute.setType(typeid(double));
			else {
				//std::cerr << "setType: std::string" << std::endl;
				attribute.setType(typeid(std::string));
			}
			//std::cerr << "EncodingODIM::" << __FUNCTION__ << ": " << key << " = " << vField << '\n';
			attribute = v;

			/*
			if (v.getType() == typeid(std::string)){

				mout.note("here v:" );
				v.toJSON();
				std::cout << std::endl;

				mout.warn("here a:" );
				attribute.toJSON();
				std::cout << std::endl;

			}*/
			//mout << mout.endl;

			// mout.warn("writing:" , key , " = " , attribute );
		}
		else {
			mout.note("no key: " , key );
		}
		//attribute.toOStr(std::cerr); std::cerr << '\n';
	}

}







bool ODIM::getTime(drain::Time & t, const std::string &dateStr, const std::string &timeStr){ //  const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	try {
		if (!dateStr.empty())
			t.setTime(dateStr, ODIM::dateformat);
		else
			mout.note("empty date string");

		if (!timeStr.empty())
			t.setTime(timeStr, ODIM::timeformat);
		else
			mout.note("empty time string");

	}
	catch (const std::exception &e) {
		// drain::Logger mout(__FILE__, __FUNCTION__);
		mout.fail(e.what());
		return false;
	}

	return true;
}




bool ODIM::setTime(const drain::Time & t){

	try {
		date = t.str(ODIM::dateformat);
		time = t.str(ODIM::timeformat);
	}
	catch (const std::exception &e) {
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.warn(e.what() );
		return false;
	}

	return true;

}

bool ODIM::setTime(const std::string & s){

	drain::Logger mout(__FILE__, __FUNCTION__);

	const size_t nDate = 8; // "YYYYmmdd"
	const size_t nTime = 6; // "HHMMSS"

	const size_t n = s.size();

	if (n < nDate){ // short...
		date = s;
		date.append(nDate - n, '0'); // pad
		time = "000000";
		mout.warn("suspiciously short date: ", s, " => ", date);
		return false;
	}
	else { // default case
		date = s.substr(0, nDate);
		if ((n-nDate) < nTime){ // less than 6 time digits
			time = s.substr(nDate);
			time.append(nTime - (n-nDate), '0'); // pad
		}
		else {
			time = s.substr(nDate, nTime);
			// warn microsecs?
		}
	}

	return n >= (nDate+nTime);

}

void ODIM::adjustGeometry(size_t cols, size_t rows){

	drain::Logger mout(__FILE__, __FUNCTION__);
	const drain::image::AreaGeometry a(area);
	setGeometry(cols, rows);
	mout.note(a, " => ", area);
	mout.unimplemented("resolution adjustment");
}


void ODIM::updateLenient(const ODIM & odim){

	drain::Logger mout(__FILE__, __FUNCTION__);


	if (object.empty())
		object = odim.object;

	if (quantity.empty())
		quantity = odim.quantity;


	if (quantity == odim.quantity){
		EncodingODIM::updateLenient(odim);
	}
	else {
		mout.revised("different quantity, scaling not copied");
	}


	// NEW 2017/03
	if (product.empty()){
		product = odim.product;
		prodpar = odim.prodpar;
	}

	//if (date.empty() || time.empty()){
	if (date.empty()){
		date = odim.date;
		if (!odim.time.empty())
			time = odim.time;
		//else time = "000000";
	}

	if (time.empty()){
		time = "000000";
	}

	// mout.attention("comparing ", startdate, '-', starttime, " <- ", odim.startdate, '-', odim.starttime);

	if (!odim.startdate.empty()){
		if (startdate.empty() || (odim.startdate < startdate)){
			startdate = odim.startdate;
			starttime = odim.starttime;
		}
		else if ((odim.startdate == startdate) && (odim.starttime < starttime)){
			starttime = odim.starttime;
		}
	}

	if (startdate.empty())
		startdate = date; //"999999";

	if (starttime.empty())
		starttime = time; // "999999";

	// mout.note("comparing ", enddate, '-', endtime, " <- ", odim.enddate, '-', odim.endtime);

	if (!odim.enddate.empty()){
		if (enddate.empty() || (odim.enddate > enddate)){
			enddate = odim.enddate;
			endtime = odim.endtime;
		}
		else if ((odim.enddate == enddate) && (odim.endtime > endtime)){
			endtime = odim.endtime;
		}
	}

	if (enddate.empty())
		enddate = date; // NEW

	if (endtime.empty())
		endtime = time; // NEW


	if (source.empty())
		source = odim.source;

	if (NI == 0.0)
		NI = odim.NI;

	//mout.note("raimo" , starttime , ":" , enddate );

}


}  // namespace rack

