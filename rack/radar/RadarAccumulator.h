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


#include <drain/image/Sampler.h>
#include <drain/util/Proj4.h>
#include <drain/image/AccumulatorGeo.h>
#include "data/ODIM.h"
#include "data/Data.h"
#include "data/DataCoder.h"
#include "Geometry.h"
#include "product/ProductOp.h"

// // using namespace std;

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
	RadarAccumulator() : defaultQuality(0.5) { //, undetectValue(-52.0) {
		odim.type.clear();
		//odim.ACCnum = 0;
	}

	virtual
	~RadarAccumulator(){};

	/// Adds data that is in the same coordinate system as the accumulator.
	/*
	 *  Both the input data and the accumulation array are in the same coordinate system.
	 */
	void addData(const pdata_src_t & srcData, const pdata_src_t & srcQuality, double weight, int i0, int j0);

	void extract(const OD & odimOut, DataSet<DstType<OD> > & dstProduct, const std::string & quantities) const;

	/// Input data selector.
	DataSelector dataSelector;

	/// For storing the scaling and encoding of (1st) input or user-defined values. Also for bookkeeping of date, time, sources etc.
	/*
	 *  Helps in warning if input data does not match the expected / potential targetEncoding
	 */
	OD odim;

	/// Book keeping for new data. Finally, in extraction phase, added to odid.ACCnum .
	// size_t count;

	/// If source data has no quality field, this value is applied for (detected) data.
	double defaultQuality;

	/// Not critical. If set, needed to warn if input data does not match the expected / potential targetEncoding
	inline
	void setTargetEncoding(const std::string & encoding){
		targetEncoding = encoding;
	}

	inline
	const std::string & getTargetEncoding(){
		return targetEncoding;
	}

	virtual
	inline
	std::ostream & toStream(std::ostream & ostr) const {
		this->AC::toStream(ostr);
		ostr << ' ' << rack::EncodingODIM(odim);
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
		//mout.debug() << converter.toStr() << mout.endl;
		//converter.undetectValue   = undetectValue;
		// uses also DataCoder::undetectQualityCoeff
		AC::addData(srcData.data, srcQuality.data, converter, weight, i0, j0);
	}
	else {
		mout.info() << "No quality data available with input; using unweighted compositing." << mout.endl;
		//mout.warn() << cartSrc << mout.endl;
		ODIM qualityOdim;
		getQuantityMap().setQuantityDefaults(qualityOdim, "QIND");

		DataCoder converter(srcData.odim, qualityOdim);
		//converter.undetectValue   = undetectValue;
		// uses also DataCoder::undetectQualityCoeff
		AC::addData(srcData.data, converter, weight, i0, j0);
	}

	//++odim.ACCnum;
	odim.update(srcData.odim); // Time, date, new
	// quantity?

	//mout.note() << "before: " << this->odim << mout.endl;
	//mout.note() << "after:  " << this->odim << mout.endl;

}



template  <class AC, class OD>
bool RadarAccumulator<AC,OD>::checkCompositingMethod(const ODIM & dataODIM) const {

	drain::Logger mout("RadarAccumulator", __FUNCTION__);

	mout.debug() << "start, quantity=" << dataODIM.quantity << mout.endl;

	const AccumulationMethod & rule = this->getMethod();
	if (rule.name == "WAVG"){

		const drain::ReferenceMap & wavgParams = rule.getParameters();
		const double p = wavgParams.get("p", 1.0);
		const double p2 = p/2.0;
		mout.info() << rule << ", min="<< dataODIM.getMin() << mout.endl;

		if ((p2 - floor(p2)) == 0.0){
			mout.info() << "WAVG with p=" << p << " = 2N, positive pow() results" << mout.endl;
			const Quantity & q = getQuantityMap().get(dataODIM.quantity);
			if (q.hasUndetectValue){
				const double bias = wavgParams.get("bias", 0.0);
				if (bias > q.undetectValue){
					mout.warn() << "WAVG with p=" << p <<" = 2N, undetectValue=" << q.undetectValue << " < bias="<< bias  << mout.endl;
					mout.warn() << "consider adjusting bias("<< bias <<") down to quantity (" << dataODIM.quantity << ") zero: " << q.undetectValue << mout.endl;
					//rule.setParameter("bias", q.undetectValue);
					return true;
				}
			}
		}
	}

	return false;


}



