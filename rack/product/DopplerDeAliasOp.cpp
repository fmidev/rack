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
*//**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

    This file is part of AnoRack, a module of Rack for C++.

    AnoRack is not free software.

 */

#include <cmath>

#include <drain/util/FunctorBank.h>
#include <drain/util/Fuzzy.h>
#include <drain/util/Geo.h>

#include <drain/image/File.h>
#include <drain/imageops/SlidingWindowOp.h>


#include "radar/Geometry.h"
#include "radar/Doppler.h"

#include "DopplerDeAliasOp.h"




namespace rack {


/// Doppler dealiasing window
/**
 *  \tparam F - functor, e.g. drain::Fuzzifier used for scaling the result
 */
/*
template <class F>
class DopplerDeal : public SlidingRadarWindow<F> {

 */

class DopplerDeAliasConfig : public RadarWindowConfig {
public:

	//DopplerDeAliasConfig(const PolarODIM & odimSrc, int widthM=1500, double heightD=3.0) :
	DopplerDeAliasConfig(int widthM=1500, double heightD=3.0) :
		//RadarWindowConfig(odimSrc, width, height){
		RadarWindowConfig(widthM, heightD){
		//RadarWindowConfig(odimSrc.getBeamBins(widthM), odimSrc.getAzimuthalBins(heightD)){
	};

};

class DopplerDeAliasWindow : public SlidingRadarWindow<DopplerDeAliasConfig> {
//drain::image::SlidingWindowT<> { // TODO: move to Analysis.h, Templatize => SlidingWindowT


public:

	DopplerDeAliasWindow(const DopplerDeAliasConfig & conf, const PolarODIM & odimOut) :
		SlidingRadarWindow<DopplerDeAliasConfig>(conf) , odimOut(odimOut), functor(NULL), dSpan(3), dSpan2(2.0*dSpan) { };

	inline
	void setDstFrame2(drain::image::Image &d2){
		dst2.setView(d2);
	}

	inline
	void setImageLimits() const {
		src.adjustCoordinateHandler(coordinateHandler);
	}

	virtual
	void initialize(){

		drain::Logger mout("DopplerDeAliasWindow", __FUNCTION__);
		//mout.warn() << mout.endl;

		setImageLimits();
		setLoopLimits();
		this->location.setLocation(0,0);

		//NI = (this->odimSrc.NI != 0.0) ? this->odimSrc.NI : 0.01*this->odimSrc.wavelength * this->odimSrc.lowprf / 4.0;
		NI = this->odimSrc.getNyquist();

		mout.debug() << "NI=" << NI << mout.endl;

		// TODO use src.
		NI2 = 2.0 * NI;

		// Azimuthal resolution (radians per beam).
		BEAM2RAD = 2.0*M_PI/src.getHeight(); //2.0*M_PI/conf.odimSrc.nrays;

		/// Set maximum quality to undetectValue, and 50% quality at NI/4.
		diffQuality.set(0.0, NI/1.0);

		matrixInformation.set(0.0, -0.2, 250);

		area = static_cast<double>(conf.width * conf.height);

		vMax = odimOut.getMax();
		if (drain::Type::call<drain::typeIsInteger>(odimOut.type))
			mout.warn() << "max abs wind for (u or v): " << vMax << mout.endl;

		//coordinateHandler.setPolicy(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT,CoordinatePolicy::WRAP); // move to Op?

		if (!functorSetup.empty()){
			const drain::FunctorBank & functorBank = drain::getFunctorBank();
			const size_t index = functorSetup.find(':');
			const std::string functorName = functorSetup.substr(0, index);
			if (functorBank.has(functorName)){
				mout.note() << "using "<< functorName << mout.endl;
				drain::UnaryFunctor & test = functorBank.get(functorName).clone();
				functor = & test; //functorBank.get(functorName).clone(); // todo fbank exeption
				if (index != std::string::npos){
					functor->setParameters(functorSetup.substr(index+1), '=', ':');
				}
				mout.note() << *functor << mout.endl;
				mout.warn() << "map alt: " << test(10.0)<< mout.endl;
				mout.warn() << "map alt: " << (*functor)(10.0)<< mout.endl;
			}
			else {
				mout.warn() << "could not find functor '" << functorName << "'" << mout.endl;
				functor = NULL;
			}
		}

		if (functor){
			mout.info() << "functor='" << *functor << "'" << mout.endl;
		}

	}


