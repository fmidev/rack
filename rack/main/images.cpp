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

#include <set>
#include <map>
#include <ostream>



#include <drain/util/Log.h>
#include <drain/util/RegExp.h>

#include <drain/image/Image.h>
#include <drain/image/TreeSVG.h>

#include <drain/imageops/ImageMod.h>
#include <drain/imageops/ImageOpBank.h>
#include <drain/imageops/FastAverageOp.h>

#include <drain/prog/CommandRegistry.h>


#include "rack.h"
#include "product/DataConversionOp.h"
#include "data/ODIM.h"
//#include "hi5/Hi5.h"

#include "commands.h"
#include "images.h"

// #include <pthread.h>

namespace rack {

CommandEntry<CmdImage> cmdImage("image");
CommandEntry<CmdPhysical> cmdPhysical("iPhysical");


void CmdImage::convertImage(const HI5TREE & src, const DataSelector & selector, const std::string & parameters, drain::image::Image &dst){

	drain::Logger mout("CmdImage", __FUNCTION__);

	std::string path = "";
	DataSelector::getPath(src, DataSelector("(data|quality)[0-9]+/?$", selector.quantity), path);
	if (path.empty()){
		mout.warn() << "path empty" << mout.endl;
		return;
	}

	const Data<PolarSrc> srcData(src(path));  // limit?

	DataConversionOp<PolarODIM> op;
	// mout.warn() << "srcOdim:" << srcData.odim << mout.endl;
	op.odim.importMap(srcData.odim);
	// mout.note() << "dstOdim:" << op.odim << mout.endl;

	// mout.note() << "params :" << parameters << mout.endl;
	ProductBase::handleEncodingRequest(op.odim, parameters);
	// mout.warn() << "dstOdim:" << op.odim << mout.endl;

	op.processImage(srcData, dst);

}








namespace {




// Adds alpha channel containing current data.
class CmdImageAlpha : public BasicCommand {
public: //re
	//std::string properties;
	//volatile
	// DataConversionOp copierHidden;

	CmdImageAlpha() : BasicCommand(__FUNCTION__, "Adds a transparency channel. Creates current image if nonexistent. See --target") {
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout; = resources.mout;

		RackResources & resources = getResources();

		DataSelector iSelector("/data$");
		iSelector.setParameters(resources.select);
		resources.select.clear();

		// Create a copy (getResources().colorImage or getResources().grayImage), and point *img to it.
		// Because getResources().currentImage is const, non-const *img is needed.
		drain::image::Image * img;
		if (resources.currentImage == &getResources().colorImage){
			mout.info() << " using colour image" << mout.endl;
			img = & resources.colorImage;
		}
		else {
			if (resources.currentImage != &resources.grayImage){
				mout.note() << " no current image, creating..." << mout.endl;
				CmdImage::convertImage(*resources.currentHi5, iSelector, "", resources.grayImage);  // TODO check
				resources.currentGrayImage = &resources.grayImage;
				resources.currentImage     = &resources.grayImage;
			}
			img = &resources.grayImage;
		}

		/// Add empty alpha channel
		img->setAlphaChannelCount(1);
		mout.debug() << "image:"  << *resources.currentImage << mout.endl;
		mout.debug() << "alpha:"  <<  resources.currentImage->getAlphaChannel() << mout.endl;

		//const drain::image::Image &d = VolumeOp::getData(*resources.currentHi5, 0); // options["quantity?"]); // typically, first "DBZ.*"
		//
		const drain::image::Image &alpha = DataSelector::getData(*resources.currentHi5, iSelector);
		PolarODIM odimAlpha(alpha);

		DataConversionOp<PolarODIM> copier; //(copierHidden);
		copier.odim.setValues(resources.targetEncoding, '=');
		resources.targetEncoding.clear();
		const std::string type(1, drain::Type::getTypeChar(img->getType()));
		if (copier.odim.type != type){
			mout.note() << " using the type of base image: " << type << mout.endl;
		}
		copier.odim.type = type; //drain::Type::getTypeChar(img->getType());

		mout.debug(2) << "odimAlpha:  "  <<  odimAlpha << mout.endl;
		mout.debug(2) << "odimOut: " << copier.odim << mout.endl;
		mout.debug(2) << "copier: " << copier << mout.endl;
		copier.traverseImageFrame(odimAlpha, alpha, copier.odim, img->getAlphaChannel());

		img->properties["what.gainAlpha"] = copier.odim.gain;
		img->properties["what.offsetAlpha"] = copier.odim.offset;

	}
};
// static CommandEntry<CmdImageAlpha> cmdImageAlpha("imageAlpha");
static CommandEntry<CmdImageAlpha> cmdImageAlpha("imageAlpha");



class CmdImageFlatten : public SimpleCommand<std::string> {
public: //re

