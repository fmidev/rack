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


#include <drain/Log.h>
#include <string>

#include <drain/util/Output.h> // debugging threads
#include <drain/prog/CommandInstaller.h>

#include "data/SourceODIM.h"
#include "data/ODIMPathTools.h"
#include "product/ProductBase.h"

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



double Compositor::applyTimeDecay(Composite & composite, double w, const ODIM & odim) const {

	RackContext & ctx  = this->template getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// const RackResources & resources = getResources();
	// RackContext & ctx = getContext<RackContext>();
	// Composite & composite = ctx.getComposite(RackContext::PRIVATE);

	if (composite.decay < 1.0){

		const double delayMinutes = composite.getTimeDifferenceMinute(odim);  // assume update done
		//mout.info("Delay minutes: " , delayMinutes );

		const double delayWeight = ::pow(composite.decay, delayMinutes);
		mout.info("Delay minutes: ", delayMinutes, " -> scaled delay weight: ", delayWeight);
		if (delayWeight < 0.01)
			mout.warn("decay (delay weight coeff) below 0.01" );  // SKIP?
		w *= delayWeight;

	}
	else if (composite.decay > 1.0){
		mout.warn("decay coeff (" , composite.decay , ") above 1.0, adjusting 1.0." );
		composite.decay = 1.0;
	}

	// mout.debug("Decay based weight w=", w);

	return w;
}


// drain::Flags::ivalue_t // Hdf5Context::h5_role::ivalue_t
void Compositor::add(Composite & composite, int inputFilter, bool updateSelector) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);


	/*
	mout.debug("add A1 " + ctx.getName());
	std::ostream & logOrig = std::cerr;
	std::stringstream sstr;
	sstr << "thread-" << ctx.getId() << ".log";
	drain::Output output(sstr.str());
	ctx.log.setOstr(output);
	(std::ostream &)output << "# LOG: " << sstr.str() << '\n';
	mout.debug("add A2 #" + ctx.getName());
	*/


	if (ctx.statusFlags){
		mout.warn("Status flags before accumulating: ", ctx.statusFlags);
	}

	RackResources & resources = getResources();

	// Check
	if (ctx.statusFlags.isSet(drain::Status::INPUT_ERROR) ){ // ! resources.inputOk){
		mout.note("last INPUT inapplicable, skipping it" );
		ctx.select.clear(); // ?
		return;
	}

	// Check
	if (ctx.statusFlags.isSet(drain::Status::DATA_ERROR) ){ // (! resources.dataOk){
		mout.note("last DATA inapplicable, skipping it" );
		ctx.select.clear(); // ?
		return;
	}


	// Changed order 2022/12

	//mout.debug("add B #" + ctx.getName());

	#pragma omp critical
	{
		/// Set default method, if unset.
		if (!composite.isMethodSet()){
			composite.setMethod("MAXIMUM");  // ("LATEST");
			mout.note(" compositing method unset, setting:" , composite.getMethod() );
		}

		/// Set dfault encoding for final (extracted) product. Applied by RadarAccumulator.
		//  If needed, initialize with input metadata.
		if (!resources.baseCtx().targetEncoding.empty()){
			mout.attention("encoding: ", resources.baseCtx().targetEncoding);
			composite.setTargetEncoding(resources.baseCtx().targetEncoding);
			resources.baseCtx().targetEncoding.clear(); // OMP?
		}

		// This was bad for --cCreate/-c , so moved to --cAdd
		if (updateSelector){
			composite.updateInputSelector(ctx.select);
		}
	}
	ctx.select.clear();

	mout.debug("add C #" + ctx.getName());

	const Hi5Tree & src = ctx.getHi5(inputFilter);

	if (src.empty()){
		mout.warn("thread #", ctx.getName(), ": input data empty? Filter =",
				drain::FlagResolver::getKeys(drain::EnumDict<Hdf5Context::Hi5Role>::dict, inputFilter, '|')) ;
				//drain::EnumDict<Hdf5Context::h5_role>::dict.getKey(inputFilter, '|'));
	}


	const RootData<SrcType<ODIM> > root(src);
	mout.debug("Src root /what: " , root.getWhat() );

	// mout.info("now: vmap" );
	// already in Composite mout.debug("Using composite selector: " , composite.dataSelector ); // consider: if composite.dataSelector...?

	/// Main
	const drain::Variable & object = root.getWhat()["object"];

	// std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << " type: " << typeid(a).name() << std::endl;

	if ((object == "COMP") || (object == "IMAGE")){
		//if ((object == "SCAN") || (object == "PVOL")){
		mout.info("Cartesian input data, ok" );
		addCartesian(composite, src);
	}
	else {
		//else if (ctx.currentHi5 == & ctx.cartesianHi5){
		if ((object == "SCAN") || (object == "PVOL"))
			mout.info("polar input data, ok");
		else if (object.empty())
			mout.warn("empty what:object, assuming polar");
		else
			mout.warn("suspicious what:object=", object, ", assuming polar");
		addPolar(composite, src);
	}
	/*
	else {
		mout.error("current H5 data inapplicable for compositing" );
	}

	*/

	//ctx.log.setOstr(logOrig);


}



