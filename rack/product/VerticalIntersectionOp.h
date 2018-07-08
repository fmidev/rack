/*

    Copyright 2012 -    Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack software for C++.

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

#ifndef VERTICALIntersectionOp_H_
#define VERTICALIntersectionOp_H_

#include "VolumeOp.h"
#include "data/VerticalODIM.h"

namespace rack {

// PLACEHOLDER  Computes vertical Intersections of reflectivity (DBZH), including raw and polarized reflectivity.
/*
 *   The resulting image is oriented "naturally", ie. minHeight and maxHeight mapped to the bottom and top rows, respectively.
 */
class VerticalIntersectionOp: public rack::VolumeOp<VerticalProfileODIM> {

public:

	VerticalIntersectionOp() :
				VolumeOp<VerticalProfileODIM>("VerticalIntersectionOp","Computes vertical dBZ distribution in within range [minRange,maxRange] km.") {

			/*
				reference("minHeight", odim.minheight, minHeight);
				reference("maxHeight", odim.maxheight, maxHeight);
				//reference("type", odim.type, type); // TODO
				reference("type", odim.type, "d");
				reference("gain", odim.gain, gain);
				reference("offset", odim.offset, offset);

				//reference("minHeight", this->minHeight, minHeight); todo: angles
				//reference("elangleMin", dataSelector.elangleMin, -10.0);
				//reference("elangleMax", dataSelector.elangleMax, +90.0);
				dataSelector.path = ".* /data[0-9]+/?$";
				dataSelector.quantity = "DBZ|TH|TV|RHOHV|LDR|PHIDP|KDP";
			*/
	}


	virtual ~VerticalIntersectionOp(){};

	VerticalCrossSectionODIM odim;


protected:

	virtual
	void filterGroups(const HI5TREE &src, const std::list<std::string> & paths, HI5TREE &dst) const;

};









}

#endif /* VERTICALIntersectionOp_H_ */
