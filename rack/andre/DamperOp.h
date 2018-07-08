/**

    Copyright 2011-2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
/*
 * DamperOp.h
 *
 *  Created on: Aug 7, 2011
 *      Author: mpeura
 */

#ifndef DamperOP_H_
#define DamperOP_H_


#include <drain/util/Fuzzy.h>
#include <drain/image/Intensity.h>
#include <drain/image/File.h>

#include "hi5/Hi5.h"
#include "radar/Geometry.h"

#include "RemoverOp.h"

using namespace drain::image;

namespace rack {

/**  Turns reflectance (DBZH) values of anomalies down smoothly applying fuzzy deletion operator.
 *
 *   TODO: add minus-dbz range down to e.g. -25?
 */
class DamperOp: public RemoverOp {

public:

	/** Default constructor.
	 *  \param threshold - quality values below this will be considered
	 *  \param undetectThreshold - q index below which bins are marked 'undetect'.
	 *  \param dbzMin - reflectance towards which values are decremented.
	 */
	DamperOp(double threshold=0.5, double undetectThreshold=0.1, double dbzMin=-40.0) :  // double slope=0.0,
		RemoverOp(__FUNCTION__,"Removes anomalies with fuzzy deletion operator."){

		dataSelector.quantity = "^DBZH$";

		parameters.reference("threshold", this->threshold = threshold);
		parameters.reference("undetectThreshold", this->undetectThreshold = undetectThreshold);
		parameters.reference("dbzMin", this->dbzMin = dbzMin);

	};


	double threshold;
	double undetectThreshold;
	double dbzMin;

protected:

	/**
	 *  Practically, qualityRoot == dstDataRoot
	 */
	//void filterDataGroup(const HI5TREE &srcRoot, HI5TREE &dstDataRoot, const std::string &path) const;

	// void filterImage(const PolarODIM & odim, Image &data, Image &quality) const;
	virtual
	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;
};


}

#endif /* ERASEROP_H_ */