template  <class AC, class OD>
void RadarAccumulator<AC,OD>::extract(const OD & odimOut, DataSet<DstType<OD> > & dstProduct, const std::string & quantities) const {


	drain::Logger mout("RadarAccumulator", __FUNCTION__);
	// mout.warn() << "is root?\n" << dst << mout.endl;

	const std::type_info & t = drain::Type::getTypeInfo(odimOut.type);

	typedef enum {DATA,QUALITY} datatype;
	int dCounter=0;
	int qCounter=0;

	OD odimFinal;
	odimFinal = odimOut;
	odimFinal.gain = 0.0; // ?

	const QuantityMap & qm = getQuantityMap();

	for (size_t i = 0; i < quantities.length(); ++i) {

		ODIM odimQuality;
		odimQuality.quantity = "QIND";

		std::stringstream dataPath;

		datatype type = DATA;
		char field = quantities.at(i);
		switch (field) {
			case 'm': // ???
			case 'D':
			case 'p': // ???
				mout.warn() << "non-standard layer code; use 'd' for 'data' instead" << mout.endl;
				// no break
			case 'd':
				type = DATA;
				odimFinal = odimOut; // consider update
				// odimQuality.setQuantityDefaults("QIND");
				qm.setQuantityDefaults(odimQuality, "QIND");  // note: will not SET quantity !
				odimQuality.quantity = "QIND";
				break;
			case 'C':
				mout.warn() << "non-standard layer code; use 'c' for 'count' instead" << mout.endl;
				// no break
			case 'c':
				type = QUALITY;
				// odimQuality.setQuantityDefaults("COUNT", "C"); ok
				odimQuality.type = drain::Type::getTypeChar(typeid(unsigned char));
				odimQuality.gain   = 1;
				odimQuality.offset = 0;
				odimQuality.undetect = 256;
				odimQuality.nodata = -1;
				odimQuality.quantity = "COUNT";
				break;
			case 'w':
				// no break
				type = QUALITY;
				odimFinal = odimOut; // WHY?
				//odimQuality.setQuantityDefaults("QIND", "C");
				qm.setQuantityDefaults(odimQuality, "QIND", "C");
				odimQuality.undetect = 256;
				odimQuality.nodata = -1;  // this is good, because otherwise nearly-undetectValue-quality CAPPI areas become no-data.
				break;
			case 's':
				type = QUALITY;
				odimQuality = odimOut;
				odimQuality.quantity += "DEV";
				odimQuality.gain *= 20.0;  // ?
				//const std::type_info & t = Type::getType(odimFinal.type);
				odimQuality.offset = round(drain::Type::call<drain::typeMin, double>(t) + drain::Type::call<drain::typeMax, double>(t))/2.0;
				//odimQuality.offset = round(drain::Type::call<drain::typeMax,double>(t) + drain::Type::getMin<double>(t))/2.0;  // same as data!
				break;
			default:
				mout.error() << "Unsupported field code: '" << field << "'" << mout.endl;
				break;
		}

		mout.info() << "extracting field " << field << mout.endl;


		if (type == DATA){
			dataPath << "data" << ++dCounter;
			mout.debug() << "target: " << EncodingODIM(odimFinal) << mout.endl;
		}
		else if (type == QUALITY){
			dataPath << "quality" << ++qCounter;
			mout.debug() << "target: " << EncodingODIM(odimQuality) << mout.endl;
		}


		if (odimFinal.quantity.empty()){
			odimFinal.quantity = "UNKNOWN"; // for example --cPlotFile carries no information on quantity
			mout.note() << "quantity=" << odimFinal.quantity << mout.endl;
		}

		//PlainData<DstType<OD> >
		pdata_dst_t & dstData = (type == DATA) ? dstProduct.getData(odimFinal.quantity) : dstProduct.getQualityData(odimQuality.quantity);

		//DataDst dstData(dataGroup); // FIXME "qualityN" instead of dataN creates: /dataset1/qualityN/quality1/data
		//mout.warn() << "odimFinal: " << odimFinal << mout.endl;

		if (type == DATA){
			mout.debug() << "DATA/" << field << mout.endl;
			//mout.warn() << dstData.odim << mout.endl;
			//if (!dstData.data.typeIsSet())
			  //dstData.data.setType(t);
			dstData.odim.importMap(odimFinal);
			dstData.data.setType(odimFinal.type);
			//mout.debug()  << "quantity=" << dstData.odim.quantity << mout.endl;
		}
		else {
			mout.debug() << "QUALITY/" << field << mout.endl;
			dstData.odim.updateFromMap(odimQuality);
			//dstData.odim.importMap(odimQuality);
		}

		DataCoder converter(odimFinal, odimQuality); // (will use only either odim!)

		mout.debug()  << "converter: " << converter.toStr() << mout.endl;

		this->Accumulator::extractField(field, converter, dstData.data);

		//mout.debug()  << "dstData: " << dstData.odim << mout.endl;
		mout.debug()  << "updating local tree attributes" << mout.endl;
		//hi5::Writer::writeFile("test0.h5", dstProduct.tree);
		//@= dstData.updateTree();
		//std::cerr << __FUNCTION__ << ':' << dstData.tree << std::endl;
		//hi5::Writer::writeFile("test1.h5", dstProduct.tree);


	}

	//updateGeoData()

	mout.debug()  << "updating local tree attributes" << mout.endl;
	//odimFinal.
	//@= dstProduct.updateTree(odimFinal);


	//mout.debug() << "Finished " << accumulator.getMethod().name << mout.endl;
	mout.debug() << "finished" << mout.endl;

}


}  // rack::


#endif /* RADAR_DATA_PICKER_H */

// Rack
