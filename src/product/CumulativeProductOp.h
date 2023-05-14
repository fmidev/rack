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

#ifndef CumulativePolarProduct_OP_H_
#define CumulativePolarProduct_OP_H_

#include "drain/util/Log.h"
#include "drain/util/Fuzzy.h"
#include "drain/image/Accumulator.h"

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

	/// Returns the primary output quantity (ODIM \c what:quantity , like DBZH)
	/*
	virtual inline
	const std::string & getOutputQuantity() const {
		static const std::string defaultQuantity("DBZH");
		return odim.quantity.empty() ? defaultQuantity : odim.quantity;
	}
	*/
	virtual inline
	const std::string & getOutputQuantity(const std::string & inputQuantity = "") const {
		if (!inputQuantity.empty())
			return inputQuantity; //outputQuantity;
		else
			return odim.quantity;
	}

	// Public for ConvOp
	virtual
	void computeSingleProduct(const DataSetMap<PolarSrc> & srcSweeps, DataSet<PolarDst> & dstProduct) const;



protected:

	CumulativeProductOp(const std::string & name = __FUNCTION__,
			const std::string &description = "", const std::string & accumulationMethod = "LATEST") :
		PolarProductOp(name, description), accumulationMethod(accumulationMethod){
		//, undetectValue(-40), relativeUndetectWeight(0.95) {  // , method(method), p(p), q(q)
		//dataSelector.path = "^.*/data[0-9]+$";

		// Empty values imply automagic
		odim.type = "";
		odim.scaling.scale = 0.0;
		odim.area.height  = 0;
		odim.area.width  = 0;
		odim.rscale = 0.0;

	};



	std::string accumulationMethod;



	virtual
	inline
	void setGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		drain::Logger mout(__FUNCTION__, __FILE__); //REPL name+"(CumulativeProductOp)", __FUNCTION__);
		mout.warn() << "setGeometry??" << mout.endl;
		copyPolarGeometry(dstData.odim, dstData);
	}



	virtual
	void processData(const Data<PolarSrc> & src, RadarAccumulator<Accumulator,PolarODIM> & cumulator) const = 0;



};




//================================================


}  // namespace rack

#endif /* RACKOP_H_ */

// Rack
