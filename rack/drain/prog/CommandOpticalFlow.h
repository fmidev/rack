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

#ifndef COMMAND_OFLOW_H_
#define COMMAND_OFLOW_H_

//#include "CommandRegistry.h"
#include <drain/prog/CommandInstaller.h>
#include "drain/imageops/FastOpticalFlowOp.h"
#include "drain/imageops/FastOpticalFlowOp2.h"
#include "drain/imageops/BlenderOp.h"


namespace drain {

using namespace image;



/// Utility for creating a Command that runs FastOpticalFlowOp operator.
/**
 *   Optical flow needs two input images, computes derivatives on them
 *   and produces an approximation of motion, ie. the flow.
 *
 *   This utility provides interfacing for those data.
 *
 */
class CmdOpticalFlowBase :  public BeanCommand<FastOpticalFlow2Op> {

public:

	/*
	CmdOpticalFlowBase() {
		//bean.parameters.link("smoothing", smoothing="doubleSmoother:coeff=0.95", "imageOp");
		//average,gaussianAverage,distanceTransformFill
	}
	*/

	/*
	CmdOpticalFlowBase(const CmdOpticalFlowBase & cmd) {
		//bean.parameters.copyStruct(cmd.getParameters(), cmd, *this);
	}
	*/

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

// Rack
