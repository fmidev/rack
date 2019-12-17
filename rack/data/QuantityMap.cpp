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

#include "Quantity.h"
#include "QuantityMap.h"

namespace rack {


void QuantityMap::initialize(){

	Quantity & DBZH = add("DBZH");
	DBZH.set('C').setScaling(0.5, -32);
	DBZH.set('S').setScaling(0.01, -0.01*(128*256));
	DBZH.setZero(-32.0);

	copy("TH",    DBZH );
	copy("DBZHC", DBZH );

	Quantity & VRAD = add("VRAD");
	VRAD.set('C').setScaling(    0.5, -64.0);  // nodata = 0?  IRIS
	//VRAD.set('S').setScaling( 0.0025, -0.0025*(256.0*128.0)); // nodata = 0?
	VRAD.set('S').setRange(-100, 100);
	copy("VRADH", VRAD);
	copy("VRADV", VRAD);
	copy("VRADDH", VRAD);
	copy("VRADDV", VRAD);

	Quantity & VRAD_DIFF = add("VRAD_DIFF");
	VRAD_DIFF.set('C').setRange(-32,  32.0);
	VRAD_DIFF.set('S').setRange(-256, 256.0);

	Quantity & VRAD_DEV = add("VRAD_DEV");
	VRAD_DEV.set('C').setRange(0,  64.0);
	VRAD_DEV.set('S').setRange(0, 128.0);

	Quantity & RHOHV = add("RHOHV");
	RHOHV.set('C').setScaling(0.004); //
	RHOHV.set('S').setScaling(0.0001); //

	set("ZDR", 'C').setScaling( 0.1, -12.8); //
	set("ZDR", 'S').setScaling( 0.01, -0.01*(128*256)); //

	set("RATE", 'C').setScaling( 0.05); // nodata = 0?
	set("RATE", 'S').setScaling( 0.0005); // nodata = 0?

	set("HGHT", 'C').setScaling( 0.05);   //   255 => 12.5km
	set("HGHT", 'S').setScaling( 0.0002); // 65535 => 13.x km

	Quantity & QIND = add("QIND");
	QIND.set('C').setScaling( 1.0/250.0);   //
	QIND.set('S').setScaling( 1.0/(256.0*256.0-1.0));
	QIND.setZero(0.0);

	copy("PROB", QIND);

	Quantity & COUNT = add("COUNT");
	COUNT.set('S'); // default type short int
	COUNT.set('C'); //
	COUNT.set('I'); //
	COUNT.set('L'); //
	COUNT.set('f'); // Floats needed in infinite accumulations
	COUNT.set('d'); //

	Quantity & AMVU = add("AMVU");
	AMVU.set('C').setRange(-100,100);
	AMVU.set('c').setRange(-127,127);
	AMVU.set('S').setRange(-327.68, +327.68);
	AMVU.set('s').setScaling(0.01);
	AMVU.set('d');
	copy("AMVV", AMVU);

	Quantity & CLASS = add("CLASS");
	CLASS.set('C');
	CLASS.set('S');

}

bool QuantityMap::setQuantityDefaults(EncodingODIM & dstODIM, const std::string & quantity, const std::string & values) const {  // TODO : should it add?

	drain::Logger mout("QuantityMap", __FUNCTION__);

	mout.debug(2) << "for quantity=" << quantity << ", values=" << values << mout.endl;
	//if (quantity.empty())
	//	quantity = dstODIM.quantity;

	drain::ReferenceMap refMap;
	if (!values.empty()){
		dstODIM.grantShortKeys(refMap);
		refMap.setValues(values);   // essentially, sets dstODIM.type (str values will be reset, below)
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
			//dstODIM.updateFromMap(qit->second);
			// finally, set desired scaling values, overriding those just set...
			if (!values.empty()){
				refMap.setValues(values);
			}
			mout.debug(1) << "updated dstODIM: "  << dstODIM << mout.endl;
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



// Rack