// Originally crom create
void Compositor::addPolar(Composite & composite, const Hi5Tree & src) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, getName());

	//mout.timestamp("BEGIN_CART_CREATE");

	mout.debug("CART #" + ctx.getName());


	RackResources & resources = getResources();

	if (src.empty()){
		mout.warn("src data empty – no polar data loaded? Skipping...");
		return;
	}

	bool projectAEQD = false;
	//mout.warn(composite.getBoundingBoxDeg().getArea() );
	//composite.toStream(std::cerr);
	mout.debug(composite);
	//mout.warn("checkb");

	// TODO: prune
	if (!composite.isDefined()){

		mout.info("Initialising (like) a single-radar Cartesian");

		if (!composite.projectionIsSet()){
			mout.note("using radar-specific AEQD projection");
			projectAEQD = true;
		}
		else {
			mout.note("using predefined projection: ", composite.getProjection());
			//projectAEQD = true;
			projectAEQD = false;
		}

		// see single below
	}

	// mout.warn("FLAGS: " , ctx.statusFlags );

	try {
		ctx.statusFlags.reset(); // ALL?
		//ctx.statusFlags.unset(RackResources::DATA_ERROR); // resources.dataOk = false; // return if input not ok?

		#pragma omp critical
		{

			mout.debug("CART CRITICAL-1 ", ctx.getName());

			mout.debug("composite.dataSelector: ", composite.dataSelector);
			// mout.debug("composite.dataSelector.pathMatcher: " , composite.dataSelector.pathMatcher );
			// mout.special("composite.dataSelector.pathMatcher.front: " , composite.dataSelector.pathMatcher.front().flags.keysToStr );

			mout.revised("Removed composite.dataSelector.updateBean()");
			// composite.dataSelector.updateBean(); // quantity   // 2024/10

			if (composite.dataSelector.getMaxCount() != 1){
				mout.debug("composite.dataSelector.count ", composite.dataSelector.getMaxCount(), " > 1"); // , setting to 1.");
				//composite.dataSelector.setMaxCount(1);
			}
		}

		mout.debug("composite.dataSelector: ", composite.dataSelector );

		ODIMPath dataPath;
		composite.dataSelector.getPath(src, dataPath);
		if (dataPath.empty()){
			mout.warn("Create composite: no group found with selector:", composite.dataSelector);
			//resources.inputOk = false; // REMOVE?
			ctx.statusFlags.set(drain::Status::DATA_ERROR); // ctx.statusFlags.set(RackResources::DATA_ERROR); // resources.dataOk = false;
			return;
		}

		const Data<PolarSrc> polarSrc(src(dataPath)); // NOTE direct path, not from dataSet.getData() ! (because may be plain /qualityN data)


		/// GET INPUT DATA

		if (polarSrc.data.isEmpty() ){
			mout.warn(composite.dataSelector);
			mout.warn("skipping empty input data: quantity=", polarSrc.odim.quantity, ", path:", dataPath, "(/data?)");  // was: dataSetPath
			ctx.statusFlags.set(drain::Status::DATA_ERROR); // ctx.statusFlags.set(RackResources::DATA_ERROR); // resources.dataOk = false; // REMOVE?
			return;
		}

		//mout.accept<LOG_INFO>("using input path: ", dataPath, " [", polarSrc.odim.quantity, "] elangle=", polarSrc.odim.elangle);

		ODIMPathElem current = dataPath.back();
		ODIMPath parent = dataPath; // note: typically dataset path, but may be e.g. "data2", for "quality1"
		parent.pop_back();

		mout.accept<LOG_INFO>("using input path: ", parent, "|/", current, " [", polarSrc.odim.quantity, "] elangle=", polarSrc.odim.elangle);

		#pragma omp critical
		{

			mout.debug("CART CRITICAL-2 #", ctx.getName());

			// mout.warn("composite: " , composite.odim );
			// mout.warn("FLAGS: " , ctx.statusFlags );
			if (!composite.odim.isSet()){

				composite.odim.type = "";

				composite.odim.updateLenient(polarSrc.odim);
				//composite.odim.updateFromMap(polarSrc.odim); // REMOVED. Overwrites time

				const std::string & encoding = composite.getTargetEncoding();
				if (!encoding.empty()){
					mout.debug("Predefined encoding '", encoding, "' (str)");
				}
				else {
					mout.hint("Use --encoding if specific data type and scaling needed");
					// This is somewhat disturbing but perhaps worth it.
					/*
					if (projectAEQD){
						mout.note("Adapting encoding of input (AEQD): " , EncodingODIM(composite.odim) );
					}
					else {
						mout.note("Storing encoding of first input: " , EncodingODIM(composite.odim) );
					}
					*/
				}
				mout.debug("Storing metadata: " , composite.odim );
				composite.odim.completeEncoding( encoding); // NEW: unneeded? WAS: note, needed even if encoding==""
			}
			else {

				if (!resources.baseCtx().targetEncoding.empty()){
					mout.reject<LOG_WARNING>("Target encoding request (", resources.baseCtx().targetEncoding , ") skipped, keeping original " , EncodingODIM(composite.odim) );
				}

				// Compare timestamps
				drain::Time tComposite;
				drain::Time tData;

				composite.odim.getTime(tComposite);
				polarSrc.odim.getTime(tData);
				int mins = abs(tComposite.getTime() - tData.getTime())/60;
				if (mins > 5){ // TODO tunable threshold + varying levels of warnings

					if (mins > 1440){ // TODO tunable threshold + varying levels of warnings
						mout.warn("Time difference over ", (mins/1440), " DAYS");
					}
					else if (mins > 60){ // TODO tunable threshold + varying levels of warnings
						mout.warn("Time difference over ", (mins/60), " HOURS");
					}
					else if (mins > 15){
						mout.warn("time difference over ", mins, " minutes");
					}
					else { // TODO tunable threshold + varying levels of warnings
						mout.note("time difference ", mins, " minutes");
					}

					mout.info("composite: ", tComposite.str());
					mout.info("input:     ", tData.str());
				}
			}

			// composite.checkInputODIM(polarSrc.odim);
			// Apply user parameters.
			mout.debug2("accumulating polar data...");
			mout.debug( "subComposite defined: BBOX=", composite.getBoundingBoxDeg(), ", quantity: ", composite.odim.quantity, "'");
			mout.debug2("subComposite: ", composite, '\n', composite.odim);

			// subComposite.addPolar(polarSrc, 1.0, projectAEQD); // Subcomposite: always 1.0.
			// const PlainData<PolarSrc> & srcQuality = polarSrc.hasQuality() ? polarSrc.getQualityData("QIND");
			// ODIMPathElem	current = dataPath.back();
			// ODIMPath..parent  = dataPath; // note: typically dataset path, but may be e.g. "data2", for "quality1"
			// parent.pop_back();



			//mout.attention("current: ", src(parent));
			//hi5::Hi5Base::writeText(src(parent), std::cout);

			const drain::VariableMap & what = src(dataPath)[ODIMPathElem::WHAT].data.attributes;
			//			mout.attention(parent, '|', current, " ATTR: ", what);
			// TODO: consider copying all the features to composite.metadata(Map) ?
			const drain::Variable & legend = what["legend"];

			if (!legend.empty()){
				legend.toMap(composite.legend, ',', ':');
				mout.attention("TODO: copy LEGEND: ", legend, "-> ", drain::sprinter(composite.legend));
			}
			else {
				composite.legend.clear();
			}


			//mout.warn(parent , "/HOW" , src(parent)[ODIMPathElem::HOW].data.attributes );
			//mout.warn(datasetPath , "/HOW" , src[datasetPath][ODIMPathElem::HOW].data.attributes );
			const drain::VariableMap & how = src(parent)[ODIMPathElem::HOW].data.attributes;

			if (how["angles"].getElementCount() > 0){
				how["angles"].toSequence(composite.odim.angles);
			}
			else if (!polarSrc.odim.angles.empty()){
				composite.odim.angles = polarSrc.odim.angles;
			}
			else {
				composite.odim.angles.resize(1, polarSrc.odim.elangle);
				//compositeAngles = polarSrc.odim.elangle;
			}
			//composite.metadataMap["how:angles"] = polarSrc.odim.elangle;


		} // OMP CRITICAL


		double w = weight;

		mout.attention<LOG_DEBUG>("CART MAIN #" + ctx.getName());

		if (current.is(ODIMPathElem::QUALITY)){ // rare
			mout.info("plain quality data, ok (no further quality data)");  // TODO: fix if quality/quality (BirdOp)
			static const Hi5Tree t;
			static const PlainData<PolarSrc> empty(t);
			composite.addPolar(polarSrc, empty, 1.0, projectAEQD); // Subcomposite: always 1.0.
			//DATA_ONLY = true;
		}
		else {

			w = applyTimeDecay(composite, w, polarSrc.odim);
			mout.info("final quality weight=", w);

			if (polarSrc.hasQuality("QIND")){
				mout.info("Using local qualitydata");
				composite.addPolar(polarSrc, polarSrc.getQualityData("QIND"), w, projectAEQD); // Subcomposite: always 1.0.
			}
			else {
				// TODO: use ctx.getHi5(RackContext::CURRENT, RackContext::POLAR);
				DataSet<PolarSrc> dataSetSrc((*ctx.currentPolarHi5)(parent));
				//dataSetSrc.getQualityData2()
				const PlainData<PolarSrc> & srcDataSetQuality = dataSetSrc.getQualityData("QIND");
				if (!srcDataSetQuality.data.isEmpty()){
					mout.info("Using shared (dataset-level) quality data, path=" , parent );
				}
				else {
					mout.info("no quality data (QIND) found under path=" , parent );
				}
				composite.addPolar(polarSrc, srcDataSetQuality, w, projectAEQD); // Subcomposite: always 1.0.
			}
		}
		//mout.warn("FLAGS: " , ctx.statusFlags );
		//drain::Point2D<>
		//composite.projGeo2Native.projectFwd(polarSrc.odim.lon, polarSrc.odim.lat, x, y);

		mout.debug2("finished");

		if (composite.odim.product=="PPI"){
			if (composite.odim.prodpar.empty()){
				if (polarSrc.odim.elangle == 0.0){
					mout.suspicious(DRAIN_LOG_VAR(polarSrc.odim.elangle));
				}
				composite.odim.prodpar = "?"; // init
				composite.odim["what:prodpar"] = polarSrc.odim.elangle;
				// ODIM requirement
				mout.revised(DRAIN_LOG_VAR(polarSrc.odim.elangle));
				mout.revised(DRAIN_LOG_VAR(composite.odim.product), " -> adding: ", DRAIN_LOG_VAR(composite.odim.prodpar));
			}
		}

		//ctx.setStatus("lonPx", polarSrc.odim.lat);

		if (projectAEQD){
			ctx.setStatus("RANGE", polarSrc.odim.getMaxRange());
			// ctx.getStatus()["RANGE"] = polarSrc.odim.getMaxRange();
			// drain::getRegistry().getStatusMap(false)["RANGE"] = polarSrc.odim.getMaxRange();
		}

		//drain::Variable & angles = composite.metadataMap["how:angles"];
		//mout.warn("HOW" , polarSrc.getHow() );
		//mout.warn("HOW" , srcGroup[ODIMPathElem::HOW].data.attributes );
		//composite.metadataMap["how:angles"] = polarSrc.odim.angles;
		// elangles = ;

		ctx.unsetCurrentImages(); //?

	}
	catch (std::exception & e) {
		//std::cerr << e.what() << std::endl;
		mout.warn(e.what() );
	}


}


