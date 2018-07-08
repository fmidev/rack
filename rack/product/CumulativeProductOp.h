/*

    Copyright 2001 - 2016  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

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

#ifndef CumulativePolarProduct_OP_H_
#define CumulativePolarProduct_OP_H_

#include <drain/util/Log.h>
#include <drain/util/Fuzzy.h>
#include <drain/image/Accumulator.h>

#include "radar/Geometry.h"
#include "radar/Constants.h"
#include "radar/RadarAccumulator.h"
#include "data/Data.h"

#include "PolarProductOp.h"
// temporary?


namespace rack {

using namespace drain::image;


///  Base class for flat (2D) products computed in the polar coordinate system.
/**
 *  Operator for radar data in HDF5 input, producing another HDF as output
 *  H5 data is in form Tree<NodeH5>.
 *
 */
class CumulativeProductOp : public PolarProductOp {
public:


	// Public for ConvOp
	virtual
	void processDataSets(const DataSetMap<PolarSrc> & srcSweeps, DataSet<PolarDst> & dstProduct) const;



protected:

	CumulativeProductOp(const std::string & name = __FUNCTION__,
			const std::string &description = "", const std::string & accumulationMethod = "LATEST") :
		PolarProductOp(name, description), accumulationMethod(accumulationMethod){
		//, undetectValue(-40), relativeUndetectWeight(0.95) {  // , method(method), p(p), q(q)
		//dataSelector.path = "^.*/data[0-9]+$";

		// Empty values imply automagic
		odim.type = "";
		odim.gain = 0.0;
		odim.nrays  = 0;
		odim.nbins  = 0;
		odim.rscale = 0.0;

	};



	std::string accumulationMethod;



	virtual
	inline
	void setGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		drain::Logger mout(name+"(CumulativeProductOp)", __FUNCTION__);
		mout.warn() << "setGeometry??" << mout.endl;
		copyPolarGeometry(dstData.odim, dstData);
	}



	virtual
	void processData(const Data<PolarSrc> & src, RadarAccumulator<Accumulator,PolarODIM> & cumulator) const = 0;



};




//================================================


}  // namespace rack

#endif /* RACKOP_H_ */
