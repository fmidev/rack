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
#include "QuantityMap.h"

namespace rack {

#define RACK_QTY_INIT(qty) qty(map_t::operator [](#qty))

QuantityMap::QuantityMap() : map_t(),
		RACK_QTY_INIT(TH),
		RACK_QTY_INIT(DBZ),
		RACK_QTY_INIT(VRAD),
		RACK_QTY_INIT(ZDR),
		RACK_QTY_INIT(RHOHV),
		RACK_QTY_INIT(KDP),
		RACK_QTY_INIT(QIND),
		RACK_QTY_INIT(PROB),
		RACK_QTY_INIT(FUZZY)
		{
}

QuantityMap::QuantityMap(const QuantityMap & m) : map_t(m),
		RACK_QTY_INIT(TH),
		RACK_QTY_INIT(DBZ),
		RACK_QTY_INIT(VRAD),
		RACK_QTY_INIT(ZDR),
		RACK_QTY_INIT(RHOHV),
		RACK_QTY_INIT(KDP),
		RACK_QTY_INIT(QIND),
		RACK_QTY_INIT(PROB),
		RACK_QTY_INIT(FUZZY)
{
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.warn("? copy const <QuantityMap>");
}

/*
QuantityMap::QuantityMap(const map_t & m) : map_t(m){
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.warn("? copy const <map>");
}
*/

QuantityMap::QuantityMap(const std::initializer_list<std::pair<std::string, Quantity> > & inits) : map_t(),
		RACK_QTY_INIT(TH),
		RACK_QTY_INIT(DBZ),
		RACK_QTY_INIT(VRAD),
		RACK_QTY_INIT(ZDR),
		RACK_QTY_INIT(RHOHV),
		RACK_QTY_INIT(KDP),
		RACK_QTY_INIT(QIND),
		RACK_QTY_INIT(PROB),
		RACK_QTY_INIT(FUZZY)
{
	assign(inits);
}

void QuantityMap::assign(const std::initializer_list<std::pair<std::string, Quantity> > & inits) {
	for (const auto & entry: inits){
		// std::cerr << entry.first << ' ' << entry.second.variants << '\n';
		Quantity & qty = (*this)[entry.first];
		qty = entry.second;
		//std::cerr << '\t' << qty << '\n';
	}
	//return *this;
}

std::ostream & QuantityMap::toStream(std::ostream & ostr) const {
	for (const auto & entry: *this){
		ostr << entry.first << ' ';
		if (!entry.second.keySelector.empty()){
			ostr << '(' << entry.second.keySelector << ')' <<' ';
		}
		ostr << "– " << entry.second.name << '\n';
		ostr << entry.second; // << '\n';
	}
	return ostr;
}




QuantityMap::const_iterator QuantityMap::retrieve(const std::string & key) const {

		// Attempt 1: find exact match
		const_iterator it = find(key);

		// Attempt 2: find a compatible variant
		if (it == end()){
			//for (const auto & entry: *this){
			for (it=begin(); it!=end(); ++it){
				// std::cerr << "TESTING " << key << " -> " << it->first << ' ' << it->second << " [" << it->second.variants << "]\n";
				if (it->second.keySelector.test(key, false)){
					// std::cerr << "NEW: found " << key << " <-> " << it->first << " [" << it->second << "]\n";
					// return entry.second;
					break;
				}
			}
			// Give up, leaving it==end()
		}

		return it;

}


QuantityMap::iterator QuantityMap::retrieve(const std::string & key) {

	// Attempt 1: find exact match
	iterator it = find(key);

	// Attempt 2: find a compatible variant
	if (it == end()){
		for (it=begin(); it!=end(); ++it){
			if (it->second.keySelector.test(key, false)){
				// std::cerr << "NEW: found " << key << " <-> " << it->first << " [" << it->second << "]";
				// return entry.second;
				break;
			}
		}
		// Give up, leaving it==end()
	}

	return it;

}

const Quantity & QuantityMap::get(const std::string & key) const {

		const const_iterator it = retrieve(key);

		if (it != end()){ // null ok
			return it->second;
		}
		else {
			//drain::Logger mout("Quantity", __FUNCTION__);
			//mout.warn("undefined quantity=" , key );
			static const Quantity empty;
			return empty;
		}

}

Quantity & QuantityMap::get(const std::string & key) {

	const iterator it = retrieve(key); // revised 2025

	// Attempt 1: find fully matching one.
	if (it != end()){ // null ok
		return it->second;
	}
	else {
		// Warning: if this is declared (modified), further instances will override and hence confuse
		static Quantity empty;
		return empty;
	}

}



bool QuantityMap::setQuantityDefaults(EncodingODIM & dstODIM, const std::string & quantity, const std::string & values) const {  // TODO : should it add?

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug3("for quantity=" , quantity , ", values=" , values );
	//if (quantity.empty())
	//	quantity = dstODIM.quantity;

	drain::ReferenceMap refMap;
	if (!values.empty()){
		dstODIM.grantShortKeys(refMap);
		refMap.setValues(values);   // essentially, sets dstODIM.type (str values will be reset, below)
	}

	mout.debug3("searching for quantity=" , quantity );
	//const_iterator it = find(quantity);
	const_iterator it = retrieve(quantity);
	if (it != end()){

		mout.debug2("found quantity '"  , quantity , "'" );

		/// Use user-defined type. If not supplied, use default type.
		if (dstODIM.type.empty()) {
			if (it->second.defaultType)
				dstODIM.type = it->second.defaultType;
			else {
				mout.warn("type unset, and no defaultType defined for quantity=" , quantity );
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
			mout.debug2("updated dstODIM: "  , dstODIM );
			return true;
		}
		else {
			// error: type
			mout.info("quantity=" , quantity , " found, but no conf for typechar=" , typechar );
		}
	}
	else {
		mout.info("quantity=" , quantity , " not found" );
	}

	if (!dstODIM.type.empty()) {
		//const char typechar = dstODIM.type.at(0);
		const drain::Type t(dstODIM.type);
		mout.debug("applying universal defaults (1,0,min,max) for typechar=" , t );
		dstODIM.scaling.set(1.0, 0.0); // check! Maybe "physical" range 0.0 ...1.0 better than gain=1.0
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

	static QuantityMap quantityMap = {
			// {"DBZH", {
			{"DBZ", {
					"Radar reflectivity",
					{"DBZ", "DBZ[HV]", "DBZ[HV]C", "T[HV]"},
					{-32.0, +60.0},
					'C',
					{
							{'C', 0.5, -32.0},
							{'S', 0.01, -0.01*(128*256)}
					},
					-32.0 // "virtual" zero
			}
			},
			{"DBZHDEV", {
					"Deviation of radar reflectivity",
					{"DBZH_DEV"},
					{-100.0, 100.0},
					'C',
					{
							{'C', 1.0, -128},
							{'S'}
					},
					0.0
			}
			},
			{"VRAD", {
					"Radial velocity",
					{"VRAD", "VRAD[HV]"},
					{-100.0, 100},
					'C',
					{
							{'C', 0.5, -64.0},
							{'S', {-100.0,+100.0}}
					}
			}
			},
			{"VRAD_DEV", {
					"Radial velocity, deviation",
					{},
					{-100.0, 100},
					'C',
					{
							{'C', 0.0, 64.0},
							{'S', 0, 128.0}
					}
			}
			},
			{"VRAD_DIFF", {
					"Radial velocity difference", {},
					{-100.0, 100},
					'C',
					{
							{'C', {-32, +32}},
							{'S', {-256.0,+256.0}}
					}
			}
			},
			{"RHOHV", {
					"Polarimetric cross-correlation", {},
					{0.0, 1.0},
					'S',
					{
							{'C', 0.004,  -0.004},
							{'S', 0.0001, -0.0001}
					}
			}
			},
			{"ZDR", {
					"Polarimetric difference", {},
					{0.0, 1.0},
					'S',
					{
							{'C', 0.1,  -12.8},
							{'S', 0.01, -0.01*(128.0*256.0)}
					}
			}
			},
			{"KDP", {
					"Polarimetric differential phase", {},
					{-120.0, +120.0},
					'S',
					{
							{'C'},
							{'S', 0.01, -0.01*(128.0*256.0)}
					}
			}
			},
			{"PHIDP", {
					"Polarimetric differential phase", {"PhiDP"},
					{-180.0, +180.0},
					'S',
					{
							{'C'},
							{'S'}
					}
			}
			},
			{"RATE", {
					"Precipitation rate", {"RAINRATE"},
					'S',
					{
							{'C', 0.05},
							{'S', 0.0005}
					}
			}
			},
			{"HCLASS", {
					"Classification (Vaisala)", {"HydroCLASS"},
					'S',
					{
							{'C', 1.0},
							{'S', 1.0}
					}
			}
			},
			{"CLASS", {
					"Classification", {"CLASS_.*"},
					'S',
					{
							{'C', 1.0},
							{'S', 1.0}
					}
			}
			},
			{"QIND", {
					"Quality index", {"QIND_.*"},
					{0.0, 1.0},
					'C',
					{
							{'C', 1.0/250.0},
							{'S', 1.0/65535.0}
					}
			}
			},
			{"PROB", {
					"Probability", {"PROB_.*"},
					{0.0, 1.0},
					'C',
					{
							{'C', 1.0/250.0},
							{'S', 1.0/65535.0}
					}
			}
			},
			{"COUNT", {
					"Count", {"COUNT_.*"},
					'C',
					{
							{'C'}, {'S'}, {'I'}, {'L'}, {'f'}, {'d'}
					}
			}
			},
			{"AMV", {
					"Atmospheric motion [vector component]", {"AMV[UV]"},
					'C',
					{
							{'C', {-100,100}},
							{'c', {-127,127}},
							{'S', {-327.68, +327.68}},
							{'s', 0.01},
							{'f'},
							{'d'}
					}
			}
			},
	};

	drain::Logger mout(__FILE__, __FUNCTION__);

	const bool FIRST_INIT = true; // !quantityMap.hasQuantity("HGHT");

	if (ODIM::versionFlagger.isSet(ODIM::KILOMETRES)){
		if (FIRST_INIT){ // TODO: only if changed?
			mout.debug("Using kilometres for HGHT and HGHTDEV (ODIM version: ", ODIM::versionFlagger, ")");
		}
		quantityMap["HGHT"] = {
				"Height/altitude [km]", {"ALT", "ALTITUDE"},
				'S',
				{
						{'C', 0.01},    //   255 => 25,5km
						{'S', 0.0004}   // 65535 => 26.2 km
				}
		};
		quantityMap["HGHTDEV"] = {
				"Height/altitude deviation [km]", {"ALT", "ALTITUDE"},
				'S',
				{
						{'C', {-10.0, +10.0}}, //   255 => 12.5km
						{'S', {-20.0, +20.0}}  // 65535 => 13.x km
				}
		};
	}
	else {

		if (FIRST_INIT){ // TODO: only if changed?
			mout.debug("Using metres for HGHT and HGHTDEV (ODIM version: ", ODIM::versionFlagger, ")");
		}

		quantityMap["HGHT"] = {
				"Height/altitude [m]", {"ALT", "ALTITUDE"},
				'S',
				{
						{'C', 100.0},
						{'S', 0.4}
				}
		};
		quantityMap["HGHTDEV"] = {
				"Height/altitude deviation [m]", {"HGHT_?DEV","ALTDEV", "ALTITUDE"},
				'S',
				{
						{'C', {-10000.0, +10000.0}},
						{'S', {-20000.0, +20000.0}}
				}
		};
	}

	return quantityMap;
}


}  // rack::