	//const PolarODIM & conf.odimSrc; // NOW in conf!


	const PolarODIM & odimOut;

	std::string functorSetup; // TODO: use that of Window::

	mutable double BEAM2RAD;

	//mutable double signCos;
	//mutable double signSin;

protected:


	/// Handling altitude
	drain::UnaryFunctor *functor; // pointer :-(

	drain::FuzzyBell2<double> diffQuality;
	drain::FuzzyBell2<double> matrixInformation;

	virtual
	void addLeadingValue(double x){};

	virtual
	void removeTrailingValue(double x){};

	/// Add the pixel located at (p.x,p.y) to the window statistic/quantity.
	/**
	 *   Exceptionally, uses locationLead because derivatives are computed \em around Point p .
	 */
	virtual
	void addPixel(Point2D<int> &p){

		diff = 0.0; // debugging only

		// Compute derivative
		locationTmp.setLocation(p.x, p.y - dSpan);
		if (!coordinateHandler.validate(locationTmp))
			return;
		d1 = src.get<double>(locationTmp);

		locationTmp.setLocation(p.x, p.y + dSpan);
		if (!coordinateHandler.validate(locationTmp))
			return;
		d2 = src.get<double>(locationTmp);

		if ((d1 != odimSrc.undetect) && (d1 != odimSrc.nodata) && (d2 != odimSrc.undetect) && (d2 != odimSrc.nodata)){

			w += 1.0;

			d1 = odimSrc.scaleForward(d1);
			d2 = odimSrc.scaleForward(d2);
			deriveDifference(d1, d2, diff);

			/// Consider LUT here
			//c = signSin *sin(p.y * BEAM2RAD);
			//s = signCos *cos(p.y * BEAM2RAD);

			// ORIG
			c = -sin(p.y * BEAM2RAD);
			s = +cos(p.y * BEAM2RAD);

			sTs += s*s;
			cTc += c*c;
			cTs += c*s;
			sTd += s*diff;
			cTd += c*diff;

			// Experimental
			locationTmp.setLocation(p);
			coordinateHandler.handle(locationTmp);
			//if (coordinateHandler.validate(locationTmp)){ // always ok, because between span; but handling must be done!
			d1 = src.get<double>(locationTmp);
			if ((d1 != odimSrc.undetect) && (d1 != odimSrc.nodata)){
				d1 = odimSrc.scaleForward(d1)*M_PI/this->NI;
				sV += sin(d1);
				cV += cos(d1);
			}

			// Other
			sumDiff  += diff; // weight
			sumDiff2 += (diff*diff); // weight

		}



	};

	/// Removes the pixel located at (p.x,p.y) from the window statistic/quantity.
	virtual
	void removePixel(Point2D<int> &p){

		// Compute derivative
		locationTmp.setLocation(p.x, p.y - dSpan);
		if (!coordinateHandler.validate(locationTmp))
			return;
		d1 = src.get<double>(locationTmp);

		locationTmp.setLocation(p.x, p.y + dSpan);
		if (!coordinateHandler.validate(locationTmp))
			return;
		d2 = src.get<double>(locationTmp);

		if ((d1 != odimSrc.undetect) && (d1 != odimSrc.nodata) && (d2 != odimSrc.undetect) && (d2 != odimSrc.nodata)){

			w -= 1.0;

			d1 = odimSrc.scaleForward(d1);
			d2 = odimSrc.scaleForward(d2);
			deriveDifference(d1, d2, diff);

			/// Consider LUT here
			//s = signCos *cos(p.y * BEAM2RAD);
			//c = signSin *sin(p.y * BEAM2RAD);

			// ORIG
			c = -sin(p.y * BEAM2RAD);
			s = +cos(p.y * BEAM2RAD);

			sTs -= s*s;
			cTc -= c*c;
			cTs -= c*s;
			sTd -= s*diff;
			cTd -= c*diff;

			// Experimental!
			locationTmp.setLocation(p);
			coordinateHandler.handle(locationTmp);
			//if (coordinateHandler.validate(locationTmp)){ // always ok, because between span; but handling must be done!
			d1 = src.get<double>(locationTmp);
			if ((d1 != odimSrc.undetect) && (d1 != odimSrc.nodata)){
				d1 = odimSrc.scaleForward(d1)*M_PI/this->NI;
				sV -= sin(d1);
				cV -= cos(d1);
			}

			sumDiff  -= diff; // weight
			sumDiff2 -= (diff*diff); // weight

		}

	};

