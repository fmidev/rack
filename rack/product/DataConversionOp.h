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

#ifndef DATACONVERSIONOP2_H_
#define DATACONVERSIONOP2_H_

#include "data/ODIM.h"
//&#include "PolarProductOp.h"
#include "VolumeTraversalOp.h"
//#include "VolumeOpNew.h"

namespace rack {

/// Converts HDF5 data to use desired data type, scaling and encoding (ODIM gain, offset, undetect and nodata values).
/**
 *  \see Conversion
 *
 */
class DataConversionOp: public VolumeTraversalOp {
public:

	DataConversionOp(const std::string & type="C", double gain=1.0, double offset=0.0,
			double undetect=0.0, double nodata=255.0, std::string copyGroupSuffix="") :
				VolumeTraversalOp(__FUNCTION__, "Converts HDF5 data to use desired data type, scaling and encoding") {

		allowedEncoding.reference("what:type", odim.type = type);
		allowedEncoding.reference("what:gain", odim.gain = gain);
		allowedEncoding.reference("what:offset", odim.offset = offset);
		allowedEncoding.reference("what:undetect", odim.undetect = undetect);
		allowedEncoding.reference("what:nodata", odim.nodata = nodata);

	}

	virtual ~DataConversionOp();


	static
	const HI5TREE & getNormalizedData(const DataSetSrc<PolarSrc> & srcDataSet, DataSetDst<PolarDst > & dstDataSet, const std::string & quantity);

	virtual
	void processDataSet(const DataSetSrc<PolarSrc> & srcSweep, DataSetDst<PolarDst> & dstProduct) const;

	/// Converts src to dst such that dst applies desired gain, offset, undetect and nodata values.
	//inline
	void processData(const PlainData<PolarSrc> & src, PlainData<PolarDst> & dst) const;


	inline
	void processImage(const PlainData<PolarSrc> & src, drain::image::Image & dst) const {
		processImage(src.odim, src.data, odim, dst);
	}


	/// Converts src to dst such that dst applies desired gain, offset, undetect and nodata values.
	/**
	 *   Sometimes this is applied directly (for alpha channel ops).
	 */
	void processImage(const PolarODIM & odimSrc, const drain::image::Image & src, const PolarODIM & odimDst, drain::image::Image & dst) const;

	// todo: cartesian?




	///
	///
	/**
	 *  Unlike other operators (RackOps), this operator is "conditional" in the sense that it
	 *  converts data only if that differ from the original.
	 */
	// Needed by DetectorOp
	/*
	virtual
	void filterGroup(const HI5TREE &srcRoot, const std::string & groupPath, HI5TREE &dstRoot) const;
	 */
protected:

	///
	// virtual 	HI5TREE & getDst(HI5TREE & dst) const {return dst;} ;




	/// PolarODIM parameters for the converted data.
	//PolarODIM odimOut;  // TODO: VolumeOp::odim enough?

	/// Type of target image. 'c' = unsigned char, 's' = short int.


	/// Suffix for trailing path element ("/data") for storing the original.
	std::string copyGroupSuffix;

	///
	//bool deleteOriginal;
};

}

#endif /* DATACONVERSIONOP_H_ */

// Rack
