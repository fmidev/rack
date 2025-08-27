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
#ifndef FUZZY_H_
#define FUZZY_H_

#include <drain/Log.h>
#include <cmath>

#include "Functor.h"
#include "Range.h"


// using namespace std;

namespace drain
{

/// A base class for fuzzy functions; also an unary functor.
/*!
 *  These operators can be used, for example, as template arguments for drain::Image::FunctorOp.
 *
 *  \tparam T  - storage type
 *
 */
template <class T> //,class T2>
class Fuzzifier : public UnaryFunctor
{
public:

	inline
	Fuzzifier(const std::string & name, const std::string & description="", double scale=1.0, double bias=0.0) : UnaryFunctor(name, description, scale, bias),
	INVERSE(false) {};  // srcMax(1.0),

	// Auto ok
	// Fuzzifier(const Fuzzifier<T> & op) : UnaryFunctor(op), INVERSE(op.INVERSE){}

	virtual
	inline
	~Fuzzifier(){};

	/// Updates internal variables. Should be called after modifying public members.
	/*
	virtual	inline
	void updateBean() const override {
		updateScale();
	}
	*/

	mutable
	bool INVERSE;

protected:

	inline
	virtual
	void updateScale() const {

		if (!INVERSE){
			this->scaleFinal = scale;
			this->biasFinal  = bias;
		}
		else {
			this->scaleFinal = -scale;
			this->biasFinal  =  bias + scale;
		}
	}

	/*
	inline
	void setReferencesAndCopy(const BeanLike & b){
		setReferences();
		this->copy(b);
		this->updateBean();
	};
	*/

private:

	//virtual
	//void setReferences() = 0;

};

/// A basic, linear transition from 0 to scale between \c (start) and \c (end) .
/*!
 *  \tparam T  - input storage type
 *  \tparam T2 - output storage type
 *

 Examples with and without physical scaling to (0.0,1.0), setting step at 50% intensity (128 and 0.5):
\code
drainage image-gray.png        --iFuzzyStep 64:192    -o fuzzyStep.png
drainage image-gray.png -R 0:1 --iFuzzyStep 0.25:0.75  -o fuzzyStep-phys.png
\endcode

When changing storage type, scaling must be given explicitly (here 256, from \c C to \c S ) or physically with \c -R :
\code
drainage image-gray.png         -T S --iFuzzyStep 64:192   -o fuzzyStep-16bit.png
drainage image-gray.png  -R 0:1 -T S --iFuzzyStep 0.25:0.5 -o fuzzyStep-16bit-phys.png
\endcode

 */
template <class T> //,class T2>
class FuzzyStep : public Fuzzifier<T> {

public:

	/// Constructor
	/**
	 *   \param leftPos - start of the step
	 *   \param rightPos   - end of the step
	 *   \param scale
	 *
	 */
	FuzzyStep(T startPos = -1.0, T endPos = 1.0, double scale = 1.0, double bias = 0.0) : Fuzzifier<T>(__FUNCTION__, "Fuzzy step function.", scale, bias), span(1.0) {
		this->parameters.link("position", this->range.tuple());
		this->parameters.link("scale", this->scale);
		this->parameters.link("bias", this->bias);
		set(startPos, endPos, scale, bias);
	};

	FuzzyStep(const FuzzyStep<T> & f) : Fuzzifier<T>(f), span(1.0){
		this->parameters.copyStruct(f.getParameters(), f, *this);
		updateBean();
	}

	~FuzzyStep(){};

	inline
	void set(double rangeMin, double rangeMax, double scale=1.0, double bias=0.0){
		this->range.min = rangeMin;
		this->range.max = rangeMax;
		this->setScale(scale, bias);
		this->updateBean();
	}

	inline
	void set(const drain::Range<double> & r, double scale=1.0, double bias=0.0){
		this->range.set(r);
		this->setScale(scale, bias);
		this->updateBean();
	}



