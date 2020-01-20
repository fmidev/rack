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

#include <cmath>

#include "Log.h"
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

	virtual
	inline
	~Fuzzifier(){};

	/// Updates internal variables. Should be called after modifying public members.
	virtual
	inline
	void update() const {
		updateScale();
	}

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

	inline
	void setReferencesAndCopy(const BeanLike & b){
		setReferences();
		this->copy(b);
		this->update();
	};

private:

	virtual
	void setReferences() = 0;

};

/// A basic, linear transition from 0 to scale between \c (start) and \c (end) .
/*!
 *  \tparam T  - input storage type
 *  \tparam T2 - output storage type
 *

 Examples with and without physical scaling to (0.0,1.0), setting step at 50% intensity (128 and 0.5):
 \code
	drainage gray.png        --fuzzyStep 128,64,255 -o fuzzyStep.png
	drainage gray.png -R 0,1 --fuzzyStep 0.5,0.25   -o fuzzyStep-phys.png
 \endcode

When changing storage type, scaling must be given explicitly (here 256, from \c C to \c S ) or physically with \c -R :
 \code
    drainage gray.png         -T S --fuzzyStep 128,64,255 -o fuzzyStep-16bit.png
	drainage gray.png  -R 0,1 -T S --fuzzyStep 0.25,0.5   -o fuzzyStep-16bit-phys.png
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
		setReferences();
		set(startPos, endPos, scale, bias);
	};

	FuzzyStep(const FuzzyStep & f) : Fuzzifier<T>(__FUNCTION__, "Fuzzy step function."), span(1.0){
		this->setReferencesAndCopy(f);
	}

	~FuzzyStep(){};

	inline
	void set(double startPos, double endPos, double scale=1.0, double bias=0.0){
		this->range.min = startPos;
		this->range.max = endPos;
		this->setScale(scale, bias);
		this->update();
	}



	virtual
	inline
	void update() const {

		drain::Logger mout(__FUNCTION__, __FILE__);

		this->INVERSE = (range.min > range.max);

		if (!this->INVERSE){
			finalRange.min  = range.min;
			finalRange.max = range.max;
			span = range.max - range.min;
		}
		else {
			//cerr << "set INVERSE" << endl;
			finalRange.min  = range.max;
			finalRange.max = range.min;
			span = range.min - range.max;
		}

		this->updateScale();

		mout.debug(1) << this->scaleFinal << ',' << this->biasFinal << mout.endl;

	}

	inline
	virtual
	double operator()(double x) const {

		if (x <= finalRange.min)
			return this->biasFinal;
		else if (x >= finalRange.max)
			return this->biasFinal + this->scaleFinal;
		else
			return this->biasFinal + this->scaleFinal*(x-finalRange.min) / span;  // div by undetectValue not possible, unless start==end

	}

	drain::Range<double> range;
	//double startPos;
	//double endPos;

protected:

	mutable double span;
	drain::Range<double> finalRange;
	//mutable double leftPos;
	//mutable double rightPos;

private:

	virtual
	void setReferences(){
		this->parameters.reference("startPos", this->range.min);
		this->parameters.reference("endPos", this->range.max);
		this->parameters.reference("scale", this->scale);
		this->parameters.reference("bias", this->bias);
	}

};

/// A basic triangular peak function with linear around the peak.
/*!
 *  \tparam T  - input storage type
 *  \tparam T2 - output storage type
 *
	\code
	drainage gray.png        --fuzzyTriangle 64,128,192,255 -o fuzzyTriangle.png
	drainage gray.png -R 0,1 --fuzzyTriangle 0.25,0.5,0.75  -o fuzzyTriangle-phys.png
	drainage gray.png -T S   --fuzzyTriangle 64,128,192,255 -o fuzzyTriangle-16bit.png
	\endcode
 */
template <class T>  //,class T2>
class FuzzyTriangle : public Fuzzifier<T> {

public:

	FuzzyTriangle(double startPos=-1.0, double peakPos=0.0, double endPos=+1.0, double scale = 1.0, T bias = 0) : Fuzzifier<T>(__FUNCTION__, "Fuzzy triangle function.", scale, bias){ // : start(start), peak(peak), end(end), scale(scale), _spanLow(start-peak), _spanHigh(end-peak)
		setReferences();
		set(startPos, peakPos, endPos, scale, bias);
	};