// Originally crom create
void Compositor::addCartesian(Composite & composite, const Hi5Tree & src) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = getResources().mout;

	// Composite & composite = getComposite();

	// NOTE: DATASET path needed for quality selection (below)
	ODIMPath dataPath;
	//composite.dataSelector.pathMatcher.setElems(ODIMPathElem::DATASET);
	if (composite.dataSelector.getMaxCount() != 1){
		mout.debug("composite.dataSelector.count=", composite.dataSelector.getMaxCount(), " > 1"); // , setting to 1.");
		//composite.dataSelector.setMaxCount(1);
	}

	composite.dataSelector.getPath(src, dataPath);
	if (dataPath.back().is(ODIMPathElem::DATA))
		dataPath.pop_back();
	//composite.dataSelector.getPathNEW((ctx.cartesianHi5), dataPath, ODIMPathElem::DATASET); // NEW 2019/05
	if (dataPath.empty()){
		mout.warn("create composite: no group found with selector:" , composite.dataSelector );
		ctx.statusFlags.set(drain::Status::DATA_ERROR);  // resources.dataOk = false;
		return;
	}

	//const ODIMPath & p = dataPath;
	mout.info("using: dataset path:  " , dataPath );

	const DataSet<CartesianSrc> cartDataSetSrc(src(dataPath), composite.dataSelector.getQuantity());

	if (cartDataSetSrc.empty()){
		mout.warn("Empty dataset(s), skipping. Selector.quantity (regexp): '" , composite.dataSelector.getQuantity() , "'" );
		return;
	}

	const Data<CartesianSrc> & cartSrc = cartDataSetSrc.getFirstData(); // quantity match done above
	if (cartSrc.odim.quantity.empty()){
		mout.warn("no quantity in data: " , cartSrc.odim );
	}
	else {
		mout.debug("quantity: " , cartSrc.odim.quantity );
	}

	if (cartSrc.data.isEmpty()){
		mout.warn("null sized tile, skipping..." );
		return;
	}

	//mout.startTiming(cartSrc.odim.source);


	// NEW: pick local or global quality field.
	const PlainData<CartesianSrc> & srcQuality = cartSrc.hasQuality() ? cartSrc.getQualityData("QIND") : cartDataSetSrc.getQualityData("QIND");

	//const double weight = (key=="cAdd") ? 1.0 : (double)value; // todo: default value?
	double w = weight;

	if (srcQuality.data.isEmpty()){
		mout.info("no quality data, using default quality:" , composite.defaultQuality );
		//resources.cDefaultQuality << mout.endl;
		w *= composite.defaultQuality; //resources.cDefaultQuality; ?
	}
	else {
		mout.info("using quality data, ok. " );
	}

	// If needed, initialize with input metadata.
	// Apply user parameters.
	// composite.ensureEncoding(cartSrc.odim, resources.targetEncoding);
	// resources.targetEncoding.clear();

	w = applyTimeDecay(composite, w, cartSrc.odim);

	mout.debug2("input properties:\n" , cartSrc.odim );

	/// If a multi-radar mainComposite is being computed, a warning/note should be given is some of these properties are
	/// being setValues implicitly ie. based on the first input:
	/// 1) method => --cMethod
	/// 2) scaling (gain and offset) => --target
	/// 3) time => --cTime  (applied by cTimeDecay)


	/// If compositing scope is undefined, use that of the tile.
	if (!composite.isDefined()){

		mout.info("Using input properties: " );
		mout.info("\t --cProj '" , cartSrc.odim.projdef , "'" );
		composite.setProjection(cartSrc.odim.projdef); // Projector::FORCE_CRS

		if (composite.getFrameWidth()*composite.getFrameHeight() == 0){
			composite.setGeometry(cartSrc.odim.area.width, cartSrc.odim.area.height);
			mout.info("\t --cSize '" , composite.getFrameWidth() , 'x' , composite.getFrameHeight() , "'" );
		}

		composite.setBoundingBoxD(cartSrc.odim.getBoundingBoxDeg());
		//composite.setBoundingBoxD(cartSrc.odim.LL_lon, cartSrc.odim.LL_lat, cartSrc.odim.UR_lon, cartSrc.odim.UR_lat);
		mout.info("\t --cBBox '" , composite.getBoundingBoxDeg() , "' # degrees" );
		if (!composite.projGeo2Native.isLongLat()){
			std::streamsize p = mout.precision(20);
			mout.info("\t --cBBox '" , composite.getBoundingBoxNat() , "' # metric" );
			mout.precision(p);
		}
				//mout.note("Using bounding box of the input: " , composite.getBoundingBoxDeg() );

		// mout.warn("Defined composite: " , composite );
	}

	// At this stage, at latest, reserve memory for the accumulation array.
	composite.allocate();
	// mout.warn("Defined composite: " , composite );

	// Update source list, time stamp etc needed also for time decay
	// => in addCartesian()

	mout.debug("Composite initialized: ", composite);

	int i0,j0;
	composite.deg2pix(cartSrc.odim.UL_lon, cartSrc.odim.UL_lat, i0, j0);
	j0++; // UL pixel is located at (0,-1) in the image coordinates!
	mout.debug2("sub image start coordinates: " , i0, ',', j0);

	// mout.attention("cartSrc: ", EncodingODIM(cartSrc.odim));
	composite.addCartesian(cartSrc, srcQuality, w, i0, j0);

	// mout.attention("composite.odim: ", EncodingODIM(composite.odim));


	ctx.setCurrentImages(cartSrc.data);
	//mout.warn("composite: " , composite );
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
    node["title"]->setText(odim.NOD); //CTXX
	*/

}