	virtual
	inline
	void updateBean() const override {

		drain::Logger mout(__FILE__, __FUNCTION__);

		this->INVERSE = (range.min > range.max);

		if (!this->INVERSE){
			rangeFinal.min = range.min;
			rangeFinal.max = range.max;
			span = range.max - range.min;
		}
		else {
			//cerr << "set INVERSE" << endl;
			rangeFinal.min = range.max;
			rangeFinal.max = range.min;
			span = range.min - range.max;
		}

		this->updateScale();

		mout.debug2(this->scaleFinal , ',' , this->biasFinal );

	}

	inline
	virtual
	double operator()(double x) const {

		if (x <= rangeFinal.min)
			return this->biasFinal;
		else if (x >= rangeFinal.max)
			return this->biasFinal + this->scaleFinal;
		else
			return this->biasFinal + this->scaleFinal*(x-rangeFinal.min) / span;  // div by undetectValue not possible, unless start==end

	}

	//std::pair<double,double> range;
	drain::Range<double> range;

protected:

	mutable double span;

	//
	mutable
	drain::Range<double> rangeFinal;
	//std::pair<double,double> rangeFinal;


};

/// A basic triangular peak function with linear around the peak.
/*!
 *  \tparam T  - input storage type
 *  \tparam T2 - output storage type
 *
	\code
	drainage image-gray.png        --iFuzzyTriangle 64:192,128     -o fuzzyTriangle.png
	drainage image-gray.png -R 0:1 --iFuzzyTriangle 0.25:0.75,0.5  -o fuzzyTriangle-phys.png
	drainage image-gray.png -T S   --iFuzzyTriangle 64:192,128     -o fuzzyTriangle-16bit.png
	\endcode
 */
template <class T>  //,class T2>
class FuzzyTriangle : public Fuzzifier<T> {

public:

	FuzzyTriangle(double startPos=-1.0, double endPos=+1.0, double peakPos=0.0, double scale = 1.0, T bias = 0) : Fuzzifier<T>(__FUNCTION__, "Fuzzy triangle function.", scale, bias){ // : start(start), peak(peak), end(end), scale(scale), _spanLow(start-peak), _spanHigh(end-peak)
		this->parameters.link("position", this->range.tuple());
		this->parameters.link("peakPos", this->peakPos);
		this->parameters.link("scale", this->scale);
		this->parameters.link("bias", this->bias);

		set(startPos, endPos, peakPos, scale, bias);

	};

	FuzzyTriangle(drain::Range<double> range, double scale = 1.0, T bias = 0) : Fuzzifier<T>(__FUNCTION__, "Fuzzy triangle function.", scale, bias){ // : start(start), peak(peak), end(end), scale(scale), _spanLow(start-peak), _spanHigh(end-peak)
		this->parameters.link("position", this->range.tuple());
		this->parameters.link("peakPos", this->peakPos);
		this->parameters.link("scale", this->scale);
		this->parameters.link("bias", this->bias);

		set(range.min, range.max, (range.min+range.max)/2, scale, bias);

	}

	FuzzyTriangle(const FuzzyTriangle & f): Fuzzifier<T>(f){
		this->parameters.copyStruct(f.getParameters(), f, *this);
		updateBean();
	}

	~FuzzyTriangle(){};

	/// Sets the parameters of the membership function.
	// std::numeric_limits<double>::signaling_NaN()
	inline
	void set(double startPos, double endPos, double peakPos=0.0, double scale=1.0, double bias=0.0){ // todo join

		this->range.set(startPos, endPos);

		this->peakPos = peakPos;
		/*
		if (!std::isnan(peakPos))
			this->peakPos = peakPos;
		else
			this->peakPos = (startPos + endPos) / 2.0;
		*/

		this->setScale(scale, bias); //
		this->updateBean();
	}


	virtual
	void updateBean() const override {

		if (!range.contains(peakPos))
			peakPos = (range.min + range.max) / 2.0;

		this->INVERSE = (range.min > range.max);

		if (!this->INVERSE){
			span.set(range.min - peakPos, range.max - peakPos);
		}
		else {
			span.set(range.max - peakPos, range.min - peakPos);
		}

		this->updateScale();

	}

	inline
	virtual
	double operator()(double x) const {

		x = x - peakPos;
		if (x > span.max)
			return this->biasFinal;
		else if (x > 0.0)
			return this->biasFinal + this->scaleFinal*(1.0 - x/span.max);
		else if (x > span.min)
			return this->biasFinal + this->scaleFinal*(1.0 - x/span.min);
		else // x < spanHi
			return this->biasFinal;

	};