	CmdImageFlatten() : SimpleCommand<>(__FUNCTION__, "Removes a alpha (transparency) channel. Adds a new background of given color.",
			"bgcolor", "0", "<gray>|<red>,<green>,<blue>") {
		// parameters.separators.clear();
		// parameters.reference("value", value, "0,0,0", "<gray>|<red>,<green>,<blue>");
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();

		if ((resources.currentImage != & resources.grayImage) && (resources.currentImage != & resources.colorImage)){
			mout.warn() << name << ": no image" << mout.endl;
			return;
		}

		drain::image::Image & img = (resources.currentImage == & resources.grayImage) ? resources.grayImage : resources.colorImage;

		if (img.getAlphaChannelCount() == 0) {
			mout.warn() << name << ": no alpha channel" << mout.endl;
			return;
		}

		drain::image::ImageFrame & imgAlpha = img.getAlphaChannel();

		const double alphaMax = imgAlpha.getMax<double>();
		if (alphaMax == 0) {
			mout.warn() << name << ": zero alphaMax" << mout.endl;
			return;
		}

		double alpha;

		Variable colorVector;
		colorVector.setType(typeid(size_t));
		colorVector = value;

		size_t address;
		for (size_t k=0; k<img.getImageChannelCount(); ++k){
			drain::image::ImageFrame & channel = img.getChannel(k);
			double replace = colorVector.get<double>(k % colorVector.getElementCount());
			for (size_t j=0; j<img.getHeight(); ++j){
				for (size_t i=0; i<img.getWidth(); ++i){
					address = channel.address(i,j);
					// std::cout << i << ',' << j << '\n';
					alpha = imgAlpha.get<double>(address)/alphaMax;
					channel.put(address, alpha*channel.get<double>(address) + (1.0-alpha)*replace);
				}
			}
		}
		img.setAlphaChannelCount(0);

	};

};
// static CommandEntry<CmdImageFlatten> cmdImageFlatten("imageFlatten");
static CommandEntry<CmdImageFlatten> cmdImageFlatten("imageFlatten");


class CmdPalette : public SimpleCommand<std::string> {
public: //re


	CmdPalette() : SimpleCommand<std::string>(__FUNCTION__, "Load and apply palette.", "filename", "", "<filename>.txt") {
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();

		cmdImage.imageSelector.setParameters(resources.select);
		resources.select.clear();


		if (!resources.inputOk){
			mout.warn() << "input failed, skipping" << mout.endl;
			return;
		}

		if (!resources.dataOk){
			mout.warn() << "data error, skipping" << mout.endl;
			return;
		}


		if (resources.currentGrayImage != &resources.grayImage){  // TODO: remove this
			mout.debug(2) << "Resetting current gray image." << mout.endl;
			mout.debug(3) << cmdImage.imageSelector << mout.endl;
			resources.currentGrayImage = & DataSelector::getData(*resources.currentHi5, cmdImage.imageSelector);
			resources.currentImage     =   resources.currentGrayImage;
		}

		if (resources.currentGrayImage->isEmpty()){
			mout.note() << " current gray image is empty.";
			//return;
		}

		//mout.debug(4) << "Current Gray: \n" << *resources.currentGrayImage << mout.endl;
		//File::write(*resources.currentGrayImage, "gray.png");

		if (!value.empty())
			resources.palette.load(value);

		mout.debug(3) << "input properties" << resources.currentGrayImage->properties << mout.endl;
		mout.debug(2) << resources.palette << mout.endl;


		//mout.debug(5) << "--gain   " << resources.currentGrayImage->properties["what:gain"] << mout.endl;
		//mout.debug(5) << "--offset " << resources.currentGrayImage->properties["what:offset"] << mout.endl;

		apply();
	}

