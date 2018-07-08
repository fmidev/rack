/**


    Copyright 2006 - 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

   Created on: Sep 30, 2010
*/

//#pragma once



#ifndef RACK_MOTION
#define RACK_MOTION


#include <drain/prog/CommandOpticalFlow.h>
#include <drain/util/Time.h>
//#include <drain/image/FlowAverageWindow.h>

//#include "../radar/Analysis.h"
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
class CmdMotionFill : public BasicCommand {

public:

	CmdMotionFill() : BasicCommand(__FUNCTION__, "Fills vectors to open areas."){
		this->parameters.reference("width",  this->conf.width  = 5, "pixels");
		this->parameters.reference("height", this->conf.height = 5, "pixels");
		this->parameters.reference("qualitySensitive", this->qualitySensitive = true, "0,1");
	};


	drain::image::FlowAverageWindow::conf_t conf;
	bool qualitySensitive;

	inline  // cf. CmdCompleteODIM
	void exec() const {  // Suits to general base class?

		Logger mout(getName(), __FUNCTION__);

		RackResources & resources = getResources();
		HI5TREE *h5 = resources.currentHi5;
		if (h5 == resources.currentPolarHi5){
			mout.warn() << "not implemented for polar coord data" << mout.endl;
			//MotionFillOp<PolarODIM> op;
		}
		else if (h5 == &resources.cartesianHi5){
			mout.note() << "dst: cartesianHi5" << mout.endl;
			MotionFillOp<CartesianODIM> op;
			op.conf.width  = this->conf.width;
			op.conf.height = this->conf.height;
			op.qualitySensitive = this->qualitySensitive;
			op.processH5(resources.cartesianHi5);
		}
		else {
			mout.warn() << "no class found for currentHi5" << mout.endl;
		}

	};

};
*/

}

#endif
