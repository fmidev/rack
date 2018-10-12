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

#include <stdexcept>
#include "DopplerOp.h"

#include <drain/image/SegmentProber.h> // for crawler

namespace rack {




void DopplerReprojectOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name, __FUNCTION__);

	//const Data<PolarSrc> & srcData = srcSweep.getFirstData(); // VRAD or VRADH



	const PlainData<PolarSrc> & srcDataU    = srcSweep.getData("AMVU");
	if (srcDataU.data.isEmpty()){
		mout.warn() << "AMVU input missing" << mout.endl;
		return;
	}

	const PlainData<PolarSrc> & srcDataV    = srcSweep.getData("AMVV");
	if (srcDataV.data.isEmpty()){
		mout.warn() << "AMVV input missing" << mout.endl;
		return;
	}

	const PlainData<PolarSrc> & srcDataVRAD = srcSweep.getData("VRAD"); // maybe empty
	const bool VRAD_SRC = !srcDataVRAD.data.isEmpty();
	if ((matchOriginal > 0) && !VRAD_SRC){
		mout.warn() << "No VRAD input, matching will be skipped" << mout.endl;
	}
	const bool MATCH_ALIASED  = (matchOriginal & 1) && VRAD_SRC;
	const bool MASK_DATA = (matchOriginal & 2) && VRAD_SRC;

	//const PlainData<PolarSrc> & dstDataV = srcSweep.getData("AMVV");

	const bool VRAD_OVERWRITE = (dstProduct.find("VRAD") != dstProduct.end());

	if (VRAD_OVERWRITE){
		mout.warn() << "Contains already VRAD, overwriting" << mout.endl;
		//return;
	}


	PlainData<PolarDst> & dstData = dstProduct.getData(odim.quantity);



	const QuantityMap & qm = getQuantityMap();
	if (dstData.data.isEmpty()){ // or !VRAD_OVERWRITE
		ProductBase::applyODIM(dstData.odim, odim, true);
		//qm.setQuantityDefaults(dstData, "VRAD", odim.type);
		setGeometry(srcDataU.odim, dstData);
		dstData.odim.NI = odim.NI;
		dstData.odim.setRange(-odim.NI, +odim.NI);
	}
	const double minCode = dstData.data.getMin<double>();
	const double maxCode = dstData.data.getMax<double>();

	PlainData<PolarDst> & dstQuality = dstProduct.getQualityData();
	qm.setQuantityDefaults(dstQuality);
	setGeometry(dstData.odim, dstQuality);

	//const double dstNI = abs(odim.NI);
	mout.info() << "Inverting (u,v) back to VRAD " << mout.endl;
	mout.info() << "src [" << srcDataU.odim.quantity << "] " << EncodingODIM(srcDataU.odim) << mout.endl;
	mout.info() << "src [" << srcDataV.odim.quantity << "] " << EncodingODIM(srcDataV.odim) << mout.endl;
	mout.info() << "dst [" << dstData.odim.quantity << "]  " << EncodingODIM(dstData.odim) << ", [" << minCode << ',' << maxCode << ']' << mout.endl;
	mout.info() << "dst [" << dstQuality.odim.quantity << "]" << EncodingODIM(dstQuality.odim) << mout.endl;

	const double srcNI2 = 2.0*srcDataVRAD.odim.getNyquist(); // 2.0*srcData.odim.NI;

	/// Azimuth in radians
	double azmR;

	/// Original value in VRAD
	double vOrig = 0.0;
	drain::image::Point2D<double> unitVOrig;

	/// Resolved (u,v), from AMVU and AMVV
	double u, v, quality;

	/// Resolved (u,v) projected back on beam
	double vReproj;
	drain::image::Point2D<double> unitVReproj;


	bool ORIG_UNDETECT;
	bool ORIG_NODATA;
	bool ORIG_USABLE = true; // ORIG_UNDETECT && ORIG_NODATA

	size_t address;

	mout.warn() << "Main " << dstData    << mout.endl;
	mout.note() << "Main " << dstQuality << mout.endl;

	for (size_t j = 0; j < dstData.data.getHeight(); ++j) {

		azmR = dstData.odim.getBeamWidth() * static_cast<double>(j) ; // window.BEAM2RAD * static_cast<double>(j);

		for (size_t i = 0; i < dstData.data.getWidth(); ++i) {

			address = dstData.data.address(i,j);
			u = srcDataU.data.get<double>(address);
			v = srcDataV.data.get<double>(address);


			if (MASK_DATA || MATCH_ALIASED){
				vOrig = srcDataVRAD.data.get<double>(address);
				ORIG_UNDETECT = (vOrig == srcDataVRAD.odim.undetect);
				ORIG_NODATA   = (vOrig == srcDataVRAD.odim.nodata);

				ORIG_USABLE = !(ORIG_UNDETECT || ORIG_NODATA);
				if (MASK_DATA && !ORIG_USABLE){
					if (ORIG_UNDETECT)
						dstData.data.put(address, dstData.odim.undetect);
					else
						dstData.data.put(address, dstData.odim.nodata);
					dstQuality.data.put(address, 0);
					continue;
				}

			}


			if (srcDataU.odim.isValue(u) && srcDataV.odim.isValue(v)){

				u = srcDataU.odim.scaleForward(u);
				v = srcDataV.odim.scaleForward(v);
				vReproj = this->project(azmR, u,v);

				if (VRAD_SRC){
					vOrig = srcDataVRAD.odim.scaleForward(vOrig);
					srcDataVRAD.odim.mapDopplerSpeed(vOrig,     unitVOrig.x,   unitVOrig.y);
					srcDataVRAD.odim.mapDopplerSpeed(vReproj, unitVReproj.x, unitVReproj.y);
				}

				if (MATCH_ALIASED && ORIG_USABLE){
					//vReproj = srcNI2*(floor(vReproj/srcNI2+0.5)-0.5) + vOrig;
					vReproj = vOrig; //srcNI2*floor(vReproj/srcNI2) + vOrig;
				}

				vReproj = dstData.odim.scaleInverse(vReproj);
				if ((vReproj > minCode) && (vReproj < maxCode)){ // continue processing
					// dstDataVRAD.data.put(address, vReproj);
					dstData.data.put(address, vReproj);
					quality = 0.5 + 0.5*(unitVReproj.x*unitVOrig.x + unitVReproj.y*unitVOrig.y);
					/*
					if (((i+50)==j) && ((i&7)==0)){
						std::cerr << i << '\t' << unitVOrig << '\t' << unitVReproj << '\t' << quality << '\n';
					}
					*/
					dstQuality.data.put(address, dstQuality.odim.scaleInverse(quality) );
				}
				else {
					dstData.data.put(address, dstData.odim.undetect); // rand() & 0xffff); //
					dstQuality.data.put(address, 0);
				};
			}
			else {
				dstData.data.put(address, dstData.odim.undetect);
			}


		}
	}
	//@ dstDataVRAD.updateTree();


}



