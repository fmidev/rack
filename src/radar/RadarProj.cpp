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
#include <drain/Log.h>
#include <math.h>
#include <limits>
#include "Geometry.h"
#include "RadarProj.h"


namespace rack {


void RadarProj::determineBoundingBoxM(double range, double & xLL, double & yLL, double & xUR, double & yUR) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	//mout.debug2("start\n" , *this );

	mout.debug2("range=" , range );

	xLL = +std::numeric_limits<double>::max();
	yLL = +std::numeric_limits<double>::max();
	xUR = -std::numeric_limits<double>::max();
	yUR = -std::numeric_limits<double>::max();

	double azimuth;
	double x,y;

	// Redesign this
	const int azimuthStep = 6;
	for (int a = 0; a < 360; a += azimuthStep) {

		/// Map AEQD => target (composite)
		azimuth = static_cast<double>(a) * drain::DEG2RAD;
		projectFwd(range*sin(azimuth), range*cos(azimuth), x, y);

		//mout.debug(5) << x << ',' << y << mout.endl;
		xLL = std::min(x, xLL);
		yLL = std::min(y, yLL);
		xUR = std::max(x, xUR);
		yUR = std::max(y, yUR);

		//mout.warn(x , ',' , y , "\t <=> " , a , '\t' , xLL , ',' , yLL , '\t' , xUR , ',' , yUR , '\t' );

	}

	/*
	if (isLongLat()){
		xLL *= drain::RAD2DEG;
		yLL *= drain::RAD2DEG;
		xUR *= drain::RAD2DEG;
		yUR *= drain::RAD2DEG;
	}
	*/

	mout.debug2(xLL , ',' , yLL , ':' , xUR , ',' , yUR );


}

/* Makes no sense
void RadarProj::determineBoundingBoxD(double range, double & xLL, double & yLL, double & xUR, double & yUR) const {

	drain::Logger mout("RadarProj", __FUNCTION__);

	mout.debug2("start " , *this );

	mout.debug2("range=" , range );

	xLL = +std::numeric_limits<double>::max();
	yLL = +std::numeric_limits<double>::max();
	xUR = -std::numeric_limits<double>::max();
	yUR = -std::numeric_limits<double>::max();


	double azimuth;
	double x,y;

	const unsigned int azimuthStep = 6;
	for (int a = 0; a < 360; a += azimuthStep) {

		azimuth = static_cast<double>(a) * drain::DEG2RAD;
		x = range*sin(azimuth);
		y = range*cos(azimuth);
		//projectFwd(range*sin(azimuth), range*cos(azimuth), x, y);
		mout.debug(5) << x << ',' << y << mout.endl;

		xLL = std::min(x,xLL);
		yLL = std::min(y,yLL);
		xUR = std::max(x,xUR);
		yUR = std::max(y,yUR);
	}

	mout.debug2(xLL , ',' , yLL , ':' , xUR , ',' , yUR );

}
*/


} // ::rack


// Rack
