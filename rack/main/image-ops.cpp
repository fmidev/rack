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
#include "drain/util/Tree.h"
#include "drain/util/ValueScaling.h"
#include "drain/image/Image.h"
#include "drain/image/ImageChannel.h"
#include "drain/image/ImageFrame.h"
#include "drain/image/ImageTray.h"
#include "drain/imageops/ImageOpBank.h"
#include "drain/prog/CommandBankUtils.h"

#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/ODIM.h"
#include "data/ODIMPath.h"
#include "data/QuantityMap.h"

#include "product/ProductBase.h"


#include "image-ops.h"
#include "resources.h"

#include "drain/imageops/CropOp.h"

namespace rack {


template <class OD>
void ImageOpExec::updateGeometryODIM(Hi5Tree & dstGroup, const std::string & quantity, drain::image::Geometry & geometry) const {

	drain::Logger mout(__FUNCTION__, __FILE__); // = resources.mout;

	// OD odim;
	typedef DstType<OD> dst_t;

	DataSet<dst_t> dstDataSet(dstGroup, quantity);


	for (typename DataSet<dst_t>::iterator dit = dstDataSet.begin(); dit != dstDataSet.end(); ++dit){

		Data<dst_t> & d = dit->second;
		const drain::image::Geometry & g = d.data.getGeometry();

		if (geometry.getArea() == 0){
			geometry.setArea(g.getWidth(), g.getHeight());
		}
		else if (geometry.area != g.area) {
			mout.warn() << "nominal geom: " << geometry << ", found dataset with: "<< g << mout.endl;
			// mout.warn() << "dataset group contains different geometries" << d.odim << mout.endl;
		}
		else if (geometry.channels != g.channels){
			mout.note() << "varying channel geometry: " << geometry.channels << ", dataset with: "<< g.channels << mout.endl;
		}

		d.odim.setGeometry(d.data.getWidth(), d.data.getHeight());
		//d.odim.setArea(d.data.getWidth(), d.data.getHeight());
		//mout.note() << "modified odim geom " << d.odim << mout.endl;


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
	datasetSelector.count = 1;
	datasetSelector.consumeParameters(ctx.select);
	mout.info() << "selector: " << datasetSelector << mout.endl;
	//const std::string quantityRegExpStr(datasetSelector.quantity); // note: might be regexp


	Hi5Tree & dst = ctx.getHi5(RackContext::CURRENT);

	ODIMPathList paths;
	datasetSelector.getPaths(dst, paths);

	if (paths.empty()){
		mout.warn() << "no paths found with selector: " << datasetSelector << mout.endl;
		return;
	}


	// DST properties
	// Check if dst quantity has been set explicitly
	std::string dstQuantitySyntax;
	bool USER_SCALING = false;
	if (!ctx.targetEncoding.empty()){ // does not check if an encoding change requested, preserving quantity?
		EncodingODIM superOdim;
		superOdim.scaling.scale = 0.0;
		superOdim.link("what:quantity", dstQuantitySyntax); // consider "${what:quantity}_FILT"
		superOdim.addShortKeys();
		superOdim.updateValues(ctx.targetEncoding); // do not clear yet
		//mout.debug() << "new quantity? - " << dstQuantity << mout.endl;
		USER_SCALING = (superOdim.scaling.scale != 0.0);
	}

	if (dstQuantitySyntax.empty())
		dstQuantitySyntax = ImageContext::outputQuantitySyntax;

	mout.experimental() << "output quantity (syntax): " << dstQuantitySyntax << mout.endl;


	const bool USER_QUANTITY = true; // !!! !dstQuantitySyntax.empty();

	if (!USER_QUANTITY){

		mout.warn() << "No output quantity, storing to a separate image" << mout;

		if (paths.size() > 1){
			paths.erase(++paths.begin());
			mout.warn() << "Found several paths - using first path only" << mout;
		}
	}

	//mout.debug() << "Selector results: " << mout.endl;
	for (const ODIMPath & path: paths)
		mout.info() << "Selector results: " << '\t' << path << mout.endl;

	mout.note() << "Use physical scale? " << ctx.imagePhysical << mout;

	// For derived quantity:
	drain::StringMapper quantitySyntaxMapper(RackContext::variableMapper);
	quantitySyntaxMapper.parse(dstQuantitySyntax);

	drain::VariableMap & statusVariables = ctx.getStatusMap();
	statusVariables["command"] = bean.getName();

	// const QuantityMap & qmap = getQuantityMap();

	// Main loop (large!); visit each /dataset<n>
	// Results will be stored in the same /dataset<n>
	for (ODIMPath & path: paths){ // non-const (for stripping leading root)

		if (path.empty()){
			mout.warn()  << "selector: " << datasetSelector << mout.endl;
			mout.error() << "empty path?" << mout.endl;
			continue;
		}

		if (path.begin()->isRoot()){
			path.pop_front();
			mout.debug() << "path started with root, trimmed it to: " << path << mout.endl;
			if (path.empty()){
				mout.error() << "empty path, skipping..." << mout.endl;
				continue;
			}
		}

		const ODIMPathElem & datasetElem = *path.begin();

		/// This makes using QIND difficult...
		mout.info() << "using: " << datasetElem << " / [" << datasetSelector.quantity << "]"<< mout.endl;

		DataSet<dst_t> dstDataSet(dst[datasetElem], datasetSelector.quantity);

		const size_t QUANTITY_COUNT = dstDataSet.size();

		if (bean.srcAlpha() && (ctx.qualityGroups == 0)){
			ctx.qualityGroups = (ODIMPathElem::DATASET | ODIMPathElem::DATA);
			mout.special() << "alpha channel required, modified: " << ctx.qualityGroups << ", see --imageQuality" << mout;
		}

		bool DATASET_QUALITY = (ctx.qualityGroups & ODIMPathElem::DATASET) && dstDataSet.hasQuality(); //
		bool SPECIFIC_QUALITY_FOUND    = false;
		bool SPECIFIC_QUALITY_MISSING  = false;

		mout.debug2() << "path: " << path << " contains " << QUANTITY_COUNT << " quantities, and... " << (DATASET_QUALITY ? " has":" has no") <<  " dataset quality (ok)" << mout.endl;

		if (QUANTITY_COUNT == 0){
			mout.warn() << "no quantities with regExp " << datasetSelector.quantity  << " to process, skipping" << mout.endl;
			if (DATASET_QUALITY)
				mout.warn() << "yet dataset-level quality exists... '" << dstDataSet.getQuality() << "'" << mout.endl;
			return;
		}

		if ((QUANTITY_COUNT > 1) && quantitySyntaxMapper.isLiteral()){
			mout.warn() << "several quantities, but single output quantity: " << quantitySyntaxMapper << mout.endl;
		}


		/// 1st loop: Add data, not quality yet (only check it)
		drain::image::ImageTray<const Channel> srcTray;
		drain::image::ImageTray<Channel> dstTray;

		//typedef std::list<std::string>  qlist_t;
		typedef std::set<std::string>  qlist_t;
		qlist_t quantityList;
		qlist_t quantityListNew;

		for (DataSet<dst_t >::iterator dit = dstDataSet.begin(); dit != dstDataSet.end(); ++dit){
			quantityList.insert(dit->second.odim.quantity);
		}

		mout.debug() << "current quantities: " << drain::sprinter(quantityList) << mout;


		for (const std::string & quantity: quantityList){

			// Yes, of dst type, but used as src
			Data<dst_t> & srcData = dstDataSet.getData(quantity);

			// size_t dstAlphaChannels = 0;
			// mout.warn() << "srcData: " << srcData << mout;

			/// SOURCE: Add src data (always from h5 struct)
			srcData.data.setScaling(srcData.odim.scaling);
			drain::ValueScaling & scaling = srcData.data.getScaling();
			scaling.setScaling(srcData.odim.scaling);
			/// TODO: clarify those...

			if (ctx.imagePhysical){ // user wants to give thresholds etc params in phys units
				if (!scaling.isPhysical()){
					drain::Range<double> range;
					const drain::image::Encoding & encoding = srcData.data.getConf();
					//srcData.data.
					range.min = encoding.requestPhysicalMin(0.0);
					range.max = encoding.requestPhysicalMax(100.0);
					mout.special() << "guessing physical range: " << range << mout;
					scaling.setPhysicalRange(range);
					//srcData.data.setPhysicalScale(srcData.odim.getMin(), srcData.odim.getMax());
				}
				mout.info() << "src scaling: " << srcData.data.getScaling() << mout;
			}
			else {

				scaling.setScaling(1.0, 0.0); // WARNING: in future, direct linking with ODIM offset and gain will cause unexpected changes?
				//mout.warn() << "src scaling: " << srcData.data.getScaling() << mout;
			}

			mout.debug() << "src :" << srcData.data << ' ' << EncodingODIM(srcData.odim) << mout.endl;

			srcTray.appendImage(srcData.data);

			// ctx.qualitySelector.pathMatcher.
			//if (ctx.qualityGroups.test(ODIMPathElem::DATA) && srcData.hasQuality()){
			if ((ctx.qualityGroups & ODIMPathElem::DATA) && srcData.hasQuality()){
				mout.debug() << path << "/[" << quantity <<  "] has quality data" << mout.endl;
				SPECIFIC_QUALITY_FOUND = true;
			}
			else {
				mout.debug() <<  path << "/[" << quantity <<  "] has no quality data (ok)" << mout.endl;
				SPECIFIC_QUALITY_MISSING = true;  // consider  && ctx.qualityGroupMatcher.test(ODIMPathElem::DATA)
			}

			mout.special() << "srcTray: " << srcTray << mout;

			/// Target:
			if (true) {

				ImageConf srcConf(srcData.data.getConf());
				srcConf.setChannelCount(1, (DATASET_QUALITY||SPECIFIC_QUALITY_FOUND) ? 1 : 0);
				mout.special() << "src conf: " << srcConf << mout;

				statusVariables["what:quantity"] = srcData.odim.quantity;

				const std::string dstQuantity = quantitySyntaxMapper.toStr(statusVariables);

				if (quantityListNew.find(dstQuantity) != quantityListNew.end()){
					mout.fail() << "output " << quantitySyntaxMapper << "='" << dstQuantity << "' exists already for input: " << quantity << mout.endl;
					break;
				}
				else {
					quantityListNew.insert(dstQuantity);
				}

				mout.note() << quantity << '/' << srcData.odim.quantity <<  " -> " << dstQuantity << mout.endl;

				Data<dst_t> & dstData = dstDataSet.getData(dstQuantity); // USER_QUANTITY ? dstDataSet.getData(dstQuantity) : dit->second;
				dstData.data.setName(dstQuantity);

				// This replaces: bean.makeCompatible(srcConf, dstData.data);

				dstData.odim.updateFromMap(srcData.odim);
				dstData.odim.quantity = dstQuantity;

				ImageConf dstConf;
				//dstConf.setCoordinatePolicy(srcConf.coordinatePolicy);

				if (!ctx.targetEncoding.empty()){
					// mout.special() << "trying: " << dstQuantity << '/' << ctx.targetEncoding << mout;
					const QuantityMap & qmap = getQuantityMap();
					if (qmap.hasQuantity(dstQuantity)){
						mout.special() << "conf exists for: " << dstQuantity << mout;
						qmap.setQuantityDefaults(dstData.odim, dstQuantity, ctx.targetEncoding);
					}
					else {
						dstData.odim.updateValues(ctx.targetEncoding);
						dstData.odim.quantity = dstQuantity; // replace syntax pattern
						if ((dstData.odim.type != srcData.odim.type) && ! USER_SCALING){
							mout.warn() << "type changed from: " << srcData.odim.type << " to " << dstData.odim.type << mout;
							mout.warn() << "but no scaling set "  << mout;
						}
					}
					// ProductBase::completeEncoding(dstData.odim, ctx.targetEncoding);
					dstConf.setType(drain::Type::getTypeInfo(dstData.odim.type));
					dstConf.setScaling(dstData.odim);
					mout.special() << "user conf: " << dstConf << mout;
				}

				mout.debug2() << "dst dataset: " << dstDataSet << mout;

				bean.getDstConf(srcConf, dstConf);

				if (!dstConf.typeIsSet()){
					dstConf.setType(srcConf.getType());
					mout.obsolete() << " setting src type? " << dstConf.getEncoding() << mout;
				}

				//dstData.odim.type = drain::Type::getTypeChar(dstConf.getType());
				mout.info() << "initial dstConf: " << dstConf << mout;

				// REALLY NEW
				bean.makeCompatible(dstConf, dstData.data);
				mout.special() << "dst (after makeCompatible):" << dstData.data << mout;

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

				mout.warn() << "dst:    " << dstData.data << " <- " << EncodingODIM(dstData.odim) << mout.endl;
				mout.warn() << "dst[0]: " << dstData.data.getChannel(0) << mout.endl;

				// dstTray.setChannels(dstData.data); // sets all the channels
				dstTray.appendImage(dstData.data);
				//dstTray.appendImage(dstData.data);
				ctx.setCurrentImages(dstData.data); // yes, last assignment applies

				//dstData.updateTree2(); // because reference, hence no destructor.
			}



		}

		mout.debug() << "new quantities: " << drain::sprinter(quantityListNew) << mout;


		mout.debug2() << "Add src quality, if found" << mout.endl;
		/// dstData was fully added already above (all the channels)

		//  Case 1: at least some specific quality is used (and dataset-level )
		if ((DATASET_QUALITY && SPECIFIC_QUALITY_FOUND) || !SPECIFIC_QUALITY_MISSING) {

			mout.debug2() << "at least some specific quality is used (and dataset-level)" << mout.endl;

			//if (DATASET_QUALITY)
			//	mout.note() << "detected dataset-level quality data: " << path << mout.endl;

			// Loop again (add specific)
			for (const qlist_t::value_type & srcQuantity: quantityList){

				mout.debug() << "considering quantity [" << srcQuantity << ']' << mout.endl;

				Data<dst_t> & srcData = dstDataSet.getData(srcQuantity);

				statusVariables["what:quantity"] = srcData.odim.quantity;

				const std::string dstQuantity = quantitySyntaxMapper.toStr(statusVariables);

				mout.info() << "dst quantity: " << dstQuantity << mout.endl;

				if ((dstQuantity != srcQuantity) && dstDataSet.has(dstQuantity)){
					mout.warn() << "Dst quantity [" << dstQuantity << "] exists already... " << mout.endl;
				}

				//Data<dst_t> & dstData = NEW_QUANTITY ? dstDataSet.getData(dstQuantity) : dit->second;

				if (srcData.hasQuality()){
					mout.info() << "using quantity[" << srcQuantity << "] specific quality data" << mout.endl;

					PlainData<dst_t> & srcQuality = srcData.getQualityData();
					//mout.note() << "name: " << srcQuality.data.getName() << mout.endl;

					srcQuality.data.getScaling().setPhysicalRange(0.0, 1.0); // not self-evident
					//srcQuality.data.setScaling(srcQuality.odim.scaling.scale, srcQuality.odim.scaling.offset);
					srcQuality.data.setScaling(srcQuality.odim.scaling);
					srcTray.alpha.set(srcQuality.data);

					/// dstData was fully added above (all the channels)
					/*
					if (USER_QUANTITY){
						// TODO: alpha check
						Data<dst_t> & dstData = dstDataSet.getData(dstQuantity);
						//mout.special() = "policy changed: if srcQuality exists, add always local dstQuality";
						PlainData<dst_t> & dstQuality = dstData.getQualityData();
						dstQuality.copyEncoding(srcQuality);
						dstTray.alpha.set(dstQuality.data);
					}
					*/
				}
				else if (DATASET_QUALITY) { // if USER_QUANTITY, dstQuality will be local

					mout.info() << "using dataset specific quality data: " << path << mout.endl;

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
				//mout.note() << "name NOW: " << srcQuality.data.getName() << mout.endl;


				if (USER_QUANTITY){
					mout.note() << "discarding quality data (associated with only some data), exiting loop"  << mout.endl;
					break;
				}

			}
		}
		else if (DATASET_QUALITY){ // ...only.

			mout.info() << path << " has dataset-level quality data (only)"  << mout.endl;

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
				mout.unimplemented() << "no logic (yet) for single input quantity, multiple output: " <<  quantitySyntaxMapper << mout;
			}
			*/
		}
		else {
			if (SPECIFIC_QUALITY_FOUND && SPECIFIC_QUALITY_MISSING)
				mout.note() << "discarding quality data (associated with only some data)"  << mout.endl;
			else
				mout.info() << "no associated quality data"  << mout.endl;
		}




		// MAIN
		mout.debug() << "Main" << mout.endl;
		//drain::image::ImageTray<const Channel> srcTray(dstTray); // fix
		mout.debug() << "src tray :\n" << srcTray << mout.endl;
		mout.debug() << "dst tray before:\n" << dstTray << mout.endl;
		// bean.process(srcTray, dstTray); //, true);
		bean.traverseChannels(srcTray, dstTray);
		// bean.traverseChannels(srcTray, dstTray);
		mout.debug() << "dst tray after:\n" << dstTray << mout.endl;

		//mout.unimplemented() = "what:gain and what:offset in HDF5 struct";
		/*
		for (const qlist_t::value_type & key: quantityListNew){
			Data<dst_t> & dstData = dstDataSet.getData(key);
			//dstData.setEncoding(dstData.data.getType(), values);
			dstData.odim.setScaling(dstData.data.getScaling().scale,  dstData.data.getScaling().offset);
			mout.unimplemented() << "? what:gain and what:offset in HDF5 struct? " << dstData.odim.scaling << mout.endl;
		}
		*/


		/*
		const ChannelGeometry & channels = dstTray.getGeometry().channels;
		if (channels.getImageChannelCount() > 0){
			ctx.setCurrentImages(dstTray.get());
		}
		else {
			// mout.warn() << "No resulting image channels" << mout.endl;
			if (channels.getAlphaChannelCount() > 0){
				mout.warn() << "Only alpha channels resulted, no conventional image channels" << mout.endl;
				ctx.setCurrentImages(dstTray.getAlpha());
			}
			else  {
				mout.warn() << "No image or alpha channels resulted" << mout.endl;
			}
		}
		*/

		drain::image::Geometry geometry(0,0);
		// Consider later DataSet<BasicDst> basic(dst);
		// basic.getWhat()["object"]....
		// updateGeometryODIM<ODIM>(dst(path), quantity, geometry);

		drain::Variable & object = dst[ODIMPathElem::WHAT].data.attributes["object"];
		if (object.toStr() == "COMP"){
			updateGeometryODIM<CartesianODIM>(dst(path), datasetSelector.quantity, geometry);
			// Non-standard (ODYSSEY) (ODIM suggests dataset1-level xsize, ysize)
			drain::VariableMap & vmap = dst[ODIMPathElem::WHAT].data.attributes;
			vmap["xsize"] = geometry.getWidth();
			vmap["ysize"] = geometry.getHeight();
		}
		else {
			// mout.special() << "vmap0: " << path << mout;
			// drain::VariableMap & vmap = dst(path)[ODIMPathElem::WHERE].data.attributes;
			// mout.warn() << "vmap0: " << vmap << mout;
			updateGeometryODIM<PolarODIM>(dst(path)[ODIMPathElem::WHERE], datasetSelector.quantity, geometry);
			//mout.warn() << "vmap1: " << vmap << mout;
		}

	}

	// Has served
	ctx.targetEncoding.clear();

	//return;
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

		drain::Logger mout(__FUNCTION__, __FILE__); //REPL this->name+"(ImageOp::)[const ChannelTray &, ChannelTray &]", __FUNCTION__);
		traverseChannelsSeparately(src, dst);
		// traverseChannelsEqually(src, dst);
		// traverseChannelsRepeated(src, dst);
		// traverseMultiChannel(src, dst);

	};


	/// Apply to single channel.
    virtual inline
    void traverseChannel(const Channel & src, Channel &dst) const {

    	drain::Logger mout(__FUNCTION__, __FILE__);

    	const drain::ValueScaling & srcScale = src.getScaling();
    	const drain::ValueScaling & dstScale = dst.getScaling();

    	Image::const_iterator  sit = src.begin();
		Image::iterator dit  = dst.begin();

		drain::Range<double> targetScaled;
		targetScaled.min = dstScale.inv(target.min);
		targetScaled.max = dstScale.inv(target.max);

		mout.warn() << "targetScaled: " << targetScaled << mout.endl;

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

	drain::Logger mout(__FUNCTION__, __FILE__);
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
