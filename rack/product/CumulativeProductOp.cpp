/*

    Copyright 2012-2014  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#include <stdexcept>

#include <drain/util/Variable.h>
// #include <drain/util/Fuzzy.h>
#include <drain/image/AccumulationMethods.h>
// #include "RackOp.h"
// #include "radar/Extractor.h"

#include "CumulativeProductOp.h"
#include "data/DataCoder.h"



namespace rack {

using namespace drain::image;



void CumulativeProductOp::processDataSets(const DataSetMap<PolarSrc> & srcSweeps, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(name+"(CumulativeProductOp::)", __FUNCTION__);
	//mout.debug(2) << "starting (" << name << ") " << mout.endl;

	if (srcSweeps.empty()){
		mout.warn() << "no data found with selector: " << dataSelector << mout.endl;
		return;
	}

	const DataSet<PolarSrc> & firstSweep =  srcSweeps.begin()->second;
	const Data<PolarSrc> & srcData = firstSweep.getFirstData();
	const std::string & quantity = srcData.odim.quantity;
	if (firstSweep.size() > 1){
		mout.info() << "several quantities, using the first one :" << quantity << mout.endl;
	}


	// Consider EchoTop, with DBZH input and HGHT output.
	const std::string dstQuantity = odim.quantity.empty() ? quantity : odim.quantity;

	Data<PolarDst> & dstData = dstProduct.getData(dstQuantity);
	//mout.warn() << "dstOdim " << dstData.odim << mout.endl;

	setEncoding(srcData.odim, dstData);

	deriveDstGeometry(srcSweeps, dstData.odim);
	dstData.data.setGeometry(dstData.odim.nbins, dstData.odim.nrays);

	RadarAccumulator<Accumulator,PolarODIM> accumulator;

	/// Some product generators may have user defined accumulation methods.
	accumulator.setMethod(drain::StringTools::replace(accumulationMethod, ":",","));
	accumulator.checkCompositingMethod(dstData.odim);
	accumulator.setGeometry(dstData.odim.nbins, dstData.odim.nrays);
	accumulator.odim.rscale = dstData.odim.rscale;

	mout.debug() << (const Accumulator &) accumulator << mout.endl;

	dstData.odim.update(odim); // product
	dstData.odim.update(srcData.odim); // date, time, etc
	dstData.odim.prodpar = getParameters().getValues();
	//mout.warn() << "'final' dstODIM " << dstData << mout.endl;

	mout.debug() << "main loop, quantity=" << quantity << mout.endl;

	for (DataSetMap<PolarSrc>::const_iterator it = srcSweeps.begin(); it != srcSweeps.end(); ++it){

		const Data<PolarSrc> & srcData = it->second.getData(quantity);

		if (srcData.data.isEmpty()){
			mout.warn() << "selected quantity=" << quantity << " not present in elangle=" << it->first << ", skipping" << mout.endl;
			continue;
		}
		mout.debug(2) << "elangle=" << it->first << mout.endl;

		processData(srcData, accumulator);
	}

	accumulator.extract(dstData.odim, dstProduct, "dw");

	//mout.warn() << "dstProduct.updateTree" << dstData.odim << mout.endl;
	//@= dstProduct.updateTree(dstData.odim);


}




}  // namespace rack
