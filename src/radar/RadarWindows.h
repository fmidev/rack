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
#ifndef RACK_RADAR_WINDOWS_H
#define RACK_RADAR_WINDOWS_H

#include <math.h>

#include <drain/Log.h>
#include <drain/TypeUtils.h>
#include <drain/util/Fuzzy.h>
#include <drain/util/Functor.h>
#include <drain/util/FunctorBank.h>
#include <drain/image/Window.h>
#include <drain/image/SegmentProber.h>
#include <drain/image/SlidingWindow.h>
#include <drain/image/GaussianWindow.h>
#include <drain/imageops/FunctorOp.h>
#include <drain/imageops/GaussianAverageOp.h>
//#include "drain/image/SequentialImageOp.h"

#include "data/ODIM.h"
#include "data/PolarODIM.h"
#include "data/Quantity.h"

using namespace drain::image;

// file RadarFunctorOp?

namespace rack {


class RadarWindowCore : public drain::image::WeightedWindowCore {


public:

	/// Will act as base class: Window<RadarWindowCore> : public RadarWindowCore {...}, init currently not supported.
	/**
	 */
	RadarWindowCore() : NI(0.0) {
	}

	/**
	 *  \param odimSrc - metadata of the source data
	 */
	//RadarWindowCore(const PolarODIM & odimSrc) : odimSrc(odimSrc) {
	//}


	//const PolarODIM & odimSrc;
	PolarODIM odimSrc;


	/// Nyquist velocity of src. Derived, if not explicit in src metadata.
	/// Maximimum unambiguous velocity (Nyquist velocity). ODIM::NI may be missing, so it's here.
	mutable double NI;

};



class RadarWindowGeom : public drain::UniTuple<double,2> {

public:

	RadarWindowGeom() : widthM(this->next()), heightD(this->next()) {
	}

	RadarWindowGeom(const RadarWindowGeom & geom) : widthM(next()), heightD(next()) {
		assign(geom.tuple());
	}

	// Beam-directional window width in metres
	double & widthM; // = 1500.0;

	// Azimuthal window height in degrees
	double & heightD; // = 3.0;

};

class RadarWindowConfig : public RadarWindowGeom, public drain::image::WindowConfig {

public:

	/// Minimum percentage of detected values in a window (not undetect and nodata)
	double contributionThreshold = 0.5;  //

	/// Compensate the polar coordinate system to correspond the Cartesian one in computations
	bool invertPolar = false;

	/// If true, use speed up to -1.0...+1.0 instead of -Vnyq...+Vnyq.
	bool relativeScale = false;  //

	/**
	 *  \param odimSrc - metadata of the source data
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	RadarWindowConfig(int widthM=1500, double heightD=3.0, double contributionThreshold = 0.5, bool invertPolar=false, bool relativeScale=false) :
		drain::image::WindowConfig(1, 1), // drain::image::WindowConfig(width, height),
		// widthM(widthM), heightD(heightD),
		contributionThreshold(contributionThreshold), invertPolar(invertPolar), relativeScale(relativeScale) {
		this->widthM = widthM;
		this->heightD = heightD;
	}

	RadarWindowConfig(const RadarWindowConfig & conf) :
		RadarWindowGeom(conf),
		drain::image::WindowConfig(conf),
		//widthM(conf.widthM),
		//heightD(conf.heightD),
		contributionThreshold(conf.contributionThreshold),
		invertPolar(conf.invertPolar),
		relativeScale(conf.relativeScale){

	}

	/**
	 *  \tparam FT - UnaryFunctor
	 *
	 *  \param odimSrc - metadata of the source data
	 *  \param ftor - scaling of the result
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	//RadarWindowConfig(drain::UnaryFunctor & ftor, int width=3, int height=3, double contributionThreshold = 0.5) :
	template <class FT>
	RadarWindowConfig(const FT & ftor, int widthM=1500, double heightD=3.0,
			double contributionThreshold = 0.5, bool invertPolar=false, bool relativeScale=false) :
		drain::image::WindowConfig(0, 0, ftor), 		//drain::image::WindowConfig(ftor, width, height),
		//widthM(widthM), heightD(heightD),
		contributionThreshold(contributionThreshold), invertPolar(invertPolar), relativeScale(relativeScale) {
		// invertPolar(false), contributionThreshold(contributionThreshold) {
		this->widthM = widthM;
		this->heightD = heightD;
	}

	// Perhaphs deprecating.
	/// Copies configuration to \c conf and update its pixel frame (width x height).
	void setPixelConf(RadarWindowConfig & conf, const PolarODIM & inputODIM) const;

	// NEW, "Inverted" setPixelConf
	/// Copies configuration from \c conf and update pixel frame (width x height).
	void adjustMyConf(const RadarWindowConfig & conf, const PolarODIM & inputODIM);


	void updatePixelSize(const PolarODIM & inputODIM);

};



/// A two-dimensional image processing window that handles the special ODIM codes and scales the result. Template parameter for drain::SlidingWindowOpT
/**
 *  \tparam C - configuration structure
 *  \tparam R - window core (input and output members)
 *
 *  drain::image::WindowCore supports single-src, single-dst (with respective weights).
 */
template <class C, class R=RadarWindowCore>
class SlidingRadarWindowBase : public drain::image::SlidingWindow<C, R> {
public:

	SlidingRadarWindowBase(int width=0, int height=0) : drain::image::SlidingWindow<C,R>(width,height), rangeNorm(1), rangeNormEnd(2), countMin(0) {
		this->resetAtEdges = true;
	};

	SlidingRadarWindowBase(const C & conf) : drain::image::SlidingWindow<C,R>(conf), rangeNorm(1), rangeNormEnd(2), countMin(0) {
		this->resetAtEdges = conf.invertPolar;
		//this->resetAtEdges = true; //conf.invertPolar;
	};

	virtual
	~SlidingRadarWindowBase(){};

	/// Sets input image and retrieves ODIM metadata from image Properties.
	//  Redefines Window<C,R>::setSrcFrame(ImageFrame)
	/**
	 *
	 */
	void setSrcFrame(const drain::image::ImageFrame & src){

		drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
		//mout.debug("src Scaling0: " , src.getScaling() );
		mout.debug2("src props for odim: " , src.getProperties() );

		this->odimSrc.updateFromMap(src.getProperties());
		mout.info("NI=" , this->odimSrc.getNyquist(LOG_WARNING) );
		mout.info("copied odim: " , EncodingODIM(this->odimSrc) );

		drain::image::SlidingWindow<C,R>::setSrcFrame(src);
		// direct should be:
		// R::setSrcFrame(src);
		mout.debug2("src Scaling: " , src.getScaling() );
	}

	/*
	void setDst(drain::image::ImageFrame & dst){
		drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
		//this->odimSrc.updateFromMap(src.getProperties());
		//mout.debug("copied odim: " , this->odimSrc );
		SlidingWindow<C, RadarWindowCore>::setDst(dst);
	}
	*/

protected:

	virtual
	void initialize() override {
		setImageLimits();
		setRangeNorm(); // interplay setLoopLimits(), with reset() and
		//if (drain::Type::call<drain::typeIsSmallInt>(this->src.getType()) && drain::Type::call<drain::drain::typeIsSmallInt>(this->dst.getType())){
		if (drain::Type::call<drain::typeIsSmallInt>(this->dst.getType())){
			drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
			//this->conf.ftor.setScale(1.0);
			mout.info("(not implemented: functor scaling for small int dst)"  );  // << this->odimSrc
		}
		// what about reset(); ?
		reset();
	};


	inline
	void setImageLimits() const {
		this->coordinateHandler.set(this->src.getGeometry(), this->src.getCoordinatePolicy());
		// this->src.adjustCoordinateHandler(this->coordinateHandler);
	}

	/// To compensate polar geometry, set applicable range for pixel area scaling.
	void setRangeNorm(){

		drain::Logger mout("SlidingRadarWindow", __FUNCTION__);

		if (this->odimSrc.area.height == 0)
			mout.error("src odim.area.height==0" );

		/// Distance [bins] at which a bin is (nearly) square, ie. beam-perpendicular and beam-directional steps are equal.
		const double r = static_cast<double>(this->odimSrc.area.height) / (2.0*M_PI);
		const int max = static_cast<int>(this->odimSrc.area.width);

		rangeNorm    = static_cast<int>(r);
		/// Distance [bins] at which a single azimuthal step is equal to conf.frame.height steps at rangeNorm.
		rangeNormEnd = static_cast<int>(r * static_cast<double>(this->conf.frame.height));
		if ((rangeNorm <= 0) || (rangeNormEnd >= max)){
			mout.note(rangeNorm , '-' , rangeNormEnd );
		}
		//
	}

	int rangeNorm;
	int rangeNormEnd;
	int countMin; // raise


	///  Returns false, if traversal should be ended.
	virtual inline
	bool reset(){

		if (this->location.x <= rangeNorm){
			this->setLoopLimits(this->conf.frame.width, this->conf.frame.height);
		}
		else if (this->location.x < rangeNormEnd){ // from 'height' down to 1
			this->setLoopLimits(this->conf.frame.width, (rangeNorm * this->conf.frame.height)/(this->location.x+1) );
			//std::cerr << "loop limits " << this->jMax << std::endl;
		}
		else {
			this->setLoopLimits(this->conf.frame.width, 1);
		}
		//drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
		//mout.warn(this->iMax , ',' , this->jMax , '\t' , this->getSamplingArea( ));
		countMin = this->conf.contributionThreshold * this->getSamplingArea();

		return SlidingWindow<C,R>::reset();
	};

};


template <class C, class R=RadarWindowCore>
class SlidingRadarWindow : public SlidingRadarWindowBase<C,R> { // drain::image::WeightedWindowCore
public:

