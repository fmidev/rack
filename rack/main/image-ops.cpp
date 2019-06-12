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

#include <data/Data.h>
#include <data/DataSelector.h>
#include <data/ODIM.h>
#include <data/ODIMPath.h>
#include <data/QuantityMap.h>
#include <image/Image.h>
#include <image/ImageChannel.h>
#include <image/ImageFrame.h>
#include <image/ImageScaling.h>
#include <image/ImageTray.h>
#include <imageops/ImageOpBank.h>
#include <main/image-ops.h>
#include <main/resources.h>
//#include <radar/Analysis.h>
#include <stddef.h>
#include <util/Cloner.h>
#include <util/Log.h>
#include <util/Range.h>
#include <util/Registry.h>
#include <util/SmartMap.h>
#include <util/Tree.h>
#include <map>
#include <utility>

namespace rack {

//CommandEntry<CmdPhysical> cmdPhysical("iPhysical");

std::string ImageOpRacklet::outputQuantity("{what:quantity}_{command}");

bool ImageOpRacklet::physical(true);

void ImageOpRacklet::exec() const {

	typedef DstType<ODIM> dst_t;

	drain::Logger mout(this->getName()+"[ImageOpRacklet]", __FUNCTION__); // = resources.mout;

	RackResources & resources = getResources();

	/*  Traverse all the datasets
	 *  - construct DataSet<Dst>(
	 *  - if (dataSetQuantity, add it for each,
	 */

	// What about iSelector?
	DataSelector imageSelector;
	imageSelector.setParameters(resources.select);
	resources.select.clear();
	imageSelector.convertRegExpToRanges();
	imageSelector.count = 1;
	mout.debug() << "selector: " << imageSelector << mout.endl;

	std::string dstQuantity;
	if (!resources.targetEncoding.empty()){ // does not check if an encoding change requested, preserving quantity?
		EncodingODIM odim;
		odim.reference("quantity", dstQuantity);
		odim.addShortKeys();
		odim.updateValues(resources.targetEncoding); // do not clear yet
		mout.debug() << "new quantity? - " << dstQuantity << mout.endl;
	}
	bool NEW_QUANTITY = !dstQuantity.empty();

	// skip quantity fow later traversal, accept now all the datasetN's ?
	// ORIG quantity => in-place
	// TODO: if same quantity, use temp?
	const std::string quantity(imageSelector.quantity);

	ODIMPathList paths;
	//imageSelector.getPathsNEW(*resources.currentHi5, paths, true);
	imageSelector.getPathsNEW(*resources.currentHi5, paths, ODIMPathElem::DATASET);

	if (paths.empty()){
		mout.warn() << "no paths found with selector: " << imageSelector << mout.endl;
		return;
	}
	else if (mout.isDebug(1)) {
		mout.warn() << "Selector results: " << mout.endl;
		for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); ++it)
			mout.warn() << *it << mout.endl;
	}

	// Main loop: visit each /dataset<n>
	for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); ++it){

		// Results will be stored in the same datasetN.
		DataSet<dst_t > dstDataSet((*resources.currentHi5)(*it), quantity);

		const size_t QUANTITY_COUNT = dstDataSet.size();

		drain::image::ImageTray<const Channel> srcTray;
		drain::image::ImageTray<Image> dstTray;

		bool DATASET_QUALITY = dstDataSet.hasQuality(); //
		bool SPECIFIC_QUALITY_FOUND    = false;
		bool SPECIFIC_QUALITY_MISSING  = false;

		// mout.warn()
		mout.debug(1) << "path: " << *it << (DATASET_QUALITY ? " has":" has no") <<  " dataset quality (ok)" << mout.endl;


		/// 1st loop: Add data, not quality yet (only check it)
		for (DataSet<dst_t >::iterator dit = dstDataSet.begin(); dit != dstDataSet.end(); ++dit){

			// Yes, src but of dst-type
			Data<dst_t> & srcData = dit->second;

			Data<dst_t> & dstData = NEW_QUANTITY ? dstDataSet.getData(dstQuantity) : dit->second;

			/// Apply physical values as thresholds etc
			if (ImageOpRacklet::physical)
				srcData.data.setScaling(srcData.odim.gain, srcData.odim.offset);
			else
				srcData.data.setScaling(1.0, 0.0); // WARNING: in future, direct linking with ODIM offset and gain will cause unexpected changes?

			if (NEW_QUANTITY){
				mout.debug() << "new quantity:" << dstQuantity << mout.endl;
				dstData.odim.update(srcData.odim);
				dstData.odim.gain = 0.0;
				//dstData.odim.quantity = dstQuantity;
				// mout.note() << "dstData.odim: " << dstData.odim << mout.endl;
				dstData.odim.addShortKeys();
				dstData.odim.setValues(resources.targetEncoding);

				QuantityMap & qmap = getQuantityMap();
				if (!qmap.setQuantityDefaults(dstData.odim)){
					mout.note() << "no predefined scaling for [" << dstQuantity << "]"<< mout.endl;
				}

				dstData.odim.setValues(resources.targetEncoding);
				resources.targetEncoding.clear();

				// Important: guess physical scaling, esp.
				dstData.data.setType(dstData.odim.type);
				//dstData.setPhysicalRange();
				dstData.data.getScaling().setPhysicalRange(dstData.odim.getMin(), dstData.odim.getMax());
				dstData.data.setScaling(dstData.odim.gain, dstData.odim.offset);
				//dstData.se
				// dstData.data.setPhysicalScale(dstData.odim.getMin(), dstData.odim.getMax());
				dstData.data.setName(dstQuantity);
				mout.debug(1) << dstData << mout.endl;
				//mout.note() << "dstData.odim: " << dstData.odim << mout.endl;

			}

			mout.note() << "using src: [" << dit->first << "] (dst: [" << dstQuantity << "])" << mout.endl;

			srcTray.appendImage(srcData.data);
			dstTray.appendImage(dstData.data);

			if (srcData.hasQuality()){
				mout.debug() << *it << "/[" << dit->first <<  "] has quality data" << mout.endl;
				SPECIFIC_QUALITY_FOUND = true;
			}
			else {
				mout.debug() <<  *it << "/[" << dit->first <<  "] has no quality data (ok)" << mout.endl;
				SPECIFIC_QUALITY_MISSING = true;
			}

			if (NEW_QUANTITY){
				if (QUANTITY_COUNT > 1){
					mout.warn() << "output quantity[" << dstQuantity << "] defined, but several input quantities defined: "<< dstDataSet << mout.endl;
					mout.warn() << "processing [" << srcData.odim.quantity << "] only " << mout.endl;
				}
				break;
			}

		}

		/// Add quality, if found.
		//  Case 1: at least some specific quality is used (and dataset-level )
		if ((DATASET_QUALITY && SPECIFIC_QUALITY_FOUND) || !SPECIFIC_QUALITY_MISSING) {


			//if (DATASET_QUALITY)
			//	mout.note() << "detected dataset-level quality data: " << *it << mout.endl;

			// Loop again (add specific)
			for (DataSet<dst_t >::iterator dit = dstDataSet.begin(); dit != dstDataSet.end(); ++dit){
				//const std::string & key = it->first;
				mout.debug() << "considering quantity [" << dit->first << ']' << mout.endl;
				Data<dst_t> & srcData = dit->second;
				Data<dst_t> & dstData = NEW_QUANTITY ? dstDataSet.getData(dstQuantity) : dit->second;

				if (srcData.hasQuality()){
					mout.note() << "using quantity[" << dit->first << "] specific quality data" << mout.endl;

					PlainData<dst_t> & srcQuality = srcData.getQualityData();
					//mout.note() << "name: " << srcQuality.data.getName() << mout.endl;

					srcQuality.data.getScaling().setPhysicalRange(0.0, 1.0); // not self-evident
					srcQuality.data.setScaling(srcQuality.odim.gain, srcQuality.odim.offset);
					srcTray.alpha.set(srcQuality.data);

					PlainData<dst_t> & dstQuality = NEW_QUANTITY ? dstData.getQualityData() : srcQuality;
					if (NEW_QUANTITY){
						//mout.warn()  << "Immediate src tray:\n" << srcTray << mout.endl;
						dstQuality.copyEncoding(srcQuality); // changes type, dangerous if src==dst
						//mout.warn()  << "preCurrent src tray:\n" << srcTray << mout.endl;
					}
					dstTray.alpha.set(dstQuality.data);

				}
				else if (DATASET_QUALITY) { // if NEW_QUANTITY, dstQuality will be local

					mout.note() << "using dataset specific quality data: " << *it << mout.endl;

					PlainData<dst_t> & srcQuality = dstDataSet.getQualityData();
					PlainData<dst_t> & dstQuality = NEW_QUANTITY ? dstData.getQualityData() : srcQuality;

					srcQuality.data.getScaling().setPhysicalRange(0.0, 1.0); // not self-evident
					srcQuality.data.setScaling(srcQuality.odim.gain, srcQuality.odim.offset);
					srcTray.alpha.set(srcQuality.data);

					dstQuality.copyEncoding(srcQuality);
					dstTray.alpha.set(dstQuality.data);
				}

				//PlainData<dst_t> & srcQuality = srcData.getQualityData();
				//mout.note() << "name NOW: " << srcQuality.data.getName() << mout.endl;


				if (NEW_QUANTITY){
					//mout.note() << "discarding quality data (associated with only some data)"  << mout.endl;
					break;
				}

			}
		}
		else if (DATASET_QUALITY){ // ...only.

			mout.note() << *it << " has dataset-level quality data (only)"  << mout.endl;

			PlainData<dst_t> & srcQuality = dstDataSet.getQualityData();

			srcQuality.data.getScaling().setPhysicalRange(0.0, 1.0); // not self-evident
			srcQuality.data.setScaling(srcQuality.odim.gain, srcQuality.odim.offset);
			srcTray.alpha.set(srcQuality.data);

			PlainData<dst_t> & dstQuality = NEW_QUANTITY ? dstDataSet.getData(dstQuantity).getQualityData() : srcQuality;
			dstTray.alpha.set(dstQuality.data);
		}
		else {
			if (SPECIFIC_QUALITY_FOUND && SPECIFIC_QUALITY_MISSING)
				mout.note() << "discarding quality data (associated with only some data)"  << mout.endl;
			else
				mout.info() << "no associated quality data"  << mout.endl;
		}


		// MAIN

		//drain::image::ImageTray<const Channel> srcTray(dstTray); // fix
		mout.debug()  << "src tray:\n" << srcTray << mout.endl;
		mout.debug(1) << "dst tray before processing:\n" << dstTray << mout.endl;
		bean.process(srcTray, dstTray); //, true);
		// bean.traverseChannels(srcTray, dstTray);
		mout.debug() << "dst tray after processing:\n" << dstTray << mout.endl;

		drain::image::Geometry geometry(0,0);
		drain::Variable & object = (*resources.currentHi5)["what"].data.attributes["object"];
		if (object.toStr() == "COMP"){
			updateGeometryODIM<CartesianODIM>((*resources.currentHi5)(*it), quantity, geometry);
			// Non-standard (ODIM suggests dataset1-level xsize, ysize)
			drain::VariableMap & vmap = (*resources.currentHi5)["where"].data.attributes;
			vmap["xsize"] = geometry.getWidth();
			vmap["ysize"] = geometry.getHeight();
		}
		else {
			updateGeometryODIM<PolarODIM>((*resources.currentHi5)(*it), quantity, geometry);
		}

	}

	//return;
}

