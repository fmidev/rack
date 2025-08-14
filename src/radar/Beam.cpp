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

#include "Beam.h"


/*! \file
 *  This file contains radar constants...
 */
// 6,370
namespace rack {


/// Given deviation from beam in degrees, return relative (normalized) beam power

double Beam::getBeamPowerDeg(double d) const {
	// Returns 0 in the center (d=0.0) and 0.5 when d = +/-0.5*width
	// e^x = ½ => x = ln(1/2) = - ln(2)
	// e^(-x²) = ½ => -x = ln(1/sqrt(2)) = -ln(sqrt2) =
	// x = ln(sqrt2)

	// 2025
	static
	const double coeff = 4.0 * log(sqrt(2.0)); // 4 = 2*2 for (1/(½width))²

	return exp(- (d*d) * coeff/(width*width));
}





} // ::rack


