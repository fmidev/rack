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

	inline
	void set(double startPos, double endPos, double scale=1.0, double bias=0.0){
		this->startPos = startPos;
		this->endPos  = endPos;
		this->setScale(scale, bias);
		this->update();
	}



	virtual
	inline
	void update() const {

		this->INVERSE = (startPos > endPos);

		if (!this->INVERSE){
			leftPos  = startPos;
			rightPos = endPos;
			span = endPos - startPos;
		}
		else {
			//cerr << "set INVERSE" << endl;
			leftPos  = endPos;
			rightPos = startPos;
			span = startPos - endPos;
		}

		this->updateScale();

	}

	inline
	virtual
	double operator()(double x) const {

		if (x <= leftPos)
			return this->biasFinal;
		else if (x >= rightPos)
			return this->biasFinal + this->scaleFinal;
		else
			return this->biasFinal + this->scaleFinal*(x-leftPos) / span;  // div by undetectValue not possible, unless start==end

	}

	double startPos;
	double endPos;

protected:

	mutable double span;
	mutable double leftPos;
	mutable	double rightPos;

private:

	virtual
	void setReferences(){
		this->parameters.reference("startPos", this->startPos);
		this->parameters.reference("endPos", this->endPos);
		this->parameters.reference("scale", this->scale);
		this->parameters.reference("bias", this->bias);
	}

};

/// A basic triangular peak function with linear around the peak.
/*!
 *  \tparam T  - input storage type
 *  \tparam T2 - output storage type
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


	/// Sets the parameters of the membership function.
	inline
	void set(double startPos, double peakPos, double endPos, double scale=1.0, double bias=0.0){ // todo join
		this->startPos = startPos;
		this->peakPos = peakPos;
		this->endPos = endPos;
		this->setScale(scale, bias); //
		this->update();
	}


	virtual
	void update() const {

		this->INVERSE = (startPos > endPos);

		if (!this->INVERSE){
			spanLow  = startPos - peakPos;
			spanHigh = endPos   - peakPos;
		}
		else {
			spanLow  = endPos   - peakPos;
			spanHigh = startPos - peakPos;
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
	double startPos;

	/// Peak position
	double peakPos;

	/// End position
	double endPos;

protected:

	mutable
	double spanLow;

	mutable
	double spanHigh;


private:

	virtual
	void setReferences(){
		this->parameters.reference("startPos", this->startPos);
		this->parameters.reference("peakPos", this->peakPos);
		this->parameters.reference("endPos", this->endPos);
		this->parameters.reference("scale", this->scale);
		this->parameters.reference("bias", this->bias);
	}

};


// TODO: rename fuzzy bell?
/// A smooth symmetric peak function that resembles the Gaussian bell curve.
/**
 *   The approximation applies
 *
 *  \tparam T  - input storage type
 *  \tparam T2 - output storage type
 */
template <class T>
class FuzzyBell : public Fuzzifier<T> {

public:

	FuzzyBell(double location=0.0, double width=1.0, double scale = 1.0, double bias = 0.0) : Fuzzifier<T>(__FUNCTION__, "Fuzzy bell function.", scale, bias), widthInv(1.0) { //	location(location), scale(scale), a(1.0/width), INVERSE(a<0.0) {};
		setReferences();
		set(location, width, scale, bias);
		//this->update();
	}

	FuzzyBell(const FuzzyBell & f) : Fuzzifier<T>(__FUNCTION__, "Fuzzy bell function."), widthInv(1.0){
		this->setReferencesAndCopy(f);
		//this->update();
	}

	inline
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

	inline
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



} // drain::

#endif /* FUZZY_H_*/

// Drain