class MultiThresholdOp : public drain::image::ImageOp {

public:

	MultiThresholdOp() : drain::image::ImageOp(__FUNCTION__) {
		parameters.reference("range",  range.vect,  "accepted range [min:max]");
		parameters.reference("target", target.vect, "result values for rejected values [low:high]");
		parameters["range"].fillArray = true;
		parameters["target"].fillArray = true;

		range.min = 0.0;
		range.max = 1.0;
		target.min = 0.0;
		target.max = 1.0;
		//targetQuality.min = 0.0;
		//targetQuality.max = 1.0;
	};

	drain::Range<double> range;
	drain::Range<double> target;
	// drain::Range<double> targetQuality;

	// Note: only a default implementation.
	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

		drain::Logger mout(this->name+"(ImageOp::)[const ChannelTray &, ChannelTray &]", __FUNCTION__);

		//traverseChannelsEqually(src, dst);
		// traverseChannelsRepeated(src, dst);
		traverseChannelsSeparately(src, dst);
		// traverseMultiChannel(src, dst);

	};


	/// Apply to single channel.
    virtual inline
    void traverseChannel(const Channel & src, Channel &dst) const {

    	drain::Logger mout(this->getName(), __FUNCTION__);

    	const drain::image::ImageScaling & srcScale = src.getScaling();
    	const drain::image::ImageScaling & dstScale = dst.getScaling();

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

    	const drain::image::ImageScaling & srcScale      = src.getScaling();
    	const drain::image::ImageScaling & srcAlphaScale = srcAlpha.getScaling();
    	const drain::image::ImageScaling & dstScale      = dst.getScaling();
    	const drain::image::ImageScaling & dstAlphaScale = dstAlpha.getScaling();



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


ImageRackletModule::list_t ImageRackletModule::rackletList;

ImageRackletModule::ImageRackletModule(const std::string & section, const std::string & prefix){

	drain::Logger mout(__FUNCTION__);

	CommandRegistry & registry = getRegistry();

	registry.setSection(section, prefix);

	// Put image utilities and other non-ops  here
	static CommandEntry<CmdPhysical> cmdPhysical; //("iPhysical");

	ImageOpBank::map_t & ops = getImageOpBank().getMap();

	for (ImageOpBank::map_t::iterator it=ops.begin(); it != ops.end(); ++it){

		const std::string & key = it->first;
		ImageOp & op = it->second.get();
		list_t::iterator lit = rackletList.insert(rackletList.end(), ImageOpRacklet(op, key));
		// mout.warn() << op.getName() << '\t' << adapter.getName() << mout.endl;
		// mout.note() << op.getName() << mout.endl;
		// mout.warn() << op.getName() << '\t' << op.getParameters().getValues() << mout.endl;
		registry.add(*lit, key);

		//if (++n > 5) break;
	}


	static MultiThresholdOp mthop;
	static ImageOpRacklet rop(mthop);
	registry.add(rop, "MultiThreshold");

	// DEBUG
	/*
	for (list_t::iterator it=rackletList.begin(); it != rackletList.end(); ++it){
		mout.warn() << it->getName() << '\t' << it->getParameters().getValues() << mout.endl;
	}
	 */


}




} // namespace rack

// Rack