class DopplerSegmentProber : public drain::image::SegmentProber<double, double> {

public:

	inline
	DopplerSegmentProber(const Channel &s, Channel &d) : drain::image::SegmentProber<double, double>(s, d), relative_NI_limit(0.9) {
	}

	inline
	DopplerSegmentProber(Channel &d) : drain::image::SegmentProber<double, double>(d, d), relative_NI_limit(0.9) {
	}

	/// Update srcOdim
	virtual
	void init(){
		drain::Logger mout("DopplerSegmentProber", __FUNCTION__);

		size=0;
		srcODIM.updateFromMap(src.getProperties());
		NI_limit = relative_NI_limit * srcODIM.getNyquist(LOG_ERR);
		limit = dst->getLimiter<double>();
	}

	virtual
	void clear(){
		counter = 0;
		size = 0; //debug
	};

	double relative_NI_limit;

	/// Nyquist wrap measure
	int counter;

	//mutable
	PolarODIM srcODIM;

protected:

	/// NI divided by 2.
	double NI_limit;
	drain::typeLimiter<double>::value_t limit; //  = dstData.data.getLimiter<double>();

	/// Returns true, if value in location (i,j) is not \c nodata nor \c undetect .
	/*
	 *
	 */
	virtual inline
	bool isValidSegment(int i, int j){
		if (i < 50)
			return false;
		return srcODIM.isValue(src.get<src_t>(i,j));
	}

	/// Application dependent
	virtual inline
	bool isValidMove(int i0, int j0, int i, int j){

		double v0 = src.get<double>(i0,j0);
		double v  = src.get<double>(i,j);
		if (srcODIM.isValue(v0) && srcODIM.isValue(v)){ // cf. srcODIM.deriveDifference()
			v0 = srcODIM.scaleForward(v0);
			v  = srcODIM.scaleForward(v);
			// Detect overflow (aliasing)
			if ((v0>+NI_limit) && (v<-NI_limit)){
				++counter;
			}
			else if ((v0<-NI_limit) && (v>+NI_limit)){
				--counter;
			}
			return true;
		}
		else
			return false;
	}


