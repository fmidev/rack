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

#ifndef PseudoRhiOP_OP_H_
#define PseudoRhiOP_OP_H_

#include <string>

#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/PolarODIM.h"
#include "data/VerticalODIM.h"
#include "product/VolumeOp.h"

namespace rack {

using namespace drain::image;

///  Pseudo Range-Height indicator
/**
 *
 */
class PseudoRhiOp : public VolumeOp<RhiODIM> {  // Consider class lift
public:

	//PseudoRhiOp(const std::string & name = "PolarProductOp", const std::string & description = "");
	PseudoRhiOp(double az_angle=0.0, long int xsize=500, long int ysize=250, double minRange=0.0, double range=0.0,
			double minHeight=0, double maxHeight=10000, double beamWidth = 0.25, double beamPowerThreshold = 0.01) : //, std::string type="C", double gain=0.5, double offset=-32.0) :
		VolumeOp<RhiODIM>("PseudoRhiOp","Computes vertical intersection in a volume in the beam direction.") {

		odim.object = "XSEC";
		odim.product = "PRHI";

		parameters.link("az_angle", odim.az_angle = az_angle, "deg");
		parameters.link("size", odim.area.tuple(xsize, ysize), "pix");
		// parameters.link("xsize", odim.area.width = xsize, "pix");
		// parameters.link("ysize", odim.area.height = ysize, "pix");

		odim.range.set(minRange, range);
		// Odim Version km/m?
		parameters.link("range",  odim.range.tuple(),  "m");

		odim.altitudeRange.set(minHeight, maxHeight);
		parameters.link("height", odim.altitudeRange.tuple(), "m" );

		/*
		parameters.link("minRange",  odim.minRange = minRange, "km");
		parameters.link("range",  odim.range = range, "km");
		parameters.link("minHeight", odim.minheight = minHeight, "m");
		parameters.link("maxHeight", odim.maxheight = maxHeight, "m");
		*/
		//link("levels", odim.levels, levels);

		parameters.link("beamWidth", this->beamWidth = beamWidth, "deg");
		parameters.link("beamPowerThreshold", this->weightThreshold.tuple(0.5*beamPowerThreshold, beamPowerThreshold), "0..1").setFill(true);

		// link("undetectValue", undetectValue, -30.0);  AUTOMATIC, see --quantity DBZH:undetectValue
		//link("type", odim.type, "C"); // TODO
		//link("gain", odim.scaling.scale, 0.5);
		//link("offset", odim.scaling.offset, -32.0);

		allowedEncoding.link("type", odim.type = "C");  // TODO: automatic?
		allowedEncoding.link("gain", odim.scaling.scale);
		allowedEncoding.link("offset", odim.scaling.offset);

		dataSelector.setQuantities("^DBZH$");

	}

	virtual
	inline
	~PseudoRhiOp(){};


	double beamWidth;
	// double weightThreshold; // = 0.1;
	drain::Range<double> weightThreshold = {0.1,0.2}; // = 0.1;

	virtual
	void setPolarSelector(const PolarSelector & ps) override;


	/// Implements VolumeOp::filter.
	virtual
	void computeSingleProduct(const DataSetMap<PolarSrc> & src, DataSet<RhiDst> & dstProduct) const;

protected:

	inline
	void setGeometry(const PolarODIM & srcODIM, PlainData<RhiDst> & dstData) const;

	/// Simple non-gaussian model for a beam.
	/**
	 *   \param beamWidth2 - beam width squared
	 *   \param angle - elevation angle in the same units as beam width
	 */
	static inline
	double relativeBeamPower(double angle, double beamWidth2){
		return beamWidth2 / (beamWidth2 + angle*angle);
	}


};


}  // namespace rack

#endif /* RACKOP_H_ */

// Rack
