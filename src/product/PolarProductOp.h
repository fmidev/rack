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

#ifndef RACK_PolarProductOP
#define RACK_PolarProductOP

#include "data/PolarODIM.h"
#include "VolumeOp.h"

namespace rack {

// using namespace drain::image;

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

	PolarProductOp(const std::string & name = __FUNCTION__, const std::string & description = "");


	PolarProductOp(const PolarProductOp & op);

	virtual
	~PolarProductOp(){};

	static
	const CoordinatePolicy polarCoordPolicy;



protected:

	bool aboveSeaLevel; // True, if altitude (input or output) is measured from sea level or radar site.

	inline
	void copyPolarGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		dstData.odim.area.width  = srcODIM.area.width;
		dstData.odim.area.height  = srcODIM.area.height;
		dstData.odim.rscale = srcODIM.rscale;
		dstData.data.setGeometry(dstData.odim.area.width, dstData.odim.area.height);
		if (!dstData.odim.type.empty()){
			dstData.data.setType(dstData.odim.type.at(0));
		}
		else {
			// warn type not set?
		}
	};

	// Copy Polar could be enough?
	virtual	inline
	void setGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		copyPolarGeometry(srcODIM, dstData);
	}

	///  Based on input data, find geometry that maximises range and ray count.
	/**
	 *  This method searches for
	 *  - largest beam count ( \c nrays ), if not set
	 *  - smallest beam directional resolution (\c rscale ), if not set
	 *  - \c rscale and \c nbins producing the largest range, if both not set
	 *
     *  Also creates \c elangles array.
     *
     *  \param srcSweeps - input data
     *  \param dstOdim - metadata for/of output data
     *
     */
	virtual
	void deriveDstGeometry(const DataSetMap<PolarSrc> & srcSweeps, PolarODIM & dstOdim, bool MINIMISE_RSCALE=false) const;

	/// Update timestamps etc
	/**
    *  \param srcSweeps - input data
    *  \param dstOdim - metadata for/of output data
    *
    */
	virtual
	void determineMetadata(const DataSetMap<PolarSrc> & srcSweeps, PolarODIM & dstOdim) const;


};


}  // namespace rack

#endif /* RACKOP_H_ */

// Rack
