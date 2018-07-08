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
#ifndef BEAMALTITUDE_H_
#define BEAMALTITUDE_H_

#include <algorithm>



//#include "RackOp.h"
#include "PolarProductOp.h"

namespace rack
{

using namespace drain::image;


/// A single-ray "product" storing the altitude at each bin.
/** The main idea is to compute one equidistant arc at a time.
   Has no parameters, but target scaling shoud be "S,100" or "C,0.1", for example.
*/
class BeamAltitudeOp : public PolarProductOp {

public:

	BeamAltitudeOp() : // const std::string & type="S", double gain=100.0, long int nbins=0, double rscale=0) :
		PolarProductOp(__FUNCTION__,"Computes the altitude at each bin")
	{

		parameters.reference("altitudeReference", this->aboveSeaLevel = true, "0=radar site|1=sea level");

		odim.product  = "ALTITUDE";
		odim.quantity = "HGHT";

		dataSelector.quantity = "";
		dataSelector.path = "dataset[0-9]+/data[0-9]+/?$";  // NOTE! A dataset for each elevation groups; should suffit for nbins and rscale. However, if a user wants to use quantity, /dataN/ should be reached.
		dataSelector.count = 1;

		this->allowedEncoding.reference("type",   odim.type = "C");
		this->allowedEncoding.reference("gain",   odim.gain = 0.1);
		this->allowedEncoding.reference("offset", odim.offset = 0.0);

		odim.nrays = 1;

		// allowedEncoding.reference("type", odim.type, "S");
		// allowedEncoding.reference("gain", odim.gain, 0.001);

	};

	virtual
	void processData(const Data<PolarSrc> & src, Data<PolarDst> &dst) const;

protected:

	virtual
	void setGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		dstData.odim.nbins = (odim.nbins>0) ? odim.nbins : srcODIM.nbins;
		dstData.odim.nrays = 1;
		dstData.odim.rscale = (static_cast<double>(srcODIM.nbins) * srcODIM.rscale + srcODIM.rstart) / static_cast<double>(dstData.odim.nbins);
		dstData.data.setGeometry(dstData.odim.nbins, dstData.odim.nrays);
	};


};





}  // ::rack

#endif /*POLARCappi_H_*/