	inline
	SlidingRadarWindow(int width=0, int height=0) : SlidingRadarWindowBase<C,R>(width, height) {
	};

	inline
	SlidingRadarWindow(const C & conf) : SlidingRadarWindowBase<C,R>(conf) {
	};


	virtual inline
	void removePixel(drain::Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			double x = this->src.template get<double>(p);
			if ((x != this->odimSrc.nodata) && (x != this->odimSrc.undetect))
				removeTrailingValue(this->odimSrc.scaleForward(x));
		}
	};

	virtual inline
	void addPixel(drain::Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			double x = this->src.template get<double>(p);
			if ((x != this->odimSrc.nodata) && (x != this->odimSrc.undetect))
				addLeadingValue(this->odimSrc.scaleForward(x));
		}
	};

	/// Handles the converted (natural-scaled) value.
	virtual inline
	void removeTrailingValue(double x){}; // = 0;

	/// Handles the converted (natural-scaled) value.
	virtual inline
	void addLeadingValue(double x){}; // = 0;



};

/**
 *  \tparam C - RadarWindowConfig
 *
 */
template <class C>
class RadarWindowAvg : public SlidingRadarWindow<C> {
public:

	//drain::UnaryFunctor & myFunctor;

	RadarWindowAvg(int width=0, int height=0) :
		SlidingRadarWindow<C>(width,height),
		//myFunctor(this->conf.getFunctor()),
		sum(0.0),
		count(0) {
	};

	RadarWindowAvg(const RadarWindowAvg & window) :
		SlidingRadarWindow<C>(window),
		//myFunctor(this->conf.getFunctor(window.conf.getFunctorName())), // kludge
		sum(0.0),
		count(0) {
	};

	RadarWindowAvg(const C & conf) :
		SlidingRadarWindow<C>(conf),
		// myFunctor(this->conf.getFunctor(conf.getFunctorName())),
		sum(0.0),
		count(0) {
	};




	virtual
	inline
	~RadarWindowAvg(){};

	typedef RadarWindowAvg<C> unweighted;

protected:

	double sum;
	int count;

	virtual
	inline
	void clear(){
		sum = 0.0;
		count = 0;
	};

	virtual	inline
	void removeTrailingValue(double x){
		sum -= x;
		--count;
	};

	virtual	inline
	void addLeadingValue(double x){
		sum += x;
		++count;
	};

	virtual	inline
	void write(){
		if (count > 0){
			//if (this->location.x == this->location.y)
			//	std::cerr << sum << '\t' << count << '\n';
			this->dst.putScaled(this->location.x, this->location.y, this->myFunctor(sum/static_cast<double>(count)));
		}
		else
			this->dst.put(this->location, this->odimSrc.undetect); // ?
	};


};

/**
 *  \tparam C - RadarWindowConfig (contains also weight support)
 *
 */
template <class C>
class RadarWindowWeightedAvg : public SlidingRadarWindow<C> {
public:


	// Consider metres & degrees?
	RadarWindowWeightedAvg(int width=0, int height=0) : SlidingRadarWindow<C>(width,height) {
	};

	RadarWindowWeightedAvg(const RadarWindowWeightedAvg<C> & window) : SlidingRadarWindow<C>(window) {
	};

	RadarWindowWeightedAvg(const C & conf) : SlidingRadarWindow<C>(conf) {
	};

	virtual inline
	~RadarWindowWeightedAvg(){};

protected:

	int count = 0;

	// Copied from SlidingStripeAverage
	typedef float sum_t;
	sum_t w = 0.0;
	sum_t sum = 0.0;
	sum_t sumW = 0.0;

	virtual	inline
	void clear() final {
		this->sum   = 0.0;
		this->sumW  = 0.0;
		this->count = 0;
	};


	virtual	inline
	void addPixel(drain::Point2D<int> & p) final {
		if (this->coordinateHandler.validate(p)){
			double x = this->src.template get<double>(p);
			if (this->odimSrc.isValue(x)){
				this->w     = this->srcWeight.template get<sum_t>(p);
				this->sum  += w*this->odimSrc.scaling.fwd(x);
				this->sumW += w;
				++this->count;
			}
		}
	};

