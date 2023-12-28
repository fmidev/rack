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


#include "drain/image/Sampler.h"
//#include "drain/util/Proj4.h"
#include "drain/image/AccumulatorGeo.h"
#include "data/ODIM.h"
#include "data/ODIMPath.h"
#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/DataCoder.h"
#include "data/QuantityMap.h"
#include "Geometry.h"
#include "product/ProductOp.h"


namespace rack {

/// Data array for creating composites and accumulated polar products (Surface rain fall or cluttermaps)
/**

    \tparam AC - accumulator type (plain drain::image::Accumulator or drain::image::AccumulatorGeo)
    \tparam OD - odim type (PolarODIM or CartesianODIM) metadata container

 */
template <class AC, class OD>
class RadarAccumulator : public AC {

public:

	/// Input data type
	typedef PlainData<SrcType<OD const> > pdata_src_t;
	typedef PlainData<DstType<OD> >       pdata_dst_t;

	/// Default constructor
	RadarAccumulator() : dataSelector(ODIMPathElem::DATA|ODIMPathElem::QUALITY), defaultQuality(0.5), counter(0) { //, undetectValue(-52.0) {
		odim.type.clear();
		//odim.ACCnum = 0;
	}

	virtual
	~RadarAccumulator(){};

	/// Adds data that is in the same coordinate system as the accumulator. Weighted with quality.
	/*
	 *  Both the input data and the accumulation array are in the same coordinate system.
	 */
	void addData(const pdata_src_t & srcData, const pdata_src_t & srcQuality, double weight, int i0, int j0);

	/// Adds data that is in the same coordinate system as the accumulator. Weighted with quality and count.
	void addData(const pdata_src_t & srcData, const pdata_src_t & srcQuality, const pdata_src_t & srcCount);

