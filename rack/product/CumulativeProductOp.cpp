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

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL name+"(CumulativeProductOp::)", __FUNCTION__);
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


	// Consider EchoTop, with DBZH input and HGHT output; but CAPPI should adapt to input quantity
	const std::string dstQuantity = odim.quantity.empty() ? quantity : odim.quantity;

	Data<PolarDst> & dstData = dstProduct.getData(dstQuantity);
	//mout.warn() << "dstOdim " << dstData.odim << mout.endl;

	setEncoding(srcData.odim, dstData);

	deriveDstGeometry(srcSweeps, dstData.odim);
	dstData.data.setGeometry(dstData.odim.geometry.width, dstData.odim.geometry.height);

	RadarAccumulator<Accumulator,PolarODIM> accumulator;

	/// Some product generators may have user defined accumulation methods.
	accumulator.setMethod(drain::StringTools::replace(accumulationMethod, ":",","));
	accumulator.checkCompositingMethod(dstData.odim);
	accumulator.setGeometry(dstData.odim.geometry.width, dstData.odim.geometry.height);
	accumulator.odim.rscale = dstData.odim.rscale;

	mout.debug() << (const Accumulator &) accumulator << mout.endl;

	//dstData.odim.NI =
	srcData.odim.getNyquist(); // to prevent warning in the next cmd
	dstData.odim.updateLenient(odim); // product
	dstData.odim.updateLenient(srcData.odim); // date, time, etc
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

// Rack
