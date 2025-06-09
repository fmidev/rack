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


#ifndef DOPPLER_DEV_OP_H_
#define DOPPLER_DEV_OP_H_

/*
#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/PolarODIM.h"
#include "data/QuantityMap.h"
#include <drain/image/Geometry.h>
#include <drain/image/Image.h>
#include <drain/image/ImageChannel.h>
#include <drain/image/Window.h>
#include <drain/imageops/SlidingWindowOp.h>
#include "product/ProductOp.h"
*/
#include <drain/Log.h>
#include "radar/Doppler.h"
#include <drain/util/Fuzzy.h>
#include "DopplerWindowOp.h"
//#include <cmath>
#include <string>


namespace rack {


class DopplerDevOp : public DopplerWindowOp<DopplerDevWindow> {
public:

	/**
	 *   \param width  - radial extent of window, in metres
	 *   \param height - azimuthal extent of window, in degrees
	 */
	DopplerDevOp(int widthM = 1500, double heightD = 3.0) :
		DopplerWindowOp<DopplerDevWindow>(__FUNCTION__, "Computes std.dev.[m/s] of Doppler field", widthM, heightD) {
		parameters.link("relativeScale", this->conf.relativeScale = true, "true|false");

		//this->conf.relativeScale = true;
		odim.quantity = "VRAD_DEV"; // VRAD_C ?
		odim.scaling.offset = 0.0;
		odim.scaling.scale   = 0.0;
		odim.type = "C";
		odim.nodata = 255; // TODO
	};

	virtual ~DopplerDevOp(){};

protected:

	virtual inline
	double getTypicalMin(const PolarODIM & srcODIM) const {
		return 0.0;
	}

	virtual inline
	double getTypicalMax(const PolarODIM & srcODIM) const {
		return (this->conf.relativeScale) ? +1.0 : +srcODIM.getNyquist();
	}


};


}


#endif /* DOPPLERDevOP_H_ */