	void apply() const{

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();


		PaletteOp  op(resources.palette);
		// The intensities will  be mapped first: f' = gain*f + offset
		const VariableMap  & props = resources.currentGrayImage->properties;

		/// Principally ODIM needed, but PolarODIM contains Nyquist velocity information, if needed.
		const PolarODIM imgOdim(props);
		// mout.warn() << imgOdim << mout.endl;

		op.scale   = imgOdim.gain;   // props.get("what:gain", 1.0);
		op.offset  = imgOdim.offset; // props.get("what:offset",0.0);
		//const std::string quantity = props["what:quantity"];

		if (imgOdim.quantity == "VRAD"){ // rescale relative to NI (percentage -100% ... +100%)

			const double NI = imgOdim.getNyquist(); //props["how:NI"];
			if (NI != 0.0){
				// EncodingODIM odim;
				// PolarODIM odim(imgOdim);
				/*
				odim.gain   = op.scale;
				odim.offset = op.offset;
				odim.type = props.get("what:type","C");
				*/
				//odim.optimiseVRAD();

				const double data_min = imgOdim.scaleInverse(-NI);
				const double data_max = imgOdim.scaleInverse(+NI);
				const double data_mean = (data_max+data_min)/2.0;
				op.scale = 2.0/(data_max-data_min);
				op.offset = - data_mean * op.scale;

				//op.scale  = odim.gain;
				//op.offset = odim.offset;
				//mout.warn() << odim << mout.endl;
				//mout.warn() << data_mean << mout.endl;
				//mout.warn() << odim.scaleInverse(0) << mout.endl;
				//op.setSpecialCode("undetectValue", odim.scaleInverse(0.0));
			}
			else {
				mout.warn() << "No Nyquist velocity (NI) found in metadata." << mout.endl;
			}
		}

		op.setSpecialCode("nodata",   props["what:nodata"]);
		op.setSpecialCode("undetect", props["what:undetect"]);

		mout.debug() << op << mout.endl;
		//std::cout << op << std::endl;
		// if (gain == 0.0){  TODO: or from --gain ?	gain = 1.0;
		//op.setParameter("scale", gain);
		//op.setParameter("bias",offset);
		op.process(*resources.currentGrayImage, resources.colorImage);
		resources.colorImage.properties = props;
		//File::write(resources.colorImage, "color.png");
		resources.currentImage = & resources.colorImage;

	}
};
static CommandEntry<CmdPalette> cmdPalette("palette");


class CmdPaletteRefine : public SimpleCommand<int> {
public: //re
	CmdPaletteRefine() : SimpleCommand<int>(__FUNCTION__, "Refine colors", "count", 256){
	};

	void exec() const {
		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();
		resources.palette.refine(value);
		cmdPalette.apply();

	}
};
// static CommandEntry<CmdPaletteRefine> cmdPaletteRefine("paletteRefine");
static CommandEntry<CmdPaletteRefine> cmdPaletteRefine("paletteRefine");



class CmdLegendOut : public SimpleCommand<> {
public: //re
	CmdLegendOut() : SimpleCommand<>(__FUNCTION__, "Save palette as a legend to a SVG file.", "filename", "", "<filename>.svg") {
		//parameters.separators.clear();
		//parameters.reference("filename", filename, "", "<filename>.svg");
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();
		TreeSVG svg;
		resources.palette.getLegend(svg, true);

		std::string outFileName = resources.outputPrefix + value;
		//if (value.empty())
		std::ofstream ofstr(outFileName.c_str(), std::ios::out);
		if (!ofstr.good()){
			mout.error() << "could not open file: " << outFileName << mout.endl;
		}
		ofstr << svg;
		ofstr.close();
	};

};
static CommandEntry<CmdLegendOut> cmdLegendOut("legendOut");

} // namespace ::







class CmdPlot : public SimpleCommand<std::string> {
public:

	CmdPlot() : SimpleCommand<std::string>(__FUNCTION__, "Plot", "i,j,...", "0,0"){
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();
		std::list<std::string> l;
		//std::map<std::string,std::string> m;
		DataSelector selector("data[0-9]+$");
		selector.setParameters(resources.select);
		resources.select.clear();
		mout.debug() << "selector: " << selector << mout.endl;
		// DataSelector::getPaths(*resources.currentHi5, selector, l); // todo getFirstData
		//DataSelector::getPathsByQuantity(*resources.currentHi5, selector, m); // key==quantity, so only one (last) path obtained
		DataSelector::getPaths(*resources.currentHi5, selector, l);
		//size_t count = m.size();
		size_t count = l.size();
		mout.info() << "found: " << count << " paths " << mout.endl;
		drain::image::ImageTray<Channel> tray;
		//size_t index = 0;
		//for (std::map<std::string,std::string>::const_iterator it = m.begin(); it != m.end(); ++it){
		for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it){
			//mout.note() << "selected: " << it->first << '=' << it->second << mout.endl;
			mout.note() << "selected: " << *it << mout.endl;
			//drain::image::Image & channel = (*resources.currentHi5)(it->second)["data"].data.dataSet;
			drain::image::Channel & channel = (*resources.currentHi5)(*it)["data"].data.dataSet.getChannel(0);
			if (channel.isEmpty()){
				mout.warn() << "empty image in " << *it << "/data, skipping" << mout.endl;
				continue;
			}
			tray.appendImage(channel);
			mout.debug(1) << channel << mout.endl;
			std::string path(*it);
			if (DataSelector::getQualityPath(*resources.currentHi5, path)){
				mout.note() << "associated quality field for ["<< *it << "] found in " << path << mout.endl;
				if (tray.alpha.empty()){
					drain::image::Channel & quality = (*resources.currentHi5)(path).data.dataSet.getChannel(0);
					tray.appendAlpha(quality);
				}
				else {
					mout.note() << "skipping (using first quality data only)" << mout.endl;
				}
			}
			else {
				mout.debug() << "no quality data" << mout.endl;
			}
		}

