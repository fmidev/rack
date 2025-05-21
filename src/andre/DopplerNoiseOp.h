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
#include "drain/imageops/ImageOp.h"
#include "radar/Doppler.h"

#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

/// Detects suspicious variance in Doppler speed (VRAD).
/**
 *
 */
class DopplerNoiseOp: public DetectorOp {

public:

	/**
	 *
	 *  \param speedDevThreshold - Minimum of bin-to-bin Doppler speed (VRAD) deviation (m/s)
	 *  \param windowWidth - window width [m]
	 *  \param windowHeightD - window height [deg]
	 *
	 *  \include aDopplerNoise.hlp
	 */
	inline
	DopplerNoiseOp(double speedDevThreshold = 3.0, double windowWidthM = 1500, double windowHeightD = 3.0) :

		DetectorOp(__FUNCTION__, "Detects suspicious variance in Doppler speed (VRAD).", "nonmet.biol.dopplernoise"){

		// dataSelector.path = "da ta[0-9]+/?$";
		// dataSelector.setQuantityRegExp("^(VRAD|VRADH)$");
		dataSelector.setQuantities("VRADH:VRAD");
		// dataSelector.setMaxCount(1);

		parameters.link("speedDevThreshold", this->speedDevThreshold = speedDevThreshold, "Minimum of bin-to-bin Doppler speed (VRAD) deviation (m/s)");
		parameters.link("windowWidth", this->conf.widthM = windowWidthM, "window width, beam-directional (m)"); //, "[m]");
		parameters.link("windowHeight", this->conf.heightD = windowHeightD, "window height, azimuthal (deg)"); //, "[d]");
		this->conf.relativeScale = false;
		this->conf.invertPolar = true;
	};

	inline
	DopplerNoiseOp(const DopplerNoiseOp &op):  DetectorOp(op), conf(op.conf){
		parameters.copyStruct(op.getParameters(), op, *this);
	}

	virtual
	inline
	~DopplerNoiseOp(){};

	double speedDevThreshold = 3.0;
	DopplerDevWindow::conf_t conf;


	virtual
	void runDetection(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & dstAux) const;



};


}

#endif

// Rack
