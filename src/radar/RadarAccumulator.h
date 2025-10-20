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
#ifndef RADAR_ACCUMULATOR_H
#define RADAR_ACCUMULATOR_H


#include <sstream>

// #include <drain/image/Sampler.h>drain::image::Accumulator::FieldType
#include <drain/image/Accumulator.h>  //  ostream field...
#include <drain/image/AccumulatorGeo.h>

#include "data/ODIM.h"
#include "data/ODIMPath.h"
#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/DataCoder.h"
#include "data/QuantityMap.h"
// #include "product/RadarProductOp.h"
#include "Geometry.h"


namespace rack {

/// Todo: consider non-template class:
/*
 * with Cart / polar linkage
 *
 *  getProperties("where:proj", comp.projDef)
 *  getProperties("where:xsize", comp.width ?)
 *  getProperties("where:nbins", comp.width ?)
 *
 */

class RadarAccumulatorBase {


public:

	inline
	RadarAccumulatorBase() : dataSelector(ODIMPathElem::DATA|ODIMPathElem::QUALITY), defaultQuality(0.5), counter(0){};

	inline
	RadarAccumulatorBase(const RadarAccumulatorBase & base) : dataSelector(ODIMPathElem::DATA|ODIMPathElem::QUALITY),
	defaultQuality(base.defaultQuality), counter(base.counter){};

	void extractFinally(const drain::image::Accumulator & accumulator, drain::image::Accumulator::FieldType field,
			const ODIM &srcODIM, const DataCoder & dataCoder,
			ODIM &dstODIM, drain::image::Image & dstImage, const drain::Rectangle<int> & cropArea);

	/// Input data selector.
	DataSelector dataSelector;
	// DataSelector dataSelector(ODIMPathElem::DATA);
	// dataSelector.pathMatcher.setElems(ODIMPathElem::DATA);

	typedef std::map<int,std::string> legend_t;
	legend_t legend;


	/// If source data has no quality field, this value is applied for (detected) data.
	double defaultQuality;

	///  Book-keeping for new data. Finally, in extraction phase, added to odim.ACCnum .
	/**  Must be kept separate during accumulation. The accumulation array channel count keeps track of bin hits.
	 *
	 */
	size_t counter;

	inline
	const std::string & getTargetEncoding(){
		return targetEncoding;
	}


protected:

	static
	bool checkCompositingMethod(const AccumulationMethod & rule, const ODIM & dataODIM);

	std::string targetEncoding;


};

/// Data array for creating composites and accumulated polar products (Surface rain fall or cluttermaps)
/**

    \tparam AC - accumulator type (plain drain::image::Accumulator or drain::image::AccumulatorGeo)
    \tparam OD - odim type (PolarODIM or CartesianODIM) metadata container

 */
template <class AC, class OD>
//class RadarAccumulator : public AC { // deprecating AC!
class RadarAccumulator : public RadarAccumulatorBase, public AC {

public:

	/// Input data type
	typedef PlainData<SrcType<OD const> > pdata_src_t;
	typedef PlainData<DstType<OD> >       pdata_dst_t;
	typedef drain::image::Accumulator::FieldType field_t;


	/// Default constructor
	RadarAccumulator() { // , undetectValue(-52.0) {
		odim.type.clear();
		odim.ACCnum = 0; // NEW
	}

	virtual
	~RadarAccumulator(){};

	/// Adds data that is in the same coordinate system as the accumulator. Weighted with quality.
	/*
	 *  Counter example: this method is \i not called when polar data is added to a Cartesian composite.
	 *
	 *  \param i0 - offset in horizontal direction
	 *  \param j0 - offset in vertical direction
	 */
	void addData(const pdata_src_t & srcData, const pdata_src_t & srcQuality, double weight, int i0, int j0);