	virtual inline
	void removePixel(drain::Point2D<int> & p) final {
		if (this->coordinateHandler.validate(p)){
			double x = this->src.template get<double>(p);
			if (this->odimSrc.isValue(x)){
				this->w     = this->srcWeight.template get<sum_t>(p);
				this->sum  -= w*this->odimSrc.scaling.fwd(x);
				this->sumW -= w;
				--this->count;
			}
		}
	};

	virtual inline
	void write() final {
		if (sumW > 0.0){ // then also count>0
			this->dst.put(this->location, this->odimSrc.scaling.inv(this->sum/sumW));
			//this->dstWeight.put(this->location, this->scalingW.inv(sumW/static_cast<sum_t>(this->count)));
			this->dstWeight.put(this->location, sumW/static_cast<sum_t>(this->count)); // "uses" original scaling
		}
		else {
			this->dst.put(this->location, this->odimSrc.undetect); // change
			this->dstWeight.put(this->location, 0);
		}

	}

	/*
	virtual	inline
	void removeTrailingValue(double x) final {
		sum -= x;
		--count;
	};

	virtual	inline
	void addLeadingValue(double x) final {
		sum += x;
		++count;
	};

	virtual	inline
	void write(){
		if (count > 0){
			//if (this->location.x == this->location.y)
			//	std::cerr << sum << '\t' << count << '\n';
			this->dst.putScaled(this->location.x, this->location.y, this->myFunctor(sum/static_cast<double>(count)));
		}
		else
			this->dst.put(this->location, this->odimSrc.undetect); // ?
	};
	*/


};


/**
 *  \tparam F - functor, e.g. drain::Fuzzifier used for scaling the result
 */
template <class C>
class RadarWindowSoftMax : public SlidingRadarWindow<C> {
public:

	RadarWindowSoftMax(int width=0, int height=0, double coeff=1.0) : SlidingRadarWindow<C>(width,height), coeff(1.0), coeffInv(1.0/coeff), sum(0.0), count(0) {};


	virtual inline
	~RadarWindowSoftMax(){};

	void setCoeff(double c){
		if (c==0.0)
			throw std::runtime_error("RadarWindowSoftMax: zero coeff");
		coeff = c;
		coeffInv = 1.0/c;
	};

protected:

	double coeff;
	double coeffInv;

	double sum;
	int count;

	virtual inline
	void clear(){
		sum = 0.0;
		count = 0;
	};

	virtual	inline
	void removeTrailingValue(double x){
		sum -= ::exp(coeff * x);
		--count;
	};

	virtual inline
	void addLeadingValue(double x){
		sum += ::exp(coeff * x);
		++count;
		//if ((p.x == p.y) && (x > -15.0))
		//	std::cerr << "handleLeadingPixel" << p << ':' << x << '\t' << count << ':' << sum << std::endl;
	};

	virtual inline
	void write(){
		if (count > 0)
			this->dst.put(this->location, this->fuzzifier(coeffInv*::log(sum/static_cast<double>(count))));
		//this->dst.put(this->location, this->functor(sum/static_cast<double>(count)));
	};


};

/// Sliding window for computing standard deviation of scalar quantities.
/**
 *  \tparam F - functor, e.g. drain::Fuzzifier used for scaling the result
 *
 *  \see DopplerDevWindow for respective operator for Doppler data [VRAD].
 */
template <class F>
class RadarWindowStdDev : public SlidingRadarWindow<F> {
public:

	RadarWindowStdDev(int width=0, int height=0) : SlidingRadarWindow<F>(width,height), sum(0.0), sum2(0.0), count(0) {};


	virtual	inline
	~RadarWindowStdDev(){};

protected:

	double sum;
	double sum2;
	int count;

	virtual	inline
	void clear(){
		sum  = 0.0;
		sum2 = 0.0;
		count = 0;
	};

	virtual
	inline
	void removeTrailingValue(double x){
		sum -= x;
		sum2 -= x*x;
		--count;
	};

	virtual
	inline
	void addLeadingValue(double x){
		sum += x;
		sum2 += x*x;
		++count;
		//if ((this->p.x == this->p.y) && (x > -15.0))
		//	std::cerr << "handleLeadingPixel" << this->p << ':' << x << '\t' << count << ':' << sum << std::endl;
	};

	virtual
	inline
	void write(){

		if (count > 0){
			double countD = static_cast<double>(count);
			this->dst.put(this->location, this->fuzzifier( ::sqrt(sum2/countD - sum*sum/(countD*countD)) ));
			/*
			countD = sum2/countD - sum*sum/(countD*countD);
			if ((this->p.x == this->p.y) && (count > 5))
				std::cerr << __FUNCTION__ << this->p << ':' << countD << std::endl;
			 */
		}
		else
			this->dst.put(this->location, 0);

	};


};







} // rack::




#endif

// Rack