	FuzzyTriangle(const FuzzyTriangle & f): Fuzzifier<T>(__FUNCTION__, "Fuzzy triangle function."){
		this->setReferencesAndCopy(f);
	}

	~FuzzyTriangle(){};

	/// Sets the parameters of the membership function.
	inline
	void set(double startPos, double peakPos, double endPos, double scale=1.0, double bias=0.0){ // todo join
		this->range.min = startPos;
		this->peakPos = peakPos;
		this->range.max = endPos;
		this->setScale(scale, bias); //
		this->update();
	}


	virtual
	void update() const {

		this->INVERSE = (range.min > range.max);

		if (!this->INVERSE){
			spanLow  = range.min - peakPos;
			spanHigh = range.max - peakPos;
		}
		else {
			spanLow  = range.max - peakPos;
			spanHigh = range.min - peakPos;
		}

		this->updateScale();

	}

	inline
	virtual
	double operator()(double x) const {

		x = x - peakPos;
		if (x > spanHigh)
			return this->biasFinal;
		else if (x > 0.0)
			return this->biasFinal + this->scaleFinal*(1.0 - x/spanHigh);
		else if (x > spanLow)
			return this->biasFinal + this->scaleFinal*(1.0 - x/spanLow);
		else // x < spanHi
			return this->biasFinal;

	};

	/// Starting position
	//double startPos;

	/// Peak position
	double peakPos;

	/// End position
	//double endPos;
	drain::Range<double> range;

protected:

	mutable
	double spanLow;

	mutable
	double spanHigh;


private:

	virtual
	void setReferences(){
		this->parameters.reference("startPos", this->range.min);
		this->parameters.reference("peakPos", this->peakPos);
		this->parameters.reference("endPos", this->range.max);
		this->parameters.reference("scale", this->scale);
		this->parameters.reference("bias", this->bias);
	}

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
 *
 */
template <class T>
class FuzzyBell : public Fuzzifier<T> {

public:

	FuzzyBell(double location=0.0, double width=1.0, double scale = 1.0, double bias = 0.0) : Fuzzifier<T>(__FUNCTION__, "Fuzzy bell function.", scale, bias), widthInv(1.0) { //	location(location), scale(scale), a(1.0/width), INVERSE(a<0.0) {};
		setReferences();
		set(location, width, scale, bias);
		//this->update();
	}

	FuzzyBell(const FuzzyBell & f) : Fuzzifier<T>(__FUNCTION__, "Fuzzy bell function."), location(0.0), width(1.0), widthInv(1.0){
		this->setReferencesAndCopy(f);
		//this->update();
	}

	virtual
	~FuzzyBell(){};

	void set(double location = 0.0, double width = 1.0, double scale=1.0, double bias=0.0){
		this->location = location;
		this->width = width;
		this->setScale(scale, bias);
		this->update();
	}


	virtual
	void update() const {
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

	double location;
	double width;

protected:


	//double scale;
	mutable
	double widthInv;

private:

	void setReferences(){
		this->parameters.reference("location", this->location);
		this->parameters.reference("width", this->width);
		this->parameters.reference("scale", this->scale);
		this->parameters.reference("bias", this->bias);
	}

};

/// A smooth symmetric peak function that resembles the Gaussian bell curve. Diminishes quicker than FuzzyPeak.
/*!
 *  \tparam T  - input storage type
 *  \tparam T2 - output storage type
 */
template <class T>
class FuzzyBell2 : public Fuzzifier<T> {

public:

	FuzzyBell2(double location=0.0, double width=1.0, double scale = 1.0,  double bias = 0.0) : Fuzzifier<T>(__FUNCTION__, "Fuzzy Gaussian-like bell function.", scale, bias) {
		setReferences();
		set(location, width, scale, bias);
	};

	FuzzyBell2(const FuzzyBell2 & f) : Fuzzifier<T>(__FUNCTION__, "Fuzzy Gaussian-like bell function.") {
		this->setReferencesAndCopy(f);
	}

	virtual
	~FuzzyBell2(){};

