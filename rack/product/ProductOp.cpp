/*

    Copyright 2001 - 2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

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
/*
 * ProductOp.cpp
 *
 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */
#include <stdexcept>

#include <drain/util/Log.h>
#include <drain/util/Variable.h>
#include <drain/image/File.h>

#include "data/Quantity.h"
#include "ProductOp.h"

namespace rack {

//std::string ProductBase::appendResults("");
ODIMPathElem ProductBase::appendResults;

int ProductBase::outputDataVerbosity(0);


void ProductBase::help(std::ostream &ostr, bool showDescription) const {

	if (showDescription)
		ostr << name << ": " << description << '\n';

	//parameters.keys;
	const std::list<std::string> & keys = parameters.getKeyList();
	const std::map<std::string,std::string> & units = parameters.getUnitMap();

	ostr << "# Parameters:\n";
	//char separator = '\0';
	for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){
		std::map<std::string, drain::Referencer>::const_iterator pit = parameters.find(*it);
		if (pit != parameters.end()){
			//ostr << separator << ' ' << *it;
			ostr << "#   " << *it << ' ';
			if (showDescription)
				if ((static_cast<float>(pit->second) != std::numeric_limits<float>::min()) && (static_cast<double>(pit->second) != std::numeric_limits<double>::min()))
					ostr << '=' << ' ' << pit->second << ' ';  // *it === pit->first
			std::map<std::string,std::string>::const_iterator uit = units.find(*it);
			if ( uit != units.end() ){
				if (uit->second != "")
					ostr << '[' << uit->second << ']';
			}
			//separator = ',';
			ostr << '\n';
		}
		else {
			// FAIL
		}
	}
	ostr << '\n';

	if (!showDescription){
		ostr << "# Default values: ";
		parameters.getValues(ostr);
		ostr << '\n';
	}
	if ((showDescription) && (!allowedEncoding.empty())){
		ostr << "# Supported --target properties:";
		char separator = ' ';
		const std::list<std::string> & keys = allowedEncoding.getKeyList();
		for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){
		//for (drain::ReferenceMap::const_iterator it = allowedEncoding.begin(); it != allowedEncoding.end(); ++it){
			ostr << separator << *it << '(' << allowedEncoding[*it] << ')';
			separator = ',';
		}
		ostr << '\n';
	}

	//ostr << "  " << parameters << "\n";
}



void ProductBase::applyODIM(ODIM & productODIM, const ODIM & srcODIM, bool useDefaults){

	drain::Logger mout(RACK_PRODUCT_OP, __FUNCTION__);

	if (productODIM.quantity.empty()){
		productODIM.quantity = srcODIM.quantity;
		mout.info() << "set quantity=" << productODIM.quantity << mout.endl;
	}

	if  (!productODIM.isSet()){

		if (srcODIM.quantity == productODIM.quantity){ // note: may still be empty
			EncodingODIM srcBase(srcODIM);
			productODIM.updateFromMap(srcBase); // Does not copy geometry (rscale, nbins, etc).
			mout.info() << "same quantity=" << productODIM.quantity << ", copying encoding: " << EncodingODIM(productODIM) << mout.endl;
		}

		//mout.toOStr() << "set quantity=" << productODIM.quantity << ", encoding: " << EncodingODIM(productODIM) << mout.endl;
		//	getQuantityMap().setQuantityDefaults(productODIM, productODIM.quantity, encoding); // type may be unset
		productODIM.NI = srcODIM.NI;
	}

	if ((!productODIM.isSet()) && useDefaults){
		if (!productODIM.quantity.empty()){
			mout.note() << "setting quantity defaults for " << productODIM.quantity << mout.endl;
			getQuantityMap().setQuantityDefaults(productODIM, productODIM.quantity, productODIM.type);
			mout.info() << "setQuantityDefaults: quantity=" << productODIM.quantity << ", " << EncodingODIM(productODIM) << mout.endl;
		}
		else if (!productODIM.type.empty()){
			mout.warn() << "type set, but quantity unset?" << mout.endl;
			productODIM.setTypeDefaults();
			mout.info() << "setTypeDefaults: " << EncodingODIM(productODIM) <<mout.endl;
		}
	}


	/*
	else if (!encoding.empty()){
		mout.note() << EncodingODIM(productODIM) << mout.endl;
		mout.warn() << " productODIM.gain set, tried to reset with: " << encoding << mout.endl;
	}
	*/

	// mout.toOStr() << "set quantity=" << productODIM.quantity << ", encoding: " << EncodingODIM(productODIM) << mout.endl;
	if (srcODIM.quantity == productODIM.quantity){
		if (srcODIM.getMin() < productODIM.getMin()){
			mout.warn() << "input range ("<< srcODIM.quantity << ", min="<< srcODIM.getMin() <<") possibly wider than target range (min="<< productODIM.getMin() << ")"<< mout.endl;
		}
	}

	ProductBase::setODIMspecials(productODIM);

}

void ProductBase::handleEncodingRequest(ODIM & dstODIM, const std::string & encoding){

	drain::Logger mout("ProductBase", __FUNCTION__);

	if (dstODIM.quantity.empty()){
		mout.warn() << "quantity empty, odim=" << EncodingODIM(dstODIM) << mout.endl;
	}

	drain::ReferenceMap typeRef;

	/// If dstODIM gain unset or type change requested, initialise with quantity defaults
	std::string type;
	typeRef.reference("type", type = dstODIM.type);
	typeRef.updateValues(encoding);
	//
	if ((dstODIM.gain == 0) || (type != dstODIM.type)){
		mout.debug() << "type changed or gain==0, applying quantity defaults for quantity=" << dstODIM.quantity << mout.endl;
		getQuantityMap().setQuantityDefaults(dstODIM, dstODIM.quantity, encoding); // type may be unset
		if (dstODIM.optimiseVRAD())
			mout.note() << "optimized VRAD for NI=" << dstODIM.NI << mout.endl;
		//  type may be unset ?
		mout.info() << "set quantity=" << dstODIM.quantity << ", encoding: " << EncodingODIM(dstODIM) << mout.endl;
	}


	if (!encoding.empty()){  // user wants to change something...

			// NOTE: dstODIM might be of derived class, and op[] provides access to ["rscale"] for example.
		dstODIM.addShortKeys(typeRef); // This adapts to actual type (PolarODIM, CartesianODIM)
		typeRef.setValues(encoding);

		if (dstODIM.quantity.empty()){
			mout.warn() << "quantity not set" << mout.endl;
			dstODIM.setTypeDefaults();
			typeRef.setValues(encoding);
		}
			mout.info() << "dstOdim: " << EncodingODIM(dstODIM) << mout.endl;
	}
	else {
		mout.debug() << "empty encodingRequest, no changes (quantity=" << dstODIM.quantity << ", encoding: " << EncodingODIM(dstODIM) << ")" << mout.endl;
	}
	// else	no action...

}




void ProductBase::setAllowedEncoding(const std::string & keys) {
	std::list<std::string> l;
	drain::StringTools::split(keys, l);
}

void ProductBase::setODIMspecials(ODIM & dstODIM){

	drain::Logger mout(RACK_PRODUCT_OP, __FUNCTION__);

	if (dstODIM.distinguishNodata("VRAD")){
		mout.note() << "setting nodata=" << dstODIM.nodata << mout.endl;
		mout.info() << " to distinguish it from undetect="<< dstODIM.undetect << mout.endl;
	}

	if (dstODIM.product == "SCAN"){
		dstODIM.product = "PPI";
	}

}




}  // rack::
