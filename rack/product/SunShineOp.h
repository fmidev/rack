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

// Rack
