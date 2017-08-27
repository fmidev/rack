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
// Algorithm is based on the study made by Brandon Hickman from The University Of Helsinki

#include <drain/util/Fuzzy.h>
//#include <drain/image/SlidingWindowMedianOp.h>
#include <drain/image/File.h>

//#include "hi5/Hi5Write.h"

#include "data/ODIM.h"
#include "data/Data.h"
#include "radar/Geometry.h"
#include "radar/Precipitation.h"


#include "RainRateDPSimpleOp.h"




using namespace drain::image;

namespace rack {




void RainRateDPSimpleOp::processDataSet(const DataSetSrc<PolarSrc> & sweepSrc, DataSetDst<PolarDst> & dstProduct) const {

	/// For monitoring cmd execution as text dump. Use --debug (or \c --verbose \c \<level\> to define verbosity.
	drain::MonitorSource mout(name, __FUNCTION__);
	mout.info() << "start" <<  mout.endl;


	/// Retrieve dBZ data
	const Data<PolarSrc> & dbzSrc = sweepSrc.getData("DBZH");
	const bool DBZ = !dbzSrc.data.isEmpty();  // or: || dbzParams.empty() ?
	if (!DBZ)
		mout.warn() << "DBZH missing" <<  mout.endl;
	else {
		mout.debug() << "dBZ params:" << dbzParams;
		mout.debug() << dbzSrc.odim <<  mout.endl;

	}

	/// Retrieve Hydro class data
	const Data<PolarSrc> & hclassSrc = sweepSrc.getData("HCLASS");
	const bool HCLASS = !hclassSrc.data.isEmpty();
	if (!HCLASS)
		mout.warn() << "HCLASS missing" <<  mout.endl;
	else {
		//mout.debug() << "HCLASS params:" << HCLASSParams;
		mout.debug() << hclassSrc.odim <<  mout.endl;
	}

	/// Retrieve KDP data
	const Data<PolarSrc> &  kdpSrc = sweepSrc.getData("KDP");
	const bool KDP = !kdpSrc.data.isEmpty();
	if (!KDP)
		mout.warn() << "KDP missing" <<  mout.endl;
	else {
		mout.debug() << kdpSrc.odim<< mout.endl;
	}

	/// Retrieve RhoHV data
	const Data<PolarSrc> & rhohvSrc = sweepSrc.getData("RHOHV");
	const bool RHOHV = !rhohvSrc.data.isEmpty();
	if (!RHOHV)
		mout.warn() << "RHOHV missing" << mout.endl;
	else {
		mout.debug() << rhohvSrc.odim << mout.endl;
	}

	// Parse data thresholds
	drain::ReferenceMap dataThreshodParamsV;
	double rhohvThr;
	double dbzHailThr;
	double dbzHeavyThr;
	double kdpHeavyThr;
	dataThreshodParamsV.reference("rhohvThr", rhohvThr = 0.85);
	dataThreshodParamsV.reference("dbzHailThr", dbzHailThr = 50.0);
	dataThreshodParamsV.reference("dbzHeavyThr", dbzHeavyThr = 20.0);
	dataThreshodParamsV.reference("kdpHeavyThr", kdpHeavyThr = 0.2);
	dataThreshodParamsV.setValues(dataThreshodParams);   // MP/ muutin semantiikkaa, Joonas - tarkista toiminta...
	mout.debug() << "Data thr. coeff:" << dataThreshodParamsV << mout.endl;

	const int nbins = dbzSrc.data.getWidth();
	const int nrays = dbzSrc.data.getHeight();


	// Initialize result data.
	Data<PolarDst> & rate = dstProduct.getData(odim.quantity);  // "RATE"

	/// Assign target metadata
	/*
	setEncoding(dbzSrc.odim, rate);
	rate.odim.prodpar = getParameters().getValues();
	rate.odim.nbins = nbins;
	rate.odim.nrays = nrays;
	rate.odim.rscale = dbzSrc.odim.rscale;
	rate.data.setType(rate.odim.type.at(0));
	rate.data.setGeometry(nbins, nrays);
	rate.updateTree();
	*/
    //
	//## Data quality
	PlainData<PolarDst> & rateQuality = rate.getQualityData("QIND");
	//rateQuality.setQuantityDefaults("QIND");
	rateQuality.data.setGeometry(nbins, nrays);
	rateQuality.updateTree();

	const double maxQuality = rateQuality.odim.scaleInverse(1.0);
	//const double undetectQuality = 0.75 * maxQuality; // MP/ tuli unused warning

	//Probability of freezing
	//double freeze = sweepSrc.
	double p = 0.0;
	drain::FuzzyStep<double> freezingProbability(1000.0*freezingLevel, 1000.0*(freezingLevel+freezingLevelThickness));
	drain::FuzzyBell<double> precipQuality(0.0, 10.0, maxQuality); // QPE based on DBZH
	const bool SCAN = (dbzSrc.odim.product == "SCAN");

	// bin altitudes
	double height = 0.0;
	double distance = 0.0;

	// Distance to the bin along the beam (in metres).
	double beam;
	const double elangleR = dbzSrc.odim.elangle * rack::DEG2RAD;
	//## Data quality

	double dbz;
	double rhohv;
	double kdp;
	int	hclass;

	PrecipitationZ dbzP;
	PrecipitationKDP kdpP;

	dbzP.setParameters(dbzParams);
	kdpP.setParameters(kdpParams);

	bool dbzIsValid;
	bool kdpIsValid;
	bool rhohvIsValid;
	bool hclassIsValid;

	double r; // mm/h
	double quality; // quality 0-1
	for (int i = 0; i < nbins; ++i) {
		if (SCAN){
			beam = static_cast<double>(i)*dbzSrc.odim.rscale + dbzSrc.odim.rscale/2.0 +  dbzSrc.odim.rstart;
			height = Geometry::heightFromEtaBeam(elangleR, beam ) - dbzSrc.odim.height;
			distance = Geometry::groundFromEtaBeam(elangleR, beam);
			p = freezingProbability(height);
			if ((i&15) == 0)
				mout.debug() << beam << "m, h=" << height << " p=" << p << mout.endl;
		}
		for (int j =0; j < nrays; ++j) {
			r = -1.0;
			quality = 0.0;
			if (RHOHV){
				rhohv = rhohvSrc.data.get<double>(i, j);

				if (rhohv == rhohvSrc.odim.undetect) {
					rhohvIsValid = false;
				}
				else if (rhohv == rhohvSrc.odim.nodata) {
					rhohvIsValid = false;
				}
				else {
					rhohv = rhohvSrc.odim.scaleForward(rhohv);
					rhohvIsValid = true;
				}
			}
			else{
				rhohvIsValid = false;
			}


			if (DBZ){

				dbz = dbzSrc.data.get<double>(i,j);

				if (dbz == dbzSrc.odim.undetect) {
					dbzIsValid = false;
				}
				else if (dbz == dbzSrc.odim.nodata) {
					dbzIsValid = false;
				}
				else {
					dbz = dbzSrc.odim.scaleForward(dbz);
					dbzIsValid = true;
				}
			}
			else
				dbzIsValid = false;

			if (KDP) {
				kdp = kdpSrc.data.get<double>(i,j);
				if (kdp == kdpSrc.odim.undetect) {
					kdpIsValid = false;
				}
				else if (kdp == kdpSrc.odim.nodata) {
					kdpIsValid = false;
				}
				else {
					kdp = kdpSrc.odim.scaleForward(kdp);
					kdpIsValid = true;
				}
			}
			else{
				kdpIsValid = false;
			}

			if (HCLASS) {
				hclass = hclassSrc.data.get<int>(i,j);
				if (hclass == hclassSrc.odim.undetect) {
					hclassIsValid = false;
				}
				else if (hclass == hclassSrc.odim.nodata) {
					hclassIsValid = false;
				}
				else {
					hclass = hclassSrc.odim.scaleForward(hclass);
					hclassIsValid = true;
				}
			}
			else{
				hclassIsValid = false;
			}



			// Rain rate calculation method based on the one studied by Brandon Hickman
			// Limiting conditions for the algorithm:
			// 1. Distance from the radar must be atleast 10 km
			// 2. Dual. pol. rain rate equations can only be applied to liquid water

			if (DBZ && KDP){ //This if statement is needed only to separate the actual algorithm from the debug algorithms that use only one rr method
				// Non meteorological echo filtering using rhohv

				if (distance > 10000){ // Algorithm shouldn't be used within 10 km radius from the radar
					if (p < 0.75) { // Algorithm should only be used below the freezing level
						if (rhohvIsValid && (rhohv < rhohvThr)){
							r = 0.0;
						}
						else if(dbzIsValid){
							if (hclassIsValid && hclass == 2) { // hydroclass shows water
								if (kdpIsValid) {
									if (dbz > dbzHailThr){
										r = kdpP.rainRate(kdp);
									}
									else{
										if ((dbz > dbzHeavyThr) && (kdp > kdpHeavyThr)){
											r = kdpP.rainRate(kdp);
										}
										else{
											r = dbzP.rainRate(dbz); //, p);
											quality = precipQuality(r);
										}
									}
								}
							}
							else{
								r = dbzP.rainRate(dbz); //, p);
								quality = precipQuality(r);
							}
						}
					}
					else{
						//If freezing probability is higher than threshold
						r = dbzP.rainRate(dbz); //, p);
						quality = precipQuality(r);
					}
				}
				else
					r = dbzP.rainRate(dbz); //, p);
					quality = precipQuality(r);
				}
			if(r < 50.0){
				rate.data.put(i,j, rate.odim.undetect/2);
				rateQuality.data.put(i, j, 0);
			}
			else if (r > 10000.0){
				mout.debug() << r << mout.endl;
				rate.data.put(i,j, rate.odim.nodata);
				rateQuality.data.put(i, j, 0);
			}
			else {
				// TODO: check bounds (and use nodata again?)
				mout.debug() << r << mout.endl;
				rate.data.put(i,j, rate.odim.scaleInverse(r));  // maybe some day: rate.putScaled(r)
				//quality = precipQuality(r);
				rateQuality.data.put(i, j, quality);
			}
		}
	}

}



}

// Rack