	/// Start and end position
	drain::Range<double> range;

	/// Peak position
	mutable // adjusted if outside range
	double peakPos = 0.0;


protected:

	mutable
	drain::Range<double> span = {-1.0, +1.0};


};



/// A smooth symmetric peak function that resembles the Gaussian bell curve.
/**
 *
 * \tparam T  - input type  (typically double)
 * \tparam T2 - output type (typically double)
 *
 * \param location - center of the peak
 * \param width    - half-width of the peak; if negative, the peak will be upside down
 * \param scale - multiplication in scaling (scale*x + bias); default=1.0
 * \param bias  - offset in scaling (scale*x + bias); default=0.0
 *
 *  The approximation applies
	\code
	drainage image-gray.png        --iFuzzyBell 128,16   -o fuzzyBell.png
	drainage image-gray.png -R 0:1 --iFuzzyBell 0.5,0.2  -o fuzzyBell-phys.png
	drainage image-gray.png -T S   --iFuzzyBell 128,16   -o fuzzyBell-16bit.png
	\endcode
 */
template <class T>
class FuzzyBell : public Fuzzifier<T> {

public:

	FuzzyBell(double location=0.0, double width=1.0, double scale = 1.0, double bias = 0.0) : Fuzzifier<T>(__FUNCTION__, "Fuzzy bell function.", scale, bias), widthInv(1.0) { //	location(location), scale(scale), a(1.0/width), INVERSE(a<0.0) {};
		//setReferences();
		this->parameters.link("location", this->location = location);
		this->parameters.link("width", this->width = width);
		this->parameters.link("scale", this->scale = scale);
		this->parameters.link("bias", this->bias = bias);
		//set(location, width, scale, bias);
		this->updateBean();
	}

	FuzzyBell(const FuzzyBell & f) : Fuzzifier<T>(f) {
		this->parameters.copyStruct(f.getParameters(), f, *this);
		this->updateBean();
	}

	virtual
	~FuzzyBell(){};

	void set(double location = 0.0, double width = 1.0, double scale=1.0, double bias=0.0){
		this->location = location;
		this->width = width;
		this->setScale(scale, bias);
		this->updateBean();
	}

	// New 2025
	void set(const UniTuple<double,2> & range, double scale=1.0, double bias=0.0){
		this->location = 0.5 * (range[0] + range[1]);
		this->width    =       (range[1] - range[0]);
		this->setScale(scale, bias);
		this->updateBean();
	}


	virtual
	void updateBean() const override {
		this->widthInv = 1.0/width;
		this->INVERSE = (width<0.0);
		this->updateScale();
	}

	inline
	virtual
	double operator()(double x) const {

		x = widthInv * (x - this->location);
		return this->biasFinal + this->scaleFinal/(1.0 + x*x);

	};

	double location = 0.0;
	double width = 1.0;

protected:

	//double scale;
	mutable
	double widthInv = 1.0;


};

/// A smooth symmetric peak function that resembles the Gaussian bell curve. Diminishes quicker than FuzzyPeak.
/*!
 *  \tparam T  - input storage type
 *  \tparam T2 - output storage type
 *
    drainage image-gray.png        --iFuzzyBell2 128,16   -o fuzzyBell2.png
	drainage image-gray.png -R 0:1 --iFuzzyBell2 0.5,0.2  -o fuzzyBell2-phys.png
	drainage image-gray.png -T S   --iFuzzyBell2 128,16   -o fuzzyBell2-16bit.png
 *
 */
template <class T>
class FuzzyBell2 : public Fuzzifier<T> {

public:

	FuzzyBell2(double location=0.0, double width=1.0, double scale = 1.0,  double bias = 0.0) : Fuzzifier<T>(__FUNCTION__, "Fuzzy Gaussian-like bell function.", scale, bias) {
		//setReferences();
		this->parameters.link("location", this->location = location);
		this->parameters.link("width", this->width = width);
		this->parameters.link("scale", this->scale = scale);
		this->parameters.link("bias", this->bias = bias);
		updateBean();
	};

