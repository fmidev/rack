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
#ifndef RACK_ANALYSIS_H
#define RACK_ANALYSIS_H

#include <math.h>

#include <drain/util/Log.h>
#include <drain/util/Fuzzy.h>

//#include <drain/image/FuzzyOp.h>

#include <drain/util/Functor.h>
#include <drain/util/FunctorBank.h>
#include <drain/util/TypeUtils.h>

#include <drain/image/Window.h>
#include <drain/image/SegmentProber.h>
#include <drain/image/SlidingWindow.h>
#include <drain/image/GaussianWindow.h>
#include <drain/imageops/FunctorOp.h>
#include <drain/imageops/GaussianAverageOp.h>

//#include <drain/image/SequentialImageOp.h>

#include "data/ODIM.h"
#include "data/Quantity.h"
// #include "VolumeOp.h"



using namespace drain::image;

// file RadarFunctorOp?

namespace rack {

/// Checks \c nodata and \c undetect before calling the functor
/**
 *  \tparam F - unary functor, e.g. drain::FuzzyTriangeor other derived from UnaryFUnctor
 */
// NOTE may be bad, because UnaryFunctorOp pre-scales etc.
template <class F>
class RadarDataFunctor : public F  {
    public: //re 
	PolarODIM odimSrc;

	double nodataValue;
	double undetectValue;

	RadarDataFunctor(double nodataValue = 0.0, double undetectValue = 0.0) : nodataValue(nodataValue),undetectValue(undetectValue) {
	}

	virtual
	inline
	double operator()(double src) const {
		if (src == odimSrc.nodata)
			return nodataValue;
		else if (src == odimSrc.undetect)
			return undetectValue;
		else
			return F::operator()(odimSrc.scaleForward(src));
	};

};

///
/**
 *  \tparam F - functor, e.g. drain::FuzzyTriange
 */
/*
template <class F>
class NaturalRadarDataFunctor : public RadarDataFunctor<F>  {
    public: //re 
	const Quantity & q;
	const PolarODIM & odimStd;


	NaturalRadarDataFunctor(const std::string & quantity, char type='C') : q(getQuantityMap().get(quantity)), odimStd(q[type]){
	}


	virtual
	inline
	double operator()(double src) const {
		if ((src != this->odimSrc.nodata) && (src != this->odimSrc.undetect))
			return F::operator()(this->odimSrc.scaleForward(src));
		else
			return q.undetectValue;
	};



};
*/

/// Convenience (abbreviation)
template <class F>
class RadarFunctorOp : public drain::image::UnaryFunctorOp<F> {
public:

	PolarODIM odimSrc;
	bool LIMIT;
	double nodataValue;
	double undetectValue;

	RadarFunctorOp(bool limit = true) : LIMIT(limit), nodataValue(0.0), undetectValue(0.0) {
		//if (adaptParameters)
		this->getParameters().append(this->functor.getParameters());
	};


	virtual ~RadarFunctorOp(){};

	/// Process the image.
	/**
	 */
	inline
	virtual
	void traverseChannel(const Channel &src, Channel & dst) const {

		drain::Logger mout(getImgLog(), this->name+"(FunctorOp)", __FUNCTION__);
		mout.debug() << "start" << mout.endl;

		// const double dstMax = dst.scaling.getMax<double>();
		const double dstMax = dst.getMax<double>();
				// drain::Type::call<drain::typeMax, double>(dst.getType());

		typedef drain::typeLimiter<double> Limiter;
		Limiter::value_t limit = drain::Type::call<Limiter>(dst.getType());

		Image::const_iterator s  = src.begin();
		Image::iterator d = dst.begin();
		double s2;
		if (this->LIMIT){
			while (d != dst.end()){
				s2 = static_cast<double>(*s);
				if (s2 == odimSrc.nodata)
					*d = nodataValue;
				else if (s2 == odimSrc.undetect)
					*d = undetectValue;
				else
					//*d = dst.scaling.limit<double>(dstMax * this->functor(odimSrc.scaleForward(s2)));
					*d = limit(dstMax * this->functor(odimSrc.scaleForward(s2)));
				++s;
				++d;
			}
		}
		else {
			while (d != dst.end()){
				s2 = static_cast<double>(*s);
				if (s2 == odimSrc.nodata)
					*d = nodataValue;
				else if (s2 == odimSrc.undetect)
					*d = undetectValue;
				else
					*d = dstMax * this->functor(odimSrc.scaleForward(s2));
				++s;
				++d;
			}
		}

	}


protected:



};



// file RadarWindowOp?

class RadarWindowCore : public drain::image::WeightedWindowCore {


public:


	/**
	 *  \param odimSrc - metadata of the source data
	 */
	//RadarWindowCore(const PolarODIM & odimSrc) : odimSrc(odimSrc) {
	//}

	/// Will act as base class: Window<RadarWindowCore> : public RadarWindowCore {...}, init currently not supported.
	/**
	 *
	 */
	RadarWindowCore() {
	}

	//const PolarODIM & odimSrc;
	PolarODIM odimSrc;


	/// Nyquist velocity of src. Derived, if not explicit in src metadata.
	/// Maximimum unambiguous velocity (Nyquist velocity). ODIM::NI may be missing, so it's here.
	mutable double NI;

};

class RadarWindowConfig : public drain::image::WindowConfig {

public:

	// Beam-directional window width in metres
	int widthM;

	// Azimuthal window height in degrees
	double heightD;

	/// Compensate the polar coordinate system to correspond the Cartesian one in computations
	bool invertPolar;

	double contributionThreshold;  //

	/// If true, use speed up to -1.0...+1.0 instead of -Vnyq...+Vnyq.
	bool relativeScale;  //

	/**
	 *  \param odimSrc - metadata of the source data
	RadarWindowConfig(const PolarODIM & odimSrc) :
			drain::image::WindowConfig(1, 1),
			odimSrc(odimSrc), invertPolar(false), contributionThreshold(1.0) {
	}
	 */

	/**
	 *  \param odimSrc - metadata of the source data
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	//RadarWindowConfig(int width=3, int height=3, double contributionThreshold = 0.5) :
	RadarWindowConfig(int widthM=1500, double heightD=3.0, double contributionThreshold = 0.5, bool invertPolar=false, bool relativeScale=false) :
		drain::image::WindowConfig(1, 1), // drain::image::WindowConfig(width, height),
		widthM(widthM), heightD(heightD), invertPolar(invertPolar), contributionThreshold(contributionThreshold), relativeScale(relativeScale) {
	}


	/**
	 *  \param odimSrc - metadata of the source data
	 *  \param ftor - scaling of the result
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	//RadarWindowConfig(drain::UnaryFunctor & ftor, int width=3, int height=3, double contributionThreshold = 0.5) :
	RadarWindowConfig(drain::UnaryFunctor & ftor, int widthM=1500, double heightD=3.0,
			double contributionThreshold = 0.5, bool invertPolar=false, bool relativeScale=false) :
		drain::image::WindowConfig(ftor), 		//drain::image::WindowConfig(ftor, width, height),
		widthM(widthM), heightD(heightD), invertPolar(invertPolar), contributionThreshold(contributionThreshold), relativeScale(relativeScale) {
		// invertPolar(false), contributionThreshold(contributionThreshold) {
	}

	inline
	void updatePixelSize(const PolarODIM & odimSrc){
		drain::Logger mout("RadarWindowConfig", __FUNCTION__);
		//mout.note() << odimSrc << mout.endl;
		this->width  = odimSrc.getBeamBins(this->widthM);
		this->height = odimSrc.getAzimuthalBins(this->heightD);
		//mout.note() << this->width << '<' << this->widthM << mout.endl;
		//mout.note() << this->height << '<' << this->heightD << mout.endl;
	}

protected:



};


/// A two-dimensional image processing window that handles the special ODIM codes and scales the result. Template parameter for drain::SlidingWindowOpT
/**
 *  \tparam C - configuration structure
 *  \tparam R - window core (input and output members)
 *
 *  drain::image::WindowCore supports single-src, single-dst (with respective weights).
 */
template <class C, class R=RadarWindowCore>
class SlidingRadarWindow : public SlidingWindow<C, R> { // drain::image::WeightedWindowCore
public:

	SlidingRadarWindow(int width=0, int height=0) : SlidingWindow<C,R>(width,height), rangeNorm(1), rangeNormEnd(2), countMin(0) {
		this->resetAtEdges = true;
	};

	SlidingRadarWindow(const C & conf) : SlidingWindow<C,R>(conf), rangeNorm(1), rangeNormEnd(2), countMin(0) {
		this->resetAtEdges = conf.invertPolar;
	};

	virtual
	~SlidingRadarWindow(){};

