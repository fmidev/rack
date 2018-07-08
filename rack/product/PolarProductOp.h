/*

    Copyright 2001 - 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