	/**
	 *  \param odimOut - metadata container (PolarODIM or CartesianODIM)
	 *  \param quantities - layers (d=data, w=weight, ...)
	 */
	void extract(const OD & odimOut, DataSet<DstType<OD> > & dstProduct,
			const std::string & quantities, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

	/// Input data selector.
	DataSelector dataSelector;
	// DataSelector dataSelector(ODIMPathElem::DATA);
	// dataSelector.pathMatcher.setElems(ODIMPathElem::DATA);


	/// For storing the scaling and encoding of (1st) input or user-defined values. Also for bookkeeping of date, time, sources etc.
	/*
	 *  Helps in warning if input data does not match the expected / potential targetEncoding
	 */
	OD odim;

	/// If source data has no quality field, this value is applied for (detected) data.
	double defaultQuality;

	/// Book keeping for new data. Finally, in extraction phase, added to odid.ACCnum .
	size_t counter;

	/// Not critical. If set, needed to warn if input data does not match the expected / potential targetEncoding
	inline
	void setTargetEncoding(const std::string & encoding){
		targetEncoding = encoding;
		if (odim.quantity.empty()){
			//drain::ReferenceMap m;
			ODIM m;
			m.link("what:quantity", odim.quantity); // appends
			m.addShortKeys();
			m.updateValues(encoding);
		}
		//odim.setValues(encoding); // experimental
	}

	inline
	void consumeTargetEncoding(std::string & encoding){
		if (!encoding.empty()){
			targetEncoding = encoding;
			if (odim.quantity.empty()){
				//drain::ReferenceMap m;
				ODIM m;
				m.link("what:quantity", odim.quantity); // appends
				m.addShortKeys();
				m.updateValues(encoding);
			}
		}
		encoding.clear();
	}


	inline
	const std::string & getTargetEncoding(){
		return targetEncoding;
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
	bool checkCompositingMethod(const ODIM & srcODIM) const;


protected:

	std::string targetEncoding;

};


template  <class AC, class OD>
void RadarAccumulator<AC,OD>::addData(const pdata_src_t & srcData, const pdata_src_t & srcQuality, double weight, int i0, int j0){

	drain::Logger mout("RadarAccumulator", __FUNCTION__);

	if (!srcQuality.data.isEmpty()){
		mout.info() << "Quality data available with input; using quality as weights in compositing." << mout.endl;
		DataCoder converter(srcData.odim, srcQuality.odim);
		// uses also DataCoder::undetectQualityCoeff
		AC::addData(srcData.data, srcQuality.data, converter, weight, i0, j0);
	}
	else {
		mout.info("No quality data available with input, ok." );
		ODIM qualityOdim;
		getQuantityMap().setQuantityDefaults(qualityOdim, "QIND");

		DataCoder converter(srcData.odim, qualityOdim);
		// uses also DataCoder::undetectQualityCoeff
		AC::addData(srcData.data, converter, weight, i0, j0);
	}

	counter += std::max(1L, srcData.odim.ACCnum);
	odim.updateLenient(srcData.odim); // Time, date, new

	//mout.note("after:  " , this->odim );

}

template  <class AC, class OD>
void RadarAccumulator<AC,OD>::addData(const pdata_src_t & srcData, const pdata_src_t & srcQuality, const pdata_src_t & srcCount){

	drain::Logger mout(__FILE__, __FUNCTION__);
	//mout.info() << "Quality data available with input; using quality as weights in compositing." << mout.endl;
	DataCoder converter(srcData.odim, srcQuality.odim);
	AC::addData(srcData.data, srcQuality.data, srcCount.data, converter);
	odim.updateLenient(srcData.odim); // Time, date, new
	counter = std::max(1L, srcData.odim.ACCnum);
}


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



template  <class AC, class OD>
void RadarAccumulator<AC,OD>::extract(const OD & odimOut, DataSet<DstType<OD> > & dstProduct,
		const std::string & quantities, const drain::Rectangle<int> & crop) const {
	// , const drain::Rectangle<double> & bbox) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	const std::type_info & t = drain::Type::getTypeInfo(odimOut.type);

	typedef enum {DATA,QUALITY} datatype;
	int dCounter=0;
	int qCounter=0;

	OD odimFinal;
	odimFinal = odimOut;
	odimFinal.scaling.scale = 0.0; // ?

	const QuantityMap & qm = getQuantityMap();

	/** Determines if quality is stored in
	 *  /dataset1/quality1/
	 *  /dataset1/data1/quality1/
	 */
	bool DATA_SPECIFIC_QUALITY = false;

	// Consider redesign, with a map of objects {quantity, type,}
	for (size_t i = 0; i < quantities.length(); ++i) {

		ODIM odimQuality;
		odimQuality.quantity = "QIND";

		std::stringstream dataPath;

		datatype type = DATA;
		char field = quantities.at(i);
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
				odimFinal = odimOut; // consider update
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
			case 'w':
				// no break
				type = QUALITY;
				odimFinal = odimOut; // (Because converted needs both data and weight to encode weight?)
				qm.setQuantityDefaults(odimQuality, "QIND", "C");
				odimQuality.quantity = "QIND";
				//odimQuality.undetect = 256;
				//odimQuality.nodata = -1;  // this is good, because otherwise nearly-undetectValue-quality CAPPI areas become no-data.
				break;
			case 'W':
				mout.warn("experimental: quality [QIND] type copied from data [" , odimOut.quantity , ']' );
				// no break
				type = QUALITY;
				odimFinal = odimOut; // (Because converted needs both data and weight to encode weight?)
				qm.setQuantityDefaults(odimQuality, "QIND", odimOut.type);
				odimQuality.quantity = "QIND";
				//odimQuality.undetect = 256;
				//odimQuality.nodata = -1;  // this is good, because otherwise nearly-undetectValue-quality CAPPI areas become no-data.
				break;
			case 's':
				type = QUALITY;
				odimQuality = odimOut;
				odimQuality.quantity += "DEV";
				//const QuantityMap & qm = getQuantityMap();
				if (qm.hasQuantity(odimQuality.quantity)){
					qm.setQuantityDefaults(odimQuality, odimQuality.quantity, odimQuality.type);
					mout.note("found quantyConf[", odimQuality.quantity, "], OK");
					mout.special("Quality: ", EncodingODIM(odimQuality));
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
			dataPath << "data" << ++dCounter;
			mout.debug("target: " , EncodingODIM(odimFinal) );
		}
		else if (type == QUALITY){
			dataPath << "quality" << ++qCounter;
			mout.debug("target: " , EncodingODIM(odimQuality) );
		}


		if (odimFinal.quantity.empty()){
			odimFinal.quantity = "UNKNOWN"; // for example --cPlotFile carries no information on quantity
			mout.note("quantity=", odimFinal.quantity);
		}

		//PlainData<DstType<OD> >
		mout.debug("searching dstData... DATA=", (type == DATA));
		//pdata_dst_t & dstData = (type == DATA) ? dstProduct.getData(odimFinal.quantity) : dstProduct.getQualityData(odimQuality.quantity);
		//mout .debug3() << "dstData: " << dstData << mout.endl;

		//DataDst dstData(dataGroup); // FIXME "qualityN" instead of dataN creates: /dataset1/qualityN/quality1/data
		//mout.warn("odimFinal: " , odimFinal );
		DataCoder dataCoder(odimFinal, odimQuality); // (will use only either odim!)
		mout.debug("dataCoder: ", dataCoder);
		mout.debug2("dataCoder: data: ", dataCoder.dataODIM);
		mout.debug2("dataCoder: qind: ", dataCoder.qualityODIM);

		if (!crop.empty()){
			mout.unimplemented("dstData.data resize + Accumulator::extractField");
		}
		//dstData.data

		if (type == DATA){

			mout.debug("DATA/" , field );
			//mout.warn(dstData.odim );
			pdata_dst_t & dstData = dstProduct.getData(odimFinal.quantity);
			dstData.odim.importMap(odimFinal);
			dstData.data.setType(odimFinal.type);
			mout.debug3("dstData: " , dstData );
			//mout.debug("quantity=" , dstData.odim.quantity );
			this->Accumulator::extractField(field, dataCoder, dstData.data, crop);
		}
		else {
			mout.debug("QUALITY/" , field );
			//pdata_dst_t & dstData = dstProduct.getData(odimFinal.quantity);
			typedef QualityDataSupport<DstType<OD> > q_data_t;
			q_data_t & qualityOwner = (DATA_SPECIFIC_QUALITY) ? (q_data_t &) dstProduct.getData(odimFinal.quantity) : (q_data_t &) dstProduct;
			pdata_dst_t & dstData = qualityOwner.getQualityData(odimQuality.quantity);
			dstData.odim.updateFromMap(odimQuality);
			mout.debug3("dstData: " , dstData );
			//dstData.odim.importMap(odimQuality);
			this->Accumulator::extractField(field, dataCoder, dstData.data, crop);
		}


		// mout.debug("updating local tree attributes");

	}


	//mout.debug("updating local tree attributes" );
	// mout.debug("finished" );

}


}  // rack::


#endif /* RADAR_ACCUMULATOR */

// Rack
