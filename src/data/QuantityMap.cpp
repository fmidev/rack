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

#include "drain/util/Log.h"

#include "Quantity.h"
#include "QuantityMap.h"

namespace rack {


void QuantityMap::initialize(){


	drain::Logger mout(__FILE__, __FUNCTION__);

	// OLD

	Quantity & DBZH = add("DBZH");
	DBZH.set('C').setScaling(0.5, -32);
	DBZH.set('S').setScaling(0.01, -0.01*(128*256));
	DBZH.setZero(-32.0);

	copy("TH",    DBZH );
	copy("DBZHC", DBZH );

	Quantity & DBZHDEV = add("DBZHDEV"); // RadarAccumulator
	DBZHDEV.set('C').setScaling(1, -128);
	DBZHDEV.set('S').setRange(-100.0, +100.0);
	DBZHDEV.setZero(0.0);

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

	// NEw

	if (false){

		map_t & m = *this;

		m["AMVU"] = {"Atmospheric motion, lateral component", {0,0}, 'C',
			{
				{'C', {-100,100} } /* range */ ,
				{'S', {-327.68,327.68} } /* range */ ,
				{'c', {-127,127}} /* range */ ,
				{'d'},
				{'s', 0.01,-0.01}
			}
		};

		m["AMVV"] = {"Atmospheric motion, lateral component", {0,0}, 'C',
				{
						{'C', {-100,100} } /* range */ ,
						{'S', {-327.68,327.68} } /* range */ ,
						{'c', {-127,127}} /* range */ ,
						{'d'},
						{'s', 0.01,-0.01}
				}
		};

		m["CLASS"] = {"Classification", {0,0}, 'C',
			{
				{'C', 1},
				{'S', 1},
				{'c', 1},
				{'s', 1}
			}
		};

		m["COUNT"] = {"Count", {}, 'C',
			{
				{'C', 1},
				{'I', 1},
				{'L', 1},
				{'S', 1},
				{'d', 1.0},
				{'f', 1.0}
			}
		};

		m["DBZH"] = {"Radar reflectivity, horz component", {-32.0, +64.0}, 'C',
			{
				{'C', 0.5,-32},
				{'S', 0.01,-327.68}
			},
			-32
		};

		m["DBZHC"] = {"Radar reflectivity, horz component, corrected", {-32.0, +64.0}, 'C',
			{
				{'C', 0.5,-32},
				{'S', 0.01,-327.68}
			},
			-32
		};

		m["DBZHDEV"] = {"Radar reflectivity, horz component, deviation", {0,0}, 'C',
			{
				{'C', 1,-128},
				{'S', {-100,100}}, /* range */
				{'f'}
			},
			0
		};

		m["HGHT"] = {"Height", {0,0}, 'C',
			{
				{'C', 0.05,-0.05},
				{'S', 0.0002,-0.0002}
			}
		};

		m["PROB"] = {"Probability", {0.0,1.0}, 'C',
			{
				{'C', 0.004,-0.004},
				{'S'}
			},
			0.0
		};

		m["QIND"] = {"Quality index", {0,0}, 'C',
			{
				{'C', 0.004,-0.004},
				{'S'}
			},
			0.0
		};

		m["RATE"] = {"Precipitation rate (mm)", {0,0}, 'C',
			{
				{'C', 0.05,-0.05},
				{'S', 0.0005,-0.0005}
			},
			0.0
		};

		m["RHOHV"] = {"Rho HV", {0.0,1.0}, 'C',
			{
				{'C', 0.004,-0.004},
				{'S', 0.0001,-0.0001}
			}
		};

		m["TH"] = {"Total radar reflecticity", {-32.0,+64.0}, 'C',
			{
				{'C', 0.5,-32},
				{'S', 0.01,-327.68}
			},
			-32.0
		};

		m["VRAD"] = {"Doppler velocity", {-64.0,+64.0}, 'C',
			{
				{'C', 0.5,-64},
				{'S', {-100.0,+100.0} } /* range */
			}
		};

		m["VRADDH"] = {"Doppler velocity", {-64.0,+64.0}, 'C',
				{
						{'C', 0.5,-64},
						{'S', {-100.0,+100.0} } /* range */
				}
		};

		m["VRADDV"] = {"Doppler velocity", {0,0}, 'C',
				{
						{'C', 0.5,-64},
						{'S', {-100.0,+100.0} } /* range */
				}
		};

		m["VRADH"] = {"Doppler velocity", {0,0}, 'C',
				{
						{'C', 0.5,-64},
						{'S', {-100.0,+100.0} } /* range */
				}
		};

		m["VRADV"] = {"Doppler velocity", {0,0}, 'C',
				{
						{'C', 0.5,-64},
						{'S', {-100.0,+100.0} } /* range */
				}
		};

		m["VRAD_DEV"] = {"Doppler velocity", {0,0}, 'C',
				{
						{'C', {0.0, 64}} /* range */ ,
						{'S', {0.0, 128}} /* range */
				}
		};

		m["VRAD_DIFF"] = {"VRAD_DIFFdesc", {0,0}, 'C',
			{
				{'C', {-32,32}, 255, 0, 0.252964,-32.253 } /* range */ ,
				{'S', {-256,256}, 65535, 0, 0.00781286,-256.008 } /* range */
			}
		};

		m["ZDR"] = {"ZDRdesc", {-10.0,+10.0}, 'C',
			{
				{'C', 0.1,-12.8},
				{'S', 0.01,-327.68}
			}
		};

	}


	// mout.attention(drain::sprinter(*this, drain::Sprinter::cppLayout));
	/*
	Quantity quantity = {"dBZ", {-32.0, 64.0}, 'C',
			{
					{'s', {1.2, 3.4}, 0.0, 255.0, {1.1, 2.2}}
					//			{'C', {'C', {0.5,  -32.0},  0.0, 255.0}},
					//			{'S', {'S', {0.01, -324.6}}}
			},
			-33.0
	};
	*/
	//typedef drain::EnumFlagger<drain::MultiFlagger<EncodingODIM::ExplicitSetting> > AdaptionFlagger;


	// typedef EncodingODIM ;
	if (false){

		for (auto & entry: *this){

			Quantity & q = entry.second;
			//std::cout << "// Quantity " << entry.first << " " << q.name << "\n";
			if (q.name.empty()){
				q.name = "Description...";
			}
			std::cout << "m[\"" << entry.first << "\"] = {";
			// std::cout << " {";

			std::cout << '"' << q.name << '"' << ", {";
			if (!q.physicalRange.empty())
				std::cout << q.physicalRange;
			std::cout << "}, '" << q.defaultType << "',\n";

			std::cout << "\t{";
			char sep = 0;
			for (const auto & entry2: entry.second){
				std::cout << sep; sep = ',';
				std::cout << "\n\t\t{";
				std::cout << "'" << entry2.first << "', ";
				const EncodingODIM & e2 = entry2.second;
				if (e2.explicitSettings & EncodingODIM::RANGE)
					std::cout << "{"<< e2.scaling.physRange << "}, " ;
				else
					std::cout << e2.scaling.scale << ',' << e2.scaling.offset << ", "; // << ' ';
				//std::cout << ", {"<< e2.scaling.scale << ',' << e2.scaling.offset << "}, " << e2.nodata << ", " << e2.undetect; // << ' ';
				std::cout << e2.nodata << ", " << e2.undetect; // << ' ';
				if (e2.explicitSettings & EncodingODIM::RANGE){
					std::cout << ", " << e2.scaling.scale << ',' << e2.scaling.offset; // << ' ';
				}
				else {
					if (!e2.scaling.physRange.empty())
						std::cout << ", {"<< e2.scaling.physRange << "} ";
				}
				std::cout << " }";

				if (e2.explicitSettings){
					std::cout << " /* " << drain::FlagResolver::getKeys(EncodingODIM::settingDict,e2.explicitSettings) << " */";
				}
				/*
				if (e2.explicitSettings & EncodingODIM::SCALING)
					std::cout << " / * scale * / ";
				if (e2.explicitSettings & EncodingODIM::RANGE)
					std::cout << " / * range * / ";
				*/
			}
			std::cout << "\n\t}";
			if (q.hasUndetectValue()){
				std::cout << ",\n\t" << q.undetectValue;
			}
			std::cout << "\n};\n\n";
		}

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
	const_iterator it = find(quantity);
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
	static QuantityMap quantityMap;
	return quantityMap;
}



}  // namespace rack



// Rack
