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
#ifndef RACK_ANALYSIS_DOPPLER_H
#define RACK_ANALYSIS_DOPPLER_H

#include <math.h>

#include <drain/util/Debug.h>
#include <drain/util/Fuzzy.h>

//#include <drain/image/FuzzyOp.h>

#include <drain/util/Functor.h>
#include <drain/util/FunctorBank.h>

#include <drain/image/Window.h>
#include <drain/image/SlidingWindow.h>
//#include <drain/image/SequentialImageOp.h>


#include "Analysis.h"



using namespace drain::image;

namespace rack {



class DopplerWindowConfig : public RadarWindowConfig {
    public: //re 
	//double contributionThreshold;  // raise?

	DopplerWindowConfig(const PolarODIM & odimSrc, drain::UnaryFunctor & ftor, int widthM=500, double heightD=1.0, double contributionThreshold=0.5) :
		RadarWindowConfig(odimSrc, ftor, widthM, heightD, contributionThreshold){ //, contributionThreshold(contributionThreshold) {

	}

	DopplerWindowConfig(const PolarODIM & odimSrc, int widthM=500, double heightD=1.0, double contributionThreshold=0.5) :
		RadarWindowConfig(odimSrc, widthM, heightD, contributionThreshold) { //, contributionThreshold(contributionThreshold) {

	}


};

/**
 *  \tparam C - window parameters (including functor)
 */
//template <class C>
class DopplerWindow : public SlidingRadarWindow<DopplerWindowConfig> {
public:


	DopplerWindow(const DopplerWindowConfig & conf) : // countThreshold(0),
		SlidingRadarWindow<DopplerWindowConfig>(conf),  unitSpeedCoeff(0) , sumI(0.0), sumI2(0.0), sumJ(0.0), sumJ2(0.0), count(0){}; //, countMin(0) {};

	virtual
	inline
	~DopplerWindow(){};

	//int countThreshold;

protected:

	/// Speed scaled to radians [-M_PI,M_PI]
	double unitSpeedCoeff;

	// cumulants
	double sumI;
	double sumI2;
	double sumJ;
	double sumJ2;
	int count;

	//int countMin;

	virtual
	inline
	void initialize(){
		this->setRangeNorm();
		if (this->conf.odimSrc.NI == 0.0){
			unitSpeedCoeff = 1.0;
			drain::MonitorSource mout("RadarWindowDopplerDev", __FUNCTION__);
			mout.error() << "No Nyquist velocity value (NI) in metadata." << mout.endl;
		}
		else {
			unitSpeedCoeff = M_PI/this->conf.odimSrc.NI;
		}
	};


	virtual
	inline
	void clear(){
		sumI  = 0.0;
		sumI2 = 0.0;
		sumJ  = 0.0;
		sumJ2 = 0.0;
		count = 0;
	};


	virtual
	inline
	void removeTrailingValue(double x){
		x *= unitSpeedCoeff;
		double t;
		t = cos(x);
		sumI  -= t;
		sumI2 -= t*t;
		t = sin(x);
		sumJ  -= t;
		sumJ2 -= t*t;
		--count;
	};

	virtual
	inline
	void addLeadingValue(double x){
		x *= unitSpeedCoeff;
		double t;
		t = cos(x);
		sumI  += t;
		sumI2 += t*t;
		t = sin(x);
		sumJ  += t;
		sumJ2 += t*t;
		++count;
		//if ((this->p.x == this->p.y) && (x > -15.0))
		//	std::cerr << "handleLeadingPixel" << this->p << ':' << x << '\t' << count << ':' << sum << std::endl;
	};



};


class DopplerAverageWindow : public DopplerWindow {

public:

	DopplerAverageWindow(const DopplerWindowConfig & conf) : DopplerWindow(conf) {

	}

	typedef DopplerAverageWindow unweighted;

protected:

	virtual
	inline
	void write(){
		//if ((this->location.x == this->location.y) && (this->location.x%15  == 0))
		//	std::cerr << "write" << this->location << ':' << '\t' << count << ':' << (this->conf.contributionThreshold * this->samplingArea) << std::endl;

		//if (count > (this->conf.contributionThreshold * static_cast<double>(this->samplingArea))){ // TODO threshold 0.5?
		if (count > countMin){ // TODO threshold 0.5?

			// double w = this->conf.odimSrc.NI * atan2(sumJ, sumI)/M_PI; // assume odimDst == odimSrc
			//this->dst.put(this->location, this->conf.odimSrc.scaleInverse(this->conf.odimSrc.NI * atan2(sumJ, sumI)/M_PI)); // assume odimDst == odimSrc
			this->dst.put(this->location, this->conf.odimSrc.scaleInverse(atan2(sumJ, sumI) / this->unitSpeedCoeff)); // assume odimDst == odimSrc
			/*
				if ((this->location.x == this->location.y) && (this->location.x%15  == 0))
					std::cerr << "write" << this->location << ':' << '\t' << count << ':' << sumI2 << std::endl;
			 */
		}
		else
			this->dst.put(this->location, this->conf.odimSrc.undetect);

	};

};

class DopplerDevWindow : public DopplerWindow {

public:

	DopplerDevWindow(const DopplerWindowConfig & conf) : DopplerWindow(conf) {
	}

	typedef DopplerDevWindow unweighted;

protected:

	virtual
	inline
	void write(){

		//if (count > (this->conf.contributionThreshold * static_cast<double>(this->samplingArea))){ // TODO threshold 0.5?
		if (count > countMin){ // TODO threshold 0.5?
			double countD = static_cast<double>(count);
			this->dst.put(this->location, this->conf.ftor( sqrt((sumI2-sumI*sumI/countD + sumJ2-sumJ*sumJ/countD)/countD) ));
			/*
				if ((this->location.x == this->location.y) && (this->location.x%15  == 0))
					std::cerr << "write" << this->location << ':' << '\t' << count << ':' << sumI2 << std::endl;
			 */
		}
		else
			this->dst.put(this->location, 0);

	};

};

} // rack::




#endif

// Rack