	inline
	virtual
	void write(){


		div = (sTs*cTc - cTs*cTs); // TODO check

		//if (abs(div) < 0.01)			return;
		if (abs(div) < 0.01){
			dst.put(location.x,  location.y, odimOut.undetect);
			dst2.put(location.x, location.y, odimOut.undetect);
			dstWeight.put(location.x, location.y, 0);
			return;
		}

		/*
		 *  Matrix =
		 *  cTc  cTs  *  sTd
		 *  cTs  sTs  *  cTd
		 *
		 */
		u = ( cTc*sTd - cTs*cTd) / div;
		v = (-cTs*sTd + sTs*cTd) / div;

		if ((abs(u) >= vMax) || (abs(v) >= vMax)){
			dst.put(location.x,  location.y, odimOut.nodata);
			dst2.put(location.x, location.y, odimOut.nodata);
			dstWeight.put(location.x, location.y, 0.1);
			return;
		}


		/// Relative information (determinant divided by max volume)
		//quality = div / sqrt((sTs*sTs + cTs*cTs)*(cTc*cTc + cTs*cTs));
		//quality = 1.0;
		quality = w/area; // not good! conv spots may behave nice.
		// quality = sqrt(sV*sV + cV*cV)/w; // count
		//quality = div / (u*u+v*v);

		// quality *=  1.0/(1.0 + sqrt((sumDiff2 - sumDiff*sumDiff/w) /w));

		if (functor){
			quality *= (*functor)(Geometry::heightFromEtaBeam(odimSrc.elangle*drain::DEG2RAD, odimSrc.getBinDistance(location.x)));
			/*
			quality = Geometry::heightFromEtaBeam(conf.odimSrc.elangle*DEG2RAD, conf.odimSrc.getBinDistance(location.x));
			if (isDiag(50)){
				std::cerr << quality << " => ";
			}
			quality = (*functor)(quality);
			//
			if (isDiag(50)){
				std::cerr << quality << '\n';
				//std::cerr << *functor << '\n';
			}
			*/
		}
		odimOut.getMin();

		dst.put(location.x,  location.y, odimOut.scaleInverse(u));
		dst2.put(location.x, location.y, odimOut.scaleInverse(v));

		dstWeight.put(location.x, location.y, 250.0 * quality); //diffQuality(diff));
		//dstWeight.put(location.x, location.y, diffQuality(diff));

	};


protected:

	drain::image::ImageView dst2;

	virtual
	void clear(){

		//drain::Logger mout("DopplerDeAliasWindow", __FUNCTION__);
		//mout.warn() << mout.endl;

		w = 0.0;

		sTs = 0.0;
		cTc = 0.0;
		cTs = 0.0;
		sTd = 0.0;
		cTd = 0.0;

		/// Experimental
		sumDiff  = 0.0;
		sumDiff2 = 0.0;
		sV = 0.0;
		cV = 0.0;

	}

	bool isDiag(int step){
		return (location.x == location.y) && (location.x % step == 0);
	}

	/// Given two \e aliased doppler speeds computes the difference. Assumes that it is less than Nyquist velocity.
	// Todo: return also quality
	inline
	void deriveDifference(double v1, double v2, double & dOmega){

		/// Raw value (m/s)
		dOmega = v2 - v1;
		if (dOmega < -this->NI)
			dOmega += NI2;
		else if (dOmega > this->NI)
			dOmega -= NI2;

		dOmega = dOmega/(dSpan2*BEAM2RAD);
	}


	//drain::LinearScaling velocityScaling;

private:

	double u;
	double v;