	void setSrcFrame(const drain::image::ImageFrame & src){
		drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
		this->odimSrc.updateFromMap(src.getProperties());
		mout.info()  << "NI=" << this->odimSrc.NI << mout.endl;
		mout.debug() << "copied odim: " << this->odimSrc << mout.endl;

		SlidingWindow<C, R>::setSrcFrame(src);
	}

	/*
	void setDst(drain::image::ImageFrame & dst){
		drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
		//this->odimSrc.updateFromMap(src.getProperties());
		//mout.debug() << "copied odim: " << this->odimSrc << mout.endl;
		SlidingWindow<C, RadarWindowCore>::setDst(dst);
	}
	*/

protected:

	inline
	void setImageLimits() const {
		this->src.adjustCoordinateHandler(this->coordinateHandler);
	}

	virtual
	void initialize(){
		setImageLimits();
		setRangeNorm(); // replaced by reset() and setLoopLimits()
		//if (drain::Type::call<drain::typeIsSmallInt>(this->src.getType()) && drain::Type::call<drain::drain::typeIsSmallInt>(this->dst.getType())){
		if (drain::Type::call<drain::typeIsSmallInt>(this->dst.getType())){
			drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
			//this->conf.ftor.setScale(1.0);
			mout.info() << "(not implemented: functor scaling for small int dst)"  << mout.endl;  // << this->odimSrc
		}
	};



	/// To compensate polar geometry, set applicable range for pixel area scaling.
	void setRangeNorm(){

		/// Distance [bins] at which a bin is (nearly) square, ie. beam-perpendicular and beam-directional steps are equal.
		const double r = static_cast<double>(this->odimSrc.nrays) / (2.0*M_PI);

		rangeNorm    = static_cast<int>(r);
		/// Distance [bins] at which a single azimuthal step is equal to conf.height steps at rangeNorm.
		rangeNormEnd = static_cast<int>(r * static_cast<double>(this->conf.height));
		if ((rangeNorm <= 0) || (rangeNormEnd >= this->odimSrc.nbins)){
			drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
			mout.note() << rangeNorm << '-' << rangeNormEnd << mout.endl;
		}
		//
	}

	int rangeNorm;
	int rangeNormEnd;
	int countMin; // raise


	///  Returns false, if traversal should be ended.
	virtual
	inline
	bool reset(){

		if (this->location.x <= rangeNorm){
			this->setLoopLimits(this->conf.width, this->conf.height);
		}
		else if (this->location.x < rangeNormEnd){ // from 'height' down to 1
			this->setLoopLimits(this->conf.width, (rangeNorm * this->conf.height)/(this->location.x+1) );
			//std::cerr << "loop limits " << this->jMax << std::endl;
		}
		else {
			this->setLoopLimits(this->conf.width, 1);
		}
		//drain::Logger mout("SlidingRadarWindow", __FUNCTION__);
		//mout.warn() << this->iMax << ',' << this->jMax << '\t' << this->getSamplingArea( )<< mout.endl;
		countMin = this->conf.contributionThreshold * this->getSamplingArea();

		return SlidingWindow<C,R>::reset();
	};


	virtual inline
	void removePixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			double x = this->src.template get<double>(p);
			if ((x != this->odimSrc.nodata) && (x != this->odimSrc.undetect))
				removeTrailingValue(this->odimSrc.scaleForward(x));
		}
	};

	virtual inline
	void addPixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			double x = this->src.template get<double>(p);
			if ((x != this->odimSrc.nodata) && (x != this->odimSrc.undetect))
				addLeadingValue(this->odimSrc.scaleForward(x));
		}
	};

	/// Handles the converted (natural-scaled) value.
	virtual
	void removeTrailingValue(double x) = 0;

	/// Handles the converted (natural-scaled) value.
	virtual
	void addLeadingValue(double x) = 0;



};

/**
 *  \tparam F - functor, e.g. drain::Fuzzifier used for scaling the result
 */
template <class C>
class RadarWindowAvg : public SlidingRadarWindow<C> {
public:

	RadarWindowAvg(int width=0, int height=0) : SlidingRadarWindow<C>(width,height), sum(0.0), count(0) {};


	virtual
	inline
	~RadarWindowAvg(){};

protected:

	double sum;
	int count;

	virtual
	inline
	void clear(){
		sum = 0.0;
		count = 0;
	};

	virtual
	inline
	void removeTrailingValue(double x){
		sum -= x;
		--count;
	};

	virtual
	inline
	void addLeadingValue(double x){
		sum += x;
		++count;
	};