	//inline
	void set(double location = 0.0, double width = 1.0, double scale=1.0, double bias=0.0) {
		this->location = location;
		this->width = width;
		this->setScale(scale, bias);
		update();
	}



	inline
	virtual
	double operator()(double x) const {
		x = steepness * (x - location);
		x = 1.0 + x*x;
		return this->biasFinal + this->scaleFinal/(x*x); // *x*x*x*x

	};


	virtual
	void update() const {
		steepness = sqrt(sqrt(2.0)-1.0)/width;
		this->INVERSE = (width<0.0);
		this->updateScale();
	}

	double location;
	double width;

protected:

	mutable
	double steepness;


private:

	void setReferences(){
		this->parameters.reference("location", this->location);
		this->parameters.reference("width", this->width);
		this->parameters.reference("scale", this->scale);
		this->parameters.reference("bias", this->bias);
	}

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
		Fuzzifier<T>(__FUNCTION__, "Fuzzy sign function.", scale, bias), absWidth(0.0) { // : location(location), width(fabs(width)), scale(width<0.0 ? -scale : scale), bias(bias)
		this->setReferences();
		set(location, width, scale, bias);
	};


	FuzzySigmoid(const FuzzySigmoid & f) : Fuzzifier<T>(__FUNCTION__, "Fuzzy sign function."), absWidth(0.0) {
		this->setReferencesAndCopy(f);
	}

	~FuzzySigmoid(){};

	inline
	void set(double location=0.0, double width=1.0, double scale=1.0, double bias=0.0){
		this->location = location;
		this->width = width;
		this->setScale(scale, bias); //
		this->update();
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

	double location;
	double width;

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


private:

	void setReferences(){
		this->parameters.reference("location", this->location);
		this->parameters.reference("width", this->width);
		this->parameters.reference("scale", this->scale);
		this->parameters.reference("bias", this->bias);
	}

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
		this->setReferences();
		set(location, width, scale, bias);
	};

	FuzzyStepsoid(const FuzzyStepsoid & f): Fuzzifier<T>(__FUNCTION__), widthFinal(1.0) {
		this->setReferencesAndCopy(f);
	}

	~FuzzyStepsoid(){};

	inline
	void set(double location=0.0, double width=1.0, double scale=1.0, double bias=0.0){
		this->location = location;
		this->width    = width;
		this->setScale(scale, bias); //
		this->update();
	}

	virtual
	void update() const {
		this->INVERSE = (width<0.0);
		this->updateScale();
		widthFinal = fabs(width);
		//widthFinal = this->INVERSE ? -width : width; // abs(width) FOR INT!
		//std::cerr << this->getName() << ' ' << this->getParameters() << '\n';
		//std::cerr << location << ',' << width << ',' << widthFinal << ',' << (int)this->INVERSE << '\n';
	}

	inline
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

	double location;

	double width;

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

private:

	void setReferences(){
		this->parameters.reference("location", this->location);
		this->parameters.reference("width", this->width);
		this->parameters.reference("scale", this->scale);
		this->parameters.reference("bias", this->bias);
	}

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
		setReferences();
		set(location, width, scale, bias);
	};

	FuzzyTwinPeaks(const FuzzyTwinPeaks & f) : Fuzzifier<T>(__FUNCTION__, "Fuzzy function of two peaks.") {
		this->setReferencesAndCopy(f);
	}

	virtual
	~FuzzyTwinPeaks(){};

	//inline
	void set(double location = 0.0, double width = 1.0, double scale=1.0, double bias=0.0) {
		this->location = location;
		this->width = width;
		this->setScale(scale, bias);
		update();
	}

	virtual
	void update() const {
		this->INVERSE = (width<0.0);
		steepness = 1.0/fabs(width);
		this->updateScale();
	}

	inline
	virtual
	double operator()(double x) const {
		x = steepness * (x - location);
		return this->biasFinal + this->scaleFinal*(x*x)/(1.0 + x*x*x*x);
	};



	double location;
	double width;

protected:

	mutable
	double steepness;


private:

	void setReferences(){
		this->parameters.reference("location", this->location);
		this->parameters.reference("width", this->width);
		this->parameters.reference("scale", this->scale);
		this->parameters.reference("bias", this->bias);
	}

};


} // drain::

#endif /* FUZZY_H_*/

// Drain