	FuzzyBell2(const FuzzyBell2 & f) : Fuzzifier<T>(f) {
		this->parameters.copyStruct(f.getParameters(), f, *this);
		updateBean();
	}

	virtual
	~FuzzyBell2(){};

	//inline
	void set(double location = 0.0, double width = 1.0, double scale=1.0, double bias=0.0) {
		this->location = location;
		this->width = width;
		this->setScale(scale, bias);
		updateBean();
	}

	/// Set peak to a given half-width range.
	// New 2025
	void set(const UniTuple<double,2> & range, double scale=1.0, double bias=0.0){
		this->location = 0.5 * (range[0] + range[1]);
		this->width    =       (range[1] - range[0]);
		this->setScale(scale, bias);
		this->updateBean();
	}


	virtual	inline
	double operator()(double x) const {
		x = steepness * (x - location);
		x = 1.0 + x*x;
		return this->biasFinal + this->scaleFinal/(x*x); // *x*x*x*x
	};


	virtual
	void updateBean() const override {
		steepness = sqrt(sqrt(2.0)-1.0)/width;
		this->INVERSE = (width<0.0);
		this->updateScale();
	}

	double location = 0.0;
	double width = 1.0;

protected:

	mutable
	double steepness = 1.0;


};

/// A smooth step function, by default from -1.0 to +1.0.
/*!
*  \tparam T  - input storage type
*  \tparam T2 - output storage type
*/
template <class T> //,class T2>
class FuzzySigmoid  : public Fuzzifier<T> {

public:

	FuzzySigmoid(double location=0.0, double width=1.0, double scale=1.0, double bias=0.0) :
		Fuzzifier<T>(__FUNCTION__, "Fuzzy sign function.", scale, bias), absWidth(0.0) {
		this->parameters.link("location", this->location = location);
		this->parameters.link("width", this->width = width);
		this->parameters.link("scale", this->scale = scale);
		this->parameters.link("bias", this->bias = bias);
		this->updateBean();
	};


	FuzzySigmoid(const FuzzySigmoid & f) : Fuzzifier<T>(f), absWidth(f.absWidth) {
		this->parameters.copyStruct(f.getParameters(), f, *this);
		//this->setReferencesAndCopy(f);
		//set(location, width, scale, bias);
		this->updateBean();
	}

	~FuzzySigmoid(){};

	inline
	void set(double location=0.0, double width=1.0, double scale=1.0, double bias=0.0){
		this->location = location;
		this->width = width;
		this->setScale(scale, bias); //
		this->updateBean();
	}


	inline
	virtual
	double operator()(double x) const {
		x = x - this->location;
		if (x > 0.0)
			return  this->biasFinal + this->scaleFinal * x/(absWidth + x);
		else if (x < 0.0)
			return  this->biasFinal + this->scaleFinal * x/(absWidth - x);
		else // x==0.0
			return  this->biasFinal;
	};

	double location = 0.0;
	double width    = 1.0;

protected:

	inline
	virtual
	void updateScale() const {

		this->INVERSE = (width<0.0);

		if (!this->INVERSE){
			this->scaleFinal = +this->scale;
			this->biasFinal  =  this->bias;
			this->absWidth = width;
		}
		else {
			this->scaleFinal = -this->scale;
			this->biasFinal  =  this->bias;
			this->absWidth   = -width;
		}
	}

	mutable
	double absWidth;


};

/// A smooth step function between 0.0 and 1.0. Increasing (decreasing) with positive (negative) width. Otherwise like FuzzySigmoid.
/*!
*  \tparam T  - input storage type
*  \tparam T2 - output storage type
*/
template <class T>
class FuzzyStepsoid  : public Fuzzifier<T> {

public:

	FuzzyStepsoid(double location=0.0, double width=1.0, double scale=1.0, double bias=0.0) : Fuzzifier<T>(__FUNCTION__, "Fuzzy step function", scale, bias), widthFinal(1.0) {
		//this->setReferences();
		//this->parameters.link("location", this->range.tuple()); consider
		this->parameters.link("location", this->location = location);
		this->parameters.link("width", this->width = width);
		this->parameters.link("scale", this->scale = scale);
		this->parameters.link("bias", this->bias = bias);
		this->updateBean();
	};

