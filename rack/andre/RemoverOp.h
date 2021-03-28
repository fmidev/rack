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

#ifndef RACK_RemoverOP_H_
#define RACK_RemoverOP_H_


#include "drain/util/Fuzzy.h"
#include "drain/image/Intensity.h"
#include "drain/image/File.h"

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
	inline
	RemoverOp(double threshold = 0.5): AndreOp("Remover", "Simple anomaly removal operator."){
		parameters.link("threshold", this->threshold = threshold, "probability");
		parameters.link("replace", this->replace = "nodata", "nodata|undetect|<physical_value>");
		parameters.link("clearQuality", this->clearQuality = true);
		// dataSelector.path = ".*da ta[0-9]+/?$";
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
	// void filterGroup(const Hi5Tree &qualityRoot, const std::string &path, Hi5Tree &dstDataRoot) const;

	/// Cleaning a the data array of one quantity (measurement parameter).  Called by processDataSet.
	/**
	 *  \param srcData - input data and its quality data (global or updated local).
	 *  \param dstData - output data and its local quality data (new or modified).
	 */
	//
	virtual
	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;

	virtual
	void processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
				PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstQIND) const;


protected:

	/** Constructor for derived classes.
	 */
	RemoverOp(const std::string &name, const std::string & description) :
		AndreOp(name, description){
		dataSelector.quantity = "^[A-Z]+";
	};

private:

	double threshold;
	std::string replace;
	bool clearQuality;

};


}

#endif /* CorrectoROP_H_ */

// Rack
