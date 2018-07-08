/*

    Copyright 2016 -   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef SunShineOp_H_
#define SunShineOp_H_

#include <algorithm>



//#include "RackOp.h"
#include "PolarProductOp.h"

namespace rack
{

using namespace drain::image;


/// A single-ray "product" storing the altitude at each bin.
/**
*/
class SunShineOp : public PolarProductOp {

public:

	SunShineOp() : // const std::string & type="S", double gain=100.0, long int nbins=0, double rscale=0) :
		PolarProductOp(__FUNCTION__, "Computes the sun position")
	{
		dataSelector.count = 1;  // or 0?  does not need input data arrays

		odim.product = "SUNSHINE";
		odim.nbins = 500;
		odim.nrays = 360;
		odim.rscale = 500;

		odim.type = "C";
		odim.gain = 1.0/200.0;
		odim.offset = -0.1;

		//parameters.reference("quantity", odim.quantity);
		parameters.reference("timestamp", timestamp="200527071845");
		parameters.reference("quantity", odim.quantity="SUNSHINE");

		odim.quantity = "SUNSHINE";
		//dataSelector.quantity = "";
		// NOTE! A dataset for each elevation groups; should suffit for nbins and rscale.
		// However, if a user wants to use quantity, /dataN/ should be reached.


		this->allowedEncoding.reference("type", odim.type);
		this->allowedEncoding.reference("gain", odim.gain);
		this->allowedEncoding.reference("offset", odim.offset);


	};

	std::string timestamp;
	//std::string quantity;

	virtual
	void processData(const Data<PolarSrc> & src, Data<PolarDst> & dst) const;



};





}  // ::rack

#endif /*POLARCappi_H_*/