	virtual inline
	void visit(int i, int j){

		++size;
		if ((size % 100) == 0){
			std::cerr << "size = " << size << '\n';
		}
		// No need to call update(), updating is already done by  isValidMove()
		double x = //static_cast<double>(counter) * 2.0 * srcODIM.NI +
				srcODIM.scaleForward(src.get<double>(i,j));
		x = limit(dst->getScaling().fwd(x));
		if (x == 0.0)
			x = 1.0;
		//dst->put(i,j, x);
		//dst->putScaled(i, j, x);
		// value = counter; // TODO scaled counter+xx
		dst->put(i,j, counter | 1);
		// dst->put(i,j, rand() & 0xffff);
	}


};


void DopplerCrawlerOp::processData(const Data<src_t > & srcData, Data<dst_t > & dstData) const {

	drain::Logger mout(name, __FUNCTION__);

	ProductBase::applyODIM(dstData.odim, odim, true);
	dstData.data.setScaling(dstData.odim.gain, dstData.odim.offset);

	DopplerSegmentProber prober(srcData.data, dstData.data);
	prober.init();

	mout.warn() << "src: " << srcData << mout.endl;
	mout.warn() << "dst: " << dstData << mout.endl;

	double x;
	for (size_t i=0; i<srcData.data.getWidth(); ++i){

		for (size_t j=0; j<srcData.data.getHeight(); ++j){

			x = srcData.data.get<double>(i,j);
			if (prober.srcODIM.isValue(x)){

				prober.probe(i,j,1);
			}

		}

	}


}


/*
void DopplerModulatorOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(name, __FUNCTION__);

	dstData.initialize(typeid(unsigned char), srcData.data.getGeometry());
	dstData.setPhysicalRange(-1, +1);

	const double NI = srcData.odim.getNyquist();
	if (NI == 0){
		mout.error() << "NI=0" << mout.endl;
		return;
	}

	const double W  = M_PI/NI;

	const double orderD = static_cast<double>(order);
	double sinBeam, cosBeam;

	PlainData<PolarDst> & dstU1 = dstData.getQualityData("UC");
	dstU1.setPhysicalRange(-1, +1);
	PlainData<PolarDst> & dstV1 = dstData.getQualityData("VC");
	dstV1.setPhysicalRange(-1, +1);
	PlainData<PolarDst> & dstU2 = dstData.getQualityData("US");
	dstU2.setPhysicalRange(-1, +1);
	PlainData<PolarDst> & dstV2 = dstData.getQualityData("VS");
	dstV2.setPhysicalRange(-1, +1);

	drain::image::AreaGeometry g(srcData.data.getWidth(), size_t(1));
	dstU1.data.setGeometry(g);
	dstU2.data.setGeometry(g);
	dstV1.data.setGeometry(g);
	dstV2.data.setGeometry(g);

	double u1,u2,v1,v2;

	double vrad, w;

	unsigned int count;

	for (size_t i=0; i<srcData.data.getWidth(); i++){

		count = 0;
		u1=u2=v1=v2 = 0;


		for (size_t j=0; j<srcData.data.getHeight(); j++){

			sinBeam = sin(orderD * srcData.odim.getAzimuth(j));
			cosBeam = cos(orderD * srcData.odim.getAzimuth(j));

			vrad = srcData.data.get<double>(i,j);
			if (srcData.odim.isValue(vrad)){

				vrad = srcData.odim.scaleForward(srcData.data.get<double>(i,j));
				w = vrad*W;

				u1 += cos(w) * cosBeam;
				v1 += cos(w) * sinBeam;
				u2 += sin(w) * cosBeam;
				v2 += sin(w) * sinBeam;

				dstData.data.putScaled(i, j, cos(w) * cosBeam );

				++count;

			}


		}

		dstU1.data.putScaled(i, 0, u1 / static_cast<double>(count) );
		dstV1.data.putScaled(i, 0, v1 / static_cast<double>(count) );
		dstU2.data.putScaled(i, 0, u2 / static_cast<double>(count) );
		dstV2.data.putScaled(i, 0, v2 / static_cast<double>(count) );

	}


}
*/

}  // rack::

// Rack
