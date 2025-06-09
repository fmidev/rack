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

#include <drain/Log.h>
#include <drain/TypeUtils.h>
#include <math.h>

#include <drain/util/Fuzzy.h>

//#include <drain/image/FuzzyOp.h>

#include <drain/util/Functor.h>
#include <drain/util/FunctorBank.h>
//#include <drain/image/Window.h>
#include <drain/image/SegmentProber.h>
// #include <drain/image/SlidingWindow.h>
//#include <drain/image/GaussianWindow.h>
#include <drain/imageops/FunctorOp.h>
//#include <drain/imageops/GaussianAverageOp.h>

//#include <drain/image/SequentialImageOp.h>

#include "data/ODIM.h"
#include "data/PolarODIM.h"
#include "data/Quantity.h"
// #include "VolumeOp.h"



// using namespace drain::image;

// file RadarFunctorOp?

namespace rack {

/// Checks \c nodata and \c undetect before calling the functor
/**
 *  \tparam F - unary functor, e.g. drain::FuzzyTriangeor str derived from UnaryFUnctor
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


class RadarFunctorBase  {
public:

	PolarODIM odimSrc;
	//bool LIMIT;
	double nodataValue;
	double undetectValue;

	RadarFunctorBase() :  nodataValue(0.0), undetectValue(0.0) {
		//if (adaptParameters)
		//LIMIT = limit;
	};


	void apply(const drain::image::Channel &src, drain::image::Channel & dst, const drain::UnaryFunctor & ftor, bool LIMIT = true) const;

};

/// Convenience (abbreviation)
template <class F>
class RadarFunctorOp : public RadarFunctorBase, public drain::image::UnaryFunctorOp<F> {
public:

	/*
	PolarODIM odimSrc;
	bool LIMIT;
	double nodataValue;
	double undetectValue;

	RadarFunctorOp(bool limit = true) : LIMIT(limit), nodataValue(0.0), undetectValue(0.0) {
		//if (adaptParameters)
		this->getParameters().append(this->functor.getParameters());
	};
	*/
	inline
	RadarFunctorOp(){
		this->LIMIT = true;
	};



	virtual inline
	~RadarFunctorOp(){};

	/// Process the image.
	/**
	 */
	virtual
	void traverseChannel(const drain::image::Channel &src, drain::image::Channel & dst) const {
		apply(src, dst, this->functor, this->LIMIT);
	}

	/// Process the image.
	/**
	virtual
	void traverseChannelOLD(const drain::image::Channel &src, drain::image::Channel & dst) const {

		drain::Logger mout(__FILE__, __FUNCTION__); //REPL getImgLog(), this->name+"(RadarFunctorOp)", __FUNCTION__);
		mout.debug("start" );

		// const double dstMax = dst.scaling.getMax<double>();
		//// NEW 2019/11 const double dstMax = dst.getEncoding().getTypeMax<double>();
		// drain::Type::call<drain::typeMax, double>(dst.getType());
		const drain::ValueScaling & dstScaling = dst.getScaling();

		typedef drain::typeLimiter<double> Limiter;
		Limiter::value_t limit = drain::Type::call<Limiter>(dst.getType());

		drain::image::Image::const_iterator s  = src.begin();
		drain::image::Image::iterator d = dst.begin();
		double s2;
		if (this->LIMIT){
			while (d != dst.end()){
				s2 = static_cast<double>(*s);
				if (s2 == odimSrc.nodata)
					*d = nodataValue;
				else if (s2 == odimSrc.undetect)
					*d = undetectValue;
				else
					// *d = dst.scaling.limit<double>(dstMax * this->functor(odimSrc.scaleForward(s2)));
					// *d = limit(dstMax * this->functor(odimSrc.scaleForward(s2)));
					*d = limit(dstScaling.inv(this->functor(odimSrc.scaleForward(s2))));
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
					*d = dstScaling.inv(this->functor(odimSrc.scaleForward(s2)));
					// *d = dstMax * this->functor(odimSrc.scaleForward(s2));
				++s;
				++d;
			}
		}

	}


protected:
	 */



};



// file RadWinOp?



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
class PolarSegmentProber : public drain::image::SizeProber { // SegmentProber<int,int>
public:

	// PolarSegmentProber(const Image & src, Image & dst) : SizeProber(src.getChannel(0), dst.getChannel(0)){};

	PolarSegmentProber(const drain::image::Channel & src, drain::image::Channel & dst) : drain::image::SizeProber(src, dst){};

	/// Operation performed in each segment location (i,j). A function to be redefined in derived classes.
	virtual inline
	void update(int i, int j){
		size += i;
	}

};






} // rack::




#endif

// Rack
