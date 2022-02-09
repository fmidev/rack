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


#include <string>

#include "drain/util/Log.h"

///#include "drain/prog/Command.h"
///
#include "drain/prog/CommandInstaller.h"


#include "data/SourceODIM.h"

#include "resources.h"  // for RackContext?

#include "composite.h"  // for cmdFormat called by
#include "cartesian-add.h"
#include "cartesian-extract.h"
#include "cartesian-bbox.h"
#include "cartesian-create.h"
#include "cartesian-grid.h"
#include "cartesian-motion.h"
#include "cartesian-plot.h"




namespace rack {



Composite & Compositor::getComposite() const {

	RackContext & ctx  = this->template getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	if (ctx.composite.isDefined()){ // raw or product
		mout.debug() << "private composite" << mout.endl;
		return ctx.composite;
	}

	RackContext & baseCtx = getResources().baseCtx();
	if (baseCtx.composite.isDefined()){ // raw or product
		mout.debug() << "shared composite" << mout.endl;
		return baseCtx.composite;
	}

	// Undefined, but go on...
	return ctx.composite;

};

double Compositor::applyTimeDecay(double w, const ODIM & odim) const {

	RackContext & ctx  = this->template getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	//const RackResources & resources = getResources();
	//RackContext & ctx = getContext<RackContext>();

	Composite & composite = getComposite();

	if (composite.decay < 1.0){

		const double delayMinutes = composite.getTimeDifferenceMinute(odim);  // assume update done
		mout.info() << "Delay minutes: " << delayMinutes << mout.endl;

		const double delayWeight = ::pow(composite.decay, delayMinutes);
		mout.info() << "Scaled delay weight: "  << delayWeight  << mout.endl;
		if (delayWeight < 0.01)
			mout.warn() << "decay (delay weight coeff) below 0.01" << mout.endl;  // SKIP?
		w *= delayWeight;

	}
	else if (composite.decay > 1.0){
		mout.warn() << "decay coeff (" << composite.decay << ") above 1.0, adjusting 1.0." << mout;
		composite.decay = 1.0;
	}

	return w;
}



void Compositor::add(drain::Flags::value_t inputFilter) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	mout.timestamp("BEGIN_CART_ADD");

	RackResources & resources = getResources();

	// Check
	if (ctx.statusFlags.isSet(drain::StatusFlags::INPUT_ERROR) ){ // ! resources.inputOk){
		mout.note() << "last INPUT inapplicable, skipping it" << mout.endl;
		ctx.select.clear(); // ?
		return;
	}

	// Check
	if (ctx.statusFlags.isSet(drain::StatusFlags::DATA_ERROR) ){ // (! resources.dataOk){
		mout.note() << "last DATA inapplicable, skipping it" << mout.endl;
		ctx.select.clear(); // ?
		return;
	}


	Composite & composite = getComposite();

	/// Set default method, if unset.
	if (!composite.isMethodSet()){
		composite.setMethod("MAXIMUM");  // ("LATEST");
		mout.note() << " compositing method unset, setting:" << composite.getMethod() << mout;
	}

	/// Set input data selector (typically, by quantity)
	if (!ctx.select.empty()){
		// mout.warn() << "Setting selector=" << resources.select << mout.endl;
		const std::string quantityOrig(composite.dataSelector.quantity);

		//composite.dataSelector.setParameters(resources.baseCtx().select);
		composite.dataSelector.setParameters(ctx.select);  // consume (clear)?

		mout.warn() << "Adjusted composite input selector=" << ctx.select << " -> " << composite.dataSelector << mout;
		//resources.select = "quantity=" + composite.dataSelector.quantity;
		//resources.select.clear(); // PROBLEMS HERE?

		// TODO: selecor.quantity is allowed to be regExp?
		// TODO: what if one wants to add TH or DBZHC in a DBZH composite?
		if (!quantityOrig.empty() && (quantityOrig != composite.dataSelector.quantity)){
			mout.warn() << "quantity selector changed, resetting accumulation array" << mout;
			composite.clear();
			composite.odim.quantity.clear();
			composite.odim.scaling.set(0.0, 0.0);
		}
	}
	else {
		if (composite.dataSelector.quantity.empty()){
			mout.info() << "Setting selector quantity=" << composite.odim.quantity << mout;
			composite.dataSelector.quantity = composite.odim.quantity; // consider "^"+...+"$"
			//
		}
	}



