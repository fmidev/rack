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
#ifndef DRAIN_FUNCTOR_H_
#define DRAIN_FUNCTOR_H_

//
//#include <cmath>

//#include <vector>
//#include <iostream>
#include <stdexcept>
//#include <map>

#include "Log.h"
#include "BeanLike.h"

namespace drain
{

// consider: template<N,T=double> : UniTuple<T,N>()
// or add variadic set() to SmartMap

/// Base class for sequential computations. Optional scaling utilities.
class Functor : public BeanLike {

public:


	inline
	void setScale(double scale, double bias=0.0) {
		this->scale = scale;
		this->bias  = bias;
		updateBean();
	}

	virtual inline
	void updateBean() const {
		updateScale();
		//drain::Logger mout(__FILE__, __FUNCTION__);
		//mout.debug2() << "final scale,bias: " << this->scaleFinal << ',' << this->biasFinal << mout.endl;
	}


protected:


	Functor(const std::string & name, const std::string & description="", double scale=1.0, double bias=0.0) : BeanLike(name, description),
		scale(scale), bias(bias), scaleFinal(scale), biasFinal(bias) {} // dstMax(0.0),

	Functor(const Functor & fct) : BeanLike(fct) {
		setScale(fct.scale, fct.bias);
	};

	/*
	Functor(const Functor & fct) : BeanLike(fct){
		parameters.copyStruct(fct.parameters, fct, *this); //
	}
	*/

	virtual
	~Functor(){};

	virtual inline

	void updateScale() const {
		this->scaleFinal = scale; // * SCALE;
		this->biasFinal  = bias;  // * SCALE;
	}

	/// Relative scale, typically 1. Optional.
	double scale;

	/// "Relative" bias, typically 0.  Optional.
	double bias;

	/// Scaling factor after encodings of src and dst images are known.
	mutable
	double scaleFinal;

	/// Scaling factor after encodings of src and dst images are known.
	mutable
	double biasFinal;


};


struct UnaryFunctor : public Functor {

	UnaryFunctor(const std::string & name, const std::string & description="", double scale=1.0, double bias=0.0) : Functor(name, description, scale, bias) {}


	virtual
	double operator()(double s) const = 0;

};

struct IdentityFunctor : public UnaryFunctor {

	IdentityFunctor() : UnaryFunctor(__FUNCTION__, "Returns the input value without changes."){};

	inline
	double operator()(double s) const {return s;};

};

class BinaryFunctor : public Functor {

public:

	BinaryFunctor(const std::string & name, const std::string & description="", double scale=1.0, double bias=0.0) : Functor(name, description, scale, bias) {}

	virtual
	double operator()(double s1, double s2) const = 0;

};

}


#endif

// Drain