		//mout.warn() << tray << mout.endl;

		drain::image::ImagePlot plot;
		plot.setParameters(value);
		plot.traverseChannels(tray);

		/*
		if (!l.empty()){
			const std::list<std::string>::const_iterator it = l.begin();
			mout.info() << "selected: " << *it << mout.endl;
		}
		 */

	}
};
static CommandEntry<CmdPlot> cmdPlot;
//static CommandEntry<CmdPaletteRefine> cmdPaletteRefine("paletteRefine");


std::string ImageOpRacklet::outputQuantity("{what:quantity}_{command}");

void ImageOpRacklet::exec() const {

	typedef DstType<ODIM> dst_t;

	drain::Logger mout(this->getName()+"[ImageOpRacklet]", __FUNCTION__); // = resources.mout;

	RackResources & resources = getResources();

	/*  Traverse all the datasets
	 *  - construct DataSet<Dst>(
	 *  - if (dataSetQuantity, add it for each,
	 */

	DataSelector imageSelector("dataset[0-9]+$","");
	imageSelector.count = 1;
	imageSelector.setParameters(resources.select);
	resources.select.clear();
	// skip quantity fow later traversal, accept now all the datasetN's
	const std::string quantity(imageSelector.quantity);
	imageSelector.quantity.clear();

	mout.debug(2) << imageSelector << mout.endl;

	std::list<std::string> l;
	DataSelector::getPaths(*resources.currentHi5, imageSelector, l);
	if (l.empty()){
		mout.warn() << "no paths found with selector: " << imageSelector << mout.endl;
		return;
	}

	// Main loop: visit each /dataset<n>
	for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it){


		// Results will be stored in the same datasetN.
		DataSet<dst_t > dstDataSet((*resources.currentHi5)(*it), quantity);

		drain::image::ImageTray<const Channel> srcTray;
		drain::image::ImageTray<Image> dstTray;

		bool DATASET_QUALITY = dstDataSet.hasQuality(); //
		bool SPECIFIC_QUALITY_FOUND    = false;
		bool SPECIFIC_QUALITY_MISSING  = false;

		//
		// mout.warn()
		mout.debug(1) << "path: " << *it << (DATASET_QUALITY ? " has":" has not") <<  " dataset quality" << mout.endl;

		/// 1st loop: Add data, not quality yet (only check it)
		for (DataSet<dst_t >::iterator dit = dstDataSet.begin(); dit != dstDataSet.end(); ++dit){

			Data<dst_t> & data = dit->second;

			/// Apply physical values as thresholds etc
			if (cmdPhysical.value)
				data.data.setScaling(data.odim.gain, data.odim.offset);
			else
				data.data.setScaling(1.0, 0.0);

			data.data.setName(dit->first);

			srcTray.appendImage(data.data);
			dstTray.appendImage(data.data);
			if (data.hasQuality()){
				mout.debug() << *it << " /[" << dit->first <<  "] has quality data" << mout.endl;
				SPECIFIC_QUALITY_FOUND = true;
			}
			else {
				mout.debug() <<  *it << " /[" << dit->first <<  "] has no quality data" << mout.endl;
				SPECIFIC_QUALITY_MISSING = true;
			}

		}

		/// Add quality, if found.
		// Case 1: at least some specific quality is used (and dataset-level )
		if ((DATASET_QUALITY && SPECIFIC_QUALITY_FOUND) || !SPECIFIC_QUALITY_MISSING) {
			if (DATASET_QUALITY)
				mout.note() << "using dataset-level quality data: " << *it << mout.endl;
			// else
			mout.note() << "using data-level quality data"  << mout.endl;
			// Loop again (add specific)
			for (DataSet<dst_t >::iterator dit = dstDataSet.begin(); dit != dstDataSet.end(); ++dit){
				//const std::string & key = it->first;
				mout.note() << "considering quantity " << dit->first << mout.endl;
				Data<dst_t > & data = dit->second;
				if (data.hasQuality()){
					mout.note() << "using quantity(" << dit->first << ") specific quality data" << mout.endl;
					PlainData<dst_t> & q = data.getQualityData();
					//dstTray.alpha.appendImage(data.getQualityData().data);
					q.data.getScaling().setPhysicalRange(0.0, 1.0); // not self-evident
					q.data.setScaling(q.odim.gain, q.odim.offset);
					srcTray.alpha.set(q.data);
					dstTray.alpha.set(q.data);
				}
				else if (DATASET_QUALITY) {
					//dstTray.alpha.appendImage(dstDataSet.getQualityData().data);
					PlainData<dst_t> & q = dstDataSet.getQualityData();
					q.data.getScaling().setPhysicalRange(0.0, 1.0); // not self-evident
					q.data.setScaling(q.odim.gain, q.odim.offset);
					srcTray.alpha.set(q.data);
					dstTray.alpha.set(q.data);
				}
			}
		}
		else if (DATASET_QUALITY){ // ...only.
			mout.note() << *it << " has dataset-level quality data (only)"  << mout.endl;
			//dstTray.alpha.appendImage(dstDataSet.getQualityData().data);
			PlainData<dst_t> & q = dstDataSet.getQualityData();
			q.data.getScaling().setPhysicalRange(0.0, 1.0); // not self-evident
			q.data.setScaling(q.odim.gain, q.odim.offset);
			srcTray.alpha.set(q.data);
			dstTray.alpha.set(q.data);
		}
		else {
			if (SPECIFIC_QUALITY_FOUND && SPECIFIC_QUALITY_MISSING)
				mout.note() << "discarding quality data (associated with only some data)"  << mout.endl;
			else
				mout.info() << "no associated quality data"  << mout.endl;
		}


		//drain::image::ImageTray<const Channel> srcTray(dstTray); // fix
		mout.debug()  << "src tray:\n" << srcTray << mout.endl;
		mout.debug(1) << "dst tray before processing:\n" << dstTray << mout.endl;
		bean.process(srcTray, dstTray); //, true);
		// bean.traverseChannels(srcTray, dstTray);
		mout.debug() << "dst tray after processing:\n" << dstTray << mout.endl;


	}

	return;
}