	virtual
	inline
	void write(){
		if (count > 0)
			this->dst.put(this->location, this->fuzzifier(sum/static_cast<double>(count)));
		//dst.put(location, odimSrc.scaleInverse(sum/static_cast<double>(count)));
	};


};

/**
 *  \tparam F - functor, e.g. drain::Fuzzifier used for scaling the result
 */
template <class C>
class RadarWindowSoftMax : public SlidingRadarWindow<C> {
public:

	RadarWindowSoftMax(int width=0, int height=0, double coeff=1.0) : SlidingRadarWindow<C>(width,height), coeff(1.0), coeffInv(1.0/coeff), sum(0.0), count(0) {};


	virtual
	inline
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

	virtual
	inline
	void clear(){
		sum = 0.0;
		count = 0;
	};

	virtual
	inline
	void removeTrailingValue(double x){
		sum -= exp(coeff * x);
		--count;
	};

	virtual
	inline
	void addLeadingValue(double x){
		sum += exp(coeff * x);
		++count;
		//if ((p.x == p.y) && (x > -15.0))
		//	std::cerr << "handleLeadingPixel" << p << ':' << x << '\t' << count << ':' << sum << std::endl;
	};

	virtual
	inline
	void write(){
		if (count > 0)
			this->dst.put(this->location, this->fuzzifier(coeffInv*log(sum/static_cast<double>(count))));
		//this->dst.put(this->location, this->functor(sum/static_cast<double>(count)));
	};


};

/**
 *  \tparam F - functor, e.g. drain::Fuzzifier used for scaling the result
 */
template <class C>
class RadarWindowStdDev : public SlidingRadarWindow<C> {
public:

	RadarWindowStdDev(int width=0, int height=0) : SlidingRadarWindow<C>(width,height), sum(0.0), sum2(0.0), count(0) {};


	virtual
	inline
	~RadarWindowStdDev(){};

protected:

	double sum;
	double sum2;
	int count;

	virtual
	inline
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
			this->dst.put(this->location, this->fuzzifier( sqrt(sum2/countD - sum*sum/(countD*countD)) ));
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


/// Computes in polar coordinates.
/**
 *  The area of a sector of a circle is
 *  \f[
 *    A_s = \frac{1}{2}ra = \frac{1}{2}\alpha r^2
 *  \f]
 *  where \f$r\f$ is radius and \f$p = \alpha r\f$ arc of the sector.
 *
 *  Consider a radar sweep and a bin \f$b(i,j)\f$ where \f$i\f$ is the range index and \f$j\f$ is the azimuthal index.
 *  The corresponding physical area is
 *  \f[
 *    A_b(i,j) = \frac{1}{2}(r_2-r_1)a = \frac{1}{2}\mathrm{rscale}\cdot \frac{2\pi}{\mathrm{nrays}}(i\cdot \mathrm{rscale}) = \frac{\pi(\mathrm{rscale})^2}{\mathrm{nrays}} i
 *  \f]
 *  where \f$\mathrm{rscale}\f$ is the phycal bin length and \f$\mathrm{nrays}\f$ the number of beams in the sweep.
 *  Notice that the area is a function of \f$i\f$ only, but we write both indices \f$(i,j)\f$ for clarity.
 *
 *  Consider a segment \f$\Phi\f$ in a radar sweep. The segment can be identified with the set of coordinate
 *  of the involved bins: \f$\Phi = \{(i,j)\}\f$
 *  In computing the area of segment \f$ \Phi \f$ , PolarSegmentProber primarily computes a simple integer quantity
 *  \f[
 *    D_\Phi = \sum_\Phi i
 *  \f]
 *  The physical area is hence
 *  \f[
 *    A_\Phi = \sum_\Phi A_b(i,j) = \sum_\Phi \frac{\pi(\mathrm{rscale})^2}{\mathrm{nrays}} i = \frac{\pi(\mathrm{rscale})^2}{\mathrm{nrays}} \sum_\Phi  i = \frac{\pi(\mathrm{rscale})^2}{\mathrm{nrays}} D_\Phi
 *  \f]
 *
 */
class PolarSegmentProber : public drain::image::SegmentProber<int,int> {
public:

	PolarSegmentProber(const Image & src, Image & dst) : SegmentProber<int,int>(src.getChannel(0), dst.getChannel(0)){};

	/// Operation performed in each segment location (i,j). A function to be redefined in derived classes.
	virtual
	inline
	void update(int i, int j){
		size += i;
	}

};






} // rack::




#endif