	FuzzyStepsoid(const FuzzyStepsoid & f): Fuzzifier<T>(f), widthFinal(f.widthFinal) {
		this->parameters.copyStruct(f.getParameters(), f, *this);
		this->updateBean();
	}

	~FuzzyStepsoid(){};

	inline
	void set(double location=0.0, double width=1.0, double scale=1.0, double bias=0.0){
		this->location = location;
		this->width    = width;
		this->setScale(scale, bias); //
		this->updateBean();
	}

	virtual
	void updateBean() const override {
		this->INVERSE = (width<0.0);
		this->updateScale();
		widthFinal = fabs(width);
		//widthFinal = this->INVERSE ? -width : width; // abs(width) FOR INT!
		//std::cerr << this->getName() << ' ' << this->getParameters() << '\n';
		//std::cerr << location << ',' << width << ',' << widthFinal << ',' << (int)this->INVERSE << '\n';
	}

	virtual
	double operator()(double x) const {
		x = x - this->location;
		if (x > 0.0)
			return  this->biasFinal + this->scaleFinal * x/(widthFinal + x);  // was: biasFinal+ ...
		else if (x < 0.0)
			return  this->biasFinal + this->scaleFinal * x/(widthFinal - x);
		else // x==0.0
			return  this->biasFinal;
	};

	double location = 0.0;
	double width    = 1.0;

protected:

	inline
	virtual
	void updateScale() const {

		//const double SCALE = (this->dstMax != 0.0) ? this->dstMax : 1.0;

		if (!this->INVERSE){
			this->scaleFinal = +0.5*this->scale; //*SCALE;
			this->biasFinal  = this->bias + 0.5*this->scale; // )*SCALE;
		}
		else {
			this->scaleFinal = -0.5*this->scale; // *SCALE;
			this->biasFinal  = this->bias + 0.5*this->scale;  // )*SCALE;
		}

	}

	mutable
	double widthFinal;

};

/// A function taking shape of two peaks, with a zero in the middle .
/*!
 *  \tparam T  - input storage type
 *  \tparam T2 - output storage type
 */
template <class T>
class FuzzyTwinPeaks : public Fuzzifier<T> {

public:

	FuzzyTwinPeaks(double location=0.0, double width=1.0, double scale = 1.0,  double bias = 0.0) :
		Fuzzifier<T>(__FUNCTION__, "Fuzzy function of two peaks.", scale, bias) {
		/// TODO: use range instead
		this->parameters.link("location", this->location = location);
		this->parameters.link("width", this->width = width);
		this->parameters.link("scale", this->scale = scale);
		this->parameters.link("bias", this->bias = bias);
		this->updateBean();
	};

	FuzzyTwinPeaks(const FuzzyTwinPeaks & f) : Fuzzifier<T>(__FUNCTION__, "Fuzzy function of two peaks.") {
		this->parameters.copyStruct(f.getParameters(), f, *this);
		this->updateBean();
	}

	virtual
	~FuzzyTwinPeaks(){};

	//inline
	void set(double location = 0.0, double width = 1.0, double scale=1.0, double bias=0.0) {
		this->location = location;
		this->width = width;
		this->setScale(scale, bias);
		updateBean();
	}

	virtual
	void updateBean() const override {
		this->INVERSE = (width<0.0);
		steepness = 1.0/fabs(width);
		this->updateScale();
	}


	virtual inline
	double operator()(double x) const {
		x = steepness * (x - location);
		return this->biasFinal + this->scaleFinal*(x*x)/(1.0 + x*x*x*x);
	};



	double location = 0.0;
	double width = 1.0;

protected:


	virtual inline
	void updateScale() const {

		if (this->width >= 0.0){
			this->scaleFinal = +2.0*this->scale;
			this->biasFinal  =  this->bias;
		}
		else {
			this->scaleFinal = -2.0*this->scale;
			this->biasFinal  =  this->bias;
		}
	}

	mutable
	double steepness = 1.0;


};


} // drain::

#endif /* FUZZY_H_*/

// Drain
