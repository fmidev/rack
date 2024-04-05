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

#ifndef RAINRATE_ECHO_TOP2_OP
#define RAINRATE_ECHO_TOP2_OP

#include "PolarProductOp.h"

using namespace drain::image;

namespace rack {

struct ReferencePoint : public drain::UniTuple<double,2> {

	double & reflectivity;
	double & height;

	ReferencePoint(double reflectivity=0.0, double height=0.0) : reflectivity(this->next()), height(this->next()){
		this->set(reflectivity, height);
	};

	ReferencePoint(const ReferencePoint & p): reflectivity(this->next()), height(this->next()){
		this->set(p);
	};
};

/**
 *  \tparam T - double for interface (0.0..1.0), unsigned-char for internal encoding.
 */
template <class T>
struct MethodWeights : public drain::UniTuple<T,5> {

	/// Highest priority
	T & interpolation;
	T & overShooting; // beam over Z < Zt
	T & underShooting; // highest beam Z > Zt
	T & weak;
	T & clear;

	MethodWeights() : interpolation(this->next()), overShooting(this->next()), underShooting(this->next()), weak(this->next()), clear(this->next()){
	};

	template<typename ... TT>
	MethodWeights(const TT &... args) : interpolation(this->next()), overShooting(this->next()), underShooting(this->next()), weak(this->next()), clear(this->next()){
		this->set(args...);
	};

	MethodWeights(const MethodWeights & p): drain::UniTuple<double,5>(p),
			interpolation(this->next()), overShooting(this->next()), underShooting(this->next()), weak(this->next()), clear(this->next()) {
	};

	inline
	bool useInterpolation(){
		return (interpolation > overShooting);
	}

};



///
/**
 *
 */
class EchoTop2Op: public PolarProductOp {

public:

	/// Numeric alues can be applied as quality index (with gain=0.004)
	typedef enum {
		/** Unset (not processsed) */
		UNDEFINED=0,
		/** Internal error in the computation */
		ERROR=16,
		/** No echo found */
		CLEAR=64,
		/** The highest bin exceeds the threshold -> use reference point */
		UNDERSHOOTING=96,
		/** The bin exceeding the Z threshold has an \c undetect ("dry") bin above -> replace \c undetect with low Z (dB) */
		OVERSHOOTING=128,
		WEAK=192, /// Echoes only below threshold detected
		/** The threshold value was passed between adjacent beams, hence can be interpolated */
		INTERPOLATION=250
	} Reliability;

	typedef drain::EnumFlagger<drain::SingleFlagger<Reliability> > reliabilityFlagger;

	EchoTop2Op(double threshold = 20.0);

	virtual inline
	~EchoTop2Op(){};

	// TEST
	void computeSingleProduct(const DataSetMap<src_t> & srcSweeps, DataSet<dst_t> & dstProduct) const;


	double threshold = 0.0;

	MethodWeights<double> weights;

	ReferencePoint reference = {-50.0, 15000.0};
	double dryTopDBZ = NAN;

	bool EXTENDED = false;


};


} // rack::

#endif