	mutable double vMax;

	// Matrix determinant
	double div;

	// Output quality
	double quality;

	// Used for computing derivatives in addPixel/removePixel
	Point2D<int> locationTmp;

	/// Maximimum unambiguous velocity (Nyquist velocity). ODIM::NI may be missing, so it's here.
	//mutable double NI;

	/// Nyquist range = 2*NI.
	mutable double NI2;


	/// The span of bins when approximating the derivative with
	const int    dSpan;
	/// dSpan2 = 2.0*dSpan
	const double dSpan2;


	double diff;



	/// Statistics

	//  sum of differences
	double sumDiff;
	//  sum differences Squared
	double sumDiff2;

	// count/weight (quality weight)
	double w;
	// area, const (max weight)
	double area;

	/// Speed derivative projected on beam, sine
	double sTd;

	/// Speed derivative projected on beam, cosine
	double cTd;

	/// Sine squared
	double sTs;

	/// Cosine squared
	double cTc;

	/// Sine x cosine
	double cTs;


	// Experimental
	/// sum of velocity sine
	double sV;
	/// sum of velocity cosine
	double cV;



	/// TMP variables

	// Speed differences
	double d1,d2;

	/// Sine, cosine
	double s,c;

};


void DopplerDeAliasOp::processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name, __FUNCTION__);

	const Data<PolarSrc> & srcData = srcSweep.getFirstData(); // VRAD or VRADH

	if (srcData.data.isEmpty()){
		// Actually this should be in higher level
		mout.warn() << "data empty" << mout.endl;
		return;
	}

	/*if (srcData.odim.NI == 0.0){
		mout.warn() << "NI (Nyquist interval) zero or not found." << mout.endl;

	}*/

	//dstProduct.odim.prodpar = getParameters().getKeys();

	PlainData<PolarDst> & dstDataU   = dstProduct.getData("AMVU");
	dstDataU.data.clear();
	PlainData<PolarDst> & dstDataV   = dstProduct.getData("AMVV");
	dstDataV.data.clear();
	PlainData<PolarDst> & dstQuality = dstProduct.getQualityData();
	dstQuality.data.clear();

	ProductBase::applyODIM(dstDataU.odim, odim, true);
	mout.debug(1) << "dstDataU.odim" << EncodingODIM(dstDataU.odim) << mout.endl;
	ProductBase::handleEncodingRequest(dstDataU.odim, encodingRequest);
	mout.debug(2) << "dstDataU.odim" << EncodingODIM(dstDataU.odim) << mout.endl;
	dstDataU.data.setType(dstDataU.odim.type);
	setGeometry(srcData.odim, dstDataU);
	mout.debug() << "dstDataU.odim" << EncodingODIM(dstDataU.odim) << mout.endl;

	ProductBase::applyODIM(dstDataV.odim, odim, true);
	ProductBase::handleEncodingRequest(dstDataV.odim, encodingRequest);
	dstDataV.data.setType(dstDataV.odim.type);
	setGeometry(srcData.odim, dstDataV);

	getQuantityMap().setQuantityDefaults(dstQuality, "QIND");
	setGeometry(srcData.odim, dstQuality);

	/*
	mout.warn() << "scr" << srcData << mout.endl;
	mout.warn() << "U" << dstDataU << mout.endl;
	mout.warn() << "V" << dstDataV << mout.endl;
	//mout.warn() << "VRADC" << dstDataVRAD << mout.endl;
	mout.warn() << "QIND" << dstQuality << mout.endl;
	*/
	const DopplerDeAliasConfig conf(widthM, heightD);

	DopplerDeAliasWindow window(conf, dstDataU.odim);

	window.conf.updatePixelSize(srcData.odim);
	//window.resetAtEdges = true;

	// window.signCos = +1; //(testSigns & 1) ? +1 : -1;
	// window.signSin = -1;(testSigns & 2) ? +1 : -1;

	//window.setSize(width, height);
	window.setSrcFrame(srcData.data);
	window.setDstFrame(dstDataU.data);
	window.setDstFrame2(dstDataV.data);
	window.setDstFrameWeight(dstQuality.data);

	window.functorSetup = altitudeWeight;

	// MAIN OPERATION
	window.run();


	dstDataU.odim.prodpar = getParameters().getKeys();
	dstDataU.odim.update(srcData.odim); // date, time, etc

	/// If desired, run also new, dealiased VRAD field
	if (odim.NI != 0.0){

		PlainData<PolarDst> & dstDataVRAD = dstProduct.getData("VRAD"); // de-aliased

		const QuantityMap & qm = getQuantityMap();
		qm.setQuantityDefaults(dstDataVRAD, "VRAD", "S");
		//const double dstNI = abs(odim.NI);
		dstDataVRAD.odim.setRange(-odim.NI, +odim.NI);
		mout.info() << "dealiasing (u,v) to VRAD " << EncodingODIM(dstDataVRAD.odim) << mout.endl;
		setGeometry(srcData.odim, dstDataVRAD);
		const double srcNI2 = 2.0*srcData.odim.getNyquist(); // 2.0*srcData.odim.NI;
		const double min = dstDataVRAD.data.getMin<double>();
		const double max = dstDataVRAD.data.getMax<double>();
		double azm;

		/// Original value in VRAD
		double vOrig;
		drain::image::Point2D<double> unitVOrig;

		/// Resolved (u,v), from AMVU and AMVV
		double u, v;

		/// Resolved (u,v) projected back on beam
		double vReproj;
		drain::image::Point2D<double> unitVReproj;

		/// Ambiguous part (2N * V_Nyq)

		size_t address;

		for (size_t j = 0; j < dstDataVRAD.data.getHeight(); ++j) {
			azm = window.BEAM2RAD * static_cast<double>(j);
			for (size_t i = 0; i < dstDataVRAD.data.getWidth(); ++i) {
				address = dstDataVRAD.data.address(i,j);
				u = dstDataU.data.get<double>(address);
				v = dstDataV.data.get<double>(address);
				if ((u != dstDataU.odim.undetect) && (u != dstDataU.odim.nodata) && (v != dstDataV.odim.undetect) && (v != dstDataV.odim.nodata)){
					u = dstDataU.odim.scaleForward(u);
					v = dstDataV.odim.scaleForward(v);
					vReproj = project(azm, u,v);
					//vReproj = alias(vReproj, odim.NI);
					if (matchAliased){ // NICKNAME
						vOrig = srcData.data.get<double>(address);
						if ((vOrig != srcData.odim.undetect) && (vOrig != srcData.odim.nodata)){
							vOrig = srcData.odim.scaleForward(vOrig);
							srcData.odim.mapDopplerSpeed(vOrig,     unitVOrig.x,   unitVOrig.y);
							srcData.odim.mapDopplerSpeed(vReproj, unitVReproj.x, unitVReproj.y);
							vReproj = srcNI2*floor(vReproj/srcNI2) + vOrig;
						}
					}

					vReproj = dstDataVRAD.odim.scaleInverse(vReproj);
					if ((vReproj > min) && (vReproj < max)){ // continue processing
						dstDataVRAD.data.put(address, vReproj);
						//u = unitVReproj.x-unitV.x
						dstQuality.data.put(address, dstQuality.odim.scaleInverse(0.5 + (unitVReproj.x*unitVOrig.x + unitVReproj.y*unitVOrig.y)/2.0) );
					}
					else {
						dstDataVRAD.data.put(address, dstDataVRAD.odim.nodata);
						dstQuality.data.put(address, 0);
					};
				}
				else {
					dstDataVRAD.data.put(address, dstDataVRAD.odim.undetect);
				}
			}
		}
		//@ dstDataVRAD.updateTree();
	}


	/// If desired, compute VVP
	if (VVP && false){

		mout.info() << "computing VVP " << mout.endl;

		PlainData<PolarDst> & dstDataHGHT = dstProduct.getData("HGHT"); // altitude
		PlainData<PolarDst> & dstDataVVPU = dstProduct.getData("VVPU"); // wind profile
		PlainData<PolarDst> & dstDataVVPV = dstProduct.getData("VVPV"); // wind profile
		PlainData<PolarDst> & dstDataVVPQ = dstProduct.getData("VVPQ"); // wind profile quality

		const QuantityMap & qm = getQuantityMap();
		qm.setQuantityDefaults(dstDataHGHT, "HGHT", "S");
		qm.setQuantityDefaults(dstDataVVPU, "AMVU", "S");
		qm.setQuantityDefaults(dstDataVVPV, "AMVV", "S");
		qm.setQuantityDefaults(dstDataVVPQ, "QIND", "C");

		const size_t cols = srcData.data.getWidth();
		const size_t rows = srcData.data.getHeight();
		dstDataVVPU.setGeometry(1, cols); // yes transpose
		dstDataVVPV.setGeometry(1, cols);
		dstDataVVPQ.setGeometry(1, cols);
		dstDataHGHT.setGeometry(1, cols);

		double u, v, q;
		double uSum, vSum, qSum;
		unsigned int counter;
		double beta, h;
		const double eta = srcData.odim.elangle * (M_PI/180.0);
		size_t address;
		for (size_t i = 0; i < cols; ++i) {
			beta = srcData.odim.getGroundAngle(i);
			h = 0.001 * Geometry::heightFromEtaBeta(eta, beta);
			//mout.note() << "col:" << i << " h=" << h << " = ";
			dstDataHGHT.data.put(i, dstDataHGHT.odim.scaleInverse(h));
			//mout << dstDataHGHT.odim.scaleForward(dstDataHGHT.data.get<double>(i)) << mout.endl;
			uSum = vSum = qSum = 0.0;
			counter = 0;
			for (size_t j = 0; j < rows; ++j) {
				address = dstQuality.data.address(i,j);
				q = dstQuality.odim.scaleForward(dstQuality.data.get<double>(address));
				if (q > 0.01){
					u = dstDataU.data.get<double>(address);
					v = dstDataV.data.get<double>(address);
					if ((u != dstDataU.odim.undetect) && (u != dstDataU.odim.nodata) && (v != dstDataV.odim.undetect) && (v != dstDataV.odim.nodata)){
						u = dstDataU.odim.scaleForward(u);
						v = dstDataV.odim.scaleForward(v);
						uSum += u;
						vSum += v;
						qSum += q;
						++counter;
					}
				}
			}
			if (qSum > 0.01){
				dstDataVVPU.data.put(i, dstDataVVPU.odim.scaleInverse(uSum/qSum));
				dstDataVVPV.data.put(i, dstDataVVPU.odim.scaleInverse(vSum/qSum));
				dstDataVVPQ.data.put(i, dstDataVVPU.odim.scaleInverse(qSum/static_cast<double>(counter)));
			}
			else {
				dstDataVVPU.data.put(i, dstDataVVPU.odim.nodata);
				dstDataVVPV.data.put(i, dstDataVVPV.odim.nodata);
				dstDataVVPQ.data.put(i, dstDataVVPU.odim.scaleInverse(0));
			}
		}
		// mout.warn() << "computing VVP ended " << mout.endl;
		//@ dstDataVVPU.updateTree();
		//@ dstDataVVPV.updateTree();
		//@ dstDataVVPQ.updateTree();
		//@ dstDataHGHT.updateTree();
	}

	/// Run also
	/*
	if (odim.NI < 0.0){
		mout.warn() << "deviation, " << srcData << mout.endl;
		SlidingWindowOpT<RadarWindowDopplerDev<PolarODIM> > vradDevOp; // op not needed
		vradDevOp.setSize(width, height);
		vradDevOp.window.setSize(width, height);
		vradDevOp.window.countThreshold = (width*height)/5;  // require 20% of valid samples
		//const double pos = 5.4321/vradSrc.odim.NI;
		//vradDevOp.window.functor.set( 0.8*pos, 1.2*pos, 255.0 );
		vradDevOp.window.functor.gain = 0.01;
		vradDevOp.window.functor.offset = 0.0;
		vradDevOp.window.odimSrc = srcData.odim;
		vradDevOp.filter(srcData.data, dstQuality.data);
	}
	*/

	//drain::image::File::write(dst,"DopplerDeAliasOp.png");
	mout.debug(3) << window.odimSrc << mout.endl;


}





}
