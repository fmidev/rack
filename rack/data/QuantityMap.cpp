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

#include "Quantity.h"
#include "QuantityMap.h"

namespace rack {


/*
void QuantityMap::set(const std::string & key, char typecode, double gain, double offset, double undetect, double nodata){

	Quantity & q = (*this)[key];

	if (!typecode)
		typecode = 'C';

	q.set(typecode, gain, offset, undetect, nodata);

	//return q;
}

void QuantityMap::set(const std::string & key, char typecode, double gain, double offset){

	// TODO: if (isSigned(typecode)) ...

	if (isnan(offset))
		offset = -gain;

	return set(key, typecode, gain, offset, drain::Type::getMin<double>(typecode), drain::Type::call<drain::typeMax,double>(typecode));

}
*/

/*
EncodingODIM QuantityMap::set(const std::string & key, char typecode){
	return get(key).set(typecode);
}

void QuantityMap::set(const std::string & key, const Quantity & q){
	set(key, q.defaultType);
	get(key) = q;
}
*/


void QuantityMap::initialize(){

	set("DBZH", 'C').setScaling(0.5, -32);
	set("DBZH", 'S').setScaling(0.01, -0.01*(128*256));
	get("DBZH").setZero(-32.0);

	copy("TH", get("DBZH"));
	copy("DBZHC", get("DBZH"));

	set("VRAD", 'C').setScaling(0.5, -64.0);  // nodata = 0?  IRIS
	set("VRAD", 'S').setScaling( 0.0025, -0.0025*(256.0*128.0)); // nodata = 0?
	const Quantity & VRAD = get("VRAD");
	copy("VRADH", VRAD);
	copy("VRADV", VRAD);
	copy("VRADDH", VRAD);
	copy("VRADDV", VRAD);

	set("RHOHV", 'C').setScaling(0.004); //
	set("RHOHV", 'S').setScaling(0.0001); //

	set("ZDR", 'C').setScaling( 0.1, -12.8); //
	set("ZDR", 'S').setScaling( 0.01, -0.01*(128*256)); //

	set("RATE", 'C').setScaling( 0.05); // nodata = 0?
	set("RATE", 'S').setScaling( 0.0005); // nodata = 0?

	set("HGHT", 'C').setScaling( 0.05);   //   255 => 12.5km
	set("HGHT", 'S').setScaling( 0.0002); // 65535 => 13.x km

	set("QIND", 'C').setScaling( 1.0/250.0);   //
	set("QIND", 'S').setScaling( 1.0/(256.0*256.0-1.0));
	get("QIND").setZero(0.0);

	copy("PROB", get("QIND"));

	set("COUNT", 'S'); // default type short int
	set("COUNT", 'C');   //
	set("COUNT", 'I');   //
	set("COUNT", 'L');   //

	set("AMVU", 'C').setRange(-100,100);
	set("AMVU", 'S').setRange(-200,200);
	set("AMVU", 'd');
	copy("AMVV", get("AMVU"));

	set("CLASS", 'C');
	set("CLASS", 'S');


}

bool QuantityMap::setQuantityDefaults(EncodingODIM & dstODIM, const std::string & quantity, const std::string & values) const {  // TODO : should it add?

	drain::Logger mout("QuantityMap", __FUNCTION__);

	mout.debug(2) << "for quantity=" << quantity << ", values=" << values << mout.endl;
	//if (quantity.empty())
	//	quantity = dstODIM.quantity;

	drain::ReferenceMap refMap;
	if (!values.empty()){
		dstODIM.addShortKeys(refMap);
		refMap.setValues(values);   // essentially, sets dstODIM.type (other values will be reset, below)
	}

	mout.debug(2) << "searching for quantity=" << quantity << mout.endl;
	const_iterator it = find(quantity);
	if (it != end()){

		mout.debug(1) << "found quantity '"  << quantity << "'" << mout.endl;

		/// Use user-defined type. If not supplied, use default type.
		if (dstODIM.type.empty()) {
			if (it->second.defaultType)
				dstODIM.type = it->second.defaultType;
			else {
				mout.warn() << "type unset, and no defaultType defined for quantity=" << quantity <<  mout.endl;
				return false;
			}
		}

		/// find type conf for this Quantity and basetype
		const char typechar = dstODIM.type.at(0);
		Quantity::const_iterator qit = it->second.find(typechar);
		if (qit != it->second.end()){
			//std::cerr << "OK q=" << quantity << ", type=" << typechar << std::endl;
			// initialize values to defaults
			dstODIM = qit->second;
			// finally, set desired scaling values, overriding those just set...
			if (!values.empty()){
				refMap.setValues(values);
			}
			return true;
		}
		else {
			// error: type
			mout.info() << "quantity=" << quantity << " found, but no conf for typechar=" << typechar << mout.endl;
		}
	}
	else {
		mout.info() << "quantity=" << quantity << " not found" << mout.endl;
	}

	if (!dstODIM.type.empty()) {
		//const char typechar = dstODIM.type.at(0);
		const drain::Type t(dstODIM.type);
		mout.debug() << "applying universal defaults (1,0,min,max) for typechar=" << t << mout.endl;
		dstODIM.gain   = 1.0;
		dstODIM.offset = 0.0;
		dstODIM.undetect = drain::Type::call<drain::typeMin, double>(t); //drain::Type::getMin<double>(typechar);
		dstODIM.nodata =   drain::Type::call<drain::typeMax, double>(t); //drain::Type::call<drain::typeMax,double>(typechar);
		// finally, set desired scaling values, overriding those just set...
		if (!values.empty()){
			refMap.setValues(values);
		}
	}

	return false;

}

QuantityMap & getQuantityMap() {
	static QuantityMap quantityMap;
	return quantityMap;
}



}  // namespace rack


