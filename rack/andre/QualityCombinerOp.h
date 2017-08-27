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
	void updateLocalQuality(const DataSetSrc<> & srcDataSet, Data<PolarDst> & dstData);

	/// Quality index value (0.5 by default) under which CLASS information is updated. Otherwise class is "meteorogical enough".
	static
	double CLASS_UPDATE_THRESHOLD;


protected:

	//std::string targetQuantity;

	virtual
	void processDataSet(const DataSetSrc<> & srcSweep, DataSetDst<> & dstProduct) const;


	/*
	virtual
	void processSweep(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const;
	*/

};

}

#endif /* POLARTOCARTESIANOP_H_ */

// Rack
