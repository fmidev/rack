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
#ifndef CAPPI2_OP_H_
#define CAPPI2_OP_H_

//#include <algorithm>



//#include "RackOp.h"
#include "CumulativeProductOp.h"

#include "radar/Beam.h"

namespace rack
{

using namespace drain::image;


/// CAPPI and Pseudo-CAPPI: the constant altitude planar position indicator product
/**
 *
 *
 */
class CappiOp : public CumulativeProductOp {

public:

	/// Nominal height of horizontal intersection.
	double altitude = 1000.0; // meters

	//
	Beam beam = 1.0; // degrees
	double weightMin = -0.1; // limit for

	bool COMPUTE_HGHT = false;

	/// Pseudo-CAPPI: the constant altitude planar position indicator.
	/**
	 *  \param altitude - altitude of the horizontal intersection
	 *  \param aboveSeaLevel - measure altitude from sea level (true) or radar site altitude (false)
	 *  \param beamWidth - half-power width of Gaussian beam model
	 *  \param weightMin - threshold [0...1] for normalized beam power interpreted as weight; set -1 to include "pseudo" areas
	 *  \param accumulationMethod - define how dBZ and weight of each bin contributes to the product
	 */
	CappiOp(double altitude=1000.0, bool aboveSeaLevel=true, double beamWidth=1.0, double weightMin=-0.1, std::string accumulationMethod="WAVG:1:8:-40");

	/// Copy constructor.
	CappiOp(const CappiOp &op);
	//inline CappiOp(const CappiOp &op) : CumulativeProductOp(op), altitude(1000.0), weightMin(-1.0) {};

	void processData(const Data<PolarSrc> & data, RadarAccumulator<Accumulator,PolarODIM> & accumulator) const override;

	virtual inline
	const std::string & getOutputQuantity(const std::string & inputQuantity = "") const override {
		static const std::string HGHT("HGHT");
		if (COMPUTE_HGHT){
			return HGHT;
		}
		else {
			return CumulativeProductOp::getOutputQuantity(inputQuantity);
		}
	}



};

}  // ::rack

#endif
