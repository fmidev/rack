/*

MIT License

Copyright (c) 2024 FMI Open Development / Markus Peura, first.last@fmi.fi

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

#ifndef ECHO_TOP2_OP
#define ECHO_TOP2_OP


#include "radar/RadarWindows.h"

#include "PolarProductOp.h"

using namespace drain::image;

namespace rack {

/// A single measurement (bin), with height coordinate and measurement value (reflectivity).
/**
 *
 */
struct Measurement : public drain::UniTuple<double,2> {

	double & reflectivity;
	double & height;

	Measurement(double reflectivity=NAN, double height=0.0) : reflectivity(this->next()), height(this->next()){
		this->set(reflectivity, height);
	};

	Measurement(const Measurement & p): reflectivity(this->next()), height(this->next()){
		this->set(p);
	};
};

/*
struct Measurement {

	/// Pointer to data array (DBZH)
	const Data<src_t> * dataPtr = nullptr;

	/// Index of the bin at the ground distance D of all the elevation beams considered
	size_t binIndex = 0;

	/// Height from the ground surface (from radar site, ASL)
	double height = 0;

	/// Reflectivity observed at the current bin.
	double reflectivity = NAN;

	/// Reliability/confidence of the measurement value, maximal (1.0) when reflectivity is the threshold value.
	// double quality = 0.0;
};
*/

/**
 *  \tparam T - double for interface (0.0..1.0), unsigned-char for internal encoding.
 */
template <class T>
struct MethodWeights : public drain::UniTuple<T,5> {

	typedef T value_t;

	/// Highest to lowest certainty
	T & interpolation;
	T & interpolation_dry;  // beam over Z < Z_thr
	T & extrapolation_up; // truncated strong echo underShooting highest beam Z > Z_thr
	T & extrapolation_down;
	T & clear;   // same as undetect, but needs a quality index value
	T error = 0; // needs a quality index value (0)

	MethodWeights() : interpolation(this->next()), interpolation_dry(this->next()), extrapolation_up(this->next()), extrapolation_down(this->next()), clear(this->next()){
	};

	template<typename ... TT>
	MethodWeights(const TT &... args) : interpolation(this->next()), interpolation_dry(this->next()), extrapolation_up(this->next()), extrapolation_down(this->next()), clear(this->next()){
		this->set(args...);
	};

	MethodWeights(const MethodWeights & p): drain::UniTuple<double,5>(p),
			interpolation(this->next()), interpolation_dry(this->next()), extrapolation_up(this->next()), extrapolation_down(this->next()), clear(this->next()) {
	};

	/*
	inline
	bool useInterpolation() const {
		return (interpolation > overShooting);
	}
	*/


};



///
/**
 *
 */
class EchoTop2Op: public PolarProductOp {

public:

	EchoTop2Op(double threshold = 20.0);

	virtual inline
	~EchoTop2Op(){};

	// TEST
	void computeSingleProduct(const DataSetMap<src_t> & srcSweeps, DataSet<dst_t> & dstProduct) const;

	///
	/**
	 *   Division by zero should not occur, as arguments m1 and m2 are distinguished by reflectivity less/greater than threshold.
	 *   Also, reference point higher than -32 dBZ will be warned.
	 */
	static inline
	double getSlope(const Measurement & m1, const Measurement & m2){
		return (m1.height - m2.height) / (m1.reflectivity - m2.reflectivity);
	}

	/*
	static inline
	double getSlope(double heightStronger, double heightWeaker, double reflectivityStronger, double reflectivityWeaker){
		return (heightStronger - heightWeaker) / (reflectivityStronger - reflectivityWeaker);
	}
	*/


protected:

	double threshold = 0.0;

	MethodWeights<double> weights;

	/// Virtual measurement high aloft, towards which reflectivity is expected to decrease.
	Measurement reference = {-50.0, 15000.0};

	/// Unless NaN, use the value like a measured dBZ
	double undetectReflectivity = NAN;

	/// Optional reference smoothin window. Width (metres) and height (degrees)
	//drain::UniTuple<double,2> refWindow = {0.0,0.0};
	RadarWindowGeom refWindow; // = {0.0,0.0};

protected:

	//bool EXTENDED = false;
	bool EXTENDED_OUTPUT = true;


};

} // rack::

/// Numeric alues can be applied as quality index (with gain=0.004)
/*
typedef enum {
	/ ** Unset (not processsed) * /
	UNDEFINED=0,
	/ ** Internal error in the computation * /
	ERROR=16,
	/ ** No echo found * /
	CLEAR=64,
	/ ** The highest bin exceeds the threshold -> use reference point * /
	UNDERSHOOTING=96,
	/ ** The bin exceeding the Z threshold has an \c undetect ("dry") bin above -> replace \c undetect with low Z (dB) * /
	OVERSHOOTING=128,
	WEAK=192, /// Echoes only below threshold detected
	/ ** The threshold value was passed between adjacent beams, hence can be interpolated * /
	INTERPOLATION=250
} Reliability;

typedef drain::EnumFlagger<drain::SingleFlagger<Reliability> > reliabilityFlagger;
*/


#endif
