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
#include <stdexcept>

#include "drain/util/Variable.h"
// #include "drain/util/Fuzzy.h"
#include "drain/image/FilePng.h"
#include "drain/image/AccumulationMethods.h"
// #include "RackOp.h"
// #include "radar/Extractor.h"

#include "CumulativeProductOp.h"
#include "data/DataCoder.h"



namespace rack {

using namespace drain::image;



void CumulativeProductOp::computeSingleProduct(const DataSetMap<PolarSrc> & srcSweeps, DataSet<PolarDst> & dstProduct) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (srcSweeps.empty()){
		mout.warn("no data found with selector: " , dataSelector );
		// TODO: DATA ERROR?
		return;
	}

	const DataSet<PolarSrc> & firstSweep =  srcSweeps.begin()->second;
	const Data<PolarSrc> & srcData = firstSweep.getData(dataSelector.getQuantitySelector()); // firstSweep.getFirstData();
	const std::string & quantity = srcData.odim.quantity;

	/* OLD
	if (firstSweep.size() > 1){
		mout.info("several quantities, using the first one :" , quantity );
	}
	*/
	mout.special("selected [", quantity, "]");


	// Consider EchoTop, with DBZH input and HGHT output; but CAPPI should adapt to input quantity
	//const std::string dstQuantity = odim.quantity.empty() ? quantity : odim.quantity;
	const std::string dstQuantity = getOutputQuantity(quantity);

	Data<PolarDst> & dstData = dstProduct.getData(dstQuantity);
	//mout.warn("dstOdim " , dstData.odim );

	setEncoding(srcData.odim, dstData);

	deriveDstGeometry(srcSweeps, dstData.odim);
	//dstData.data.setGeometry(dstData.odim.area.width, dstData.odim.area.height);
	dstData.data.setGeometry(dstData.odim.area);

	RadarAccumulator<Accumulator,PolarODIM> accumulator;

	/// Some product generators may have user defined accumulation methods.
	accumulator.setMethod(drain::StringTools::replace(accumulationMethod, ":", ","));
	accumulator.checkCompositingMethod(dstData.odim);
	accumulator.accArray.setGeometry(dstData.odim.area.width, dstData.odim.area.height);
	accumulator.odim.rscale = dstData.odim.rscale;

	mout.debug((const Accumulator &) accumulator );

	//dstData.odim.NI =
	srcData.odim.getNyquist(); // to prevent warning in the next cmd
	dstData.odim.updateLenient(odim); // product
	dstData.odim.updateLenient(srcData.odim); // date, time, etc
	dstData.odim.prodpar = getParameters().getValues();

	dstData.odim.angles.clear(); // DO NOT USE clear(), it changes address of 1st elem
	//dstData.odim.angles.resize(0); // DO NOT USE clear(), it changes address of 1st elem

	mout.debug("main loop, quantity=" , quantity );

	//for (DataSetMap<PolarSrc>::const_iterator it = srcSweeps.begin(); it != srcSweeps.end(); ++it){
	for (const auto & entry: srcSweeps){

		const Data<PolarSrc> & srcData = entry.second.getData(quantity);

		if (srcData.data.isEmpty()){
			mout.warn("selected quantity=", quantity, " not present in index=", entry.first, ", skipping");
			continue;
		}
		mout.debug3("index: ", entry.first);
		//mout.warn("elangle=" , it->first );

		processData(srcData, accumulator);

		//mout.attention("elangle >> angles");
		dstData.odim.angles.push_back(srcData.odim.elangle);
		//dstData.odim.angles.push_back(entry.first);

	}

	// mout.warn("eka: " , drain::sprinter(dstData.odim.angles) );

	//if (mout.isDebug(LOG_DEBUG))
	accumulator.extract(dstData.odim, dstProduct, "dwC");
	//else
	// OK mout.warn("eka: " , drain::sprinter(dstData.odim.angles) );
		//accumulator.extract(dstData.odim, dstProduct, "dw");
	//mout.warn("toka:" , drain::sprinter(dstData.odim.angles) );

	//mout.warn("koka:" , drain::sprinter(dstProduct.getFirstData().odim.angles) );
	// OK mout.warn("moka:" , drain::sprinter(dstData.odim) );

	/// Important: RELINK => resize, relocate (Address of dstData.odim.angles[0] may have changed.)
	//  Alternatively, dstData.odim.angles.reserve(90) etc upon ODIM constr?
	// dstData.odim.link("how:angles", dstData.odim.angles);
	// drain::image::FilePng::write(dstProduct.getData("DBZH").data, "debug.png");
	//mout.warn("dstProduct.updateTree" , dstData.odim );
	//@= dstProduct.updateTree(dstData.odim);


}




}  // namespace rack

// Rack
