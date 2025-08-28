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

#include <drain/Variable.h>

#include <drain/image/FilePng.h>
#include <drain/image/AccumulationMethods.h>

#include "data/DataCoder.h"

#include "CumulativeProductOp.h"


namespace rack {

using namespace drain::image;


CumulativeProductOp::CumulativeProductOp(const std::string & name, const std::string &description, const std::string & accumulationMethod) :
	PolarProductOp(name, description), accumulationMethod(accumulationMethod){
	// , undetectValue(-40), relativeUndetectWeight(0.95) {  // , method(method), p(p), q(q)
	// dataSelector.path = "^.*/data[0-9]+$";

	// Empty values imply automagic
	odim.type = "";
	odim.scaling.scale = 0.0;
	odim.area.height  = 0;
	odim.area.width  = 0;
	odim.rscale = 0.0;

	// std::cerr << "# " << getName() << " CTR: accumulationMethod:" << accumulationMethod << '\n';
};

CumulativeProductOp::CumulativeProductOp(const CumulativeProductOp & op) : PolarProductOp(op), accumulationMethod(op.accumulationMethod){
	// std::cerr << "# " << getName() << " COPY CTR: accumulationMethod:" << accumulationMethod << '\n';
};


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
	if (dataSelector.getQuantity() != quantity){
		mout.info("selected input [", quantity, "]");
	}


	// Consider EchoTop, with DBZH input and HGHT output; but CAPPI should adapt to input quantity
	//const std::string dstQuantity = odim.quantity.empty() ? quantity : odim.quantity;
	const std::string dstQuantity = getOutputQuantity(quantity);

	mout.special("output quantity: ", dstQuantity);
	Data<PolarDst> & dstData = dstProduct.getData(dstQuantity);
	//mout.warn("dstOdim " , dstData.odim );

	setEncoding(srcData.odim, dstData);
	mout.special("output encoding: ", ODIM(dstData.odim));

	deriveDstGeometry(srcSweeps, dstData.odim);
	//dstData.data.setGeometry(dstData.odim.area.width, dstData.odim.area.height);
	dstData.data.setGeometry(dstData.odim.area);

	RadarAccumulator<Accumulator,PolarODIM> accumulator;

	/// Some product generators may have user defined accumulation methods.
	// mout.attention("accumulationMethod: ", accumulationMethod);
	// mout.warn(DRAIN_LOG_VAR(accumulationMethod));

	accumulator.setMethod(drain::StringTools::replace(accumulationMethod, ":", ","));
	accumulator.checkCompositingMethod(dstData.odim);
	accumulator.accArray.setGeometry(dstData.odim.area.width, dstData.odim.area.height);
	accumulator.odim.rscale = dstData.odim.rscale;
	// TODO: accumulator.setGeometry(dstData.odim);

	mout.debug("accumulator got: ", accumulator.getMethod());

	mout.debug((const Accumulator &) accumulator );

	//dstData.odim.NI =
	srcData.odim.getNyquist(); // to prevent warning in the next cmd
	dstData.odim.updateLenient(odim); // product
	// dstData.odim.updateLenient(srcData.odim); // date, time, etc MOVED INSIDE LOOP 2024/03/+03
	// dstData.odim.prodpar = getParameters().getValues();
	//dstData.odim.prodpar = getLastArguments() + " #" + getParameters().getValues();
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

		// mout.attention("elangle >>>", srcData.odim.elangle);
		dstData.odim.updateLenient(srcData.odim);
		dstData.odim.angles.push_back(srcData.odim.elangle);
		// mout.attention("elangle <<<", srcData.odim.elangle);

	}

	// drain::image::FilePng::write(accumulator.accArray.data, "debug.png");

	// mout.warn("eka: " , drain::sprinter(dstData.odim.angles) );

	//if (mout.isDebug(LOG_DEBUG))
	accumulator.extractOLD(dstData.odim, dstProduct, "dwC");

	/* 2025 no no...
	DataCoder dataCoder(dstData.odim, dstData.getQualityData().odim); // (
	// for (const Accumulator::FieldType field: {Accumulator::FieldType::DATA, ...})
	drain::image::Image &img = dstData.data;
	const drain::Rectangle<int> area(0, 0, img.getWidth(), img.getHeight());
	accumulator.extractField(Accumulator::FieldType::DATA,   dataCoder, dstData.data, area);
	accumulator.extractField(Accumulator::FieldType::WEIGHT, dataCoder, dstData.data, area);
	*/

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
