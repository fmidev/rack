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

    Copyright 2001 - 2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

    This file is part of AnoRack, a module of Rack for C++.

    AnoRack is not free software.

*/

#ifndef DOPPLER_DEALIAS_WINDOW_H_
#define DOPPLER_DEALIAS_WINDOW_H_


#include "DopplerWindowOp.h"

#include <cmath>


namespace rack {



class DopplerDeAliasConfig : public RadarWindowConfig {
public:

	inline
	DopplerDeAliasConfig(int widthM=1500, double heightD=3.0) :
		RadarWindowConfig(widthM, heightD){
	};

};

class DopplerDeAliasWindow : public SlidingRadarWindow<DopplerDeAliasConfig> {
	//drain::image::SlidingWindowT<> { // TODO: move to Analysis.h, Templatize => SlidingWindowT


public:

	DopplerDeAliasWindow(const DopplerDeAliasConfig & conf, const PolarODIM & odimOut) :
		SlidingRadarWindow<DopplerDeAliasConfig>(conf) , odimOut(odimOut), functor(NULL)
		//, dSpan(3), dSpan2(2.0*dSpan)
		{ };

	inline
	void setDstFrame2(drain::image::Image &d2){
		dst2.setView(d2);
	}

	inline
	void setImageLimits() const {
		src.adjustCoordinateHandler(coordinateHandler);
	}

	virtual
	void initialize();

	/// Computes the derivative (m/s)/beam. Often rescaled with beam width.
	inline
	bool getDerivative(const Point2D<int> &p, double & diff, bool debug=false) const {

		Point2D<int> pTmp;

		pTmp.setLocation(p.x, p.y - 1);
		if (!this->coordinateHandler.validate(pTmp))
			return false;
		const double d1 = src.get<double>(pTmp);

		pTmp.setLocation(p.x, p.y + 1);
		if (!this->coordinateHandler.validate(pTmp))
			return false;
		const double d2 = src.get<double>(pTmp);

		if (odimSrc.deriveDifference(d1, d2, diff)){
			diff *= 0.5; // due to above +1/-1
			/*
			if (debug && isDiag(2)){
				std::cerr << " {" << d1 << ',' << d2 << "} \t => (" << odimSrc.scaleForward(d1) << ',' << odimSrc.scaleForward(d2) << "}\t => " << diff << '\n';
			}
			*/
			return true;
		}
		else
			return false;

	}


	const PolarODIM & odimOut;

	std::string functorSetup; // TODO: use that of Window::


protected:

	/// Handling altitude
	drain::UnaryFunctor *functor; // pointer :-(   TODO: use that of Window::

	drain::FuzzyBell2<double> diffQuality;
	drain::FuzzyBell2<double> matrixInformation;

	virtual inline
	void addLeadingValue(double x){};

	virtual inline
	void removeTrailingValue(double x){};

	//inline 	bool retrieveSpeedSample(d1, d2)

	/// Add the pixel located at (p.x,p.y) to the window statistic/quantity.
	/**
	 *   Uses locationLead because derivatives are computed \e around Point p .
	 */
	virtual
	void addPixel(Point2D<int> &p);

	/// Removes the pixel located at (p.x,p.y) from the window statistic/quantity.
	virtual
	void removePixel(Point2D<int> &p);

	virtual
	void write();

protected:

	drain::image::ImageView dst2;

	virtual
	void clear();

	inline
	bool isDiag(int step) const {
		return (location.x == location.y) && (location.x % step == 0);
	}

	/// Given two \e aliased doppler speeds computes the difference. Assumes that it is less than Nyquist velocity.

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
	// mutable double NI2;


	/// The span of bins when approximating the derivative with
	//const int    dSpan;
	/// dSpan2 = 2.0*dSpan
	//const double dSpan2;


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


class DopplerDeAliasWindowWeighted : public DopplerDeAliasWindow {


public:

	DopplerDeAliasWindowWeighted(const DopplerDeAliasConfig & conf, const PolarODIM & odimOut) :
		DopplerDeAliasWindow(conf, odimOut)	{
	};

};


}  // ::rack


#endif /* DOPPLERDEALIASOP_H_ */
