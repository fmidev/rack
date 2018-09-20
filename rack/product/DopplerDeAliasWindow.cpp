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

#include <cmath>

#include <drain/util/FunctorBank.h>
#include <drain/util/Fuzzy.h>
#include <drain/util/Geo.h>

#include <drain/image/File.h>
#include <drain/imageops/SlidingWindowOp.h>


#include "radar/Geometry.h"
#include "radar/Doppler.h"

#include "DopplerDeAliasWindow.h"




namespace rack {




void DopplerDeAliasWindow::initialize(){

	drain::Logger mout("DopplerDeAliasWindow", __FUNCTION__);
	//mout.warn() << mout.endl;

	setImageLimits();
	setLoopLimits();
	this->location.setLocation(0,0);

	//NI = (this->odimSrc.NI != 0.0) ? this->odimSrc.NI : 0.01*this->odimSrc.wavelength * this->odimSrc.lowprf / 4.0;
	NI = this->odimSrc.getNyquist();

	mout.debug() << "NI=" << NI << mout.endl;

	// TODO use src.
	//NI2 = 2.0 * NI;

	// Azimuthal resolution (radians per beam).
	//BEAM2RAD = 2.0*M_PI/src.getHeight(); //2.0*M_PI/conf.odimSrc.nrays;

	/// Set maximum quality to undetectValue, and 50% quality at NI/4.
	diffQuality.set(0.0, NI/1.0);

	matrixInformation.set(0.0, -0.2, 250);

	area = static_cast<double>(conf.width * conf.height);

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


void DopplerDeAliasWindow::addPixel(Point2D<int> &p){

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

	if (odimSrc.deriveDifference(d1, d2, diff)){

		diff = diff/(2.0*dSpan*odimSrc.getBeamWidth());

		w += 1.0;

		/// Consider LUT here
		//c = signSin *sin(p.y * BEAM2RAD);
		//s = signCos *cos(p.y * BEAM2RAD);

		// ORIG
		c = -sin(p.y * odimSrc.getBeamWidth()); //BEAM2RAD);
		s = +cos(p.y * odimSrc.getBeamWidth()); //BEAM2RAD);

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

void DopplerDeAliasWindow::removePixel(Point2D<int> &p){

	// Compute derivative
	locationTmp.setLocation(p.x, p.y - dSpan);
	if (!coordinateHandler.validate(locationTmp))
		return;
	d1 = src.get<double>(locationTmp);

	locationTmp.setLocation(p.x, p.y + dSpan);
	if (!coordinateHandler.validate(locationTmp))
		return;
	d2 = src.get<double>(locationTmp);

	if (odimSrc.deriveDifference(d1, d2, diff)){

		diff = diff/(2.0*dSpan*odimSrc.getBeamWidth());

		w -= 1.0;

		/// Consider LUT here
		//s = signCos *cos(p.y * BEAM2RAD);
		//c = signSin *sin(p.y * BEAM2RAD);

		// ORIG
		c = -sin(p.y * odimSrc.getBeamWidth()); //BEAM2RAD);
		s = +cos(p.y * odimSrc.getBeamWidth()); //BEAM2RAD);

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


void DopplerDeAliasWindow::write(){


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
	//odimOut.getMin();

	dst.put(location.x,  location.y, odimOut.scaleInverse(u));
	dst2.put(location.x, location.y, odimOut.scaleInverse(v));

	dstWeight.put(location.x, location.y, 250.0 * quality); //diffQuality(diff));
	//dstWeight.put(location.x, location.y, diffQuality(diff));

}


void DopplerDeAliasWindow::clear(){

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





} // ::rack
