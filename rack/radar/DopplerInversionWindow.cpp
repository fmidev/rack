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

#include <drain/image/ImageFile.h>
#include <cmath>

#include "drain/util/FunctorBank.h"
#include "drain/util/Fuzzy.h"
#include "drain/util/Geo.h"

#include "drain/imageops/SlidingWindowOp.h"


#include "radar/Geometry.h"
#include "radar/Doppler.h"

#include "DopplerInversionWindow.h"




namespace rack {



void DopplerInversionWindow::initialize(){

	drain::Logger mout(__FILE__, __FUNCTION__);
	//mout.warn() << mout.endl;

	setImageLimits();
	setLoopLimits();
	this->location.setLocation(0,0);

	//NI = (this->odimSrc.NI != 0.0) ? this->odimSrc.NI : 0.01*this->odimSrc.wavelength * this->odimSrc.lowprf / 4.0;
	NI = this->odimSrc.getNyquist();

	mout.debug() << "NI=" << NI << mout.endl;

	/// Set maximum quality to undetectValue, and 50% quality at NI/4.
	diffQuality.set(0.0, NI/1.0);

	matrixInformation.set(0.0, -0.2, 250);

	area = static_cast<double>(conf.frame.getArea());

	vMax = odimOut.getMax();
	const double vMin = odimOut.getMin();

	const drain::Type t(odimOut.type);
	if (drain::Type::call<drain::typeIsInteger>(t)){
		mout.note() << "AMVU, AMVV: vMin=" << vMin << ", vMax=" << vMax << mout.endl;
		if (drain::Type::call<drain::typeIsSmallInt>(t)){
			// values
			unsigned int n = 1 << 8*drain::Type::call<drain::sizeGetter>(t);
			mout.note() << "AMVU, AMVV: resolution=" << static_cast<double>(vMax-vMin)/static_cast<double>(n) << "m/s / bit" << mout.endl;
		}
	}

	//coordinateHandler.setPolicy(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT,CoordinatePolicy::WRAP); // move to Op?

	if (!functorSetup.empty()){
		const drain::FunctorBank & functorBank = drain::getFunctorBank();
		const size_t index = functorSetup.find(':');
		const std::string functorName = functorSetup.substr(0, index);
		if (functorBank.has(functorName)){
			mout.note() << "using "<< functorName << mout.endl;
			drain::UnaryFunctor & test = functorBank.clone(functorName);
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

	reset();

	qualityFunctor.set(0, 20); // origin, 20m/s half-with

}


void DopplerInversionWindow::addPixel(drain::Point2D<int> &p){


	if (getDerivative(p, diff)){

		//diff = diff/(2.0*dSpan*odimSrc.getBeamWidth());
		diff = diff/odimSrc.getBeamWidth();

		w += 1.0;

		/// Consider LUT here
		c = -sin(odimSrc.getAzimuth(p.y));  // p.y * odimSrc.getBeamWidth());
		s = +cos(odimSrc.getAzimuth(p.y));  // p.y * odimSrc.getBeamWidth());

		sTs += s*s;
		cTc += c*c;
		cTs += c*s;
		sTd += s*diff;
		cTd += c*diff;

		// Experimental
		/*
			locationTmp.setLocation(p);
			coordinateHandler.handle(locationTmp);  // always ok, because between span; but handling must be done!
			d1 = src.get<double>(locationTmp);
			if (odimSrc.isValue(d1)){
				//if ((d1 != odimSrc.undetect) && (d1 != odimSrc.nodata)){
				d1 = odimSrc.scaleForward(d1)*M_PI/this->NI;
				sV += sin(d1);
				cV += cos(d1);
			}

			// Other
			sumDiff  += diff; // weight
			sumDiff2 += (diff*diff); // weight
		 */
	}



}

void DopplerInversionWindow::removePixel(drain::Point2D<int> &p){


	if (getDerivative(p, diff)){

		//diff = diff/(2.0*dSpan*odimSrc.getBeamWidth());
		diff = diff/odimSrc.getBeamWidth();

		w -= 1.0;

		/// Consider LUT here
		c = -sin(odimSrc.getAzimuth(p.y));  // location.y * odimSrc.getBeamWidth());
		s = +cos(odimSrc.getAzimuth(p.y));  // location.y * odimSrc.getBeamWidth());

		sTs -= s*s;
		cTc -= c*c;
		cTs -= c*s;
		sTd -= s*diff;
		cTd -= c*diff;

		// Experimental!
		/*
			locationTmp.setLocation(p);
			coordinateHandler.handle(locationTmp); // always valid, because between span; but handling must be done!
			d1 = src.get<double>(locationTmp);
			if (odimSrc.isValue(d1)){
				d1 = odimSrc.scaleForward(d1)*M_PI/this->NI;
				sV -= sin(d1);
				cV -= cos(d1);
			}

			sumDiff  -= diff; // weight
			sumDiff2 -= (diff*diff); // weight
		 */
	}

}


void DopplerInversionWindow::write(){


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


	if (getDerivative(location, diff, true)){

		diff = diff/odimSrc.getBeamWidth(); // Now unit = [m/s/rad]
		//odimSrc.getAzimuth();
		c = -sin(odimSrc.getAzimuth(location.y));  // location.y * odimSrc.getBeamWidth());
		s = +cos(odimSrc.getAzimuth(location.y));  // location.y * odimSrc.getBeamWidth());
		// "e"
		//diff = (diff - (c*u + s*v)); // The error, by definition.
		//diff = diff / NI; // NOTE: NI=srcNI! Maybe unfair for src data with small NI?
		//quality = 20.0 / (20.0 + abs(diff));  // 10m/s from hät
		quality = qualityFunctor(diff - (c*u + s*v));
		//quality = qualityFunctor(diff);
		//quality = qualityFunctor(c*u + s*v);
		//quality = diff;
		// if (isDiag(2)) std::cerr << diff << '\t' << quality << '\n';

	}
	else
		quality = 0.01;

	/// Relative information (determinant divided by max volume)
	// quality = div / sqrt((sTs*sTs + cTs*cTs)*(cTc*cTc + cTs*cTs));
	//
	// quality = 1.0;
	// quality = w/area; // not good! conv spots may behave nice.
	// quality = sqrt(sV*sV + cV*cV)/w; // count
	// quality = div / (u*u+v*v);

	// quality *=  1.0/(1.0 + sqrt((sumDiff2 - sumDiff*sumDiff/w) /w));

	// Height weighting?
	if (functor){
	//	quality *= (*functor)(Geometry::heightFromEtaBeam(odimSrc.elangle*drain::DEG2RAD, odimSrc.getBinDistance(location.x)));
	}


	dst.put(location.x,  location.y, odimOut.scaleInverse(u));
	dst2.put(location.x, location.y, odimOut.scaleInverse(v));

	//dstWeight.put(location.x, location.y, quality); //diffQuality(diff));
	dstWeight.putScaled(location.x, location.y, quality); //diffQuality(diff));
	//dstWeight.put(location.x, location.y, diffQuality(diff));

}


void DopplerInversionWindow::clear(){

	//drain::Logger mout("DopplerInversionWindow", __FUNCTION__);
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





} // ::rack