void Compositor::prepareBBox(const Composite & composite, const drain::BBox & cropGeo, drain::Rectangle<int> & cropImage){

	//const drain::BBox cropGeo(bboxGeo);

	if (!cropGeo.empty()){
		// mout.special("Cropping: ", cropGeo, cropGeo.isMetric() ? " [meters]": " [degrees]");
		if (cropGeo.isMetric()){
			if (composite.isLongLat()){
				// mout.error("Cannot crop long-lat composite with a metric bbox (", bbox, ") ");
				throw std::runtime_error(
						drain::StringBuilder<>("Cannot crop long-lat composite with a metric bbox (", cropGeo,")"));
				return;
			}
			// NOTE: vert coord swap
			// composite.m2pix(cropGeo.lowerLeft.x,  cropGeo.lowerLeft.y,   cropImage.lowerLeft.x,  cropImage.upperRight.y);
			// composite.m2pix(cropGeo.upperRight.x, cropGeo.upperRight.y,  cropImage.upperRight.x, cropImage.lowerLeft.y );
			composite.m2pix(cropGeo.lowerLeft,  cropImage.lowerLeft);
			composite.m2pix(cropGeo.upperRight, cropImage.upperRight);
		}
		else {
			// composite.deg2pix(cropGeo.lowerLeft.x,  cropGeo.lowerLeft.y,   cropImage.lowerLeft.x,  cropImage.upperRight.y);
			// composite.deg2pix(cropGeo.upperRight.x, cropGeo.upperRight.y,  cropImage.upperRight.x, cropImage.lowerLeft.y );
			composite.deg2pix(cropGeo.lowerLeft,  cropImage.lowerLeft);
			composite.deg2pix(cropGeo.upperRight, cropImage.upperRight);
		}

		//++cropImage.upperRight.y;
		--cropImage.upperRight.x;
		++cropImage.upperRight.y;


	}

}


