/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

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

/// Base class for sequential computations. Optional scaling utilities.
class Functor : public BeanLike {

public:

	inline
	void setScale(double scale, double bias=0.0) {
		this->scale = scale;
		this->bias  = bias;
		update();
	}

	virtual inline
	void update() const {
		drain::Logger mout(this->name+"(Functor)", __FUNCTION__);

		updateScale();

		mout.debug(1) << "final scale,bias: " << this->scaleFinal << ',' << this->biasFinal << mout.endl;
	}

	virtual
	inline
	void setParameters(const std::string &p, char assignmentSymbol='=', char separatorSymbol=0){
		BeanLike::setParameters(p, assignmentSymbol, separatorSymbol);
		update();
	};

protected:


	Functor(const std::string & name, const std::string & description="", double scale=1.0, double bias=0.0) : BeanLike(name, description),
		scale(scale), bias(bias), scaleFinal(scale), biasFinal(bias) {} // dstMax(0.0),

	virtual
	~Functor(){};

	inline
	virtual
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
