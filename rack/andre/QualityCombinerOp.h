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

#include "drain/util/Functor.h"

#include "drain/image/File.h"

#include "product/PolarProductOp.h"



using namespace drain::image;

namespace rack {

/// Simple class marking true data with a given constant value, typically a CLASS label.
class DataMarker : public drain::UnaryFunctor {

public:

	DataMarker() : drain::UnaryFunctor(__FUNCTION__), value(0.5) {

	}

	inline
	void set(double value){
		this->value = value;
		//this->update();
	}


	inline
	double operator()(double x) const {
		return value;
	}

	double value;

};

///
/*
class DefaultOp: public PolarProductOp {  // VolumeOp<PolarODIM>

public:

	DefaultOp() :
		PolarProductOp(__FUNCTION__,"Combines detection probabilities to overall quality field QIND (and CLASS).")  // VolumeOp<PolarODIM>
	{

	}


}
*/

/// A quick QualityCombiner .
class QualityCombinerOp: public PolarProductOp {  // VolumeOp<PolarODIM>

public:

	QualityCombinerOp() :
		PolarProductOp(__FUNCTION__,"Combines detection probabilities to overall quality field QIND (and CLASS).")  // VolumeOp<PolarODIM>
	{
		// dataSelector.path = "/da ta[0-9]+$";
		dataSelector.quantity = "^DBZH$";

		allowedEncoding.clear();
	}

	inline
	~QualityCombinerOp(){};

	/// Updates quality specific \c QIND and \c CLASS to dataset level. Conditional; checks if already done.
	static
	void updateOverallQuality(const PlainData<PolarSrc> & srcQind, const PlainData<PolarSrc> & srcClass, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass); // const;

	/// Given probability data with class label, updates (?overall?) QIND
	/**
	 *   \param srcProb - probability of anomaly
	 *   \param dstQind - current data quality \c QIND.
	 *   \param label   - deprecating? anomaly class id
	 *   \param index   - deprecating? anomaly class id
	 */
	static
	void updateOverallDetection(const PlainData<PolarSrc> & srcProb, PlainData<PolarDst> & dstQind, PlainData<PolarDst> & dstClass, const std::string &label, unsigned short index); // const;


	/// Updates QIND and QCLASS from global level (dataset) to local level (data). Conditional; checks if already done.
	static
	void updateLocalQuality(const DataSet<PolarSrc> & srcDataSet, Data<PolarDst> & dstData);

	/// Quality index value (0.5 by default) under which CLASS information is updated. Otherwise class is "meteorogical enough".
	static
	double DEFAULT_QUALITY;


	static
	void initDstQuality(const PlainData<PolarSrc> & srcData, PlainData<PolarDst> & dstQind, const std::string & quantity = "");

protected:


	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;


};

} // rack::

#endif
