/**

    Copyright 2014-   Markus Peura & Joonas Karjalainen  Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */
#ifndef RACK_ANALYSIS_DOPPLER_H
#define RACK_ANALYSIS_DOPPLER_H

#include <math.h>

#include <drain/util/Log.h>
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


/*
class DopplerWindowConfig : public RadarWindowConfig {

public:

	DopplerWindowConfig(const PolarODIM & odimSrc, drain::UnaryFunctor & ftor, int widthM=500, double heightD=1.0, double contributionThreshold=0.5) :
		RadarWindowConfig(odimSrc, ftor, widthM, heightD, contributionThreshold){ //, contributionThreshold(contributionThreshold) {

	}

	DopplerWindowConfig(const PolarODIM & odimSrc, int widthM=500, double heightD=1.0, double contributionThreshold=0.5) :
		RadarWindowConfig(odimSrc, widthM, heightD, contributionThreshold) { //, contributionThreshold(contributionThreshold) {

	}



};
*/
//typedef RadarWindowConfig DopplerWindowConfig;


/**
 *  \tparam C - window parameters (including functor)
 */
class DopplerWindow : public SlidingRadarWindow<RadarWindowConfig> {
public:


	DopplerWindow(const RadarWindowConfig & conf) : // countThreshold(0),
		SlidingRadarWindow<RadarWindowConfig>(conf),  radialSpeedConv(0) , sumI(0.0), sumI2(0.0), sumJ(0.0), sumJ2(0.0), count(0){}; //, countMin(0) {};

	virtual
	inline
	~DopplerWindow(){};

	//int countThreshold;

protected:


	/// Speed scaled to radians [-M_PI, M_PI]: radialSpeedConv = M_PI/this->conf.odimSrc.NI
	/**
	 *   W = v*radialSpeedConv
	 *   v = W/radialSpeedConv
	 */
	double radialSpeedConv;

	/// Inverse of radialSpeedConv
	/**
	 */
	double radialSpeedConvInv;

	//double ;

	// cumulants
	double sumI;
	double sumI2;
	double sumJ;
	double sumJ2;
	int count;

	virtual	inline
	void initialize(){

		drain::Logger mout("RadarWindowDopplerDev", __FUNCTION__);

		this->setImageLimits();
		this->setLoopLimits();
		this->setRangeNorm();

		if (this->odimSrc.NI > 0.0){
			// using true NI
		}
		else {
			mout.info() << "no Nyquist velocity in metadata, NI==0, trying to guess" << mout.endl;
			// using derived NI
		}

		this->NI = this->odimSrc.getNyquist();
		if (this->NI == 0.0){
			mout.warn() << odimSrc << mout.endl;
			mout.error() << "Could not derive Nyquist velocity (NI) from metadata." << mout.endl;
			radialSpeedConv    = 1.0;
			radialSpeedConvInv = 1.0;
		}
		else {
			radialSpeedConv    = M_PI   / this->NI;
			radialSpeedConvInv = M_1_PI * this->NI;
		}
	};


	virtual	inline
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
		x *= radialSpeedConv;
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
		x *= radialSpeedConv;
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

	DopplerAverageWindow(const RadarWindowConfig & conf) : DopplerWindow(conf) {

	}

	typedef DopplerAverageWindow unweighted;

protected:

	virtual
	inline
	void write(){
		//if ((this->location.x == this->location.y) && (this->location.x%15  == 0))
		//	std::cerr << "write" << this->location << ':' << '\t' << count << ':' << (this->conf.contributionThreshold * this->samplingArea) << std::endl;

		if (count > countMin){ // TODO threshold 0.5?

			if (this->conf.relativeScale)
				this->dst.putScaled(this->location.x, this->location.y, atan2(sumJ, sumI) * M_1_PI); //
			else
				this->dst.putScaled(this->location.x, this->location.y, atan2(sumJ, sumI) * this->radialSpeedConvInv);
			/*
			if (this->debugDiag()){
				std::cerr << "write: " << this->location.x << ":\t"
						<< atan2(sumJ, sumI) << "~\t"
						<< ((this->conf.relativeScale?M_1_PI:this->radialSpeedConvInv) * atan2(sumJ, sumI)) << ":\t"
						<< this->dst.getScaled(this->location.x, this->location.y) << '\t'
						<< this->dst.get<double>(this->location.x, this->location.y) << '\n';
			}
			*/
		}
		else
			this->dst.put(this->location, this->odimSrc.undetect); // NOTE: may be wrong (C/S), add odimDst?

	};

};

class DopplerDevWindow : public DopplerWindow {

public:

	DopplerDevWindow(const RadarWindowConfig & conf) : DopplerWindow(conf){
		//this->conf.relativeScale = true;
	}

	typedef DopplerDevWindow unweighted;

protected:


	virtual	inline
	void write(){

		//if (count > (this->conf.contributionThreshold * static_cast<double>(this->samplingArea))){ // TODO threshold 0.5?
		if (count > countMin){ // TODO threshold 0.5?
			// double countD = static_cast<double>(count);
			double c = 1.0/static_cast<double>(count);
			c = sqrt((sumI2+sumJ2 - (sumI*sumI+sumJ*sumJ)*c)*c);

			if (this->conf.relativeScale)
				this->dst.putScaled(this->location.x, this->location.y, this->conf.ftor(c));
				//this->dst.put(this->location, this->conf.ftor(c));
			else
				//this->dst.putScaled(this->location.x, this->location.y, this->conf.ftor(c));
				// NOTE: possibly odimSrc != odimDst, (C/S), add odimDst?
				this->dst.putScaled(this->location.x, this->location.y, this->conf.ftor(NI * c) );
				//this->dst.put(this->location, this->conf.ftor(this->odimSrc.NI * c));
			/*
			if (this->debugDiag()){
				std::cerr << "write: " << this->location.x << ":\t" << (int) this->conf.relativeScale << '\t'
						<< c << ">\t"
						<< (this->conf.relativeScale ? c : c*this->odimSrc.NI ) << ":\t"
						<< this->dst.getScaled(this->location.x, this->location.y) << '\t'
						<< this->dst.get<double>(this->location.x, this->location.y) << '\n';
			}
			*/

		}
		else
			this->dst.put(this->location, 0);

	};

};

} // rack::




#endif
