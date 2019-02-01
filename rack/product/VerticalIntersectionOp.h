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
	void filterGroups(const HI5TREE &src, const std::list<ODIMPath> & paths, HI5TREE &dst) const;

};









}

#endif /* VERTICALIntersectionOp_H_ */

// Rack
 // REP