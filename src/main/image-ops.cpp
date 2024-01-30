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
#include <map>
#include <utility>

#include "drain/util/Cloner.h"
#include "drain/util/Log.h"
#include "drain/util/Range.h"
#include "drain/util/Registry.h"
#include "drain/util/SmartMap.h"
#include "drain/util/TreeOrdered.h"
#include "drain/util/ValueScaling.h"
#include "drain/image/Image.h"
#include "drain/image/ImageChannel.h"
#include "drain/image/ImageFrame.h"
#include "drain/image/ImageTray.h"
#include "drain/imageops/ImageOpBank.h"
#include "drain/imageops/CropOp.h"
#include "drain/prog/CommandBankUtils.h"

#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/ODIM.h"
#include "data/ODIMPath.h"
#include "data/QuantityMap.h"
#include "product/ProductBase.h"
#include "image-ops.h"
#include "resources.h"


namespace rack {


template <class OD>
void ImageOpExec::updateGeometryODIM(Hi5Tree & dstGroup, const std::string & quantity, drain::image::Geometry & geometry) const {

	drain::Logger mout(__FILE__, __FUNCTION__); // = resources.mout;

	// OD odim;
	typedef DstType<OD> dst_t;

	DataSet<dst_t> dstDataSet(dstGroup, quantity);
	mout.attention("changed code, selecting by quantity: ", quantity);
	//DataSet<dst_t> dstDataSet(dstGroup);
	// mout.attention("check: ");

	for (auto & entry: dstDataSet){
		// mout.note("check: ", entry.first, '=', entry.second);

		Data<dst_t> & dstData = entry.second;
		const drain::image::Geometry & g = dstData.data.getGeometry();

		if (geometry.getArea() == 0){
			geometry.setArea(g.getWidth(), g.getHeight());
		}
		else if (geometry.area != g.area) {
			mout.warn("nominal geom: ", geometry, ", found dataset with: ", g , " [", entry.first, '/', quantity, ']');
		}
		else if (geometry.channels != g.channels){
			mout.note("varying channel geometry: ", geometry.channels, ", dataset with: ", g.channels);
		}


		if (!g.isEmpty()){
			//dstDataSet.odim.setGeometry(g);
			dstData.odim.setGeometry(g.getWidth(), g.getHeight());
			 // Actually it's more complicated. a1gate, rscale etc. are dependent.
		}

		//d.odim.setArea(d.data.getWidth(), d.data.getHeight());
		//mout.note("modified odim geom " , d.odim );


	}
}


/// Designed for Rack

template <class OP>
class RackImageOpCmd : public drain::BeanCommand<OP>, public ImageOpExec  { //RackImageOpCmd {

public:

	/// Constructor that adapts an operator and its name.
	/**
	 *  \param op - image operator to be used
	 *  \param key - command name
	 */
	RackImageOpCmd() : drain::BeanCommand<OP>(){
	};

	/// Constructor that adapts an operator and its name.
	/**
	 *  \param op - image operator to be used througjh reference
	 */
	// ImageOpRacklet(drain::image::ImageOp & imageOp) : drain::BeanCommand<OP>(imageOp), key(imageOp.getName()) {};


	/// Copy constructor.
	//RackImageOpCmd(const RackImageOpCmd & rackLet) : drain::BeanCommand<OP>(rackLet), key(rackLet.key) {
	RackImageOpCmd(const RackImageOpCmd & rackLet) : drain::BeanCommand<OP>(rackLet) {
		this->bean.getParameters().copyStruct(rackLet.bean.getParameters(), rackLet.bean, this->bean);
	}

	virtual
	void exec() const {
		this->execOp(this->bean, this->template getContext<RackContext>());
	}

