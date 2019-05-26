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
#ifndef DopplerNoise_H_
#define DopplerNoise_H_

//#include "PolarProductOp.h"
#include <drain/imageops/ImageOp.h>
#include "DetectorOp.h"
#include "../radar/Doppler.h"

using namespace drain::image;

namespace rack {

///
/**

 *
 */
class DopplerNoiseOp: public DetectorOp {

public:

	/**
	 *
	 *  \param vradDevMin
	 *  \param windowWidth
	 *  \param windowHeight
	 *
	 */
	inline
	DopplerNoiseOp(double vradDevMin = 5.0, double windowWidth = 2500, double windowHeight = 5.0) :

		DetectorOp(__FUNCTION__, "Detects variance Doppler (VRAD).", "nonmet.bio.bird"){

		// dataSelector.path = "da ta[0-9]+/?$";
		dataSelector.quantity = "^(VRAD|VRADH)$";
		dataSelector.count = 1;

		parameters.reference("vradDevMin", this->vradDevMin = vradDevMin, "Minimum of bin-to-bin Doppler speed (VRAD) deviation (m/s)");
		parameters.reference("windowWidth", this->conf.widthM = windowWidth, "window width, beam-directional (m)"); //, "[m]");
		parameters.reference("windowHeight", this->conf.heightD = windowHeight, "window width, azimuthal (deg)"); //, "[d]");

	};

	virtual
	inline
	~DopplerNoiseOp(){};

	double vradDevMin;
	DopplerDevWindow::conf_t conf;
	//double windowWidth;
	//double windowHeight;

	// string functor ? TODO

	virtual
	void processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & dstAux) const;



};


}

#endif

// Rack