	/// Set dfault encoding for final (extracted) product. Applied by RadarAccumulator.
	//  If needed, initialize with input metadata.
	if (!resources.baseCtx().targetEncoding.empty()){
		composite.setTargetEncoding(resources.baseCtx().targetEncoding);
		resources.baseCtx().targetEncoding.clear(); // OMP?
	}


	//const Hi5Tree & src = ctx.getHi5(RackContext::CARTESIAN | RackContext::POLAR | RackContext::CURRENT);
	const Hi5Tree & src = ctx.getHi5(inputFilter);

	const RootData<SrcType<ODIM> > root(src);
	mout.debug() << "Src root /what: " << root.getWhat() << mout;

	//mout.info() << "now: vmap" << mout.endl;
	mout.special() << "using selector: " << composite.dataSelector << mout; // consider: if composite.dataSelector...?

	/// Main
	//if (ctx.currentHi5 == ctx.currentPolarHi5){
	// if (ctx.currentPolarHi5 != NULL){  // to get polar AMVU and AMVV data converted
	// if ((ctx.currentPolarHi5 != NULL) && !ctx.currentPolarHi5->isEmpty()){  // to get polar AMVU and AMVV data converted
	const drain::Variable & object = root.getWhat()["object"];
	if ((object == "COMP") || (object == "IMAGE")){
		//if ((object == "SCAN") || (object == "PVOL")){
		mout.info() << "Cartesian input data, ok" << mout.endl;
		addCartesian(src);
	}
	else {
		//else if (ctx.currentHi5 == & ctx.cartesianHi5){
		if ((object == "SCAN") || (object == "PVOL"))
			mout.info() << "polar input data, ok" << mout.endl;
		else if (object.isEmpty())
			mout.warn() << "empty what:object, assuming polar" << mout.endl;
		else
			mout.warn() << "suspicious what:object=" << object << ", assuming polar" << mout.endl;
		addPolar(src);
	}
	/*
	else {
		mout.error() << "current H5 data inapplicable for compositing" << mout.endl;
	}
	*/




}



