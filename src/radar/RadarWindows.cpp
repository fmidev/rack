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

#include "RadarWindows.h"

namespace rack {


void RadarWindowConfig::setPixelConf(RadarWindowConfig & conf, const PolarODIM & inputODIM) const {

	drain::Logger mout(__FUNCTION__, "RadarWindowConfig");

	// pixelConf = this->conf;  PROBLEM: ftor prevents op=
	conf.widthM  = this->widthM;
	conf.heightD = this->heightD;
	conf.invertPolar   = this->invertPolar;
	conf.contributionThreshold  = this->contributionThreshold;
	conf.relativeScale = this->relativeScale;

	conf.updatePixelSize(inputODIM);


}

// NEW, "Inverted" setPixelConf
void RadarWindowConfig::adjustMyConf(const RadarWindowConfig & conf, const PolarODIM & inputODIM) {

	drain::Logger mout(__FILE__, __FUNCTION__);

	// pixelConf = this->conf;  PROBLEM: ftor prevents op=
	this->widthM  = conf.widthM;
	this->heightD = conf.heightD;
	this->invertPolar = conf.invertPolar;
	this->contributionThreshold = conf.contributionThreshold;
	this->relativeScale = conf.relativeScale;

	updatePixelSize(inputODIM);


}

void RadarWindowConfig::updatePixelSize(const PolarODIM & inputODIM){ // DopplerWindOp wants public

	drain::Logger mout(__FILE__, __FUNCTION__);

	//mout.note(odimSrc );
	this->frame.width  = inputODIM.getBeamBins(this->widthM);
	this->frame.height = inputODIM.getAzimuthalBins(this->heightD);
	//mout.note(this->width , '<' , this->widthM );

	if (inputODIM.area.empty()){
		mout.fail(inputODIM);
		mout.fail("inputODIM area unset");
	}

	if (this->frame.width <= 0){
		// mout.note(this->frame.width  , "pix ~ " , this->widthM , "m " );
		//mout.note(*this );
		mout.warn("Requested width (" , this->widthM ,  " meters) smaller than rscale (", inputODIM.rscale ,"), setting window width=1 " );
		this->frame.width = 1;
	}

	if (this->frame.height == 0){
		mout.warn("Requested height (" , this->heightD ,  " degrees) smaller than 360/nrays (", (360.0/inputODIM.area.height) ,"), setting window height=1 " );
		this->frame.height = 1;
	}

	mout.debug("final size: ", this->frame, " px");


	//mout.note(this->height , '<' , this->heightD );

}


} // rack::

