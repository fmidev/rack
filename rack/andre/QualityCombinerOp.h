/**

    Copyright 2010-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack library for C++.

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

#ifndef QualityCombiner_SEGMENT_OP_H_
#define QualityCombiner_SEGMENT_OP_H_


//#include <drain/image/SegmentAreaOp.h>
//#include <drain/image/RunLengthOp.h>
//#include <drain/image/BasicOps.h>


//#include <drain/image/DistanceTransformOp.h>
//#include <drain/image/FuzzyOp.h>


#include <drain/image/File.h>

//#include "DetectorOp.h"
//#include "product/VolumeOp.h"
#include "product/PolarProductOp.h"


//using namespace drain::image;

using namespace drain::image;

namespace rack {

/// A quick QualityCombiner .
class QualityCombinerOp: public PolarProductOp {  // VolumeOp<PolarODIM>

public:

	//QualityCombinerOp(const std::string & targetQuantity = "QIND") :
	QualityCombinerOp() :
		PolarProductOp(__FUNCTION__,"Combines detection probabilities to overall quality field QIND (and CLASS).")  // VolumeOp<PolarODIM>
	{
		//reference("targetQuantity", this->targetQuantity, targetQuantity );

		//dataSelector.path = "/quality[0-9]+$";
		//dataSelector.quantity = "^(PROB|)$";
		dataSelector.path = "/data[0-9]+$";
		dataSelector.quantity = "^DBZH$";

		allowedEncoding.clear();
	}

	inline
	~QualityCombinerOp(){};

	/// Updates QIND and QCLASS from local level (dataN/) to local level (datasetM/).
	// ? Conditional; checks if already done.
	// Copied from AndreOp
	static
	void updateOverallQuality(const PlainData<PolarSrc> & srcQind, const PlainData<PolarSrc> & srcClass, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass); // const;

	/// Updates the overall ie. maximum field.
	// static, because external inputs like "sclutter"
	static
	void updateOverallDetection(const PlainData<PolarSrc> & srcProb, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass, const std::string &label, unsigned short index); // const;


	/// Copied from RemoverOp
	/// Updates QIND and QCLASS from global level (dataset) to local level (data). Conditional; checks if already done.
	static
	void updateLocalQuality(const DataSet<PolarSrc> & srcDataSet, Data<PolarDst> & dstData);

	/// Quality index value (0.5 by default) under which CLASS information is updated. Otherwise class is "meteorogical enough".
	static
	double CLASS_UPDATE_THRESHOLD;


protected:

	//std::string targetQuantity;

	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;


	/*
	virtual
	void processSweep(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const;
	*/

};

}

#endif /* POLARTOCARTESIANOP_H_ */
