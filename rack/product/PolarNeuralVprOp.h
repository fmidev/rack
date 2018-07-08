/*

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

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
		parameters.reference("nbins", odim.nbins = nbins);
		parameters.reference("nrays", odim.nrays = nrays);
		parameters.reference("rscale", odim.rscale = rscale);
		parameters.reference("gain", odim.gain = gain);
		parameters.reference("offset", odim.offset = offset);
		//dataSelector.path = "^.*/data[0-9]+/?$";  ///

		odim.product = "NeuralVPR";
		odim.quantity = "DBZH";
		odim.offset = 0;

		dataSelector.path = ".*/data[0-9]+/?$";
		dataSelector.quantity = "DBZ.*";

	};

	void filter(const HI5TREE &src, const std::map<double,std::string> & srcPaths, HI5TREE &dst) const;





};





}  // ::rack

#endif /*POLARNeuralVPR_H_*/
