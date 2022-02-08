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
#ifndef RACK_RADAR_BEAM
#define RACK_RADAR_BEAM

//
#include <math.h>

#include <drain/util/Fuzzy.h>
#include <drain/util/Math.h>
#include <drain/util/Geo.h>


/*! \file
 *  This file contains radar constants...
 */
// 6,370
namespace rack {


class Beam {

public:

	/// Beam width in degrees.
	double width;

	/// Set beam width in degrees.
	inline
	void setBeamWidthDeg(double width){
		this->width = width;
		fuzzyBell.set(0,  0.5 * width); // half-width
		fuzzyBell2.set(0, 0.5 * width); // half-width
	}

	/// Set beam width in radians.
	inline
	void setBeamWidthRad(double width){
		setBeamWidthDeg(width * drain::RAD2DEG);
	}


	/// Gaussian beam power
	/**
	 * 	\param d - deviation from beam center in degrees
	 *  \return relative (normalized) beam power: 0 in the center (d=0.0) and 0.5 when d = +/-0.5*width
	 */
	double getBeamPowerDeg(double d) const;

	/// Gaussian beam power
	/**
	 * 	\param d - deviation from beam center in radians
	 *  \return relative (normalized) beam power
	 */
	inline
	double getBeamPowerRad(double d) const {
		return getBeamPowerDeg(drain::RAD2DEG * d);
	}


	/** Fuzzy alternatives for fast computation.
	 *  Note: seldom needed, unless computed in the innermost loop.
	 */

	// A fuzzy beam power model, with +/- 0.1 degree beam "width".
	drain::FuzzyBell<double>  fuzzyBell;

	// A fuzzy beam power model, with +/- 0.1 degree beam "width".
	drain::FuzzyBell2<double> fuzzyBell2;
	//drain::FuzzyBell<double> beamPower:


};


} // ::rack


#endif

// Rack
