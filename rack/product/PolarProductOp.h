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

#ifndef PolarProduct_OP_H_
#define PolarProduct_OP_H_

//

#include "VolumeOp.h"
#include "../data/PolarODIM.h"

namespace rack {

using namespace drain::image;

///  Base class for flat (2D) products computed in the polar coordinate system. Volume is used as input.
/**
 *  Operator for radar data in HDF5 input, producing another HDF as output
 *  H5 data is in form Tree<NodeH5>.
 *
 *  The following parameters are involved all the polar products:
 *  - rscale
 *  - nbins
 *  - nrays
 *  - rstart
 *
 *  In addition, many polar products involve the following ones:
 *  - elangle
 *  - a1gate
 *
 *
 */
class PolarProductOp : public VolumeOp<PolarODIM> {
public:

	inline
	PolarProductOp(const std::string & name = __FUNCTION__, const std::string & description = "") : VolumeOp<PolarODIM>(name, description) {

		allowedEncoding.reference("type", odim.type = "C");
		allowedEncoding.reference("gain", odim.gain);
		allowedEncoding.reference("offset", odim.offset);
		// 2018
		allowedEncoding.reference("undetect", odim.undetect);
		allowedEncoding.reference("nodata", odim.nodata);

		allowedEncoding.reference("rscale", odim.rscale);
		allowedEncoding.reference("nrays", odim.nrays);
		allowedEncoding.reference("nbins", odim.nbins);

		aboveSeaLevel = true;

	};

	virtual
	~PolarProductOp(){};

	static
	const CoordinatePolicy polarCoordPolicy;

protected:

	bool aboveSeaLevel; // True, if altitude (input or output) is measured from sea level or radar site.

	inline
	void copyPolarGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		dstData.odim.nbins  = srcODIM.nbins;
		dstData.odim.nrays  = srcODIM.nrays;
		dstData.odim.rscale = srcODIM.rscale;
		dstData.data.setGeometry(dstData.odim.nbins, dstData.odim.nrays);
		if (!dstData.odim.type.empty()){
			dstData.data.setType(dstData.odim.type.at(0));
		}
		else {
			// warn type not set?
		}
	};

	// Copy Polar could be enough?
	virtual
	inline
	void setGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		copyPolarGeometry(srcODIM, dstData);
	}

    /// Derive appropriate output ODIM from input sweeps, user-requested ODIM and (possibly) meteorological input parameters.
    /**
     *  - \c srcSweeps
     *  - \c dstOdim
     *
     */
    //
	virtual
	void deriveDstGeometry(const DataSetMap<PolarSrc> & srcSweeps, PolarODIM & dstOdim) const;



};


}  // namespace rack

#endif /* RACKOP_H_ */

// Rack
