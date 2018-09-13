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
#ifndef POLAR_ODIM_STRUCT
#define POLAR_ODIM_STRUCT

#include <drain/util/Geo.h>

#include "ODIM.h"

namespace rack {

/// Metadata structure for single-radar data (polar scans, volumes and products).
/**
 *
 */
// Consider single-radar odim
class PolarODIM : public ODIM {


public:

	PolarODIM(group_t initialize = ALL) : ODIM(initialize){
		init(initialize);
	};

	PolarODIM(const PolarODIM & odim) : ODIM(ALL) {
		initFromMap(odim);
		getNyquist();
	}

	template <class T>
	PolarODIM(const std::map<std::string,T> & m) : ODIM(ALL) {
		initFromMap(m);
		getNyquist();
	}

	/*
	PolarODIM(const PolarODIM & odim, const drain::image::Image & img){
		initFromMap(odim);
		initFromImage(img);
	}
	*/


	PolarODIM(const drain::image::Image & img, const std::string & quantity="") : ODIM(ALL) {
		initFromImage(img, quantity);
	}



	/// Number of range bins in each ray
	long   nbins;

	/// Number of range bins in each ray
	long   nrays;

	/// Beam-directional bin length [m]
	double rscale;


	/// Longitude position of the radar antenna (degrees), normalized to the WGS-84 reference ellipsoid and datum. Fractions of a degree are given in decimal notation.
	double lon;
	/// Latitude position of the radar antenna (degrees), normalized to the WGS-84 reference ellipsoid and datum. Fractions of a degree are given in decimal notation.
	double lat;
	/// Height of the centre of the antenna in meters above sea level.
	double height;

	/// Antenna elevation angle (degrees) above the horizon.
	double elangle;
	/// The range (km) of the start of the first range bin.
	double rstart;
	/// Index of the first azimuth gate radiated in the scan.
	long   a1gate;


	double startaz;
	double stopaz;

	//double NI; // Maximum Nyquist
	double highprf; //
	double lowprf;  //
	double wavelength;

	/// Freezing level
	double freeze;


	/// Returns recommended coordinate policy (Polar coords, origin at left)
	virtual inline
	const drain::image::CoordinatePolicy & getCoordinatePolicy() const {
		using namespace drain::image;
		static const CoordinatePolicy polarLeft(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT, CoordinatePolicy::WRAP);
		return polarLeft;
	}

	/// Sets number of bins (nbins) and number of rays (nrays)
	inline
	void setGeometry(size_t cols, size_t rows){
		nbins = cols;
		nrays = rows;
	}

	/// Updates object, quantity, product and time information.
	/*!
	 *  Fills empty values. Updates time variables.
	 */
	virtual
	void update(const PolarODIM & odim);


	/// For VRAD, set encoding range to cover [-NI,NI]
	/**
	 *  \return - true if quantity was VRAD
	 */
	inline
	bool optimiseVRAD(){
		if (quantity == "VRAD"){
			setRange(-NI, NI);
			return true;
		}
		else
			return false;
	}

	// Sets how:NI from lowprf if needed. If that fails, tries to derive it from scaling (gain, offset).
	double getNyquist(bool warn = false) const;

	/// Returns elevation angle in radians
	inline
	double getElangleR() const {
		return elangle * drain::DEG2RAD;
	}

	/// Returns the distance along the beam to the center of the i'th bin.
	// CF ? Returns the distance in metres to the start of the measurement volume (i.e. the end nearer to radar).
	inline
	double getBinDistance(size_t i) const {
		return rstart + (static_cast<double>(i)+0.5)*rscale;
	}

	/// Returns the index of bin at given (bin center) distance along the beam.
	inline
	int getBinIndex(double d) const {
		return static_cast<int>((d-rstart)/rscale) ;
	}

	/// Returns the index of a ray at a given azimuth [radians].
	inline
	int getRayIndex(double d) const {
		return static_cast<int>(d*static_cast<double>(nrays)/(2.0*M_PI)) ;
	}

	/// Returns the azimuth in radians of the bin with vertical index j.
	template <class T>
	inline
	double getAzimuth(T j) const {
		return static_cast<double>(j)*2.0*M_PI / static_cast<double>(nrays);
	}

	/// Returns the span of bins for the given azimuthal span.
	/**
	 *  \see getBeamBins()
	 */
	inline
	int getAzimuthalBins(double degree) const {
		return static_cast<int>(degree * static_cast<double>(nrays)/360.0 + 0.5) ;
	}

	/// Returns the span of bins for the given distance range in meters.
	/*
	 *   Returns the number of bins corresponding to a given distance range (in meters).
	 *   \see getAzimuthalBins()
	 */
	inline
	int getBeamBins(double spanM) const {
		return static_cast<int>(spanM/rscale + 0.5) ;
	}

	/// Returns the range in metres (i.e. distance to the end of the last measurement volume).
	inline
	double getMaxRange() const {
		return rstart + static_cast<double>(nbins)*rscale;
		//return getBinDistance(nbins);
	};



	double getGroundAngle(size_t i) const {
		return (static_cast<double>(i)+0.5) * rscale / EARTH_RADIUS_43;
	}

	/// Converts Doppler speed [-NI,NI] to unit circle.
	inline
	void mapDopplerSpeed(double d, double &x, double &y) const {
		d = scaleForward(d) * M_PI/NI;
		//std::cerr << d << '\n';
		x = cos(d);
		y = sin(d);
	}


private:

	virtual // must
	void init(group_t initialize=ALL);


};





}  // namespace rack


#endif

// Rack
