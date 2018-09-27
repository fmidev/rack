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

namespace rack {




/*
void DopplerOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name, __FUNCTION__);

	const Data<PolarSrc > & srcData = srcSweep.getData("VRAD");

	if (srcData.data.isEmpty()){
		mout.warn() << "data empty" << mout.endl;
		return;
	}
	//setEncoding(srcData.odim, dstData.odim);


	w.adjustIndices(srcData.odim);
	if (w.ray2 < w.ray1){
		w.ray2 += 360;
	}
	mout.warn() << w.ray1 << '-' << w.ray2 << mout.endl;

	// mout.warn() << "ray=" << w.ray1 << ',' << w.ray2 << ", bins=" << w.bin1 << ',' << w.bin2 << mout.endl;

	size_t count = (w.ray2-w.ray1) * (w.bin2-w.bin1);
	//mout.warn() << "size " << count << mout.endl;

	PlainData<PolarDst> & dstDataU = dstProduct.getData("X");
	PlainData<PolarDst> & dstDataV = dstProduct.getData("Y");

	//const QuantityMap & qm = getQuantityMap();
	//qm.setTypeDefaults(dstDataU, "d"); //typeid(double));
	//dstDataU.setTypeDefaults("d"); //typeid(double));
	dstDataU.setEncoding(typeid(double));
	//
	dstDataU.data.setGeometry(count, 1);
	dstDataU.odim.quantity = "X"; // ???
	//dstDataU.odim.gain = 1.0;
	dstDataU.data.fill(dstDataU.odim.undetect);
	//initDst(srcData, dstDataU);
	//
	//qm.setTypeDefaults(dstDataV, "d"); //typeid(double));
	//dstDataV.setTypeDefaults("d"); //typeid(double));
	dstDataV.setEncoding(typeid(double));
	dstDataV.data.setGeometry(count, 1);
	dstDataV.odim.quantity = "Y";
	//dstDataV.odim.gain = 1.0;
	dstDataV.data.fill(dstDataV.odim.undetect);
	//initDst(srcData, dstDataV);

	//@ dstDataU.updateTree();
	//@ dstDataV.updateTree();

	//@? dstProduct.updateTree(odim);

	mout.debug() << '\t' << dstDataU.data.getGeometry() << mout.endl;
	mout.debug() << '\t' << dstDataV.data.getGeometry() << mout.endl;


	double d,x,y;
	size_t index = 0;
	int j2;
	for (int j=w.ray1; j<w.ray2; ++j){
		j2 = (j+srcData.odim.nrays)%srcData.odim.nrays;
		for (int i = w.bin1; i<w.bin2; ++i){
			d = srcData.data.get<double>(i, j2);
			//if ((d != srcData.odim.undetect) && (d != srcData.odim.nodata)){
			if (srcData.odim.isValue(d)){
				// mout.warn() << "data d: " << (double)d << mout.endl;
				srcData.odim.mapDopplerSpeed(d, x, y);
				dstDataU.data.put(index, x);
				dstDataV.data.put(index, y);
			}
			else {
				dstDataU.data.put(index, 0);
				dstDataV.data.put(index, 0);
			}
			// mout.warn() << '\t' << index << mout.endl;
			++index;
		}
	}

}

*/

void DopplerRealiasOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

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
	const bool MATCH_UNDETECT = (matchOriginal & 2) && VRAD_SRC;

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
	const double min = dstData.data.getMin<double>();
	const double max = dstData.data.getMax<double>();

	PlainData<PolarDst> & dstQuality = dstProduct.getQualityData();
	qm.setQuantityDefaults(dstQuality);
	setGeometry(dstData.odim, dstQuality);

	//const double dstNI = abs(odim.NI);
	mout.info() << "Inverting (u,v) back to VRAD " << mout.endl;
	mout.info() << "src [" << srcDataU.odim.quantity << "] " << EncodingODIM(srcDataU.odim) << mout.endl;
	mout.info() << "src [" << srcDataV.odim.quantity << "] " << EncodingODIM(srcDataV.odim) << mout.endl;
	mout.info() << "dst [" << dstData.odim.quantity << "]  " << EncodingODIM(dstData.odim) << ", [" << min << ',' << max << ']' << mout.endl;
	mout.info() << "dst [" << dstQuality.odim.quantity << "]" << EncodingODIM(dstQuality.odim) << mout.endl;

	const double srcNI2 = 2.0*srcDataVRAD.odim.getNyquist(); // 2.0*srcData.odim.NI;

	/// Azimuth in radians
	double azmR;

	/// Original value in VRAD
	double vOrig;
	drain::image::Point2D<double> unitVOrig;

	/// Resolved (u,v), from AMVU and AMVV
	double u, v;

	/// Resolved (u,v) projected back on beam
	double vReproj;
	drain::image::Point2D<double> unitVReproj;


	bool ORIG_UNDETECT;
	bool ORIG_NODATA;
	bool ORIG_UNUSABLE; // ORIG_UNDETECT && ORIG_NODATA

	size_t address;

	mout.warn() << "Main " << dstData    << mout.endl;
	mout.note() << "Main " << dstQuality << mout.endl;

	for (size_t j = 0; j < dstData.data.getHeight(); ++j) {

		azmR = dstData.odim.getBeamWidth() * static_cast<double>(j) ; // window.BEAM2RAD * static_cast<double>(j);

		for (size_t i = 0; i < dstData.data.getWidth(); ++i) {

			address = dstData.data.address(i,j);
			u = srcDataU.data.get<double>(address);
			v = srcDataV.data.get<double>(address);


			if (MATCH_UNDETECT || MATCH_ALIASED){
				vOrig = srcDataVRAD.data.get<double>(address);
				ORIG_UNDETECT = (vOrig == srcDataVRAD.odim.undetect);
				ORIG_NODATA   = (vOrig == srcDataVRAD.odim.nodata);

				ORIG_UNUSABLE = ORIG_UNDETECT || ORIG_NODATA;
				if (MATCH_UNDETECT && ORIG_UNUSABLE){
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

				if (MATCH_ALIASED && !ORIG_UNUSABLE){
					vOrig = srcDataVRAD.odim.scaleForward(vOrig);
					srcDataVRAD.odim.mapDopplerSpeed(vOrig,     unitVOrig.x,   unitVOrig.y);
					srcDataVRAD.odim.mapDopplerSpeed(vReproj, unitVReproj.x, unitVReproj.y);
					vReproj = srcNI2*floor(vReproj/srcNI2) + vOrig;
				}

				vReproj = dstData.odim.scaleInverse(vReproj);
				if ((vReproj > min) && (vReproj < max)){ // continue processing
					//dstDataVRAD.data.put(address, vReproj);
					dstData.data.put(address, vReproj);
					dstQuality.data.put(address, dstQuality.odim.scaleInverse(0.5 + (unitVReproj.x*unitVOrig.x + unitVReproj.y*unitVOrig.y)/2.0) );
				}
				else {
					dstData.data.put(address, dstData.odim.nodata); // rand() & 0xffff); //
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