	/// Name of this operator, to be recognized
	// const std::string key;



};



/*
 *  TODO: support to select QIND as a
 */
void ImageOpExec::execOp(const ImageOp & bean, RackContext & ctx) const {

	typedef DstType<ODIM> dst_t;

	drain::Logger mout(ctx.log, __FUNCTION__, bean.getName()+"[ImageOpExec]"); // = resources.mout;

	DataSelector datasetSelector(ODIMPathElem::DATASET); // -> ends with DATASET
	// datasetSelector.setMaxCount(1); OLD
	datasetSelector.setMaxCount(0); //??  NEW 2023
	datasetSelector.consumeParameters(ctx.select);
	mout.debug("selector init: ", datasetSelector);

	// TODO: use path::trimHead()
	if (datasetSelector.getPathMatcher().front().isRoot()){
		datasetSelector.trimPathMatcher();
		mout.note("stripped pathMatcher root: ", datasetSelector.getPathMatcher());
	}
	/*
	while ((!datasetSelector.pathMatcher.empty()) && datasetSelector.pathMatcher.front().isRoot()){
		datasetSelector.pathMatcher.pop_front();
		mout.note("stripped pathMatcher root: /", datasetSelector.getPathMatcher(), " -> ", datasetSelector.getPathMatcher());
	}
	*/

	const bool USE_DEFAULT_DATASET_COUNT = (datasetSelector.getMaxCount() == 0);
	if (USE_DEFAULT_DATASET_COUNT){
		datasetSelector.setMaxCount(100); // Arbitrary? Consider --iResize 500,360 for a full volume, easily over 100 data arrays.
		// mout.attention("setting  datasetSelector.count = ", datasetSelector.count);
	}

	mout.info("final image data selector: ", datasetSelector);

	Hi5Tree & dst = ctx.getHi5(RackContext::CURRENT);

	ODIMPathList paths;
	datasetSelector.getPaths(dst, paths);

	if (paths.empty()){
		mout.warn("no paths found with selector: ", datasetSelector);
		return;
	}

	if ((USE_DEFAULT_DATASET_COUNT) && (paths.size() == datasetSelector.getMaxCount())){
		mout.attention("maximum number (", datasetSelector.getMaxCount(), ") of paths retrieved using default limit, consider explicit --select count=<n>,..." );
	}

	// DST properties
	// Check if dst quantity has been set explicitly
	// OLD std::string dstQuantitySyntax;
	std::string dstQuantitySyntax = ImageContext::outputQuantitySyntax;

	bool CHANGE_SCALING = false;
	// bool USER_TYPE    = false;

	EncodingODIM superOdim;

	if (!ctx.targetEncoding.empty()){ // does not check if an encoding change requested, preserving quantity?

		superOdim.scaling.scale = 0.0;
		superOdim.type = "";
		superOdim.link("what:quantity", dstQuantitySyntax); // consider "${what:quantity}_FILT"
		superOdim.addShortKeys();
		superOdim.updateValues(ctx.targetEncoding); // do not clear yet. But risk: quantity(Syntax) causes scaling effors below.
		//mout.debug("new quantity? - " , dstQuantity );
		CHANGE_SCALING = (superOdim.scaling.scale != 0.0);
		//USER_TYPE    = (!superOdim.type.empty());
	}

	if (dstQuantitySyntax.empty()){ // == explicitly cleared by user
		// NEW
		dstQuantitySyntax="${what:quantity}";
		//dstQuantitySyntax = ImageContext::outputQuantitySyntax;
	}

	mout.experimental("output quantity (syntax): ", dstQuantitySyntax);

	/*
	const bool USER_QUANTITY = true; // !!! !dstQuantitySyntax.empty();

	if (!USER_QUANTITY){

		mout.warn("No output quantity, storing to a separate image" );

		if (paths.size() > 1){
			paths.erase(++paths.begin());
			mout.warn("Found several paths - using first path only" );
		}
	}
	*/

	//mout.debug("Selector results: " );
	for (const ODIMPath & path: paths)
		mout.info("Selector results: ", path);

	mout.note("Use physical scale? ", ctx.imagePhysical);

	// For derived quantity:
	drain::StringMapper quantitySyntaxMapper(RackContext::variableMapper);
	quantitySyntaxMapper.parse(dstQuantitySyntax);

	drain::VariableMap & statusVariables = ctx.getStatusMap();
	/*
	statusVariables["command"] = bean.getName();
	if (bean.hasParameters())
		statusVariables["commandArgs"] = bean.getParameters().getValues();
	else
		statusVariables["commandArgs"] = "";
	*/

	// const QuantityMap & qmap = getQuantityMap();

	// Main loop (large!); visit each /dataset<n>
	// Results will be stored in the same /dataset<n>
	for (ODIMPath & path: paths){ // non-const (for stripping leading root)

		if (path.empty()){
			mout.warn("selector: ", datasetSelector);
			mout.error("empty path?");
			continue;
		}

		// if (path.begin()->isRoot()){
		if (path.front().isRoot()){
			path.pop_front();
			mout.debug("path started with root, trimmed it to: ", path);
			if (path.empty()){
				mout.error("empty path, skipping...");
				continue;
			}
		}

		const ODIMPathElem & datasetElem = path.front(); // *path.begin();

		/// This makes using QIND difficult...
		mout.info("using: ", datasetElem, " / [", datasetSelector.getQuantity(), "]");

		/*
		mout.fail("1st src coords: ", dst["dataset1"]["data1"]["data"].data.image.getCoordinatePolicy());

		// Data<dst_t> foo(dst[datasetElem]["data1"], datasetSelector.quantity);
		// mout.fail("1b  src coords: ", foo.data.getCoordinatePolicy());
		Data<dst_t> koe(dst[datasetElem]["data1"], "DBZH");

		mout.fail("2st  src coords: ", dst["dataset1"]["data1"]["data"].data.image.getCoordinatePolicy());

		DataTools::updateInternalAttributes(dst["dataset1"]["data1"]);

		mout.fail("2ast src coords: ", dst["dataset1"]["data1"]["data"].data.image.getCoordinatePolicy());

		{
			mout.fail("type", drain::TypeName<Data<dst_t> >::get());
			Data<dst_t> koe(dst[datasetElem]["data1"], "DBZH");
		}
		mout.fail("2bst src coords: ", dst["dataset1"]["data1"]["data"].data.image.getCoordinatePolicy());
		*/

		DataSet<dst_t> dstDataSet(dst[datasetElem], datasetSelector.getQuantity());

		// mout.fail("3rd src coords: ", dst["dataset1"]["data1"]["data"].data.image.getCoordinatePolicy());

		// mout.unimplemented<LOG_ERR>("Stop here");

		const size_t QUANTITY_COUNT = dstDataSet.size();

		if (bean.srcAlpha() && (ctx.qualityGroups == 0)){
			ctx.qualityGroups = (ODIMPathElem::DATASET | ODIMPathElem::DATA);
			mout.special("alpha channel required, modified: ", ctx.qualityGroups, ", see --imageQuality");
			// mout.unimplemented("Still missing: ODIMPathElem::QUALITY ?"); // FIX!
		}

		mout.attention("ctx.qualityGroups~", ctx.qualityGroups, ", DATASET~", ODIMPathElem::DATASET, ", dstDataSet.hasQuality: ", dstDataSet.hasQuality());

		bool DATASET_QUALITY = (ctx.qualityGroups & ODIMPathElem::DATASET) && dstDataSet.hasQuality(); //
		bool SPECIFIC_QUALITY_FOUND    = false;
		bool SPECIFIC_QUALITY_MISSING  = false;

		mout.debug2("path: ", path, " contains ", QUANTITY_COUNT, " quantities, and... ", (DATASET_QUALITY ? " has":" has no"), " dataset quality (ok)");

		if (QUANTITY_COUNT == 0){
			mout.warn("no quantities with quantity regExp ", datasetSelector.getQuantity(), " to process, skipping");
			if (DATASET_QUALITY)
				mout.warn("yet dataset-level quality exists... '", dstDataSet.getQuality(), "'");
			return;
		}

		if ((QUANTITY_COUNT > 1) && quantitySyntaxMapper.isLiteral()){
			mout.warn("several quantities, but single output quantity: ", quantitySyntaxMapper);
		}


		/// 1st loop: Add data, not quality yet (only check it)
		drain::image::ImageTray<const Channel> srcTray;
		drain::image::ImageTray<Channel> dstTray;

		//typedef std::list<std::string>  qlist_t;
		typedef std::set<std::string>  qlist_t;
		qlist_t quantityList;
		qlist_t quantityListNew;

		/*
		for (DataSet<dst_t >::iterator dit = dstDataSet.begin(); dit != dstDataSet.end(); ++dit){
			quantityList.insert(dit->second.odim.quantity);
		}
		*/

		for (const auto & entry: dstDataSet){
			quantityList.insert(entry.first); // == entry.second.odim.quantity
		}

		mout.info("current src quantities: ", drain::sprinter(quantityList));


		for (const std::string & quantity: quantityList){

			// Yes, of dst type, but used as src
			Data<dst_t> & srcData = dstDataSet.getData(quantity);

			mout.fail("src coords: ", srcData.data.getCoordinatePolicy());


			/// SOURCE: Add src data (always from h5 struct)
			srcData.data.setScaling(srcData.odim.scaling);
			drain::ValueScaling & scaling = srcData.data.getScaling();
			scaling.setScaling(srcData.odim.scaling);
			/// TODO: clarify those...
			mout.attention("Src SCALING: ", srcData.odim.scaling);

			if (ctx.imagePhysical){ // user wants to give thresholds etc params in phys units
				if (!scaling.isPhysical()){
					drain::Range<double> range;
					const drain::image::Encoding & encoding = srcData.data.getConf();
					//srcData.data.
					range.min = encoding.requestPhysicalMin(0.0);
					range.max = encoding.requestPhysicalMax(100.0);
					mout.special("guessing physical range: ", range);
					scaling.setPhysicalRange(range);
					//srcData.data.setPhysicalScale(srcData.odim.getMin(), srcData.odim.getMax());
				}
				mout.info("src scaling: ", srcData.data.getScaling());
			}
			else {

				scaling.setScaling(1.0, 0.0); // WARNING: in future, direct linking with ODIM offset and gain will cause unexpected changes?
				//mout.warn("src scaling: " , srcData.data.getScaling() );
			}

			mout.debug("src: ", srcData.data, ' ', EncodingODIM(srcData.odim));

			srcTray.appendImage(srcData.data);

			// ctx.qualitySelector.pathMatcher.
			//if (ctx.qualityGroups.test(ODIMPathElem::DATA) && srcData.hasQuality()){
			if ((ctx.qualityGroups & ODIMPathElem::DATA) && srcData.hasQuality()){
				mout.debug(path,  "/[", quantity, "] has quality data");
				SPECIFIC_QUALITY_FOUND = true;
			}
			else {
				mout.debug(path , "/[" , quantity ,  "] has no quality data (ok)" );
				SPECIFIC_QUALITY_MISSING = true;  // consider  && ctx.qualityGroupMatcher.test(ODIMPathElem::DATA)
			}

			// mout.special("srcTray: ", srcTray);

			/// Target:
			if (true) {

				ImageConf srcConf(srcData.data.getConf());
				srcConf.setChannelCount(1, (DATASET_QUALITY||SPECIFIC_QUALITY_FOUND) ? 1 : 0);
				mout.special("src conf: ", srcConf);
				mout.special("src DATASET_QUALITY: ", DATASET_QUALITY);

				statusVariables["what:quantity"] = srcData.odim.quantity;

				const std::string dstQuantity = quantitySyntaxMapper.toStr(statusVariables);

				if (quantityListNew.find(dstQuantity) != quantityListNew.end()){
					mout.fail("output ", quantitySyntaxMapper, "='", dstQuantity, "' exists already for input: ", quantity);
					break;
				}
				else {
					quantityListNew.insert(dstQuantity);
				}

				mout.info("Processing quantity: ", quantity, '(', srcData.odim.quantity, ") -> ", dstQuantity);

				// Type is explicitly set, and differs from
				const bool CHANGE_TYPE = (!superOdim.type.empty()) && (superOdim.type != srcData.odim.type);

				const size_t origSize = dstDataSet.size();
				Data<dst_t> & dstData = dstDataSet.getData(dstQuantity); // USER_QUANTITY ? dstDataSet.getData(dstQuantity) : dit->second;
				dstData.data.setName(dstQuantity);


				if (origSize == dstDataSet.size()){
					mout.note("Image processing result: ", datasetElem, "/data?/ [", dstQuantity, "]");
				}
				else {
					mout.note("Image processing result: ", datasetElem, "/data", dstDataSet.size(), " [", dstQuantity, "]");
				}
				// This replaces: bean.makeCompatible(srcConf, dstData.data);


				dstData.odim.updateFromMap(srcData.odim);
				// dstData.odim.quantity = dstQuantity; Must be done below (override)

				ImageConf dstConf;
				//dstConf.setCoordinatePolicy(srcConf.coordinatePolicy);

				if (!ctx.targetEncoding.empty()){
					// mout.special("trying: " , dstQuantity , '/' , ctx.targetEncoding );

					if (CHANGE_TYPE || CHANGE_SCALING){

						if (CHANGE_TYPE)
							mout.attention("data storage TYPE change requested by user");

						if (CHANGE_SCALING)
							mout.attention("data SCALING change requested by user");


						const QuantityMap & qmap = getQuantityMap();
						if (qmap.hasQuantity(dstQuantity)){
							mout.special("conf exists for: ", dstQuantity);
							qmap.setQuantityDefaults(dstData.odim, dstQuantity, ctx.targetEncoding); // if only quantity?
							// NOTE: dstData.odim.quantity may NOW contain variable syntax,  "${what:quantity}_X"
						}
						else {
							dstData.odim.updateValues(ctx.targetEncoding);
							// dstData.odim.quantity = dstQuantity; // replace syntax pattern
							if (CHANGE_TYPE && ! CHANGE_SCALING){
								mout.attention("no conf for: ", dstQuantity);
								mout.warn("type changed from: ", srcData.odim.type, " to ", dstData.odim.type, ", but no scaling set ");
							}

						}
					}

					// dstData.odim.quantity = dstQuantity;
					dstConf.setType(drain::Type::getTypeInfo(dstData.odim.type));
					dstConf.setScaling(dstData.odim);
					mout.special("user conf: ", dstConf);
				}

				dstData.odim.quantity = dstQuantity;


				mout.debug2("dst dataset: " , dstDataSet );

				bean.getDstConf(srcConf, dstConf);

				if (!dstConf.typeIsSet()){
					dstConf.setType(srcConf.getType());
					mout.obsolete(" setting src type? " , dstConf.getEncoding() );
				}

				//dstData.odim.type = drain::Type::getTypeChar(dstConf.getType());
				mout.info("initial dstConf: ", dstConf);
				// Problems: if quantity remains same for dst, src image is modified here!

				//dstData.data.setType(dstConf.getType()); // NEW     2023/04/21
				//dstData.data.setType(dstConf.getType()); // REMOVED 2023/05/10 is in conf!
				// REALLY NEW
				bean.makeCompatible(dstConf, dstData.data);
				mout.special("dst (after makeCompatible):", dstData.data);

				// NEWISH dstData.data.setConf(dstConf);
				// TODO: alpha channel should be filled?
				if (dstData.data.getAlphaChannelCount() > 0){
					dstTray.setAlpha(dstData.data.getAlphaChannel());
				}

				// bean.makeCompatible(srcConf, dstData.data);

				// Use channel, because tray will use...
				dstData.data.getChannel(0).setScaling(dstData.odim.scaling);
				dstData.data.setPhysicalRange(dstData.odim.getMin(), dstData.odim.getMax());

				/*
				if (dstData.data.hasAlphaChannel()){
					dstData.data.getAlphaChannel().setPhysicalRange(0.0, 1.0, true);
				}
				*/

				mout.debug("dst:    ", dstData.data, " <- ", EncodingODIM(dstData.odim));
				mout.debug("dst[0]: ", dstData.data.getChannel(0));

				// dstTray.setChannels(dstData.data); // sets all the channels
				dstTray.appendImage(dstData.data);
				//dstTray.appendImage(dstData.data);
				ctx.setCurrentImages(dstData.data); // yes, last assignment applies

				//dstData.updateTree2(); // because reference, hence no destructor.
			}



		}

		mout.debug("resulting (new) quantities: ", drain::sprinter(quantityListNew));


		mout.debug2("Add src quality, if found");
		/// dstData was fully added already above (all the channels)

		//  Case 1: at least some specific quality is used (and dataset-level )
		if ((DATASET_QUALITY && SPECIFIC_QUALITY_FOUND) || !SPECIFIC_QUALITY_MISSING) {

			mout.debug2("at least some specific quality is used (and dataset-level)" );

			//if (DATASET_QUALITY)
			//	mout.note("detected dataset-level quality data: " , path );

			// Loop again (add specific)
			for (const qlist_t::value_type & srcQuantity: quantityList){

				mout.debug("considering quantity [", srcQuantity, "]");

				Data<dst_t> & srcData = dstDataSet.getData(srcQuantity);

				statusVariables["what:quantity"] = srcData.odim.quantity;

				const std::string dstQuantity = quantitySyntaxMapper.toStr(statusVariables);

				mout.info("dst quantity: ", dstQuantity);

				if ((dstQuantity != srcQuantity) && dstDataSet.has(dstQuantity)){
					mout.warn("New dst quantity [", dstQuantity, "] exists already in src");
				}

				//Data<dst_t> & dstData = NEW_QUANTITY ? dstDataSet.getData(dstQuantity) : dit->second;

				if (srcData.hasQuality()){
					mout.info("using quantity[" , srcQuantity , "] specific quality data" );

					PlainData<dst_t> & srcQuality = srcData.getQualityData();
					//mout.note("name: " , srcQuality.data.getName() );

					srcQuality.data.getScaling().setPhysicalRange(0.0, 1.0); // not self-evident
					//srcQuality.data.setScaling(srcQuality.odim.scaling.scale, srcQuality.odim.scaling.offset);
					srcQuality.data.setScaling(srcQuality.odim.scaling);
					srcTray.alpha.set(srcQuality.data);

					/// dstData was fully added above (all the channels)
					/*
					 *
					if (USER_QUANTITY){
						// TODO: alpha check
						Data<dst_drainage spots-16b.png --iGray '0.6:0.3:0.1' t> & dstData = dstDataSet.getData(dstQuantity);
						//mout.special() = "policy changed: if srcQuality exists, add always local dstQuality";
						PlainData<dst_t> & dstQuality = dstData.getQualityData();
						dstQuality.copyEncoding(srcQuality);
						dstTray.alpha.set(dstQuality.data);
					}
					*/
				}
				else if (DATASET_QUALITY) { // if USER_QUANTITY, dstQuality will be local

					mout.info("using dataset specific quality data: " , path );

					PlainData<dst_t> & srcQuality = dstDataSet.getQualityData();

					//srcQuality.data.getScaling().setPhysicalRange(0.0, 1.0); // not self-evident
					srcQuality.data.setPhysicalRange(0.0, 1.0); // not self-evident
					srcQuality.data.setScaling(srcQuality.odim.scaling);
					srcTray.alpha.set(srcQuality.data);

					/// dstData was fully added above (all the channels)
					/*
					if (USER_QUANTITY){
						mout.special() = "policy changed: if srcQuality (here local) exists, add always local dstQuality";
						Data<dst_t> & dstData = dstDataSet.getData(dstQuantity);
						PlainData<dst_t> & dstQuality = dstData.getQualityData();
						dstQuality.copyEncoding(srcQuality);
						dstTray.alpha.set(dstQuality.data);
					}
					*/
				}

				//PlainData<dst_t> & srcQuality = srcData.getQualityData();
				//mout.note("name NOW: " , srcQuality.data.getName() );


				if (true){  // (USER_QUANTITY / CHANGE_QUANTITY){
					mout.note("discarding quality data (associated with only some data), exiting loop");
					break;
				}

			}
		}
		else if (DATASET_QUALITY){ // ...only.

			mout.info(path, " has dataset-level quality data (only)");

			PlainData<dst_t> & srcQuality = dstDataSet.getQualityData();

			srcQuality.data.setPhysicalRange(0.0, 1.0);
			//srcQuality.data.getScaling().setPhysicalRange(0.0, 1.0); // not self-evident
			srcQuality.data.setScaling(srcQuality.odim.scaling); // ?
			//srcQuality.data.setScaling(srcQuality.odim.scaling.scale, srcQuality.odim.scaling.offset);
			srcTray.alpha.set(srcQuality.data);

			/// dstData was fully added above (all the channels)
			/*
			if (USER_QUANTITY && quantitySyntaxMapper.isLiteral()){ // UNIQUE, no variables in dstQuantitySyntax
				mout.warn() = "policy changed: if srcQuality (here global) exists, still add local dstQuality";
				// PlainData<dst_t> & dstQuality = USER_QUANTITY ? dstDataSet.getData(dstQuantity).getQualityData() : srcQuality;
				PlainData<dst_t> & dstQuality = dstDataSet.getData(dstQuantitySyntax).getQualityData();
				dstQuality.copyEncoding(srcQuality); // NEW 2021
				dstTray.alpha.set(dstQuality.data);
			}
			else {
				mout.unimplemented("no logic (yet) for single input quantity, multiple output: " ,  quantitySyntaxMapper );
			}
			*/
		}
		else {
			if (SPECIFIC_QUALITY_FOUND && SPECIFIC_QUALITY_MISSING)
				mout.note("discarding quality data (associated with only some data)");
			else
				mout.info("no associated quality data");
		}




		// MAIN
		mout.debug("Main");
		//drain::image::ImageTray<const Channel> srcTray(dstTray); // fix
		mout.debug("src tray :\n", srcTray);
		mout.debug("dst tray before:\n", dstTray);
		// bean.process(srcTray, dstTray); //, true);
		bean.traverseChannels(srcTray, dstTray);
		// bean.traverseChannels(srcTray, dstTray);
		mout.debug("dst tray after:\n", dstTray);

		//mout.unimplemented() = "what:gain and what:offset in HDF5 struct";
		/*
		for (const qlist_t::value_type & key: quantityListNew){
			Data<dst_t> & dstData = dstDataSet.getData(key);
			//dstData.setEncoding(dstData.data.getType(), values);
			dstData.odim.setScaling(dstData.data.getScaling().scale,  dstData.data.getScaling().offset);
			mout.unimplemented("? what:gain and what:offset in HDF5 struct? " , dstData.odim.scaling );
		}
		*/


		/*
		const ChannelGeometry & channels = dstTray.getGeometry().channels;
		if (channels.getImageChannelCount() > 0){
			ctx.setCurrentImages(dstTray.get());
		}
		else {
			// mout.warn("No resulting image channels" );
			if (channels.getAlphaChannelCount() > 0){
				mout.warn("Only alpha channels resulted, no conventional image channels" );
				ctx.setCurrentImages(dstTray.getAlpha());
			}
			else  {
				mout.warn("No image or alpha channels resulted" );
			}
		}
		*/

		drain::image::Geometry geometryOrig;
		drain::image::Geometry geometry(0,0);

		drain::Variable & object = dst[ODIMPathElem::WHAT].data.attributes["object"];

		// TODO: warn if a single data array resized - Rack supports no variable-sized data arrays (in ODIM metadata)

		if (object.toStr() == "COMP"){
			updateGeometryODIM<CartesianODIM>(dst(path), datasetSelector.getQuantity(), geometry); // WRONG - quantity regexp?
			// Non-standard (ODYSSEY) (ODIM suggests dataset1-level xsize, ysize)
			drain::VariableMap & where = dst[ODIMPathElem::WHAT].data.attributes;
			geometryOrig.setArea(where["xsize"], where["ysize"]);
			where["xsize"] = geometry.getWidth();
			where["ysize"] = geometry.getHeight();
			if (!geometryOrig.isEmpty()){
				where["xscale"] = static_cast<int>(where["xscale"]) * geometryOrig.getWidth()  / geometry.getWidth();
				where["yscale"] = static_cast<int>(where["yscale"]) * geometryOrig.getHeight() / geometry.getHeight();
			}
		}
		else {
			//Hi5Tree & dstWhere = dst(path)[ODIMPathElem::WHERE];
			drain::VariableMap & where = dst(path)[ODIMPathElem::WHERE].data.attributes;
			geometryOrig.setArea(where["nbins"], where["nrays"]);
			mout.debug2("where0: ", where);
			updateGeometryODIM<PolarODIM>(dst(path), datasetSelector.getQuantity(), geometry);
			mout.warn("geom: ", geometry);
			where["nbins"] = geometry.getWidth();
			where["nrays"] = geometry.getHeight();
			if (!geometryOrig.isEmpty()){
				where["rscale"] = static_cast<int>(where["rscale"])   * geometryOrig.getWidth() / geometry.getWidth();
				where["a1gate"] = static_cast<int>(where["a1gate"])   * geometry.getWidth()     / geometryOrig.getWidth();
				where["startaz"] = static_cast<int>(where["startaz"]) * geometry.getHeight()    / geometryOrig.getHeight();
				where["stopaz"] = static_cast<int>(where["stopaz"])   * geometry.getHeight()    / geometryOrig.getHeight();
			}
			mout.debug("where1: ", where);
		}

	}

	// Has served
	ctx.targetEncoding.clear();


}


/*

class MultiThresholdOp : public drain::image::ImageOp {

public:

	MultiThresholdOp() : drain::image::ImageOp(__FUNCTION__) {
		parameters.link("range",  range.tuple(),  "accepted range [min:max]");
		parameters.link("target", target.tuple(), "result values for rejected values [low:high]");
		parameters["range"].fillArray = true;
		parameters["target"].fillArray = true;

		range.set(0.0, 1.0);
		target.set(0.0, 1.0);

	};

	drain::Range<double> range;
	drain::Range<double> target;

	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

		drain::Logger mout(__FILE__, __FUNCTION__); //REPL this->name+"(ImageOp::)[const ChannelTray &, ChannelTray &]", __FUNCTION__);
		traverseChannelsSeparately(src, dst);
		// traverseChannelsEqually(src, dst);
		// traverseChannelsRepeated(src, dst);
		// traverseMultiChannel(src, dst);

	};


	/// Apply to single channel.
    virtual inline
    void traverseChannel(const Channel & src, Channel &dst) const {

    	drain::Logger mout(__FILE__, __FUNCTION__);

    	const drain::ValueScaling & srcScale = src.getScaling();
    	const drain::ValueScaling & dstScale = dst.getScaling();

    	Image::const_iterator  sit = src.begin();
		Image::iterator dit  = dst.begin();

		drain::Range<double> targetScaled;
		targetScaled.min = dstScale.inv(target.min);
		targetScaled.max = dstScale.inv(target.max);

		mout.warn("targetScaled: " , targetScaled );

		double s;
		while (true){

			if (sit == src.end())
				return;

			s = srcScale.fwd(*sit);
			if (s < range.min){
				*dit  = targetScaled.min;
			}
			else if (s > range.max){
				*dit  = targetScaled.max;
			}
			else {
				*dit = dstScale.inv(s);
			}


			++sit;
			++dit;

		}


    };

    /// Apply to single channel with alpha.
    virtual inline
    void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {

    	const drain::ValueScaling & srcScale      = src.getScaling();
    	const drain::ValueScaling & srcAlphaScale = srcAlpha.getScaling();
    	const drain::ValueScaling & dstScale      = dst.getScaling();
    	const drain::ValueScaling & dstAlphaScale = dstAlpha.getScaling();

    	Image::const_iterator  sit = src.begin();
    	Image::const_iterator sait = srcAlpha.begin();
    	Image::iterator dit  = dst.begin();
    	Image::iterator dait = dstAlpha.begin();

    	drain::Range<double> targetScaled;
    	targetScaled.min = dstScale.inv(target.min);
    	targetScaled.max = dstScale.inv(target.max);

    	const double alphaMin = dstAlphaScale.inv(0.0); // or 0?

    	double s;
    	while (true){

    		if (sit == src.end())
    			return;

    		s = srcScale.fwd(*sit);
    		if (s < range.min){
    			*dit  = targetScaled.min;
    			*dait = alphaMin;
    		}
    		else if (s > range.max){
    			*dit  = targetScaled.max;
    			*dait = alphaMin;
    		}
    		else {
    			*dit  = dstScale.inv(s);
    			*dait = dstAlphaScale.inv(srcAlphaScale.fwd(*sait));
    		}

    		++sit; ++sait;
    		++dit; ++dait;

    	}

    };



};
*/





/// For separate commands.
// ImageRackletModule::list_t ImageRackletModule::rackletList;
ImageOpModule::ImageOpModule(){ //:  CommandSection("imageOps"){

	drain::CommandBank::trimWords().insert("Functor");  // { //const std::string & section, const std::string & prefix){

	drain::Logger mout(__FILE__, __FUNCTION__);
	// NEW
	// drain::CommandBank & cmdBank = drain::getCommandBank();
	// RackImageOpInstaller installer(cmdBank);
	// drain::image::installImageOps(installer);
	drain::image::installImageOps(*this);

	//static MultiThresholdOp mthop;
	/*  Use radar data compatible --palette instead, it supports undetect and nodata
	static PaletteOp remapOp(getResources().palette);
	static ImageOpRacklet rmop(remapOp);
	registry.add(rmop, "Palette");  // Note --palette and --iPalette
	*/

}



} // namespace rack

// Rack