// Originally crom create
void Compositor::addPolar(const Hi5Tree & src) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, getName());

	mout.timestamp("BEGIN_CART_CREATE");

	RackResources & resources = getResources();

	Composite & composite = getComposite();

	//if (ctx.currentPolarHi5 == NULL){
	if (src.isEmpty()){
		mout.warn() << "no polar data loaded, skipping" << mout.endl;
		return;
	}

	bool isAeqd = false;
	// mout.warn() << composite.getBoundingBoxD().getArea() << mout.endl;
	mout.debug() << composite  << mout.endl;

	// TODO: prune
	if (!composite.isDefined()){

		mout.info() << "Initialising (like) a single-radar Cartesian" << mout.endl;

		// Try to set size. May still remain 0x0, which ok: addPolar() will set it if needed.
		// composite.setGeometry(composite.getFrameWidth(), composite.getFrameHeight()); // Check - does nothing?
		/*
		if (!ctx.projStr.empty())
			composite.setProjection(ctx.projStr);
		else
			isAeqd = true;
		*/
		if (!composite.projectionIsSet())
			isAeqd = true;
		// see single below
	}

	// mout.warn() << "FLAGS: " << ctx.statusFlags << mout.endl;


	try {
		ctx.statusFlags.reset(); // ALL?
		//ctx.statusFlags.unset(RackResources::DATA_ERROR); // resources.dataOk = false; // return if input not ok?

		mout.debug() << "composite.dataSelector: " << composite.dataSelector << mout.endl;
		// mout.debug() << "composite.dataSelector.pathMatcher: " << composite.dataSelector.pathMatcher << mout.endl;
		// mout.special() << "composite.dataSelector.pathMatcher.front: " << composite.dataSelector.pathMatcher.front().flags.keysToStr << mout.endl;


		composite.dataSelector.updateBean(); // quantity
		mout.info() << "composite.dataSelector: " << composite.dataSelector << mout.endl;

		ODIMPath dataPath;
		composite.dataSelector.getPath3(src, dataPath);
		if (dataPath.empty()){
			mout.warn() << "create composite: no group found with selector:" << composite.dataSelector << mout.endl;
			//resources.inputOk = false; // REMOVE?
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR); // ctx.statusFlags.set(RackResources::DATA_ERROR); // resources.dataOk = false;
			return;
		}

		const Data<PolarSrc> polarSrc(src(dataPath)); // NOTE direct path, not from dataSet.getData() ! (because may be plain /qualityN data)


		/// GET INPUT DATA
		if ( !polarSrc.data.isEmpty() ){
			mout.info() << "using input path:" << dataPath << ", quantity:" << polarSrc.odim.quantity << mout.endl;
		}
		else {
			mout.warn() << "skipping empty input data: quantity=" << polarSrc.odim.quantity << ", path:" << dataPath << "/data" << mout.endl;  // was: dataSetPath
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR); // ctx.statusFlags.set(RackResources::DATA_ERROR); // resources.dataOk = false; // REMOVE?
			return;
		}

		// mout.warn() << "composite: " << composite.odim << mout.endl;
		// mout.warn() << "FLAGS: " << ctx.statusFlags << mout.endl;

		if (!composite.odim.isSet()){

			composite.odim.type = "";

			composite.odim.updateLenient(polarSrc.odim);
			//composite.odim.updateFromMap(polarSrc.odim); // REMOVED. Overwrites time

			const std::string & encoding = composite.getTargetEncoding();
			if (encoding.empty()){
				// This is somewhat disturbing but perhaps worth it.
				mout.note() << "adapting encoding of first input: " << EncodingODIM(composite.odim) << mout.endl;
			}
			mout.debug() << "storing metadata: " << composite.odim << mout.endl;
			ProductBase::completeEncoding(composite.odim, encoding); // note, needed even if encoding==""
		}
		else {
			if (!resources.baseCtx().targetEncoding.empty()){
				mout.warn() << "target encoding request ("<< resources.baseCtx().targetEncoding << ") bypassed, keeping original " << EncodingODIM(composite.odim) << mout.endl;
			}
			drain::Time tComp;
			drain::Time tData;

			composite.odim.getTime(tComp);
			polarSrc.odim.getTime(tData);
			int mins = abs(tComp.getTime() - tData.getTime())/60; // why abs?
			if (mins > 120){ // TODO tunable threshold + varying levels of warnings
				mout.warn() << "time difference " << mins << " minutes" << mout.endl;
				mout.note() << "composite: " << tComp.str() << mout.endl;
				mout.note() << "data:      " << tData.str() << mout.endl;
			}
		}



		//composite.checkInputODIM(polarSrc.odim);
		// Apply user parameters.

		mout.debug()  << "subComposite defined:\n" << composite.getBoundingBoxD() << ", quantity: " << composite.odim.quantity << mout.endl;
		mout.debug2() << "subComposite: " << composite << '\n' << composite.odim << mout.endl;
		mout.debug2() << "accumulating polar data..." << mout.endl;


		// subComposite.addPolar(polarSrc, 1.0, isAeqd); // Subcomposite: always 1.0.
		// const PlainData<PolarSrc> & srcQuality = polarSrc.hasQuality() ? polarSrc.getQualityData("QIND");
		ODIMPathElem current = dataPath.back();
		ODIMPath parent  = dataPath; // note: typically dataset path, but may be e.g. "data2", for "quality1"
		parent.pop_back();

		//mout.warn() << parent << "/HOW" << src(parent)[ODIMPathElem::HOW].data.attributes << mout;
		//mout.warn() << datasetPath << "/HOW" << src[datasetPath][ODIMPathElem::HOW].data.attributes << mout;
		const drain::VariableMap & how = src(parent)[ODIMPathElem::HOW].data.attributes;
		composite.metadataMap["how:angles"].setType(typeid(double));
		composite.metadataMap["how:angles"] = how["angles"];

		double w = weight;

		if (current.is(ODIMPathElem::QUALITY)){
			mout.info()  << "plain quality data, ok (no further quality data)" << mout.endl;  // TODO: fix if quality/quality (BirdOp)
			static const Hi5Tree t;
			static const PlainData<PolarSrc> empty(t);
			composite.addPolar(polarSrc, empty, 1.0, isAeqd); // Subcomposite: always 1.0.
			//DATA_ONLY = true;
		}
		else {


			w = applyTimeDecay(w, polarSrc.odim);
			mout.info() << "final quality weight=" << w << mout.endl;

			if (polarSrc.hasQuality()){
				mout.info() << "using local qualitydata" << mout.endl;
				composite.addPolar(polarSrc, polarSrc.getQualityData("QIND"), w, isAeqd); // Subcomposite: always 1.0.
			}
			else {
				DataSet<PolarSrc> dataSetSrc((*ctx.currentPolarHi5)(parent));
				//dataSetSrc.getQualityData2()
				const PlainData<PolarSrc> & srcDataSetQuality = dataSetSrc.getQualityData("QIND");
				if (!srcDataSetQuality.data.isEmpty()){
					mout.info() << "using shared (dataset-level) quality data, path=" << parent << mout.endl;
				}
				else {
					mout.info() << "no quality data (QIND) found under path=" << parent << mout.endl;
				}
				composite.addPolar(polarSrc, srcDataSetQuality, w, isAeqd); // Subcomposite: always 1.0.
			}
		}
		//mout.warn() << "FLAGS: " << ctx.statusFlags << mout.endl;
		//drain::Point2D<>
		//composite.projR2M.projectFwd(polarSrc.odim.lon, polarSrc.odim.lat, x, y);

		mout.debug2() << "finished" << mout.endl;

		//ctx.setStatus("lonPx", polarSrc.odim.lat);

		if (isAeqd){
			ctx.setStatus("RANGE", polarSrc.odim.getMaxRange());
			// ctx.getStatus()["RANGE"] = polarSrc.odim.getMaxRange();
			// drain::getRegistry().getStatusMap(false)["RANGE"] = polarSrc.odim.getMaxRange();
		}

		//drain::Variable & angles = composite.metadataMap["how:angles"];
		//mout.warn() << "HOW" << polarSrc.getHow() << mout;
		//mout.warn() << "HOW" << srcGroup[ODIMPathElem::HOW].data.attributes << mout;


		//composite.metadataMap["how:angles"] = polarSrc.odim.angles;
		// elangles = ;

		ctx.unsetCurrentImages();

	}
	catch (std::exception & e) {
		//std::cerr << e.what() << std::endl;
		mout.warn() << e.what() << mout.endl;
	}
}


