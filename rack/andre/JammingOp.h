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

#ifndef Jamming2_OP_H_
#define Jamming2_OP_H_

#include <limits>

//#include <drain/image/AreaOp.h>
#include <drain/image/RunLengthOp.h>
#include <drain/image/BasicOps.h>


#include <drain/image/DistanceTransformOp.h>
//#include <drain/image/FuzzyOp.h>


#include <drain/image/File.h>

#include <drain/util/Math.h>

#include "DetectorOp.h"


//using namespace drain::image;

using namespace drain::image;

namespace rack {

/// A detector for widespread electromagnetic interference.
/** Detects smooth, widespread interference.
 *  Based on fitting a function that models how radar receives external emitters:
 *  \f[
 *    dbz = b_1 + b_1x + b_2*\log(x);
 *  \f]
 *
 */
class JammingOp: public DetectorOp {

public:

	/** Default constructor.
	 *
	 *   \param smoothnessThreshold - maximum standard deviation in a 5x3 sliding window
	 *   \param distanceMin - minimum sampling distance [km] for curve fitting
	 *   \param weightLower - if not 1.0, refit the curve, weighting samples lower the initial curve with this.
	 *   \param debugRow -
	 *
	 *   \param sampleContent - minimum percentage of valid (detected) bins along the beam
	 *   \param maxCurvature - b2 in fitting dbz = b0 + b1*x + b2*x*x;
	 *
	 *   5.0,0.25,2.0,0.001,40
	 */
	//	\param derivativeDifferenceMax - maximum difference between modelled and detected dBZ(range) slopes.
	// \param windowWidth - windowWidth [kilometres]
	// JammingOp(int windowWidth=5000, float windowHeight=10.0, float sensitivity = 0.5, float eWidth = 1.0f, float eHeight = 0.0f) :
	// JammingOp(double smoothnessThreshold = 5.0, double sampleContent=0.5, double weightLower = 0.1, double maxCurvature = 0.001, double distanceMin = 40.0, int debugRow=-1) :
	JammingOp(double smoothnessThreshold = 5.0, double distanceMin = 80.0, double weightLower = 2.0, int debugRow=-1) : // , double derivativeDifferenceMax = 0.0001
		DetectorOp(__FUNCTION__,"Detects broad lines caused by electromagnetic interference. Intensities should be smooth, increasing by distance.", ECHO_CLASS_JAMMING)
	{
		parameters.reference("smoothnessThreshold", this->smoothnessThreshold = smoothnessThreshold, "dBZ");
		parameters.reference("distanceMin", this->distanceMin = distanceMin, "km");
		parameters.reference("weightLower", this->weightLower = weightLower, "[0.0...1.0]");
		parameters.reference("debugRow", this->debugRow = debugRow, "index");

		UNIVERSAL = true;
		REQUIRE_STANDARD_DATA = true;
	}


	double smoothnessThreshold;
	double distanceMin;
	//double sampleContent;
	double weightLower;
	//double derivativeDifferenceMax;

	/**
	 *  Curvature is computed as a maximum difference between the fitted 2nd order polynomial curve
	 *  \f[
	 *    \hat{y_2}(x) = b_0 + b_1x + b_2x^2
	 *  \f]
	 *  and the direct line crossing its point values, \f$(0,b_0)\f$ and \f$(w,\hat{y_2}(w))\f$ where \f$w\f$ is the maximum of \f$x\f$.
	 *  Denote \f$d_y=\hat{y_2}(w)-\hat{y_2}(0)\f$.
	 *  Then, the line is
	 *  \f[
	 *    \hat{y_1}(x) = b_0 + \frac{x}{w}d_y
	 *  \f]
	 *  and the difference
	 *  \f[
	 *    d(x) = \hat{y_2}(x) - \hat{y_1}(x) = x(b_1 - \frac{d_y}{w}) + b_2x^2
	 *  \f]
	 *  and its derivative is
	 *  \f[
	 *    d'(x) = b_1 - \frac{d_y}{w} + 2b_2x .
	 *  \f]
     * Setting \f$d'(x)=0\f$ we get
	 *  \f[
	 *    x = (\frac{d_y}{w}-b_1)/2b_2
	 *  \f]
     *
	 *   \f$(0,b_0)\f$ and \f$(x_w,b_0 + b_1x_w + b_2x_w^2)\f$ where \f$x_w\f$ is the maximum of x.
	 */
	//double maxCurvature;


	int debugRow;

	/// Model for radar amplification of an external emitter.
	/**
	 *  \param distance - distance in metres
	 *
	 */
	static
	inline
	double modelledEmitter(double dBZ0, double distance){
		return dBZ0 + distance*(0.016/1000.0) + 20.0*log10(distance);
	};

protected:

	virtual
	void processData(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstProb) const;
	//void filterImage(const PolarODIM &odimIn, const Image &src, Image &dst) const;

	/*
	static
	void fitCurve(const std::vector<double> &src, const std::vector<double> &weight, std::vector<double> &coeff);

	virtual
	void filterImageOLD(const PolarODIM &odimIn, const Image &src, Image &dst) const;
	*/

	/// Returns the beam-oriented derivative of the dBZ field. In case of no-data, returns std::numeric_limits<double>::max().
	inline
	static
	double derivativeDBZ(const PolarODIM &odimIn, const Image &src, const int & i, const int & j, const int span = 10){
		const int iLower = std::max(i-span, 0);
		const double zLower = src.get<double>(iLower,j);
		if ((zLower != odimIn.nodata)&&(zLower != odimIn.undetect)){
			const int iUpper = std::min(i+span, static_cast<int>(src.getWidth())-1);
			const double zUpper = src.get<double>(iUpper,j);
			if ((zUpper != odimIn.nodata)&&(zUpper != odimIn.undetect)){
				return (zUpper - zLower)/(odimIn.rscale*static_cast<double>(iUpper-iLower));
			}
		}
		return std::numeric_limits<double>::max();
	}

	/**
	*/
	inline
	static
	double derivativeDBZ_Modelled(double yModelBase, double range, double rangeDifference){
		return (modelledEmitter(yModelBase, range+rangeDifference) - modelledEmitter(yModelBase, range-rangeDifference)) / (2.0*rangeDifference);
	}


};

}

#endif // Jamming2_OP_H_

// Rack
