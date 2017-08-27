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

#ifndef POLARCB_H_
#define POLARCB_H_

#include <algorithm>
#include "radar/Geometry.h"

//#include "RackOp.h"
#include "PolarProductOp.h"

namespace rack
{

using namespace drain::image;



/// Computes Cb s or bottoms in kilometers for a given dBZ threshold, typically 20, 40 or 45 dBZ.
/*!
 *
 */
class ConvOp : public PolarProductOp {

public:

	/// Currently type and gain are not handled due to char<=> int mapping problem.
	/**
	 *  \param echoTopDBZ - threshold for Cb
	 *  \param cellDiameter - minimum convection cell area
	 *  \param minHeight - minimum height of cells of \c echoTopDBZ
	 *  \param smoothRad - radial smoothing width for intermediate results
	 *  \param smoothAzm - azimuthal smoothing width for intermediate results
	 *
	 */
	 /*  EI KÄYTÖSSÄ \subsection alg_Cb
	 *
	 *  Radar measurement is sparse; the dBZ threshold lies seldom at the beam but more often between two beams or outside all the beams.
	 *  In computing the
	 *
	 *  - (a) Between lower and higher dBZ: the height is interpolated
	 *  - (b) Under \c undetect and above higher dBZ: the height is interpolated, assuming reference measurement \f$ (z_{r},h_{r}) \f$
	 *  - (c) Above two highest beams, which have higher, decreasing dBZ values : the height is extrapolated
	 *  - (d) Above two highest beams, which have higher, increasing dBZ values : the height is extrapolated; lower quality is assigned
	 *  - (e) Above higher dBZ and \undetect measurement : the height is extrapolated; lower quality is assigned
	 *
	 *  \image html  Cb-principle.png "The location of the dBZ threshold with respect to measurements on the beam."
	 *  \image latex Cb-principle.png "The location of the dBZ threshold with respect to measurements on the beam."
	 *
	 */
	/// Probability of convection based on reflectivity and echo tops.
	/**
	 *
	 *   This convection detection algorithm is based on the following assumptions
	 *   -# The reflectivity is typically high, over 30 dBZ
	 *   -# Convection occurs in cells, that is, circular areas having diameters around 1-5 kms.
	 *   -# The height of a convective cell typically reaches at least 5km
	 *
	 *  The user-defined parameters for the algorithm are:
	 *  -# \f$ maxEcho \f$ : minimum reflectivity directly indicating convection
	 *  -# \f$ zCell \f$ : minimum reflectivity of convective area and its top
	 *  -# \f$ cellHeight \f$ : minimum top height of a convective cell (having reflectivity > \f$ zCell \f$)
	 *  -# \f$ cellDiameter \f$ : typical diameter of a convective cell (having reflectivity > \f$ zCell \f$)
	 *
	 *  \image html  convection-principle.png "Parameters of the convection product."
	 *  \image latex convection-principle.png "Parameters of the convection product."
	 *
	 *   Computationally, these properties are handled as fuzzy membership functions.
	 *   - \f$ f_{\mathrm{zMin}}\f$ : minimum reflectivity; a fuzzy threshold at location \f$z\f$ (minimum dBZ) and steepness \f$ z_w \f$
	 *   - \f$ f_{\mathrm{echoTop}}\f$ : echo top for \f$z\f$ ; a fuzzy threshold at location \f$h\f$ (minimum height) and steepness \f$ z_w \f$
	 *   - \f$ f_{\mathrm{area}}\f$ : size of cells of at least \f$z\f$ ; a fuzzy peak at location \f$a\f$ (minimum area) and steepness \f$ a_w \f$
	 *
	 *  The resulting quantity
	 *   \f[
	 *      p_{conv} = f_{\mathrm{zMin}}(z,z_w)f_{echoTop}(z)f_{area}(a,a_w)
	 *   \f]
	 *
	 *  The quality (confidence) of the probability estimation is defined likewise as a product of the component qualities:
	 *   \f[
	 *      q_{conv} = q_{zMax}(z,z_w)q_{echoTop}(z)q_{area}(a,a_w)
	 *   \f]
	 *
	 */
	//
	ConvOp(double maxEchoThreshold = 25.0, double cellDiameter = 3.0, double echoTopThreshold = 2.0, double echoTopDBZ = 20.0, double smoothRad = 0.0, double smoothAzm = 0.0) :
		PolarProductOp(__FUNCTION__, "Computes the probability of convection based on fuzzy cell intensity, area and height."), top(true) {

		parameters.reference("maxEchoThreshold", this->maxEchoThreshold = maxEchoThreshold, "dBZ");
		parameters.reference("cellDiameter", this->cellDiameter = cellDiameter, "km");
		parameters.reference("echoTopThreshold", this->echoTopThreshold = echoTopThreshold, "km");
		parameters.reference("echoTopDBZ", this->echoTopDBZ = echoTopDBZ, "dBZ");

		parameters.reference("smoothAzm", this->smoothAzm = smoothAzm, "?");
		parameters.reference("smoothRad", this->smoothRad = smoothRad, "?");

		allowedEncoding.reference("nbins", odim.nbins = 0l);
		allowedEncoding.reference("rscale", odim.rscale = 0.0);

		// Larissa
		allowedEncoding.reference("undetect", odim.undetect = 0.0);
		allowedEncoding.reference("nodata",   odim.nodata = 0.0);

		dataSelector.quantity = "^DBZH$";
		dataSelector.path = ".*/data[0-9]+/?$";  // groups

		odim.product = "CONVECTION";
		odim.quantity = "PROB";
		//odim.gain = 1.0/250.0;
		//odim.offset = 0;
	};




	double maxEchoThreshold;
	double echoTopDBZ;
	double cellDiameter;
	double echoTopThreshold;

	double smoothAzm;
	double smoothRad;

	const bool top;

protected:

	void processDataSets(const DataSetSrcMap & srcSweeps, DataSetDst<> & dstProduct) const;
	// virtual
	// void filter(const HI5TREE & src, const std::map<double,std::string> & srcPaths, HI5TREE &dst) const;


};





} // ::rack

#endif /*POLARCbTOP_H_*/

// Rack