// Originally crom create
void Compositor::addCartesian(const Hi5Tree & src) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // = getResources().mout;


	Composite & composite = getComposite();

	// NOTE: DATASET path needed for quality selection (below)
	ODIMPath dataPath;
	//composite.dataSelector.pathMatcher.setElems(ODIMPathElem::DATASET);
	composite.dataSelector.getPath3(src, dataPath);
	if (dataPath.back().is(ODIMPathElem::DATA))
		dataPath.pop_back();
	//composite.dataSelector.getPathNEW((ctx.cartesianHi5), dataPath, ODIMPathElem::DATASET); // NEW 2019/05
	if (dataPath.empty()){
		mout.warn() << "create composite: no group found with selector:" << composite.dataSelector << mout.endl;
		ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);  // resources.dataOk = false;
		return;
	}

	//const ODIMPath & p = dataPath;
	mout.info() << "using: dataset path:  " << dataPath << mout.endl;

	const DataSet<CartesianSrc> cartDataSetSrc(src(dataPath), composite.dataSelector.quantity);

	if (cartDataSetSrc.empty()){
		mout.warn() << "Empty dataset(s), skipping. Selector.quantity (regexp): '" << composite.dataSelector.quantity << "'" << mout.endl;
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
		mout.info() << "no quality data, using default quality:" << composite.defaultQuality << mout.endl;
		//resources.cDefaultQuality << mout.endl;
		w *= composite.defaultQuality; //resources.cDefaultQuality; ?
	}
	else {
		mout.info() << "using quality data, ok. " << mout.endl;
	}

	// If needed, initialize with input metadata.
	// Apply user parameters.
	// composite.ensureEncoding(cartSrc.odim, resources.targetEncoding);
	// resources.targetEncoding.clear();

	w = applyTimeDecay(w, cartSrc.odim);

	mout.debug2() << "input properties:\n" << cartSrc.odim << mout.endl;

	/// If a multi-radar mainComposite is being computed, a warning/note should be given is some of these properties are
	/// being setValues implicitly ie. based on the first input:
	/// 1) method => --cMethod
	/// 2) scaling (gain and offset) => --target
	/// 3) time => --cTime  (applied by cTimeDecay)


	/// If compositing scope is undefined, use that of the tile.
	if (!composite.isDefined()){

		mout.note() << "Using input properties: " << mout.endl;
		mout.note() << "\t --cProj '" << cartSrc.odim.projdef << "'" << mout.endl;
		composite.setProjection(cartSrc.odim.projdef);

		if (composite.getFrameWidth()*composite.getFrameHeight() == 0){
			composite.setGeometry(cartSrc.odim.area.width, cartSrc.odim.area.height);
			mout.note() << "\t --cSize '" << composite.getFrameWidth() << 'x' << composite.getFrameHeight() << "'" << mout.endl;
		}

		composite.setBoundingBoxD(cartSrc.odim.getBoundingBoxD());
		//composite.setBoundingBoxD(cartSrc.odim.LL_lon, cartSrc.odim.LL_lat, cartSrc.odim.UR_lon, cartSrc.odim.UR_lat);
		mout.note() << "\t --cBBox '" << composite.getBoundingBoxD() << "'" << mout.endl;
		if (!composite.projR2M.isLongLat())
			mout.note() << "\t --cBBox '" << composite.getBoundingBoxM() << "'" << mout.endl;
				//mout.note() << "Using bounding box of the input: " << composite.getBoundingBoxD() << mout.endl;

		// mout.warn() << "Defined composite: " << composite << mout.endl;
	}

	// At this stage, at latest, reserve memory for the accumulation array.
	composite.allocate();
	// mout.warn() << "Defined composite: " << composite << mout.endl;

	// Update source list, time stamp etc needed also for time decay
	// => in addCartesian()

	mout.debug() << "Composite initialized: " << composite << mout.endl;

	int i0,j0;
	composite.deg2pix(cartSrc.odim.UL_lon, cartSrc.odim.UL_lat, i0, j0);
	j0++; // UL pixel is located at (0,-1) in the image coordinates!
	mout.debug2() << "sub image start coordinates: " << i0 << ',' << j0 << mout.endl;

	composite.addCartesian(cartSrc, srcQuality, w, i0, j0);

	ctx.setCurrentImages(cartSrc.data);
	//mout.warn() << "composite: " << composite << mout.endl;
	//drain::image::File::write(srcQuality.data, "srcQuality.png");

	/*
	if (ctx.svg.isEmpty()){
		ctx.svg->setType(NodeSVG::SVG);
		ctx.svg->set("width", 100);
		ctx.svg->set("height", 200);
	}

	SourceODIM odim(cartSrc.odim.source);
	TreeSVG & node = ctx.svg[odim.NOD];
    node->setType(NodeSVG::RECT);
    node->set("width",  cartSrc.odim.area.width);
    node->set("height", cartSrc.odim.area.height);
    node["title"]->ctext = odim.NOD;
	*/

}

