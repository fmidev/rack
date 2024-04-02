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
/*
 * ProductOp.cpp
 *
 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */

#include <drain/VariableAssign.h>
//#include <drain/Castable.h>
#include <drain/String.h>
#include <drain/Type.h>
#include <drain/TypeUtils.h>


#include "data/QuantityMap.h"
//#include "drain/image/File.h"
//#include "drain/util/Log.h"
//#include "drain/util/Variable.h"

#include <limits>
//#include <stdexcept>
#include <utility>

#include "main/rack.h"
#include "ProductBase.h"

namespace rack {

/*
ODIMPathElem ProductBase::appendResults;

int ProductBase::outputDataVerbosity(0);
*/

void ProductBase::help(std::ostream &ostr, bool showDescription) const {

	if (showDescription)
		ostr << name << ": " << description << '\n';

	//parameters.keys;
	const std::list<std::string> & keys = parameters.getKeyList();
	const std::map<std::string,std::string> & units = parameters.getUnitMap();

	ostr << "# Parameters:\n";
	//char separator = '\0';
	for (const std::string & key: keys){
		std::map<std::string, drain::Reference>::const_iterator pit = parameters.find(key);
		if (pit != parameters.end()){
			//ostr << separator << ' ' << *it;
			ostr << "#   " << key << ' ';
			if (showDescription)
				if ((static_cast<float>(pit->second) != std::numeric_limits<float>::min()) && (static_cast<double>(pit->second) != std::numeric_limits<double>::min()))
					ostr << '=' << ' ' << pit->second << ' ';  // *it === pit->first
			std::map<std::string,std::string>::const_iterator uit = units.find(key);
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
		for (const std::string & key: keys){
		//for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){
			ostr << separator << key << '(' << allowedEncoding[key] << ')';
			separator = ',';
		}
		ostr << '\n';
	}

	//ostr << "  " << parameters << "\n";
}



void ProductBase::applyODIM(ODIM & productODIM, const ODIM & srcODIM, bool useDefaults){

	drain::Logger mout(__FILE__, __FUNCTION__);

	const bool QUANTITY_UNSET = productODIM.quantity.empty();

	if (QUANTITY_UNSET){
		productODIM.quantity = srcODIM.quantity;
		mout.info("set quantity=" , productODIM.quantity );
	}

	if  (!productODIM.isSet()){

		// If src data and product have same quantity and storage type, adapt same encoding
		if ((srcODIM.quantity == productODIM.quantity) && (srcODIM.type == productODIM.type)){ // note: may still be empty
			EncodingODIM srcBase(srcODIM);
			productODIM.updateFromMap(srcBase); // Does not copy geometry (rscale, nbins, etc).
			mout.info("same quantity=" , productODIM.quantity , " and type, copied encoding: " , EncodingODIM(productODIM) );
		}

		//mout.toOStr() << "set quantity=" << productODIM.quantity << ", encoding: " << EncodingODIM(productODIM) << mout.endl;
		//mout.warn("productODIM.update(srcODIM)" );
		productODIM.updateLenient(srcODIM); // date, time, Nyquist(NI) - WARNING, some day setLenient may copy srcODIM encoding

	}

	if ((!productODIM.isSet()) && useDefaults){

		//mout.warn("productODIM not set above?" );

		if (!productODIM.quantity.empty()){
			mout.info("setting quantity defaults for " , productODIM.quantity );
			getQuantityMap().setQuantityDefaults(productODIM, productODIM.quantity, productODIM.type);
			mout.info("setQuantityDefaults: quantity=" , productODIM.quantity , ", " , EncodingODIM(productODIM) );
		}
		else if (!productODIM.type.empty()){
			mout.warn("type [", productODIM.type,"]set, but quantity unset?");
			productODIM.setTypeDefaults();
			mout.info("setTypeDefaults: " , EncodingODIM(productODIM) );
		}
	}

	/*
	else if (!encoding.empty()){
		mout.note(EncodingODIM(productODIM) );
		mout.warn(" productODIM.scale set, tried to reset with: " , encoding );
	}
	*/

	// mout.toOStr() << "set quantity=" << productODIM.quantity << ", encoding: " << EncodingODIM(productODIM) << mout.endl;
	if (QUANTITY_UNSET && (srcODIM.quantity == productODIM.quantity)){
		// xxx
		if (productODIM.isSet() && drain::Type::call<drain::typeIsSmallInt>(srcODIM.type)){
			if (srcODIM.getMin() < productODIM.getMin()){
				mout.note("input [", srcODIM.quantity , "] min=", srcODIM.getMin() ,") lower than supported by target  (min=", productODIM.getMin() , ")");
			}
			if (srcODIM.getMax() > productODIM.getMax()){
				mout.note("input [", srcODIM.quantity , "] max=", srcODIM.getMax() ,") higher than supported by target (max=", productODIM.getMax() , ")");
			}
		}
	}

	ProductBase::setODIMspecials(productODIM);

}

void ProductBase::completeEncoding(ODIM & dstODIM, const std::string & encoding){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (encoding.empty()){
		mout.debug("empty request (ok)" );
	}

	const std::string origQuantity(dstODIM.quantity);

	EncodingODIM odim;
	odim.type = "";
	odim.link("what:quantity", dstODIM.quantity); 	// Consider (..., bool ALLOW_QUANTITY_CHANGE=true)
	odim.addShortKeys();
	odim.updateValues(encoding);

	//mout.debug2()
	/*
	mout.warn(odim.getKeys() );
	mout.warn("request: " , encoding );
	mout.warn("dstODIM.quantity: " , dstODIM.quantity );
	mout.warn("odim: " , odim );
	*/

	if (dstODIM.quantity.empty()){
		mout.warn("quantity (still) empty, odim=" , odim );
	}

	if (dstODIM.quantity != origQuantity){
		mout.info("quantity change " , origQuantity , " => " , dstODIM.quantity , " requested, ok"  );
	}
	else if (!odim.type.empty() && (odim.type != dstODIM.type)){
		mout.info("type change " , dstODIM.type , " => " , odim.type , " requested, ok" );
	}
	else if (!dstODIM.isSet()){ // quantity set, but type or gain unset
		mout.info("dstODIM unset, applying defaults for quantity: " , dstODIM.quantity );
	}
	else {
		// That is, no "resetting" needed.
		dstODIM.addShortKeys();
		dstODIM.updateValues(encoding);
		mout.info(" (only) minor changes requested in encoding: " , encoding , " => " , EncodingODIM(dstODIM) );
		return;
	}

	//mout.warn("quantity [" , dstODIM.quantity , "]" );
	const QuantityMap & qmap= getQuantityMap();
	bool result = qmap.setQuantityDefaults(dstODIM, dstODIM.quantity, encoding);

	if (!result){
		if (qmap.hasQuantity(dstODIM.quantity)){
			if (drain::Type::call<drain::typeIsInteger>(dstODIM.type))
				mout.warn(); // Integer: underflow/overflow possible. Bit value
			else
				mout.info(); // Pretty safe, only precision issues possible
			mout << "No explicit config for storage type '" << dstODIM.type << "' for quantity [" << dstODIM.quantity << "], guessing: " << EncodingODIM(dstODIM) << mout.endl;
		}
		else {
			if (encoding.empty()){
				mout.warn("unknown quantity [" , dstODIM.quantity , "], guessing: " , EncodingODIM(dstODIM) );
			}
			else {
				mout.note("unknown quantity [" , dstODIM.quantity , "], setting: " , EncodingODIM(dstODIM) );
			}
		}

	}
}



// Under constr.
void ProductBase::setAllowedEncoding(const std::string & keys) {
	std::list<std::string> l;
	drain::StringTools::split(keys, l, ',');
}

void ProductBase::setODIMspecials(ODIM & dstODIM){

	//drain::Logger mout(__FILE__, __FUNCTION__);

	dstODIM.distinguishNodata("VRAD");
	/*
	if (dstODIM.distinguishNodata("VRAD")){
		mout.note("setting nodata=" , dstODIM.nodata , " to distinguish undetect=", dstODIM.undetect );
	}
	*/

	if (dstODIM.product == "SCAN"){
		dstODIM.product = "PPI";
	}

}

void ProductBase::setODIMsoftwareVersion(drain::VariableMap & how){
	how["software"]   = __RACK__;
	how["sw_version"] = __RACK_VERSION__;
}



}  // rack::

// Rack