ImageRackletModule::list_t ImageRackletModule::rackletList;

//ImageRackletModule::ImageRackletModule(CommandRegistry & registry){ //, const std::string & section, const std::string & prefix){
ImageRackletModule::ImageRackletModule(const std::string & section, const std::string & prefix){

	drain::Logger mout(__FUNCTION__);

	CommandRegistry & registry = getRegistry();

	registry.setSection(section, prefix);

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

	// DEBUG
	/*
	for (list_t::iterator it=rackletList.begin(); it != rackletList.end(); ++it){
		mout.warn() << it->getName() << '\t' << it->getParameters().getValues() << mout.endl;
	}
	 */


}


/*

DataSelector imageSelector(".* /data[0-9]+$","");   // Consider shared? Only for images. Not directly accessible.
imageSelector.setParameters(resources.select);
resources.select.clear();
mout.debug(2) << imageSelector << mout.endl;

std::list<std::string> l;
DataSelector::getPaths(*resources.currentHi5, imageSelector, l);

if (l.empty()){
	mout.warn() << "no matching paths with selector: " << imageSelector << mout.endl;
	return;
}
//size_t count = l.size();
mout.info() << "found: " << l.size() << " paths " << mout.endl;

Variable & v = (*resources.currentHi5)["what"].data.attributes["object"];
if (!v.typeIsSet())
	v.setType<std::string>();
const std::string object = v;
const bool CARTESIAN = (object == "COMP");

mout.info() << "data type " << (CARTESIAN?"CARTESIAN":"POLAR") << mout.endl;
mout.debug() << "selector: " << imageSelector << mout.endl;

for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it){
	mout.info() << "processing: " << *it << mout.endl;
	if (CARTESIAN)
		processDataSet<CartesianODIM>(*resources.currentHi5, *it, imageSelector.quantity, mout);
	else
		processDataSet<PolarODIM>(*resources.currentHi5, *it, imageSelector.quantity, mout);
}
DataTools::updateAttributes(*resources.currentHi5);
 */


} // namespace rack

// Rack
