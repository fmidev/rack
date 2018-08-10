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
#ifndef STATISTICSPROBER_H
#define STATISTICSPROBER_H

#include <sstream>
#include <ostream>
#include <cmath>
//#include "ImageOp.h"
//#include "Coordinates.h"
//#include "FilePng.h"
#include "SegmentProber.h"

namespace drain
{
namespace image
{

/// A structure for accumulating coordinates and computing statistics thereof: area, centroid, horizontality, verticality, variance and elongation.
/** This class is applied by SegmentStatisticProber which implements the recursive traversal through a segment.
 *
 *  All the statistics are based only on locations (i,j), not on intensities.
 *
 */
class SegmentStatistics {

public:

	SegmentStatistics(){
		clear();
	};

	~SegmentStatistics(){};

	/// Resets the statistics.
	virtual inline
	void clear(){
		size = 0;
		sumX = 0;
		sumY = 0;
		sumXX = 0;
		sumYY = 0;
		sumXY = 0;
		_updateNeeded = true;
	};

	/// Accumulate location (i,j) in the statistics.
	virtual inline
	void add(const int &i, const int &j){
		++size;
		sumX  += i;
		sumY  += j;
		sumXX += i*i;
		sumYY += j*j;
		sumXY += i*j;
	}

	/// Returns the size of the segment.
	/**
	 * Size
	 * \[
	 *  s =
	 * \]
	 */
	inline
	double getSize() const { return static_cast<double>(size);};

	/// Returns the horizontal coordinate of centre of mass.
	/**
	 * Size
	 * \[
	 *  m_x = \frac{1}{N}\sum_{i=1}^N x_i
	 * \]
	 */
	inline
	double getMeanX() const {
		updateStats();
		return cx;
	};

	/// Returns the horizontal coordinate of centre of mass.
	/**
	 * Size
	 * \[
	 *  m_x = \frac{1}{N}\sum_{i=1}^N x_i
	 * \]
	 */
	inline
	double getMeanY() const {
		updateStats();
		return cy; ;
	};

	/// Returns the horizontal variance \$\sigma_x\$ scaled with total variance \$\sigma_x + \sigma_y\$. Takes values in [0,1].
	/**
	 * Size
	 * \[
	 *  m_x = \frac{\sqrt{}}{N}
	 * \]
	 */
	inline
	double getHorizontality()  const {
		updateStats();
		return cxx / (cxx + cyy);
		//return sqrt(cxx) / sqrt(cxx + cyy);
	};

	/// Returns the vertical variance scaled with total variance. Takes values in [0,1].
	inline
	double getVerticality() const {
		updateStats();
		return cyy / (cxx + cyy);
		//return sqrt(cyy) / sqrt(cxx + cyy);
	};


	/// Returns the total variance of the pixel coordinates.
	/**
	 *
	 * Returns the squared mean radius from the centroid.
	 * In physics, \e moment of inertia corresponds to this quantity, assuming that the body has constant density.
	 */
	inline
	double getVariance() const {
		return getVarianceHorz() + getVarianceVert();
	};

	inline
	double getVarianceHorz() const {
		updateStats();
		return cxx;
	};

	inline
	double getVarianceVert() const {
		updateStats();
		return cyy;
	};