	/// Adds data that is in the same coordinate system as the accumulator.
	/*
	 *
	 *  Counter example: this method is \i not called when polar data is added to a Cartesian composite.
	 */
	void addData(const pdata_src_t & srcData, const pdata_src_t & srcQuality, const pdata_src_t & srcCount);


	/**
	 *  \param odimOut - metadata container (PolarODIM or CartesianODIM)
	 *  \param fields - layers (d=data, w=weight, C=count, ...)
	 */
	// TODO: remove this (use the new ones)
	void extractOLD(const OD & odimOut, DataSet<DstType<OD> > & dstProduct,
			const std::string & fields, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

	inline
	void extract(DataSet<DstType<OD> > & dstProduct, const std::string & fieldStr, const std::string & encoding="C", const drain::Rectangle<int> & cropArea={0,0}){
		drain::image::Accumulator::FieldList fields;
		drain::image::Accumulator::getFields(fieldStr, fields);
		extract(dstProduct, fields, encoding, cropArea);
	}

	void extract(DataSet<DstType<OD> > & dstProduct, const drain::image::Accumulator::FieldList & fields, const std::string & encoding="C", const drain::Rectangle<int> & cropArea={0,0});

	pdata_dst_t & extract(DataSet<DstType<OD> > & dstProduct, field_t field = field_t::DATA, const std::string & encoding="C", const drain::Rectangle<int> & cropArea={0,0});

	// void extractDraft(ODIM & odim, drain::image::Image & dstData, field_t field, const std::string & encoding="", const drain::Rectangle<int> & cropArea={0,0});


	/// For storing the scaling and encoding of (1st) input or user-defined values. Also for bookkeeping of date, time, sources etc.
	/*
	 *  Helps in warning if input data does not match the expected / potential targetEncoding
	 */
	// Note: consider rack::ODIM. And perhaps ODIM to inherit FlexibleVariableMap.
	OD odim;



	/// Not critical. If set, needed to warn if input data does not match the expected / potential targetEncoding
	void setTargetEncoding(const std::string & encoding);

	inline
	void consumeTargetEncoding(std::string & encoding){
		setTargetEncoding(encoding);
		encoding.clear();
	}



	virtual inline
	std::ostream & toStream(std::ostream & ostr) const {

		// std::cerr << __FILE__ << '@' << __LINE__ << std::endl;
		this->AC::toStream(ostr);
		ostr << ' ';
		// std::cerr << __FILE__ << '@' << __LINE__ << std::endl;
		odim.toStream(ostr);
		// std::cerr << __FILE__ << '@' << __LINE__ << " (end)" << std::endl;
		return ostr;
	}

	/// Warns if data scaling involves risks in using WAVG (weighted averaging)
	//bool checkCompositingMethod(const ODIM & srcODIM) const;
	inline
	bool checkCompositingMethod(const ODIM & dataODIM){
		return RadarAccumulatorBase::checkCompositingMethod(this->getMethod(), dataODIM);
	}



};


template  <class AC, class OD>
void RadarAccumulator<AC,OD>::setTargetEncoding(const std::string & encoding){

	drain::Logger mout(__FILE__, __FUNCTION__);

	targetEncoding = encoding;

	if (encoding.empty()){
		return;
	}

	std::string quantityPrev = odim.quantity;
	// if (odim.quantity.empty()){
	/*
	ODIM m;
	m.link("what:quantity", odim.quantity); // appends
	m.addShortKeys();
	m.updateValues(encoding);
	*/
	//}

	//odim.link("what:quantity", odim.quantity);
	//m.addShortKeys();
	odim.addShortKeys();
	odim.updateValues(encoding);


	if (!quantityPrev.empty()){
		if (odim.quantity != quantityPrev){
			mout.warn("Quantity changed from [", quantityPrev, "] to [", odim.quantity, "]");
		}
		else {
			mout.info("Confirming quantity [", odim.quantity, "] explicitly");
		}
	}

	// mout.attention("Quantity: [", quantityPrev, "] -> [", odim.quantity, "]");
	mout.attention<LOG_DEBUG>("Target encoding: [", encoding, "] odim: ", odim); //, "]");


}

template  <class AC, class OD>
void RadarAccumulator<AC,OD>::addData(const pdata_src_t & srcData, const pdata_src_t & srcQuality, double weight, int i0, int j0){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// mout.attention("START ", EncodingODIM(srcData.odim));

	if (!srcQuality.data.isEmpty()){
		mout.info("Quality data available with input; using quality as weights in compositing.");
		DataCoder converter(srcData.odim, srcQuality.odim);
		// uses also DataCoder::undetectQualityCoeff
		AC::addData(srcData.data, srcQuality.data, converter, weight, i0, j0);
	}
	else {
		mout.info("No quality data available with input, ok.");
		ODIM qualityOdim; // dummy
		getQuantityMap().setQuantityDefaults(qualityOdim, "QIND");

		DataCoder converter(srcData.odim, qualityOdim);
		// uses also DataCoder::undetectQualityCoeff
		AC::addData(srcData.data, converter, weight, i0, j0);
	}

	// mout.attention("END1 ", EncodingODIM(this->odim));
	odim.updateLenient(srcData.odim); // Time, date, new
	// mout.attention("END2 ", EncodingODIM(this->odim));

	counter += std::max(1L, srcData.odim.ACCnum);
	// odim.ACCnum += std::max(1L, srcData.odim.ACCnum); wrong
	// odim.ACCnum = counter; // TODO remove counter?

	//mout.note("after:  " , this->odim );

}

template  <class AC, class OD>
void RadarAccumulator<AC,OD>::addData(const pdata_src_t & srcData, const pdata_src_t & srcQuality, const pdata_src_t & srcCount){

	drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.info("Quality data available with input; using quality as weights in compositing.");
	DataCoder converter(srcData.odim, srcQuality.odim);
	AC::addData(srcData.data, srcQuality.data, srcCount.data, converter);

	odim.updateLenient(srcData.odim); // Time, date, new
	counter = std::max(1L, srcData.odim.ACCnum);         // correct
	// odim.ACCnum += std::max(1L, srcData.odim.ACCnum); // wrong
}


/*
template  <class AC, class OD>
bool RadarAccumulator<AC,OD>::checkCompositingMethod(const ODIM & dataODIM) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("start, quantity=" , dataODIM.quantity );

	const AccumulationMethod & rule = this->getMethod();
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
*/
template  <class AC, class OD>
//void Composite::extract(DataSet<DstType<CartesianODIM> > & dstProduct, const FieldList & fields, const std::string & encoding, const drain::Rectangle<int> & cropArea){
void RadarAccumulator<AC,OD>::extract(DataSet<DstType<OD> > & dstProduct, const drain::image::Accumulator::FieldList & fields, const std::string & encoding, const drain::Rectangle<int> & cropArea){

	drain::Logger mout(__FILE__, __FUNCTION__);

	for (drain::image::Accumulator::FieldType field: fields) {

		pdata_dst_t & dstData = extract(dstProduct, field, encoding, cropArea);

		if (ODIM::versionFlagger.isSet(ODIM::RACK_EXTENSIONS) && !legend.empty()){
			mout.experimental("Copying (moving) legend for ", field);
			dstData.getWhat()["legend"] = drain::sprinter(legend, "|", ",", ":").str();
			legend.clear();
		}
	}


}


template  <class AC, class OD>
typename RadarAccumulator<AC,OD>::pdata_dst_t & RadarAccumulator<AC,OD>::extract(DataSet<DstType<OD> > & dstProduct, drain::image::Accumulator::FieldType field,
		const std::string & encoding, const drain::Rectangle<int> & cropArea){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("extracting FIELD: ", field);

	char fieldChar = (char)(((int)field)&127);
	if (fieldChar != drain::image::Accumulator::getFieldChar(field)){
		mout.fail("program error: ", field, '=', fieldChar, "!=", drain::image::Accumulator::getFieldChar(field));
	}

	//const std::type_info & t = drain::Type::getTypeInfo('C'); // drain::Type::getTypeInfo(odimOut.type);
	if (!cropArea.empty()){
		mout.note("Applying cropping: bbox=", cropArea, " [pix] from ", this->accArray.getGeometry()); // this->getFrameWidth(), 'x', this->getFrameHeight());
	}

	const QuantityMap & qm = getQuantityMap();

	/** Determines if quality is stored in
	 *  /dataset1/quality1/
	 *  or
	 *  /dataset1/data1/quality1/
	 */
	//bool DATA_SPECIFIC_QUALITY = false;
	//mout.experimental<LOG_NOTICE>("EncodingODIM THIS = ", this->odim);

	ODIM odimData;
	drain::SmartMapTools::updateValues(odimData, this->odim); // Note: copies, to support extraction with different encodings
	// This should be unneeded...	// odimData.quantity = this->odim.quantity;
	ODIM odimQuality;
	// mout.experimental<LOG_NOTICE>("EncodingODIM SRC  = ", EncodingODIM(odimData));

	/// At extraction stage, we should know the quantity...

	// TODO: clean confusing mixture of referring to dataCoder.dataODIM <==> odimData
	DataCoder dataCoder(odimData, odimQuality); // (will use only either odim!)

	mout.attention("extracting field '", field, "' = ", static_cast<int>(field), " #", drain::EnumDict<drain::image::Accumulator::FieldType>::dict.getKey(field));

	{

		/*
		if (odimData.quantity.empty()){
			odimData.quantity = "UNKNOWN"; // ok; for example --cPlotFile carries no information on quantity
			mout.note("quantity=", odimData.quantity);
		}
		*/
		mout.debug(DRAIN_LOG(dataCoder));
		mout.debug2(DRAIN_LOG(dataCoder.dataODIM));
		mout.debug2(DRAIN_LOG(dataCoder.qualityODIM));

		/// Also available: if (type.isSet()) ...

		// Note: encoding is used only for DATA to avoid ambiguous setting for multiple request of fields: DATA,WEIGHT,COUNT, ...
		if (field == field_t::DATA){

			//pdata_dst_t & dstData = dstProduct.getData(this->odim.quantity);

			// TODO: clean confusing mixture of dataCoder.dataODIM <==> odimData


			// Update 1/2: from initial values to this "instantaneous" extraction
			drain::SmartMapTools::updateValues(odimData, this->odim); // lazy (qualityData does not need)

			mout.debug("extracting DATA/" , field, " [", odimData.quantity, ']');

			if (!encoding.empty()){
				mout.accept("User-defined encoding for data [", odimData.quantity, "]: ", encoding);
				//odimData.completeEncoding( encoding);
				odimData.completeEncoding(encoding);
			}
			else if (!getTargetEncoding().empty()){
				mout.ok<LOG_NOTICE>("Using initial/default encoding for data [", odimData.quantity, "]: ", getTargetEncoding());
				// odimData.completeEncoding( getTargetEncoding());
				odimData.completeEncoding(getTargetEncoding());
			}

			if (odimData.quantity.empty()){
				mout.error("unspecified quantity");
			}

			pdata_dst_t & dstData = dstProduct.getData(odimData.quantity);

			// Update 2/2: copy "instantaneous", maybe adjusted encoding to retrieved data
			extractFinally(*this, field, odimData, dataCoder, dstData.odim, dstData.data, cropArea);
			return dstData;
			/*
			drain::SmartMapTools::updateValues(dstData.odim, odimData);

			if (dataCoder.dataODIM.isSet()){
				dstData.data.setType(odimData.type);
				mout.debug3("dstData: " , dstData );
				this->Accumulator::extractField(field, dataCoder, dstData.data, cropArea);
				return dstData;
			}
			else {
				// Note: this is also checked by Accumulator, but better diagnostics (ODIM) here:
				mout.error("Target encoding unset: gain=", dataCoder.dataODIM.scaling.scale, ", type=", dataCoder.dataODIM.type);
				return dstData;
			}
			*/

		}
		else {

			switch (field){
			case field_t::WEIGHT_DS:
			case field_t::WEIGHT:
				odimQuality.quantity = "QIND";
				break;
			case field_t::COUNT_DS:
			case field_t::COUNT:
				odimQuality.quantity = "COUNT";
				break;
			case field_t::DEVIATION:
				odimQuality.quantity = this->odim.quantity + "DEV";
				break;
			default:
				mout.error("Unsupported field marker: ", field, "='", fieldChar, "'");
				//mout.error("Unsupported field marker: ", FieldFlagger::getKeysNEW2(field));
			}

			mout.debug("extracting QUALITY/" , field, " [", odimQuality.quantity, ']');

			if (qm.hasQuantity(odimQuality.quantity)){
				qm.setQuantityDefaults(odimQuality, odimQuality.quantity, odimQuality.type);
				mout.accept<LOG_DEBUG>("found quantityConf[", odimQuality.quantity, "], type=", odimQuality.type);
				mout.special<LOG_DEBUG>("Quality: ", EncodingODIM(odimQuality));
				mout.special<LOG_DEBUG>("Quality: ", odimQuality);
			}
			else if (!encoding.empty()){
				mout.accept<LOG_INFO>("User-defined encoding for quality [", odimQuality.quantity, "]: ", encoding);
				odimQuality.completeEncoding(encoding);
				mout.debug("User-defined encoding for QUALITY: -> ", odimQuality);
			}
			else {
				// mout.experimental("No predefined scaling for ", odimQuality.quantity);
				/*
				const std::type_info & t = drain::Type::getTypeInfo(odimQuality.type);
				odimQuality.scaling.scale *= 20.0;  // ?
				odimQuality.scaling.offset = round(drain::Type::call<drain::typeMin, double>(t) + drain::Type::call<drain::typeMax, double>(t))/2.0;
				if (encoding.empty()){
					mout.warn("quantyConf[" , odimQuality.quantity , "] not found, using somewhat arbitrary scaling:" );
					mout.special("Quality: ", EncodingODIM(odimQuality));
				}
				*/
				odimQuality.setType(typeid(float));
				mout.warn("quantyConf[" , odimQuality.quantity , "] not found, using float" );
				mout.special("Quality: ", EncodingODIM(odimQuality));

			}
			// else auto-scale?


			typedef QualityDataSupport<DstType<OD> > q_support_t;
			q_support_t & qualityOwner = Accumulator::isSpecific(field) ? (q_support_t &) dstProduct.getData(odimData.quantity) : (q_support_t &) dstProduct;
			pdata_dst_t & dstQuality = qualityOwner.getQualityData(odimQuality.quantity);


			// NOTE similarity... but fct still would need separating (dataCoder, odimData, odimQuality, odimNow, field, crop)
			extractFinally(*this, field, odimQuality, dataCoder, dstQuality.odim, dstQuality.data, cropArea);
			return dstQuality;
			/*
			drain::SmartMapTools::updateValues(dstQuality.odim, odimQuality);
			// if (dataCoder.squalityODIM.isSet()){
			if (dstQuality.odim.isSet()){
				// dstQuality.data.setType(odimData.type); // <- this was probably wrong
				dstQuality.data.setType(dstQuality.odim.type); // <- this was probably wrong
				mout.debug3("dstData: " , dstQuality );
				this->Accumulator::extractField(fieldChar, dataCoder, dstQuality.data, cropArea);
				return dstQuality;
			}
			else {
				// Note: this is also checked by Accumulator, but better diagnostics (ODIM) here:
				mout.error("Target encoding unset: gain=", dstQuality.odim.scaling.scale, ", type=", dstQuality.odim.type);
				//mout.error("Target encoding unset: gain=", dataCoder.qualityODIM.scaling.scale, ", type=", dataCoder.qualityODIM.type);
				return dstQuality;
			}
			*/

		}

		// mout.debug("updating local tree attributes");

	}


	// mout.debug("updating local tree attributes" );
	// mout.debug("finished" );

}

// TODO: remove this (use the new ones)
template  <class AC, class OD>
void RadarAccumulator<AC,OD>::extractOLD(const OD & odimOut, DataSet<DstType<OD> > & dstProduct,
		const std::string & fields, const drain::Rectangle<int> & crop) const {
	// , const drain::Rectangle<double> & bbox) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	const std::type_info & t = drain::Type::getTypeInfo(odimOut.type);

	typedef enum {DATA,QUALITY} datatype;

	OD odimData;
	odimData = odimOut;
	odimData.scaling.scale = 0.0; // ?

	const QuantityMap & qm = getQuantityMap();

	// Determines if quality is stored in
	//   /dataset1/quality1/
	// or
	//  /dataset1/data1/quality1/
	//
	bool DATA_SPECIFIC_QUALITY = false;

	// Consider redesign, with a map of objects {quantity, type,}
	for (size_t i = 0; i < fields.length(); ++i) {

		ODIM odimQuality;
		odimQuality.quantity = "QIND";

		// std::stringstream dataPath;

		datatype type = DATA;
		char field = fields.at(i);
		switch (field) {
			case '/':
				DATA_SPECIFIC_QUALITY = true;
				continue;
				break; // unneeded?
			case 'm': // ???
			case 'D':
			case 'p': // ???
				mout.warn() << "non-standard layer code; use 'd' for 'data' instead" << mout.endl;
				// no break
			case 'd':
				type = DATA;
				odimData = odimOut; // consider update
				qm.setQuantityDefaults(odimQuality, "QIND"); // ?
				odimQuality.quantity = "QIND";
				break;
			case 'c':
				type = QUALITY;
				qm.setQuantityDefaults(odimQuality, "COUNT");
				odimQuality.quantity = "COUNT";
				break;
			case 'C':
				type = QUALITY;
				qm.setQuantityDefaults(odimQuality, "COUNT", "d");
				odimQuality.quantity = "COUNT";
				break;
			// case 'q': // consider
			case 'w':
				// no break
				type = QUALITY;
				odimData = odimOut; // (Because converter needs both data and weight to encode weight?)
				qm.setQuantityDefaults(odimQuality, "QIND", "C");
				odimQuality.quantity = "QIND";
				//odimQuality.undetect = 256;
				//odimQuality.nodata = -1;  // this is good, because otherwise nearly-undetectValue-quality CAPPI areas become no-data.
				break;
			case 'W':
				mout.warn("experimental: quality [QIND] type copied from data [" , odimOut.quantity , ']' );
				// no break
				type = QUALITY;
				odimData = odimOut; // (Because converted needs both data and weight to encode weight?)
				qm.setQuantityDefaults(odimQuality, "QIND", odimOut.type);
				odimQuality.quantity = "QIND";
				//odimQuality.undetect = 256;
				//odimQuality.nodata = -1;  // this is good, because otherwise nearly-undetectValue-quality CAPPI areas become no-data.
				break;
			case 's':
				type = QUALITY;
				odimQuality = odimOut;
				odimQuality.quantity += "DEV";
				if (qm.hasQuantity(odimQuality.quantity)){
					qm.setQuantityDefaults(odimQuality, odimQuality.quantity, odimQuality.type);
					mout.accept<LOG_NOTICE>("found quantyConf[", odimQuality.quantity, "], type=", odimQuality.type);
					//mout.special("Quality: ", EncodingODIM(odimQuality));
					mout.special("Quality: ", EncodingODIM(odimQuality));
					mout.special("Quality: ", odimQuality);
				}
				else {
					odimQuality.scaling.scale *= 20.0;  // ?
					//const std::type_info & t = Type::getType(odimFinal.type);
					odimQuality.scaling.offset = round(drain::Type::call<drain::typeMin, double>(t) + drain::Type::call<drain::typeMax, double>(t))/2.0;
					//odimQuality.offset = round(drain::Type::call<drain::typeMax,double>(t) + drain::Type::getMin<double>(t))/2.0;  // same as data!
					mout.warn("quantyConf[" , odimQuality.quantity , "] not found, using somewhat arbitary scaling:" );
					mout.special("Quality: ", EncodingODIM(odimQuality));
				}
				break;
			default:
				mout.error("Unsupported field code: '", field, "'");
				break;
		}

		mout.debug("extracting field ", field);

		if (!crop.empty()){
			mout.experimental("Applying cropping: bbox=", crop, " [pix]");
		}

		if (type == DATA){
			mout.debug("target: " , EncodingODIM(odimData) );
		}
		else if (type == QUALITY){
			mout.debug("target: " , EncodingODIM(odimQuality) );
		}

		if (odimData.quantity.empty()){
			odimData.quantity = "UNKNOWN"; // ok; for example --cPlotFile carries no information on quantity
			mout.note("quantity=", odimData.quantity);
		}

		//PlainData<DstType<OD> >
		mout.debug("searching dstData... DATA=", (type == DATA));
		//pdata_dst_t & dstData = (type == DATA) ? dstProduct.getData(odimFinal.quantity) : dstProduct.getQualityData(odimQuality.quantity);
		//mout .debug3() << "dstData: " << dstData << mout.endl;

		//DataDst dstData(dataGroup); // FIXME "qualityN" instead of dataN creates: /dataset1/qualityN/quality1/data
		//mout.warn("odimFinal: " , odimFinal );
		mout.debug("odimData: " , EncodingODIM(odimData) );
		DataCoder dataCoder(odimData, odimQuality); // (will use only either odim!)
		mout.debug("dataCoder: ", dataCoder);
		mout.debug2("dataCoder: data: ", dataCoder.dataODIM);
		mout.debug2("dataCoder: qind: ", dataCoder.qualityODIM);

		if (!crop.empty()){
			mout.unimplemented("crop ",  crop, ", dstData.data resize + Accumulator::extractField ");
		}

		if (type == DATA){
			mout.debug("DATA/" , field, " [", odimData.quantity, ']');
			//mout.warn(dstData.odim );
			pdata_dst_t & dstData = dstProduct.getData(odimData.quantity);
			dstData.odim.importMap(odimData);
			dstData.data.setType(odimData.type);
			mout.debug3("dstData: " , dstData );
			//mout.debug("quantity=" , dstData.odim.quantity );
			this->Accumulator::extractField(field, dataCoder, dstData.data, crop);
		}
		else {
			mout.debug("QUALITY/" , field , " [", odimQuality.quantity, ']');
			//pdata_dst_t & dstData = dstProduct.getData(odimFinal.quantity);
			typedef QualityDataSupport<DstType<OD> > q_data_t;
			q_data_t & qualityOwner = (DATA_SPECIFIC_QUALITY) ? (q_data_t &) dstProduct.getData(odimData.quantity) : (q_data_t &) dstProduct;
			pdata_dst_t & dstData = qualityOwner.getQualityData(odimQuality.quantity);
			dstData.odim.updateFromMap(odimQuality);
			mout.debug3("dstData: " , dstData );
			this->Accumulator::extractField(field, dataCoder, dstData.data, crop);
		}


		// mout.debug("updating local tree attributes");

	}


	// mout.debug("updating local tree attributes" );
	// mout.debug("finished" );

}


}  // rack::


#endif /* RADAR_ACCUMULATOR */

// Rack