void Compositor::extract(const std::string & channels) const {


	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	//mout.note("MIKA") << "MÄKI" << mout;
	//mout.error() << "MÄKI" << mout;

	RackResources & resources = getResources();

	Composite & composite = getComposite();

	// Append results - why not, but Cartesian was typically used for subcompositing
	// ctx.cartesianHi5.clear();
	//resources.setSource(ctx.cartesianHi5, *this);

	//ODIMPath path("dataset1");
	ODIMPath path;

	ODIMPathElem parent(ODIMPathElem::DATASET, 1);
	if (ctx.appendResults.is(ODIMPathElem::DATASET))
		DataSelector::getNextChild(ctx.cartesianHi5, parent);
	else if (ctx.appendResults.is(ODIMPathElem::DATA)){
		DataSelector::getLastChild(ctx.cartesianHi5, parent);
		if (parent.index == 0){
			parent.index = 1;
		}
	}
	else
		ctx.cartesianHi5.clear(); // don't append, overwrite...

	path << parent; // ?
	mout.debug() << "dst path: " << path << mout.endl;

	Hi5Tree & dstGroup = ctx.cartesianHi5(path);
	DataSet<CartesianDst> dstProduct(dstGroup);

	mout .debug3() << "update geodata " << mout.endl;
	composite.updateGeoData(); // TODO check if --plot cmds don't need

	// NEW 2020/06
	RootData<CartesianDst> dstRoot(ctx.cartesianHi5);
	//CartesianODIM rootOdim; // needed? yes, because Extract uses (Accumulator &), not Composite.
	CartesianODIM & rootOdim = dstRoot.odim; // TEST
	rootOdim.updateFromMap(composite.odim);

	//mout.warn() << composite.odim << mout.endl;

	ProductBase::completeEncoding(rootOdim, composite.getTargetEncoding());


	if (!ctx.targetEncoding.empty()){
		ProductBase::completeEncoding(rootOdim, resources.baseCtx().targetEncoding);
		// odim.setValues(resources.targetEncoding, '=');
		ctx.targetEncoding.clear();
	}

	//mout.warn() << "composite: " << composite.odim << mout.endl;
	//mout.warn() << "composite: " << composite << mout.endl;
	//mout.note() << "dst odim: " << odim << mout.endl;
	mout.debug2() << "Extracting..." << mout.endl;

	composite.extract(rootOdim, dstProduct, channels);
	//mout.warn() << "extracted data: " << dstProduct << mout.endl; // .getFirstData().data

	/// Final step: write metadata

	// Note: dstRoot will write most of them upon destruction.
	// So only "additional data stored here"

	drain::VariableMap & how = dstRoot.getHow();
	how["software"]   = __RACK__;
	how["sw_version"] = __RACK_VERSION__;
	// Non-standard
	how["tags"] = composite.nodeMap.toStr(':');

	// Non-standard
	drain::VariableMap & where = dstRoot.getWhere();
	where["BBOX"].setType(typeid(double));
	where["BBOX"] = composite.getBoundingBoxD().toVector();

	if (composite.isLongLat())
		where["BBOX_native"].setType(typeid(double));
	else
		where["BBOX_native"].setType(typeid(long int));
	where["BBOX_native"] = composite.getBoundingBoxM().toVector();

	where["BBOX_data"].setType(typeid(double));
	const drain::Rectangle<double> & bboxDataD = composite.getDataExtentD();
	where["BBOX_data"] = bboxDataD.toVector();

	drain::Rectangle<int> bboxDataPix;
	composite.deg2pix(bboxDataD.lowerLeft, bboxDataPix.lowerLeft);
	composite.deg2pix(bboxDataD.upperRight, bboxDataPix.upperRight);
	where["BBOX_data_pix"].setType(typeid(short int));
	where["BBOX_data_pix"] = bboxDataPix.toVector();

	where["BBOX_overlap"].setType(typeid(double));
	where["BBOX_overlap"] = composite.getDataOverlapD().toStr();


	DataTools::updateCoordinatePolicy(ctx.cartesianHi5, RackResources::limit);
	DataTools::updateInternalAttributes(ctx.cartesianHi5);

	ctx.currentHi5 = & ctx.cartesianHi5;

	/// For successfull file io:
	ctx.statusFlags.unset(drain::StatusFlags::INPUT_ERROR); // resources.inputOk = false;

	if (dstProduct.has(composite.odim.quantity)){

		Data<CartesianDst> & dstData = dstProduct.getData(composite.odim.quantity); // OR: by odim.quantity
		if (dstData.data.isEmpty()){
			mout.warn() << "empty product data: " << dstData << mout.endl;
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
			ctx.unsetCurrentImages();
		}
		else {
			mout.debug() << "extracted quantity: " << dstProduct << mout.endl; // .getFirstData().data
			// NEW
			drain::VariableMap & how = dstProduct.getHow();
			//how["elangles"] = composite.metadataMap.get("how:elangles", {0,1,2});
			//if (composite.metadataMap.hasKey("how:angles"))
			how["angles"].setType(typeid(double));
			how["angles"] = composite.metadataMap["how:angles"];
			ctx.setCurrentImages(dstData.data);
			ctx.statusFlags.unset(drain::StatusFlags::DATA_ERROR);
		}
	}
	else {
		mout.experimental() << dstProduct << mout;
		mout.warn() << "dstProduct does not have claimed quantity: " << composite.odim.quantity << mout.endl; // .getFirstData().data
		ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
		ctx.unsetCurrentImages();
	}


	//mout.warn() << "created" << mout.endl;

	// NEW 2020/07
	ctx.select.clear();

	drain::VariableMap & statusMap = ctx.getStatusMap();
	statusMap.updateFromMap(rootOdim);

	statusMap.updateFromMap(composite.nodeMap);
	statusMap.updateFromMap(composite.metadataMap);
	// Spoils input.sh...
	//std::cout << ctx.svg << '\n';

}










