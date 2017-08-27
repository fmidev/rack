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



#include <drain/util/Fuzzy.h>

#include <drain/image/DistanceTransformFillOp.h>
#include <drain/image/File.h>
#include <drain/image/RecursiveRepairerOp.h>

#include "data/DataCoder.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"

#include "radar/Coordinates.h"
#include "radar/Composite.h"
//#include "radar/Extractor.h"


#include "cartesian-add.h"  // for cmdFormat called by


namespace rack {



void CompositeAdd::exec() const {

	drain::MonitorSource mout(name, __FUNCTION__);

	mout.timestamp("BEGIN_CART_EXEC");

	RackResources & resources = getResources();

	// Check
	if ( !resources.inputOk ){
		mout.note() << "last input inapplicable, skipping it" << mout.endl;
		resources.select.clear(); // ?
		return;
	}

	// Check
	if (! resources.dataOk){
		mout.note() << "last DATA inapplicable, skipping it" << mout.endl;
		resources.select.clear(); // ?
		return;
	}

	/// Set input data selector (typically, by quantity)
	if (!resources.select.empty()){
		resources.composite.dataSelector.setParameters(resources.select);
		resources.select.clear(); //
	}

	mout.info() << "using selector: " << resources.composite.dataSelector << mout.endl; // consider: if resources.composite.dataSelector...?

	/// Set default method, if unset.
	if (!resources.composite.isMethodSet()){
		mout.note() << " compositing method unset, setting MAX" << mout.endl;
		resources.composite.setMethod("MAX");
	}

	/// Set encoding for final (extracted) product. Applied by RadarAccumulator.
	// If needed, initialize with input metadata.
	if (!resources.targetEncoding.empty()){
		resources.composite.setTargetEncoding(resources.targetEncoding);
		resources.targetEncoding.clear();
	}


	/// Main
	//if (resources.currentHi5 == resources.currentPolarHi5){
	//if (resources.currentPolarHi5 != NULL){  // to get polar AMVU and AMVV data converted
	if ((resources.currentPolarHi5 != NULL) && !resources.currentPolarHi5->getChildren().empty()){  // to get polar AMVU and AMVV data converted
		mout.info() << "polar input data, ok" << mout.endl;
		addPolar();
	}
	else if (resources.currentHi5 == & resources.cartesianHi5){
		mout.info() << "Cartesian input data, ok" << mout.endl;
		addCartesian();
	}
	else {
		mout.error() << "current H5 data inapplicable for compositing" << mout.endl;
	}




}

// Originally crom create
void CompositeAdd::addPolar() const {

	drain::MonitorSource mout(name, __FUNCTION__);

	mout.timestamp("BEGIN_CART_CREATE");

	RackResources & resources = getResources();

	if (resources.currentPolarHi5 == NULL){
		mout.warn() << "no polar data loaded, skipping" << mout.endl;
		return;
	}



	bool isAeqd = false;

	if (!resources.composite.isDefined()){
		mout.note() << "Creating a single-radar Cartesian" << mout.endl;

		// Try to set size. May still remain 0x0, which ok: addPolar() will set it if needed.
		resources.composite.setGeometry(resources.composite.getFrameWidth(), resources.composite.getFrameHeight()); // Check - does nothing?

		if (!resources.projStr.empty())
			resources.composite.setProjection(resources.projStr);
		//subComposite.setProjection(resources.composite.getProjection());
		else
			isAeqd = true;
		// see single below
	}


	try {

		resources.dataOk = true; // return if input not ok?

		std::string dataPath;
		DataSelector::getPath(*(resources.currentPolarHi5), resources.composite.dataSelector, dataPath);
		if (dataPath.empty()){
			mout.warn() << "create composite: no group found with selector:" << resources.composite.dataSelector << mout.endl;
			//resources.inputOk = false; // REMOVE?
			resources.dataOk = false;
			return;
		}

		const Data<PolarSrc> polarSrc((*resources.currentPolarHi5)(dataPath)); // NOTE, not from dataSet! (because may be plain /qualityN data)

		/// GET INPUT DATA
		if ( !polarSrc.data.isEmpty() ){
			mout.info() << "using input quantity:" << polarSrc.odim.quantity << mout.endl;
		}
		else {
			mout.warn() << "skipping empty input data: quantity=" << polarSrc.odim.quantity << ", path:" << dataPath << "/data" << mout.endl;  // was: dataSetPath
			resources.dataOk = false; // REMOVE?
			return;
		}

		// mout.warn() << "composite: " << resources.composite.odim << mout.endl;

		if (resources.composite.odim.gain == 0.0){
			ProductOp::applyODIM(resources.composite.odim, polarSrc.odim);
			ProductOp::handleEncodingRequest(resources.composite.odim, resources.composite.getTargetEncoding());
		}

		//resources.composite.checkInputODIM(polarSrc.odim);
		// Apply user parameters.


		mout.debug()  << "subComposite defined:\n" << resources.composite.getBoundingBoxD() << ", quantity: " << resources.composite.odim.quantity << mout.endl;
		mout.debug(1) << "subComposite: " << resources.composite << '\n' << resources.composite.odim << mout.endl;
		mout.debug(1) << "accumulating polar data..." << mout.endl;




		//subComposite.addPolar(polarSrc, 1.0, isAeqd); // Subcomposite: always 1.0.
		//const PlainData<PolarSrc> & srcQuality = polarSrc.hasQuality() ? polarSrc.getQualityData("QIND");
		std::string parent;
		std::string current;
		DataSelector::getParentAndChild(dataPath, parent, current);



		if (current.find("quality") == 0){
			mout.info()  << "plain quality data, ok (no further quality data)" << mout.endl;  // TODO: fix if quality/quality (BirdOp)
			static const HI5TREE t;
			static const PlainData<PolarSrc> empty(t);
			resources.composite.addPolar(polarSrc, empty, 1.0, isAeqd); // Subcomposite: always 1.0.
			//DATA_ONLY = true;
		}
		else if (polarSrc.hasQuality()){
			mout.info() << "using local qualitydata" << mout.endl;
			resources.composite.addPolar(polarSrc, polarSrc.getQualityData("QIND"), weight, isAeqd); // Subcomposite: always 1.0.
		}
		else {
			mout.info() << "using shared (dataset-level) qualitydata" << mout.endl;
			DataSetSrc<PolarSrc> dataSetSrc((*resources.currentPolarHi5)(parent));
			const PlainData<PolarSrc> & srcDataSetQuality = dataSetSrc.getQualityData("QIND");
			if (!srcDataSetQuality.data.isEmpty()){
				mout.info() << "using shared (dataset-level) quality data, path=" << parent << mout.endl;
			}
			else {
				mout.info() << "no quality data (QIND) found under path=" << parent << mout.endl;
				//DATA_ONLY = true;
			}
			resources.composite.addPolar(polarSrc, srcDataSetQuality, weight, isAeqd); // Subcomposite: always 1.0.
		}


		mout.debug(1) << "finished" << mout.endl;

		//resources.currentHi5 = & resources.cartesianHi5;
		resources.currentImage = NULL;
		resources.currentGrayImage = NULL;

		//mout.warn() << resources.cartesianHi5 << mout.endl;

	}
	catch (std::exception & e) {
		//std::cerr << e.what() << std::endl;
		mout.warn() << e.what() << mout.endl;
	}
}


// Originally crom create
void CompositeAdd::addCartesian() const {

	drain::MonitorSource mout(name, __FUNCTION__); // = getResources().mout;

	RackResources & resources = getResources();


	DataSetSrc<CartesianSrc> cartDataSetSrc(resources.cartesianHi5["dataset1"], resources.composite.dataSelector.quantity);

	if (cartDataSetSrc.empty()){
		mout.warn() << "Empty dataset(s), skipping. Selector.quantity (regexp): '" << resources.composite.dataSelector.quantity << "'" << mout.endl;
		return;
	}

	const Data<CartesianSrc> & cartSrc = cartDataSetSrc.getFirstData(); // quantity match done above
	if (cartSrc.odim.quantity.empty()){
		mout.warn() << "no quantity in data: " << cartSrc.odim << mout.endl;
	}
	else {
		mout.debug() << "quantity: " << cartSrc.odim.quantity << mout.endl;
	}

	if (cartSrc.data.isEmpty()){
		mout.warn() << "null sized tile, skipping..." << mout.endl;
		return;
	}

	// NEW: pick local or global quality field.
	const PlainData<CartesianSrc> & srcQuality = cartSrc.hasQuality() ? cartSrc.getQualityData("QIND") : cartDataSetSrc.getQualityData("QIND");

	//const double weight = (key=="cAdd") ? 1.0 : (double)value; // todo: default value?
	double w = weight;

	if (srcQuality.data.isEmpty()){
		mout.info() << "no quality data, using default quality:" << resources.composite.defaultQuality << mout.endl;
		//resources.cDefaultQuality << mout.endl;
		w *= resources.composite.defaultQuality; //resources.cDefaultQuality; ?
	}
	else {
		mout.info() << "using quality data, ok. " << mout.endl;
	}

	// If needed, initialize with input metadata.
	// Apply user parameters.
	// resources.composite.ensureEncoding(cartSrc.odim, resources.targetEncoding);
	// resources.targetEncoding.clear();


	if (resources.composite.decay < 1.0){
		const double delayMinutes = resources.composite.getTimeDifferenceMinute(cartSrc.odim);  // assume update done
		mout.info() << "Delay minutes: " << delayMinutes << mout.endl;
		/// Todo: warn
		const double delayWeight = pow(resources.composite.decay, delayMinutes);
		mout.info() << "Delay weight: "  << delayWeight  << mout.endl;
		if (delayWeight < 0.01)
			mout.warn() << "delay weight below 1%" << mout.endl;  // SKIP?
		w *= delayWeight;
	}
	// const double decayWeight = (decay==1.0) ? 1.0 : pow();


	mout.debug(1) << "input properties:\n" << cartSrc.odim << mout.endl;

	/// If a multi-radar mainComposite is being computed, a warning/note should be given is some of these properties are
	/// being setValues implicitly ie. based on the first input:
	/// 1) method => --cMethod
	/// 2) scaling (gain and offset) => --target
	/// 3) time => --cTime  (applied by cTimeDecay)


	/// If compositing scope is undefined, use that of the tile.
	if (!resources.composite.isDefined()){

		mout.note() << "Using input properties: " << mout.endl;
		mout.note() << "\t --cProj '" << cartSrc.odim.projdef << "'" << mout.endl;
		resources.composite.setProjection(cartSrc.odim.projdef);

		if (resources.composite.getFrameWidth()*resources.composite.getFrameHeight() == 0){
			resources.composite.setGeometry(cartSrc.odim.xsize, cartSrc.odim.ysize);
			mout.note() << "\t --cSize '" << resources.composite.getFrameWidth() << 'x' << resources.composite.getFrameHeight() << "'" << mout.endl;
		}

		resources.composite.setBoundingBoxD(cartSrc.odim.LL_lon, cartSrc.odim.LL_lat, cartSrc.odim.UR_lon, cartSrc.odim.UR_lat);
		mout.note() << "\t --cBBox '" << resources.composite.getBoundingBoxD() << "'" << mout.endl;
		//mout.note() << "Using bounding box of the input: " << resources.composite.getBoundingBoxD() << mout.endl;

		// mout.warn() << "Defined composite: " << composite << mout.endl;
	}

	// At this stage, at latest, reserve memory for the accumulation array.
	resources.composite.allocate();
	// mout.warn() << "Defined composite: " << composite << mout.endl;

	// Update source list, time stamp etc needed also for time decay
	//resources.composite.updateMetadata(cartSrc.odim);

	mout.debug() << "Composite initialized: " << resources.composite << mout.endl;

	int i0,j0;
	resources.composite.deg2pix(cartSrc.odim.UL_lon, cartSrc.odim.UL_lat, i0, j0);
	j0++; // UL pixel is located at (0,-1) in the image coordinates!
	mout.debug(1) << "sub image start coordinates: " << i0 << ',' << j0 << mout.endl;


	resources.composite.addCartesian(cartSrc, srcQuality, w, i0, j0);

	//mout.warn() << "composite: " << resources.composite << mout.endl;

	//drain::image::File::write(srcQuality.data, "srcQuality.png");

}


}  // namespace rack::



// Rack
