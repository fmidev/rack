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

//#pragma once



#ifndef RACK_MOTION
#define RACK_MOTION


#include "drain/prog/CommandOpticalFlow.h"
#include "drain/util/Time.h"

#include "resources.h"


namespace rack {

class CartesianOpticalFlow : public drain::CmdOpticalFlowBase {

public:

	virtual
	~CartesianOpticalFlow(){}

	virtual inline
	const AreaGeometry & getGeometry() const {
		return areaGeometry;
	};

protected:

	mutable	AreaGeometry areaGeometry;

	/// For storing 1  st input timestamp
	mutable drain::Time t1;

	/// For storing 2nd input timestamp
	mutable	drain::Time t2;

	virtual
	void getSrcData(ImageTray<const Channel> & src) const;

	/// Retrieves the difference image needed as a temporary storage. Maybe viewed for debugging.
	/**
	 *   It is recommended that the image is of type double, as computation of
	 *   differentials (involving functors) and OpticalFlowWindow use doubles.
	 *   \param width - width of the image
	 *   \param height - height of the image
	 *   \param max - expected physical max value  (needed if small-integer data used; esp. for subtractions).
	 *   \param channels - provided resource
	 *
	 */
	virtual
	void getDiff(size_t width, size_t height, double max, ImageTray<Channel> & channels) const;

	void getMotion(size_t width, size_t height, ImageTray<Channel> & channels) const;


};


// NOTE: could be under general ProductAdapter (see RackLetAdapter and AndreAdapter), if (currentH5 == cartesian)
/*
class CmdMotionFill : public drain::BasicCommand {

public:

	CmdMotionFill() : drain::BasicCommand(__FUNCTION__, "Fills vectors to open areas."){
		this->parameters.link("width",  this->conf.width  = 5, "pixels");
		this->parameters.link("height", this->conf.height = 5, "pixels");
		this->parameters.link("qualitySensitive", this->qualitySensitive = true, "0,1");
	};


	drain::image::FlowAverageWindow::conf_t conf;
	bool qualitySensitive;

	inline  // cf. CmdCompleteODIM
	void exec() const {  // Suits to general base class?

		 drain::Logger mout(__FUNCTION__, getName());

		RackResources & resources = getResources();
		Hi5Tree *h5 = ctx.currentHi5;
		if (h5 == ctx.currentPolarHi5){
			mout.warn("not implemented for polar coord data" );
			//MotionFillOp<PolarODIM> op;
		}
		else if (h5 == &ctx.cartesianHi5){
			mout.note("dst: cartesianHi5" );
			MotionFillOp<CartesianODIM> op;
			op.conf.width  = this->conf.width;
			op.conf.height = this->conf.height;
			op.qualitySensitive = this->qualitySensitive;
			op.processH5(ctx.cartesianHi5);
		}
		else {
			mout.warn("no class found for currentHi5" );
		}

	};

};
*/

}

#endif

// Rack