/*
// TODO: change to half-time in minutes
class CompositeTimeDecay : public drain::SimpleCommand<double> {

public:

	//	CompositeTimeDecay() : drain::BasicCommand(__FUNCTION__, "Delay weight (0.9...1.0) per minute. 1=no decay. See --cTime"){
	CompositeTimeDecay() : drain::SimpleCommand<double>(__FUNCTION__, "Delay weight (0.9...1.0) per minute. 1=no decay. See --cTime", "decay", 1.0){
		//parameters.link("decay", getResources().composite.decay = 1.0, "coeff");
	};

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		//Composite & composite = getComposite();
		ctx.composite.decay = value;
	}

};
*/

/// half-time in minutes
/**
 *    x/2 = x * (1/2)^{t/T}
 *
 *	log(0.5) = t/T*log(0.5) =>
 class CompositeDecayTime : public drain::SimpleCommand<int> {

	public:
	CompositeDecayTime() : drain::SimpleCommand<int>(__FUNCTION__, "Delay half-time in minutes. 0=no decay", "time", 0, "minutes"){
		//parameters.link("halftime", getResources().composite.decay = 1.0, "coeff");
	};

	inline
	void exec() const {
		// Composite & composite = getComposite();
		RackContext & ctx = getContext<RackContext>();
		if (this->value > 0){
			ctx.composite.decay = ::pow(0.5, 1.0 / static_cast<double>(value));
		}
		else {
			ctx.composite.decay = 1.0;
		}
	}


};
*/








}  // namespace rack::



// Rack
