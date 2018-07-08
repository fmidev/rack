/*

    Copyright 2001 - 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

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

#ifndef PseudoRhiOP_OP_H_
#define PseudoRhiOP_OP_H_


#include "VolumeOp.h"

#include "data/VerticalODIM.h"

namespace rack {

using namespace drain::image;

///  Pseudo Range-Height indicator
/**
 *
 */
class PseudoRhiOp : public VolumeOp<RhiODIM> {  // Consider class lift
public:

	//PseudoRhiOp(const std::string & name = "PolarProductOp", const std::string & description = "");
	PseudoRhiOp(double az_angle=0.0, long int xsize=500, long int ysize=250, double minRange=1.0, double range=250.0,
			double minHeight=0, double maxHeight=10000, double beamWidth = 0.25, double beamPowerThreshold = 0.01) : //, std::string type="C", double gain=0.5, double offset=-32.0) :
		VolumeOp<RhiODIM>("PseudoRhiOp","Computes vertical intersection in a volume in the beam direction.") {

		odim.object = "XSEC";
		odim.product = "PRHI";

		parameters.reference("az_angle", odim.az_angle = az_angle, "deg");
		parameters.reference("xsize", odim.xsize = xsize, "pix");
		parameters.reference("ysize", odim.ysize = ysize, "pix");

		parameters.reference("minRange",  odim.minRange = minRange, "km");
		parameters.reference("range",  odim.range = range, "km");
		parameters.reference("minHeight", odim.minheight = minHeight, "m");
		parameters.reference("maxHeight", odim.maxheight = maxHeight, "m");
		//reference("levels", odim.levels, levels);

		parameters.reference("beamWidth", this->beamWidth = beamWidth, "deg");
		parameters.reference("beamPowerThreshold", this->weightThreshold = beamPowerThreshold, "0..1");

		// reference("undetectValue", undetectValue, -30.0);  AUTOMATIC, see --quantity DBZH:undetectValue

		//reference("type", odim.type, "C"); // TODO
		//reference("gain", odim.gain, 0.5);
		//reference("offset", odim.offset, -32.0);

		allowedEncoding.reference("type", odim.type = "C");  // TODO: automatic?
		allowedEncoding.reference("gain", odim.gain);
		allowedEncoding.reference("offset", odim.offset);

		dataSelector.path = ".*/data[0-9]+/?$";
		dataSelector.quantity = "^DBZH$";

	}

	virtual
	inline
	~PseudoRhiOp(){};


	double beamWidth;
	double weightThreshold; // = 0.1;
	// double undetectValue;

	/// Implements VolumeOp::filter.
	virtual
	void processDataSets(const DataSetMap<PolarSrc> & src, DataSet<RhiDst> & dstProduct) const;

protected:

	inline
	void setGeometry(const PolarODIM & srcODIM, PlainData<RhiDst> & dstData) const;
	///
	/**
	 *   \param beamWidth2 - beam width squared
	 *   \param angle - elevation angle in the same units as beam width
	 */
	inline
	static
	double relativeBeamPower(double angle, double beamWidth2){
		return beamWidth2 / (beamWidth2 + angle*angle);
	}


};


}  // namespace rack

#endif /* RACKOP_H_ */
