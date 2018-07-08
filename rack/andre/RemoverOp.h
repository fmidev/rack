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
 * RemoverOp.h
 *
 *  Created on: Aug 7, 2011
 *      Author: mpeura
 */

#ifndef CorrectorOP_H_
#define CorrectorOP_H_


#include <drain/util/Fuzzy.h>
#include <drain/image/Intensity.h>
#include <drain/image/File.h>


//#include "radar/Geometry.h"
#include "hi5/Hi5.h"
#include "data/Data.h"
#include "AndreOp.h"

using namespace drain::image;

namespace rack {

/// The simplest possible anomaly removal operator and the base class for more complex ones.
/**  Sets data values to \c nodata in locations where quality is below minDBZ.
 *   This should apply to any quantity.
 *
 *   TODO: add minus-dbz range down to e.g. -25?
 */
class RemoverOp: public AndreOp {

public:

	/** Default constructor.
	 *  \param minDBZ - Probability minDBZ, over which data is replaced by "nodata".
	 */
	RemoverOp(double threshold = 0.5): AndreOp("Remover", "Simple anomaly removal operator."){
		parameters.reference("threshold", this->threshold = threshold, "probability");
		//dataSelector.quantity = ""; // Accepts DBZH and VRAD
		dataSelector.path = ".*data[0-9]+/?$";
		dataSelector.quantity = "^DBZH$"; //|TV|VRAD|RHOHV|LDR|PHIDP|KDP";
	};

	/// NEW POLICY => DetectorOpNEW
	virtual
	void processDataSets(const DataSetMap<PolarSrc> & srcVolume, DataSetMap<PolarDst> & dstVolume) const;

	/// Process as sweep (data in one elevation angle)
	/**
	 *  \param srcDataSet - input data of one elevation; possibly several quantities (measurement parameters).
	 *  \param dstProb    - probability field ie. the result of the detection algorithm
	 *  \param aux        - auxialiary DatasetDst for keeping a copy of normalized data.
	 */
	virtual
	void processDataSet(const DataSet<PolarSrc> & srcDataSet, DataSet<PolarDst> & dstDataSet)  const;

	//Practically, qualityRoot == dstDataRoot
	// void filterGroup(const HI5TREE &qualityRoot, const std::string &path, HI5TREE &dstDataRoot) const;

	/// Cleaning a the data array of one quantity (measurement parameter).  Called by processDataSet.
	/**
	 *  \param srcData - input data and its quality data (global or updated local).
	 *  \param dstData - output data and its local quality data (new or modified).
	 */
	//
	virtual
	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;


protected:

	/** Constructor for derived classes.
	 */
	RemoverOp(const std::string &name, const std::string & description) :
		AndreOp(name, description){
		dataSelector.quantity = "^[A-Z]+";
		dataSelector.path = ".*/data[0-9]+/?$";  // unused
	};

private:

	double threshold;


};


}

#endif /* CorrectoROP_H_ */
