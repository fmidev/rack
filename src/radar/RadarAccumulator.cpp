
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
 * RadarAccumulator.cpp
 *
 *  Created on: Aug 21, 2025
 *      Author: mpeura
 */

//#include <drain/image/AccumulatorGeo.h>
#include "RadarAccumulator.h"

namespace rack {


bool RadarAccumulatorBase::checkCompositingMethod(const AccumulationMethod & rule, const ODIM & dataODIM) {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("start, quantity=" , dataODIM.quantity );

	//const AccumulationMethod & rule = this->getMethod();
	if (rule.getName() == "WAVG"){

		const drain::ReferenceMap & wavgParams = rule.getParameters();
		const double p = wavgParams.get("p", 1.0);
		const double p2 = p/2.0;
		mout.info(rule , ", min=", dataODIM.getMin() );

		if ((p2 - floor(p2)) == 0.0){
			mout.info("WAVG with p=" , p , " = 2N, positive pow() results" );
			const Quantity & q = getQuantityMap().get(dataODIM.quantity);
			if (q.hasUndetectValue()){
				const double bias = wavgParams.get("bias", 0.0);
				if (bias > q.undetectValue){
					mout.warn("WAVG with p=" , p ," = 2N, undetectValue=" , q.undetectValue , " < bias=", bias  );
					mout.warn("consider adjusting bias(", bias ,") down to quantity (" , dataODIM.quantity , ") zero: " , q.undetectValue );
					//rule.setParameter("bias", q.undetectValue);
					return true;
				}
			}
		}
	}

	return false;


}


// template  <class AC, class OD>
void RadarAccumulatorBase::extractFinally(const drain::image::Accumulator & accumulator, drain::image::Accumulator::FieldType field,
		const ODIM &srcODIM, const DataCoder & dataCoder,
		ODIM &dstODIM, drain::image::Image & dstImage, const drain::Rectangle<int> & cropArea){

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::SmartMapTools::updateValues(dstODIM, srcODIM);

	// if (dataCoder.qualityODIM.isSet()){
	if (dstODIM.isSet()){
		// dstQuality.data.setType(odimData.type); // <- this was probably wrong
		dstImage.setType(dstODIM.type); // <- this was probably wrong
		//mout.debug3("dstData: " , dstQuality );
		accumulator.extractField(field, dataCoder, dstImage, cropArea);
		//return dstQuality;
	}
	else {
		// Note: this is also checked by Accumulator, but better diagnostics (ODIM) here:
		mout.error("Target encoding unset: gain=", dstODIM.scaling.scale, ", type=", dstODIM.type);
		//mout.error("Target encoding unset: gain=", dataCoder.qualityODIM.scaling.scale, ", type=", dataCoder.qualityODIM.type);
		//return dstQuality;
	}

}

/*
enum TypeEnum {
	UNSIGNED_INT,
	DOUBLE
};

typedef drain::EnumDict<TypeEnum>::dict_t typedict_t;
const typedict_t kokeilu = {
		DRAIN_ENUM_ENTRY(TypeEnum, DOUBLE)
};
*/
/*
class TypeWrapper : public std::type_info {

	public:
	TypeWrapper() : std::type_info(typeid(void)){};

	public:
	TypeWrapper(const std::type_info & type) : std::type_info(typeid(void)){};

	public:
	TypeWrapper(const TypeWrapper & tw) : std::type_info(tw){};

};

TypeWrapper koe2(typeid(float));


typedef drain::EnumDict<TypeWrapper>::dict_t test_t;
const test_t koe = {
		{"MIKA", typeid(float)}
//		{"MIKA", TypeWrapper(typeid(float))}
};
*/
//std::type_info mika(typeid(float));

//const std::type_info &x = koe.getValue("MIKA");


//drain::Dictionary<> dict;
/*
void RadarAccumulatorBase::extractData(ODIM & odim, drain::image::Image & dst, field_t field, const std::string & encoding, const drain::Rectangle<int> & cropArea){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("extracting field ", field);

	if (odim.quantity.empty()){
		odim.quantity = "UNKNOWN"; // ok; for example --cPlotFile carries no information on quantity
		mout.note("quantity=", odim.quantity);
	}

	//PlainData<DstType<OD> >
	mout.debug("searching dstData... DATA=", (field == field_t::DATA));
	//pdata_dst_t & dstData = (field == DATA) ? dstProduct.getData(odimFinal.quantity) : dstProduct.getQualityData(odimQuality.quantity);
	//mout .debug3() << "dstData: " << dstData << mout.endl;

	//DataDst dstData(dataGroup); // FIXME "qualityN" instead of dataN creates: /dataset1/qualityN/quality1/data
	//mout.warn("odimFinal: " , odimFinal );
	ODIM odimQuality;
	DataCoder dataCoder(odim, odimQuality); // (will use only either odim!)
	//mout.experimental<LOG_NOTICE>("dataCoder SRC = ", odim);
	mout.debug("dataCoder = ", dataCoder);
	mout.debug2("dataCoder - data: ", dataCoder.dataODIM);
	mout.debug2("dataCoder - qind: ", dataCoder.qualityODIM);

	if (!dataCoder.dataODIM.isSet()){
		// Note: this is also checked by Accumulator, but better diagnostics (ODIM) here:
		mout.error("Target encoding unset: gain=", dataCoder.dataODIM.scaling.scale, ", type=", dataCoder.dataODIM.type);
	}

	/// Also available: if (type.isSet()) ...

	// Note: encoding is used only for DATA to avoid ambiguous setting for multiple request of fields: DATA,WEIGHT,COUNT, ...
	if (field == field_t::DATA){
		mout.debug("extracting DATA/" , field, " [", odim.quantity, ']');
		if (!encoding.empty()){
			mout.accept("User-defined encoding for data [", odim.quantity, "]: ", encoding);
			odim.completeEncoding( encoding);
		}
		else if (!getTargetEncoding().empty()){
			mout.ok<LOG_NOTICE>("Using previously stored encoding for data [", odim.quantity, "]: ", getTargetEncoding());
		}

		pdata_dst_t & dstData = dstProduct.getData(odim.quantity);
		drain::SmartMapTools::updateValues(dstData.odim, odim);

		dstData.data.setType(odim.type);
		mout.debug3("dstData: " , dstData );
		//this->Accumulator::extractField(fieldChar, dataCoder, dstData.data, cropArea);
		//this->Accumulator::
		this->Accumulator::extractField(field, dataCoder, dst, cropArea);
	}
}


void RadarAccumulatorBase::extractQuality(ODIM & odim, drain::image::Image & dst, drain::image::Accumulator::FieldType field, const std::string & encoding, const drain::Rectangle<int> & cropArea){

}
*/

}  // namespace rack


