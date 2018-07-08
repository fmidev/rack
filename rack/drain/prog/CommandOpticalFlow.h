/**


    Copyright 2015 Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

*/

#ifndef COMMAND_OFLOW_H_
#define COMMAND_OFLOW_H_

#include "CommandRegistry.h"
#include "CommandAdapter.h"

#include "../imageops/FastOpticalFlowOp.h"
#include "../imageops/FastOpticalFlowOp2.h"
#include "../imageops/BlenderOp.h"


namespace drain {

using namespace image;


/*
class FastOpticalFlowOp2 : public FastOpticalFlowOp {

public:

	FastOpticalFlowOp2() : blender(5,5, 'a', 'b', 1) {
		//parameters.reference("smooth", smooth="", "imageOp[/key:value]");
		parameters.append(blender.getParameters(), false);
		//parameters.reference("smooth", smooth="", "[a|g] average, gaussianAverage");
		//parameters.reference("mix", mix="", "[b|m] mix, max");
		//parameters.reference("loops", loops="", "[b|m] mix, max");
		//parameters.reference("smoothing", smoothing="doubleSmoother/coeff:0.98", "imageOp");
	}

	// std::string smooth;
	// std::string mix;
	// std::string loops;

	inline
	bool preSmooth() const {
		return !blender.getSmootherKey().empty();
	}

	mutable
	BlenderOp blender;

protected:


};
*/

/// Utility for creating a Command that runs FastOpticalFlowOp operator.
/**
 *   Optical flow needs two input images, computes derivatives on them
 *   and produces an approximation of motion, ie. the flow.
 *
 *   This utility provides interfacing for those data.
 *
 */
class CmdOpticalFlowBase :  public BeanWrapper<FastOpticalFlowOp2> {

public:

	CmdOpticalFlowBase() {
		//bean.parameters.reference("smoothing", smoothing="doubleSmoother:coeff=0.95", "imageOp");
		//average,gaussianAverage,distanceTransformFill
	}

	/// Main operation: computes differentials and produces approximation of motion
	void exec() const;

	/// Main operation: computes differentials and produces approximation of motion
	//  void compute(const ImageTray<const Channel> & src, ImageTray<Channel> & diff, ImageTray<Channel> & motion) const;

protected:


	/// Desires geometry of the resulting motion field.
	/**
	 *   Implementations may return the resolution of the original data.
	 *   In derived classes, this could be parameters set by user.
	 */
	virtual
	const AreaGeometry & getGeometry() const = 0;


	/// Retrieves the intensity images used as a basis of motion analysis
	virtual
	void getSrcData(ImageTray<const Channel> & channels) const = 0;


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


	/// The result is stored in this channel pack.
	virtual
	void getMotion(size_t width, size_t height, ImageTray<Channel> & channels) const = 0;

	void debugChannels(const ImageTray<const Channel> & channels, int i=-1, int j=-1) const;

};


} /* namespace drain */

#endif /* DRAINLET_H_ */
