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
//#include <stddef.h>

/*
#include <string>
#include <drain/image/FilePng.h>
#include <drain/image/TreeSVG.h>
#include <drain/prog/Command.h>
#include <drain/prog/CommandInstaller.h>
#include <drain/prog/CommandBank.h>
#include <drain/util/Output.h>
#include "data/SourceODIM.h" // for NOD

#include "resources.h"
//#include "fileio-svg.h"
#include "graphics.h"
*/

#include "graphics-radar.h"

namespace drain {

}


namespace rack {

void RadarSVG::getCubicBezierConf(CubicBezierConf & conf, double angleStartR, double angleEndR) const {
	//conf.sectorCount = n;
	//conf.delta = 2.0*M_PI / static_cast<double>(n);
	conf.delta = angleEndR-angleStartR;
	double k = 4.0/3.0 * ::tan(conf.delta/4.0);
	conf.radialCoeff = sqrt(1.0 + k*k);
	conf.angularOffset = ::atan(k);
}

/// Single command to draw arc
/**
 *  \param drain::Point2D<int> & imgPoint - starting point, should be "already" compatible with (radiusM, azimutthStartR)
 */
void RadarSVG::cubicBezierTo(drain::svgPATH & elem, double radiusM, double azimuthStartR, double azimuthEndR) const {

	double delta = (azimuthEndR - azimuthStartR);

	if (delta > 2.0*M_PI){
		const double rounds = ::floor(delta/2.0*M_PI);
		azimuthEndR -= rounds*2.0*M_PI;
	}
	else if (delta < -2.0*M_PI){
		const double rounds = ::floor(-delta/2.0*M_PI);
		azimuthEndR += rounds*2.0*M_PI;
	}

	if (::abs(azimuthEndR - azimuthStartR) > conf.delta){// (M_PI_4)
		// split by two
		std::cerr << "delta=" << (azimuthEndR - azimuthStartR) << " splitting... \n";
		double azimuthCenterR = (azimuthStartR + azimuthEndR)/2.0;
		std::cerr << "=> " << azimuthStartR  << '\t' << azimuthCenterR << "\n";
		std::cerr << "=> " << azimuthCenterR << '\t' << azimuthEndR << "\n";
		//exit(1);
		cubicBezierTo(elem, radiusM, azimuthStartR,  azimuthCenterR);
		cubicBezierTo(elem, radiusM, azimuthCenterR, azimuthEndR);
		return;
	}

	drain::Point2D<int> imgPoint;
	convert(radiusM, azimuthEndR, imgPoint);
	cubicBezierTo(elem, imgPoint, radiusM, azimuthStartR, azimuthEndR);
}

/**
 *  \param drain::Point2D<int> & imgPoint - starting point, should be "already" compatible with (radiusM, azimutthStartR)
 */
void RadarSVG::cubicBezierTo(drain::svgPATH & elem, drain::Point2D<int> & imgPoint, double radiusM, double azimuthStartR, double azimuthEndR) const {

	std::cerr << "Start " << azimuthStartR  << '\t' << azimuthEndR << "\n";


	CubicBezierConf conf;
	getCubicBezierConf(conf, azimuthStartR, azimuthEndR);

	// getCubicCoeff(radialResolution, bezierRadialCoeff, bezierAngularOffset);

	const double radiusCtrl = radiusM * conf.radialCoeff;
	double azimuthCtrl = 0.0;

	// Geographic coordinates [metric (or degrees?]
	drain::Point2D<double> radarPointCtrl;

	// Final image coordinates [pix]
	drain::Point2D<int> imgPointCtrl1, imgPointCtrl2;

	azimuthCtrl = azimuthStartR + conf.angularOffset;
	radarPointCtrl.set(radiusCtrl*::sin(azimuthCtrl), radiusCtrl*::cos(azimuthCtrl));
	radarGeoToCompositeImage(radarPointCtrl, imgPointCtrl1);

	azimuthCtrl = azimuthEndR - conf.angularOffset;
	radarPointCtrl.set(radiusCtrl*::sin(azimuthCtrl), radiusCtrl*::cos(azimuthCtrl));
	radarGeoToCompositeImage(radarPointCtrl, imgPointCtrl2);

	elem.absolute<drain::svgPATH::CURVE>(imgPointCtrl1, imgPointCtrl2, imgPoint);

	convert(radiusM, azimuthEndR, imgPoint);

}


} // rack