/** NOTES
 *
 *  Composite metadata
 *
 *  From input:
 *  - definite: source info
 *  - conditional (if unset): encoding
 *
 */

//void Compositor::extract(Composite & composite, const std::string & channels, const drain::Rectangle<double> & bbox) const {
// void Compositor::extract(Composite & composite, const std::string & channels, const std::string & crop) const {
void Compositor::extract(Composite & composite, const drain::image::Accumulator::FieldList & channels, const std::string & crop) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	//mout.attention("Calling: composite.extractNEW2 ", channels, " enc:", encoding);

	RackResources & resources = getResources();

	if (!composite.isDefined()){
		mout.hint("consider --cInit if an empty data array is desired");
		mout.fail("empty composite, skipping extraction");
		return;
	}

	//mout.experimental("Encoding: ", EncodingODIM(composite.odim));

	// resources.setSource(ctx.cartesianHi5, *this);

	ODIMPathElem parent(ODIMPathElem::DATASET);
	if (ctx.appendResults.is(ODIMPathElem::DATASET)){
		ODIMPathTools::getNextChild(ctx.cartesianHi5, parent);
	}
	else if (ctx.appendResults.is(ODIMPathElem::DATA)){
		ODIMPathTools::getLastChild(ctx.cartesianHi5, parent, true);  // <- CREATE
	}
	else {
		/*
		if (composite.dataSelector.getQualitySelector().isSet()){

		}
		*/
		if (!composite.extracting){
			ctx.cartesianHi5.clear(); // don't append, overwrite...
		}
		else {
			mout.attention("extracting more...");
		}
		//composite.reset();
	}

	composite.extracting = true;

	// path << parent; // ?
	ODIMPath path(parent); // IDX24
	mout.debug("composite dst path: ", path );


	mout.debug3("update geodata ");

	const drain::Rectangle<double> & bboxDataNat = composite.getDataBBoxNat();
	const drain::Rectangle<double> & bboxDataOverlapNat = composite.getDataOverlapBBoxNat();

	mout.debug("Data BBOX minimal (union): ", bboxDataNat);
	mout.debug("Data BBOX overlap (intersection): ", bboxDataOverlapNat);

	composite.updateGeoData(); // TODO check if --plot cmds don't need

	drain::Rectangle<int> cropImage; // Default: empty ()
	if (!crop.empty()){

		if (crop == "INPUT"){
			prepareBBox(composite, bboxDataNat, cropImage);
			mout.advice<LOG_NOTICE>("Equivalent command: --cExtract ", drain::sprinter(channels), ':',
					drain::sprinter(bboxDataNat.tuple(), ","));
		}
		else if (crop == "OVERLAP"){
			prepareBBox(composite, bboxDataOverlapNat, cropImage);
			mout.advice<LOG_NOTICE>("Equivalent arguments: --cExtract ", drain::sprinter(channels), ':',
					drain::sprinter(bboxDataOverlapNat.tuple(), drain::Sprinter::plainLayout) );
		}
		else {
			std::vector<double> v;
			//drain::StringTools::split(crop, v, ':');
			drain::StringTools::split(crop, v, ',');

			drain::BBox cropBBox;
			cropBBox.assignSequence(v, false);

			mout.special("Cropping: ", cropBBox, cropBBox.isMetric() ? " [meters]": " [degrees]");
			prepareBBox(composite, cropBBox, cropImage);

			if (cropImage.empty()){
				mout.advice("crop argument should be INPUT, OVERLAP or a bounding box");
				mout.error("crop area empty, check argument: '", crop, "' ");
			}
		}

		mout.info("crop: ", cropImage, " (image coords)");
		mout.advice<LOG_NOTICE>("Matching size: --cSize ", cropImage.getWidth(), ',', -cropImage.getHeight());

	}


	Hi5Tree & dstGroup = ctx.cartesianHi5(path);

	// Block dstProduct - essential for updateTree etc?
	{

		DataSet<CartesianDst> dstProduct(dstGroup);

		// NEW 2024/05/17
		// mout.debug3("update geodata ");
		// composite.updateGeoData(); // TODO check if --plot cmds don't need

		// NEW 2020/06
		RootData<CartesianDst> dstRoot(ctx.cartesianHi5);

		// ODIM test(ODIMPathElem::ROOT);
		// mout.special("dstRoot.odim: ", dstRoot.odim);

		// CartesianODIM rootOdim; // needed? yes, because Extract uses (Accumulator &), not Composite.
		// CartesianODIM & rootOdim = dstRoot.odim; // TEST
		// dstRoot.odim.updateFromMap(composite.odim);
		// mout.attention("Upd...");
		drain::SmartMapTools::updateValues(dstRoot.odim, composite.odim);

		std::string & encoding = resources.baseCtx().targetEncoding;

		mout.attention<LOG_DEBUG>("Calling: composite.extract() channels=", drain::sprinter(channels), " encoding:", encoding);
		// mout.reject<LOG_NOTICE>("pre-extract EPSG:", composite.projGeo2Native.getDst().getEPSG());
		// mout.reject<LOG_NOTICE>("pre-extract proj:", composite.projGeo2Native.getDst().getProjDef());

		/// MAIN!composite.legend
		// composite.extract(dstProduct, channels, cropImage, encoding);
		composite.extract(dstProduct, channels, encoding, cropImage);
		encoding.clear();

		/*
		if (!composite.legend.empty()){
			mout.experimental("Copying (moving) legend");
			dstProduct.getWhat()["legend"] = drain::sprinter(composite.legend, "|", ",", ":").str();
			composite.legend.clear();
		}
		*/

		// mout.warn("extracted data: " , dstProduct ); // .getFirstData().data

		/// Final step: write metadata
		//  Note: dstRoot will write most of them upon destruction.
		//  So only "additional" data stored here
		drain::VariableMap & how = dstRoot.getHow();
		ProductBase::setRackVersion(how);

		composite.odim.version = ODIM::versionFlagger.str();
		// drain::StringTools::replace(ODIM::versionFlagger, ".", "_", composite.odim.version);

		// Non-standard extensions
		if (ODIM::versionFlagger.isSet(ODIM::RACK_EXTENSIONS)){

			how["tags"] = composite.nodeMap.toStr(':');

			// Non-standard
			drain::VariableMap & where = dstRoot.getWhere();
			where["BBOX"].setType(typeid(double));
			where["BBOX"] = composite.getBoundingBoxDeg().toVector();

			if (!composite.isLongLat()){
				// where["BBOX_native"].setType(typeid(double));
				// else
				where["BBOX_native"].setType(typeid(long int));
				where["BBOX_input"].setType(typeid(long int));
				where["BBOX_overlap"].setType(typeid(long int));
			}

			where["BBOX_native"] = composite.getBoundingBoxNat().tuple(); //toVector();
			//where["BBOX_input"].setType(typeid(double));
			//where["BBOX_input"] = bboxDataNat.tuple(); // in-place
			where["BBOX_input"]   = bboxDataNat.tuple(); // toVector();
			where["BBOX_overlap"] = bboxDataOverlapNat.tuple();

			mout.debug("composite INPUT bbox (nat): ", bboxDataNat.tuple());
			/*
			if (!composite.isLongLat()){
				drain::Rectangle<double> bn;
				composite.deg2m(bboxDataD.lowerLeft,  bn.lowerLeft);
				composite.deg2m(bboxDataD.upperRight, bn.upperRight);
				where["BBOX_input_native"].setType(typeid(int));
				where["BBOX_input_native"] = bn.tuple(); // toVector();
			}
			*/

			drain::Rectangle<int> bboxDataPix;
			composite.m2pix(bboxDataNat.lowerLeft, bboxDataPix.lowerLeft);
			composite.m2pix(bboxDataNat.upperRight, bboxDataPix.upperRight);
			// composite.deg2pix(bboxDataNat.lowerLeft, bboxDataPix.lowerLeft);
			// composite.deg2pix(bboxDataNat.upperRight, bboxDataPix.upperRight);
			where["BBOX_input_pix"].setType(typeid(short int));
			where["BBOX_input_pix"] = bboxDataPix.tuple(); // toVector();
			where["SIZE_input"] << bboxDataPix.getWidth() << -bboxDataPix.getHeight();

			composite.m2pix(bboxDataOverlapNat.lowerLeft,  bboxDataPix.lowerLeft);
			composite.m2pix(bboxDataOverlapNat.upperRight, bboxDataPix.upperRight);
			// composite.deg2pix(bboxDataOverlapNat.lowerLeft,  bboxDataPix.lowerLeft);
			// composite.deg2pix(bboxDataOverlapNat.upperRight, bboxDataPix.upperRight);
			//where["SIZE_overlap"].setType(typeid(short int));
			where["SIZE_overlap"] << bboxDataPix.getWidth() << -bboxDataPix.getHeight();

			if (!composite.isLongLat()){
				/*
				drain::Rectangle<double> bn;
				composite.deg2m(bboxDataOverlapNat.lowerLeft,  bn.lowerLeft);
				composite.deg2m(bboxDataOverlapNat.upperRight, bn.upperRight);
				where["BBOX_overlap_native"].setType(typeid(int));
				where["BBOX_overlap_native"] = bn.tuple();
				*/
			}
			else {
				drain::Rectangle<double> bb;
				composite.m2deg(bboxDataOverlapNat.lowerLeft,  bb.lowerLeft);
				composite.m2deg(bboxDataOverlapNat.upperRight, bb.upperRight);
				//where["BBOX_overlap_deg"].setType(typeid(int));
				where["BBOX_overlap_deg"] = bb.tuple();
			}


		}

		mout.accept<LOG_DEBUG>("natEPSG:", composite.projGeo2Native.getDst().getEPSG());

		//const short epsg = composite.projGeo2Native.getDst().getEPSG();
		//if (epsg > 0){
		if (composite.projGeo2Native.getDst().getEPSG() > 0){
			mout.hint<LOG_DEBUG>("/how:EPSG migrated to /where:EPSG");
		}

		// DataTools::updateInternalAttributes(ctx.cartesianHi5);
		// mout.warn("updateInternalAttributes 1:",  ctx.cartesianHi5.data.attributes);

		ctx.currentHi5 = & ctx.cartesianHi5;

		/// For successfull file io:
		ctx.statusFlags.unset(drain::Status::INPUT_ERROR); // resources.inputOk = false;

		// TODO: replace this with checking n = extractNEW(...)
		if (dstProduct.has(composite.odim.quantity)){

			Data<CartesianDst> & dstData = dstProduct.getData(composite.odim.quantity); // OR: by odim.quantity
			if (dstData.data.isEmpty()){
				mout.warn("empty product data: " , dstData );
				ctx.statusFlags.set(drain::Status::DATA_ERROR);
				ctx.unsetCurrentImages();
			}
			else {
				mout.debug("extracted quantity: " , dstProduct ); // .getFirstData().data
				// NEW
				drain::VariableMap & prodHow = dstProduct.getHow();
				//how["elangles"] = composite.metadataMap.get("how:elangles", {0,1,2});
				//if (composite.metadataMap.hasKey("how:angles"))
				prodHow["angles"].setType(typeid(double));
				prodHow["angles"] = composite.odim.angles; //composite.metadataMap["how:angles"];
				ctx.setCurrentImages(dstData.data);
				ctx.statusFlags.unset(drain::Status::DATA_ERROR);
			}
		}
		else {
			mout.experimental(dstProduct );
			if (dstProduct.hasQuality()){
				mout.revised("Assigning qualityData to current image(s)");
				ctx.setCurrentImages(dstProduct.getQualityData().data);
			}
			else {
				mout.warn("dstProduct does not have claimed quantity: " , composite.odim.quantity ); // .getFirstData().data
				ctx.unsetCurrentImages();
			}
			// ctx.statusFlags.set(drain::Status::DATA_ERROR);
			// ctx.unsetCurrentImages();
		}

	} // end dstProduct block

	//mout.warn("created" );

	DataTools::updateInternalAttributes(ctx.cartesianHi5);

	// NEW 2020/07
	ctx.select.clear(); // CONSIDER STORING SELECT composites own selector?

	drain::VariableMap & statusMap = ctx.getStatusMap();
	//statusMap.updateFromMap(dstRoot.odim);
	statusMap.updateFromMap(composite.nodeMap);
	statusMap.updateFromMap(composite.odim);
	// statusMap.updateFromMap(composite.metadataMap);
	// Spoils input.sh...
	// std::cout << ctx.svg << '\n';
	// mout.warn("updateInternalAttributes 2:",  ctx.cartesianHi5.data.attributes);

}


}  // namespace rack::



// Rack
