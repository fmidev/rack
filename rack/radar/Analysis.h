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
#ifndef RACK_ANALYSIS_H
#define RACK_ANALYSIS_H

#include <math.h>

#include <drain/util/Debug.h>
#include <drain/util/Fuzzy.h>

//#include <drain/image/FuzzyOp.h>

#include <drain/util/Functor.h>
#include <drain/util/FunctorBank.h>

#include <drain/image/Window.h>
#include <drain/image/SegmentProber.h>
#include <drain/image/SlidingWindow.h>
#include <drain/image/GaussianWindow.h>
#include <drain/image/GaussianBlurOp.h>

//#include <drain/image/SequentialImageOp.h>

#include "data/ODIM.h"
#include "data/Quantity.h"
// #include "VolumeOp.h"



using namespace drain::image;

namespace rack {

/// Checks \c nodata and \c undetect before calling the functor
/**
 *  \tparam F - functor, e.g. drain::FuzzyTriange
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

	RadarFunctorOp(bool limit = false) : LIMIT(limit), nodataValue(0.0), undetectValue(0.0) {
		//if (adaptParameters)
		this->getParameters().append(this->functor.getParameters());
	};


	virtual ~RadarFunctorOp(){};

	/// Process the image.
	/**
	 */
	inline
	virtual
	void traverse(const Image &src, Image & dst) const {

		drain::MonitorSource mout(iMonitor, this->name+"(FunctorOp)", __FUNCTION__);
		mout.debug() << "start" << mout.endl;

		/*
		const double coeff  = src.getScale();
		//const double dstMax = dst.getMax<double>();
		const double scaleDst = dst.getScale();
		const double coeffDst = (scaleDst != 0.0) ? 1.0/scaleDst : dst.getMax<double>();
		if (scaleDst == 0.0)
			mout.warn() << "undetectValue scaling coeff for dst" << mout.endl;
		*/

		const double dstMax = dst.getMax<double>();

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
					*d = dst.limit<double>(dstMax * this->functor(odimSrc.scaleForward(s2)));
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



class RadarWindowConfig : public drain::image::WindowConfig {
    public: //re 
	const PolarODIM & odimSrc;

	/// Compensate the polar coordinate system to correspond the Cartesian one in computations
	bool invertPolar;

	double contributionThreshold;  //

	/**
	 *  \param odimSrc - metadata of the source data
	 */
	RadarWindowConfig(const PolarODIM & odimSrc) :
			drain::image::WindowConfig(1, 1),
			odimSrc(odimSrc), invertPolar(false), contributionThreshold(1.0) {
	}

	/**
	 *  \param odimSrc - metadata of the source data
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	RadarWindowConfig(const PolarODIM & odimSrc, int widthM=500, double heightD=1.0, double contributionThreshold = 1.0) :
			drain::image::WindowConfig(odimSrc.getBeamBins(widthM), odimSrc.getAzimuthalBins(heightD)),
			odimSrc(odimSrc), invertPolar(false), contributionThreshold(contributionThreshold) {
	}

	/**
	 *  \param odimSrc - metadata of the source data
	 *  \param ftor - scaling of the result
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	RadarWindowConfig(const PolarODIM & odimSrc, drain::UnaryFunctor & ftor, int widthM=500, double heightD=1.0, double contributionThreshold = 1.0) :
			drain::image::WindowConfig(ftor, odimSrc.getBeamBins(widthM), odimSrc.getAzimuthalBins(heightD)),
			odimSrc(odimSrc), invertPolar(false), contributionThreshold(contributionThreshold) {
	}


};


/// A two-dimensional image processing window that handles the special ODIM codes and scales the result. Template parameter for drain::SlidingWindowOpT
/**
 *  \tparam C - configuration structure
 *  // functor, e.g. drain::Fuzzifier used for scaling the result
 */
template <class C>
class SlidingRadarWindow : public SlidingWindow<C> {
public:

	// , F & ftor = drain::getFunctor("")
	SlidingRadarWindow(int width=0, int height=0) : SlidingWindow<C>(width,height), rangeNorm(1), rangeNormEnd(2), countMin(0) {
		this->resetAtEdges = true;
	};

	SlidingRadarWindow(const C & conf) : SlidingWindow<C>(conf), rangeNorm(1), rangeNormEnd(2), countMin(0) {
		this->resetAtEdges = conf.invertPolar;
	};



protected:

	virtual
	void initialize(){
		setRangeNorm();
	};

	/// To compensate polar geometry, set applicable range for pixel area scaling.
	void setRangeNorm(){

		rangeNorm = static_cast<double>(this->conf.odimSrc.nrays) / (2.0*M_PI);
		rangeNormEnd = (rangeNorm * this->conf.height);
		if ((rangeNorm <= 0) || (rangeNormEnd >= this->conf.odimSrc.nbins)){
			drain::MonitorSource mout("SlidingRadarWindow", __FUNCTION__);
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
		//drain::MonitorSource mout("SlidingRadarWindow", __FUNCTION__);
		//mout.warn() << this->iMax << ',' << this->jMax << '\t' << this->getSamplingArea( )<< mout.endl;
		countMin = this->conf.contributionThreshold * this->getSamplingArea();

		return SlidingWindow<C>::reset();
	};


	virtual
	inline
	void removePixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			double x = this->src.template get<double>(p);
			if ((x != this->conf.odimSrc.nodata) && (x != this->conf.odimSrc.undetect))
				removeTrailingValue(this->conf.odimSrc.scaleForward(x));
		}
	};

	virtual
	inline
	void addPixel(Point2D<int> & p){
		if (this->coordinateHandler.validate(p)){
			double x = this->src.template get<double>(p);
			if ((x != this->conf.odimSrc.nodata) && (x != this->conf.odimSrc.undetect))
				addLeadingValue(this->conf.odimSrc.scaleForward(x));
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

	PolarSegmentProber(const Image & src, Image & dst) : SegmentProber<int,int>(src, dst){};

	/// Operation performed in each segment location (i,j). A function to be redefined in derived classes.
	virtual
	inline
	void update(int i, int j){
		size += i;
	}

};






} // rack::




#endif

// Rack
