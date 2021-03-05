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
#ifndef POLAR_Neural_VPR_OP_H_
#define POLAR_Neural_VPR_OP_H_

#include <algorithm>



//#include "RackOp.h"
#include "CumulativeProductOp.h"

namespace rack
{

using namespace drain::image;


/// The VPRimum dBZ value in each a vertical column.
/** The main idea is to compute one equidistant arc at a time.

 */
//
// TODO: selecting certain elevations (allowed range)
class PolarNeuralVPROp : public PolarProductOp {

public:

	/// Experimental parameter handling

	/// Todo: rscale*,nbins*,nrays*,beamwidth*
	PolarNeuralVPROp(long int nbins=0, long int nrays=0, double rscale=0, double gain=0.0, double offset=0) :
		PolarProductOp("PolarNeuralVPR","Computes NeuralVPR.")
		{
		// TODO already ref'd
		parameters.link("nbins", odim.area.width = nbins);
		parameters.link("nrays", odim.area.height = nrays);
		parameters.link("rscale", odim.rscale = rscale);
		parameters.link("gain", odim.scaling.scale = gain);
		parameters.link("offset", odim.scaling.offset = offset);
		//dataSelector.path = "^.*/data[0-9]+/?$";  ///

		odim.product = "NeuralVPR";
		odim.quantity = "DBZH";
		odim.scaling.offset = 0;

		dataSelector.quantity = "DBZ.*";

	};

	void filter(const Hi5Tree &src, const std::map<double,std::string> & srcPaths, Hi5Tree &dst) const;





};





}  // ::rack

#endif /*POLARNeuralVPR_H_*/

// Rack