	/// Returns the proportion in which the variance equals the variance of an annulus.
	/**
	 *
	  The variance returned by getVariance() is defined as
	  \f[
	  E\left[ (x-x_m)^2 + (y-y_m)^2 \right] = E\left[ r^2 \right]
	  \f]
	  where \f$r\f$ is the distance from centroid \f$(x_m,y_m)\f$.

	  For an annulus, with inner radius \f$R_1\f$ and outer radius \f$R_2\f$, the area is
	  \f[
	  A = \int_{R_1}^{R_2} \int_{0}^{2\pi} r d\alpha\! dr = \pi (R_2^2 - R_1^2)
	  \f]
	  and the variance
	  \f[
	  E\left[ r^2 \right]  = \frac{1}{A} \int_{R_1}^{R_2} \int_{0}^{2\pi} r^2 r d\alpha\! dr =  \frac{2\pi}{A}\int_{R_1}^{R_2} r^3 dr
	    =  \frac{2\pi}{4\pi (R_2^2 - R_1^2)} (R_2^4 - R_1^4) =  \frac{R_2^2 + R_1^2}{2}
	  \f]
	 Hence, for circular areas the variance varies between \f$R^2/2\f$ (for circle) and \f$R^2\f$ (for infinitesimally thin annulus).
	 Obviously, these are the lower and upper limits of variance also for segments of any shape (consider a set of points, starting from two).

	 However, thin line segments achieve values close to those of annula. Consider a thing rectangle of "radius" \f$R\f$, ie height \f$2R\f$:
	  \f[
	  E\left[ (x-x_m)^2 + (y-y_m)^2 \right] \approx  E\left[  (y-y_m)^2  \right] = \int_{-R}^{R}y^2dy/\int_{-R}^{R} dy = \frac{1}{3}\frac{R^2+R^2}{R+R} = \frac{1}{3}R^2
	  \f]
     (Under construction.)

     If a segment is circular, its radius squared is \f$r^2=A/\pi\f$, where \f$A\f$ is the area returned by getSize().
     Hence, given a segment of size \f$A\f$ the variance lies between \f$A/2\pi\f$ and \f$A/\pi\f$. The annularity is the variance normalized to these limits:
     \f[
	  \mathrm{annularity} = \frac{\sigma^2-A/2\pi}{A/\pi-A/2\pi} = 2\pi\sigma^2/A - 1.
	  \f]
	 */
	inline
	double getSlimness() const {
		return 2.0*M_PI*getVariance()/sizeD - 1.0;
	};

	/// Returns the proportion in which the variance equals the variance of an annulus.
	/**
	 *   Returns getSlimness() * (1 - getElongation()).
	 */
	/*
	inline
	double getAnnularity() const {
		updateStats();
		return getSlimness() * 2.0 * eigenValue2 / (eigenValue1 + eigenValue2);
	};
	*/


	/// Returns the principal access ratio (e1-e2)/(e1+e2).
	/**
	 *  \return 0, if eigenValue1==eigenvalue2, and 1.0 if eigenValue1>>eigenvalue2.
	 *
	 *  1-e2/e1 = e1-e2 / e1
	 *
	 */
	inline
	double getElongation()  const {
		updateStats();
		return (eigenValue1 - eigenValue2) / (eigenValue1 + eigenValue2);
		//return (eigenValue1 - eigenValue2) / eigenValue1;
	};

	inline
	double getEigenValue1() const {
		updateStats();
		return eigenValue1;
	};


	inline
	double getEigenValue2() const {
		updateStats();
		return eigenValue2;
	};

	inline
	void toOstr(std::ostream & ostr) const {
		ostr << '('   << getMeanX() << ',' << getMeanY() << ')' << ' ';
		ostr << "s="   << getSize() << ',';
		ostr << "h="   << getHorizontality() << ',';
		ostr << "v="   << getVerticality() << ',';
		ostr << "S="   << getVariance() << ',';
		ostr << "C="   << '[' << cxx << ',' << cxy << ';' << cxy << ',' << cyy << ']' << ',';
		ostr << "e12=" << getEigenValue1() << ',' << getEigenValue2() << ',';
		//ostr << "TEMP=" << sqrt(cxxMcyy*cxxMcyy + 4.0*cxy2);
		ostr << "e="   << getElongation() << ',';
		ostr << "l="   << getSlimness() << ',';
	}

protected:


	inline
	void updateStats() const {

		if (!_updateNeeded)
			return;

		sizeD = static_cast<double>(size);

		cx = static_cast<double>(sumX)/sizeD;
		cy = static_cast<double>(sumY)/sizeD;

		if (size == 1){
			cxx = 1.0;
			cyy = 1.0;
			cxy = 0.0;
		}
		else {
			cxx = static_cast<double>(sumXX)/sizeD - cx*cx;
			cxy = static_cast<double>(sumXY)/sizeD - cx*cy;
			cyy = static_cast<double>(sumYY)/sizeD - cy*cy;
		}


		/// Near undetectValue
		const double epsilon = 0.00001;
		if ((cxy > -epsilon) && (cxy < epsilon)){
			if (cxx > cyy){ // ensure  eigenValue1 > eigenValue2
				eigenValue1 = cxx;
				eigenVector1x = 1.0;
				eigenVector1y = 0.0;
				eigenValue2 = cyy;
				eigenVector2x = 0.0;
				eigenVector2y = 1.0;
			}
			else {
				eigenValue1 = cyy;
				eigenVector1x = 0.0;
				eigenVector1y = 1.0;
				eigenValue2 = cxx;
				eigenVector2x = 1.0;
				eigenVector2y = 0.0;
			}
		}
		else {

			/*
			 *
			 *
			 */

			double cxy2 = cxy*cxy;
			double cxxPcyy = cxx+cyy;
			double cxxMcyy = cxx-cyy;
			double temp,scale;

			temp = sqrt(cxxMcyy*cxxMcyy + 4.0*cxy2);
			eigenValue1 = (cxxPcyy + temp)/2.0;
			eigenValue2 = (cxxPcyy - temp)/2.0;
			// std::cerr << '('   << cx << ',' << cy << ')' << ' ';
			// std::cerr << "E12,t=" << eigenValue1 << ',' << eigenValue2 << ',' << temp << '\n';

			temp = cxx-eigenValue1;
			scale = sqrt(cxy2 + temp*temp);
			eigenVector1x = temp/scale;
			eigenVector1y = cxy/scale;

			temp = cxx-eigenValue2;
			scale = sqrt(cxy2 + temp*temp);
			eigenVector2x = temp/scale;
			eigenVector2y = cxy/scale;
		}

		//elongation = eigenValue2 / eigenValue1;
		_updateNeeded = false;
	}

	mutable bool _updateNeeded;

	/// Size of segment (area).
	long int size;

	/// Sum of coordinates.
	long sumX, sumY;

	/// Sum of squared coordinates.
	long  sumXX, sumYY, sumXY;

	/// Size in double precision.
	mutable double sizeD;

	/// Center of mass, x coordinate.
	mutable double cx;

	/// Center of mass, y coordinate.
	mutable double cy;

	/// Elements of covariance matrix.
	mutable double cxx, cyy, cxy;

	mutable double eigenValue1;
	mutable double eigenVector1x;
	mutable double eigenVector1y;
	mutable double eigenValue2;
	mutable double eigenVector2x;
	mutable double eigenVector2y;
	mutable double elongation;
};



inline
std::ostream & operator<<(std::ostream & ostr, const SegmentStatistics & s){
	s.toOstr(ostr);
	return ostr;
}


/// A helper class applied by FloodFillOp and SegmentAreaOp
/**
 *   \tparam T - type of the source image (int by default, but should be floating-type, if src is).
 *   \tparam T2 - type of the destination image, the image to be filled.
 *
 *   \author Markus.Peura@fmi.fi
 */
template <class T, class T2>
class SegmentStatisticsProber : public SegmentProber<T,T2> {

public:

	SegmentStatisticsProber(const Channel &s, Channel & d, const std::string statistics="s") : SegmentProber<T,T2>(s,d), stats(statistics) {
	};

	~SegmentStatisticsProber(){};

	const std::string & stats;

	SegmentStatistics statistics;

protected:

	virtual
	inline
	void clear(){
		statistics.clear();
	};


	virtual
	inline
	void update(int i, int j){
		this->statistics.add(i,j);
	}


};


template <class T, class T2>
std::ostream & operator<<(std::ostream & ostr, const SegmentStatisticsProber<T,T2> & prober){
	ostr << (const SegmentProber<T,T2> &)prober << '\t';
	ostr << prober.statistics;
	return ostr;
}



}
}
#endif /* FLOODFILL_H_ */


// Drain
