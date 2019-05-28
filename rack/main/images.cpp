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
#include <data/ODIMPath.h>
#include <data/PolarODIM.h>
#include <hi5/Hi5.h>
#include <image/Image.h>
#include <image/ImageChannel.h>
#include <image/ImageLike.h>
#include <image/ImageTray.h>
#include <image/TreeSVG.h>
#include <imageops/ImageModifierPack.h>
#include <imageops/PaletteOp.h>
#include <main/resources.h>
#include <product/DataConversionOp.h>
#include <prog/Command.h>
#include <prog/CommandAdapter.h>
#include <radar/Analysis.h>
#include <stddef.h>
#include <util/Log.h>
#include <util/Path.h>
#include <util/RegExp.h>
#include <util/SmartMap.h>
#include <util/Tree.h>
#include <util/Type.h>
#include <util/Variable.h>
#include <util/VariableMap.h>

#include "images.h"
#include "image-ops.h"

#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <string>

// #include <pthread.h>

namespace rack {

CommandEntry<CmdImage> cmdImage("image");
//CommandEntry<CmdPhysical> cmdPhysical("iPhysical");


void CmdImage::convertImage(const HI5TREE & src, const DataSelector & selector, const std::string & parameters, drain::image::Image &dst){

	drain::Logger mout("CmdImage", __FUNCTION__);

	ODIMPath path;
	selector.getPathNEW(src, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY);

	if (path.empty()){
		mout.warn() << "path empty" << mout.endl;
		return;
	}

	const Data<PolarSrc> srcData(src(path));  // limit?

	//DataConversionOp<PolarODIM> op;
	DataConversionOp<ODIM> op;
	// mout.warn() << "srcOdim:" << srcData.odim << mout.endl;
	op.odim.updateFromMap(srcData.odim);
	// mout.note() << "dstOdim:" << op.odim << mout.endl;

	// mout.note() << "params :" << parameters << mout.endl;
	ProductBase::handleEncodingRequest(op.odim, parameters);
	// mout.warn() << "dstOdim:" << op.odim << mout.endl;

	op.processImage(srcData, dst);

	RackResources & resources = getResources();
	resources.currentGrayImage = &resources.grayImage;
	resources.currentImage     = &resources.grayImage;

}








namespace {


// Adds alpha channel containing current data.
class CmdImageAlphaBase : public BasicCommand {

public:

	CmdImageAlphaBase(const std::string & name, const std::string & description) :  BasicCommand(name, description){
	}


	drain::image::Image & getModifiableImage(const DataSelector & selector) const {

		drain::Logger mout(name, __FUNCTION__);

		RackResources & resources = getResources();

		if (resources.currentImage == &resources.colorImage){
			mout.info() << " using already existing colour image" << mout.endl;
			return resources.colorImage;
		}
		else if (resources.currentImage == &resources.grayImage){
			mout.info() << " using already existing (additional) gray image" << mout.endl;
			//if (ImageOpRacklet::physical){
			//mout.note() << " ensuring physical scale" << mout.endl;
			return resources.grayImage;
		}
		else { // resources.currentImage != &resources.grayImage
			mout.note() << " no current image, creating a copy (graylevel)" << mout.endl;
			CmdImage::convertImage(*resources.currentHi5, selector, "", resources.grayImage);  // TODO check
			//resources.currentGrayImage = &resources.grayImage;
			//resources.currentImage     = &resources.grayImage;
			return resources.grayImage;
		}
	}

};



// Adds alpha channel containing current data.
class CmdImageAlpha : public CmdImageAlphaBase {

public:

	CmdImageAlpha() : CmdImageAlphaBase(__FUNCTION__, "Adds a transparency channel. Implies additional image, creates one if needed. See --target") {
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout; = resources.mout;

		RackResources & resources = getResources();

		DataSelector iSelector;
		iSelector.setParameters(resources.select);
		resources.select.clear();

		// Source image (original data)
		ODIMPath path;
		iSelector.getPathNEW(*resources.currentHi5, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY);  // ODIMPathElem::ARRAY
		path << ODIMPathElem::ARRAY;
		mout.debug() << "alphaSrc path:"  <<  path << mout.endl;
		const drain::image::Image &alphaSrc = (*resources.currentHi5)(path).data.dataSet;
		ODIM alphaSrcODIM(alphaSrc);


		// Dst image
		// Create a copy (getResources().colorImage or getResources().grayImage), and point *img to it.
		// Because getResources().currentImage is const, non-const *img is needed.
		drain::image::Image & img = getModifiableImage(iSelector);

		/// Add empty alphaSrc channel
		img.setAlphaChannelCount(1);

		mout.debug() << "image:"  << *resources.currentImage << mout.endl;
		mout.debug() << "alphaSrc:"  <<  resources.currentImage->getAlphaChannel() << mout.endl;


		DataConversionOp<ODIM> copier; //(copierHidden);
		copier.odim.addShortKeys();
		copier.odim.setValues(resources.targetEncoding);
		resources.targetEncoding.clear();

		//const std::string type(1, drain::Type::getTypeChar(img.getType())); // rewrite
		const std::string type = drain::Type(img.getType()); // rewrite
		if (copier.odim.type != type){
			mout.note() << " using the type of base image: " << type << mout.endl;
		}
		copier.odim.type = type; //drain::Type::getTypeChar(img->getType());

		mout.debug(2) << "alphaSrcODIM:  "  <<  alphaSrcODIM << mout.endl;
		mout.debug(2) << "odimOut: " << copier.odim << mout.endl;
		mout.debug(2) << "copier: " << copier << mout.endl;
		copier.traverseImageFrame(alphaSrcODIM, alphaSrc, copier.odim, img.getAlphaChannel());

		img.properties["what.gainAlpha"]   = copier.odim.gain;
		img.properties["what.offsetAlpha"] = copier.odim.offset;

	}
};
// static CommandEntry<CmdImageAlpha> cmdImageAlpha("imageAlpha");
static CommandEntry<CmdImageAlpha> cmdImageAlpha("imageAlpha");



// NEW! Adds alpha channel containing current data.
class CmdImageTransp : public CmdImageAlphaBase {

public:

	drain::Range<double> range;
	double undetect;
	double nodata;

	CmdImageTransp() : CmdImageAlphaBase(__FUNCTION__, "Adds a transparency channel. Uses copied image, creating one if needed.") {
		parameters.reference("range",    range.vect, "min:max");
		parameters.reference("undetect", undetect=0, "opacity of 'undetect' pixels");
		parameters.reference("nodata",   nodata=1, "opacity of 'nodata' pixels"); // std::numeric_limits<double>::max()
		parameters["range"].fillArray = true;

		range.min = -std::numeric_limits<double>::max();
		range.max = +std::numeric_limits<double>::max();
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout; = resources.mout;

		RackResources & resources = getResources();

		DataSelector imageSelector;
		imageSelector.setParameters(resources.select);
		resources.select.clear();

		// Source image (original data)
		ODIMPath path;
		imageSelector.getPathNEW(*resources.currentHi5, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY);  // ODIMPathElem::ARRAY
		path << ODIMPathElem::ARRAY;
		mout.debug() << "alphaSrc path:"  <<  path << mout.endl;
		//hi5::NodeHi5 & node = (*resources.currentHi5)(path).data;
		drain::image::Image &srcImg = (*resources.currentHi5)(path).data.dataSet; // Yes non-const, see below
		EncodingODIM odim(srcImg);
		srcImg.setScaling(odim.gain, odim.offset);

		// Dst image
		// Create a copy (getResources().colorImage or getResources().grayImage), and point *img to it.
		// Because getResources().currentImage is const, non-const *img is needed.
		drain::image::Image & dstImg = getModifiableImage(imageSelector);
		dstImg.setAlphaChannelCount(1);

		RadarFunctorOp<drain::FuzzyStep<double> > fuzzyStep(true);
		fuzzyStep.odimSrc.updateFromMap(srcImg.getProperties());

		//fuzzyStep.functor.set(range.min, range.max, 1.0);
		/*
		const drain::image::ImageScaling & scaling = srcImg.getScaling();
		mout.warn() << "scaling: "  << scaling << mout.endl;
		fuzzyStep.functor.set(scaling.inv(range.min), scaling.inv(range.max), 1.0);
		*/
		if (ImageOpRacklet::physical){
			fuzzyStep.functor.set(range.min, range.max, 1.0);
		}
		else {
			//const drain::image::ImageScaling & scaling = srcImg.getScaling();
			mout.warn() << "scaling: "  << odim << mout.endl;
			//odim.scaleForward()
			const double max = Type::call<typeNaturalMax>(srcImg.getType()); // 255, 65535, or 1.0
			//srcImg.getScaling().
			fuzzyStep.functor.set(odim.scaleForward(max*range.min), odim.scaleForward(max*range.max), 1.0);
		}

		const double dstMax = Type::call<typeNaturalMax>(dstImg.getType()); // 255, 65535, or 1.0
		drain::typeLimiter<double>::value_t limit = dstImg.getEncoding().getLimiter<double>();  // t is type_info, char or std::string.
		fuzzyStep.nodataValue   = limit(dstMax*nodata);
		fuzzyStep.undetectValue = limit(dstMax*undetect);

		mout.warn() << "fuzzy: "  << fuzzyStep << mout.endl;
		fuzzyStep.traverseChannel(srcImg.getChannel(0), dstImg.getAlphaChannel(0));

	}

};
static CommandEntry<CmdImageTransp> cmdImageTransp("imageTransp");



class CmdImageFlatten : public SimpleCommand<std::string> {

public:

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

		const double alphaMax = imgAlpha.getEncoding().getTypeMax<double>();
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

public:

	CmdPalette() : SimpleCommand<std::string>(__FUNCTION__, "Load and apply palette.", "filename", "", "<filename>.txt") {
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();

		if (!resources.inputOk){
			mout.warn() << "input failed, skipping" << mout.endl;
			return;
		}

		if (!resources.dataOk){
			mout.warn() << "data error, skipping" << mout.endl;
			return;
		}

		cmdImage.imageSelector.setParameters(resources.select);
		// below resources.select.clear();

		if (resources.currentGrayImage != &resources.grayImage){  // TODO: remove this
			mout.debug(2) << "Resetting current gray image." << mout.endl;
			mout.debug(3) << cmdImage.imageSelector << mout.endl;
			ODIMPath path;
			cmdImage.imageSelector.getPathNEW(*resources.currentHi5, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY);
			mout.note() << "path:" << path << mout.endl;
			resources.currentGrayImage = & (*resources.currentHi5)(path)[odimARRAY].data.dataSet;
			//resources.currentGrayImage = & DataSelector::getData(*resources.currentHi5, cmdImage.imageSelector);
			resources.currentImage     =   resources.currentGrayImage;
		}

		if (resources.currentGrayImage->isEmpty()){
			mout.note() << " current gray image is empty.";
			//return;
		}

		//mout.debug(4) << "Current Gray: \n" << *resources.currentGrayImage << mout.endl;
		//File::write(*resources.currentGrayImage, "gray.png");


		if (!value.empty() || resources.palette.empty()){

			static RegExp quantityRegExp("^[A-Z]+[A-Z0-9_]*$");
			std::string quantity;

			static RegExp extensionRegExp("\\.([[:alnum:]]+)$");
			std::string filename;
			std::ifstream ifstr;

			// drain::StringMapper palettePath;
			// palettePath.parse("${palettePath}/palette-${what:quantity}.txt");


			if (value == "default" || (resources.palette.empty() && value.empty())){
				VariableMap & statusMap = getResources().getUpdatedStatusMap(); // getRegistry().getStatusMap(true);
				quantity = statusMap["what:quantity"].toStr();
				mout.note() << "quantity=" << quantity << mout.endl;
			}
			else if (quantityRegExp.test(value)){
				quantity = value;
			}

			if (quantity.empty()){
				filename = value;
			}
			else {
				std::stringstream s;
				s << "palette-" << quantity << ".txt";
				filename = s.str();
			}

			mout.warn() << "filename=" << filename << mout.endl;

			ifstr.open(filename.c_str(), std::ios::in);
			if (!ifstr.is_open()){
				// Test
				filename = std::string("palette/") + filename;
				mout.note() << "retry with " << filename << mout.endl;
				ifstr.open(filename.c_str(), std::ios::in);
			}

			if (!ifstr.is_open()){
				mout.error() << "could not open palette: " << filename << mout.endl;
				return;
			}

			if (!extensionRegExp.execute(filename)){
				mout.warn() << "ext=" << extensionRegExp.result[1] << mout.endl;
				if (extensionRegExp.result[1] == "json"){
					drain::JSON::tree_t json;
					drain::JSON::read(json, ifstr);
					//mout.note() << "ext=" << json << mout.endl;
					//drain::JSON::write(json);
					resources.palette.convertJSON(json);
					return;
				}
			}
			else {
				mout.warn() << "filename has no extension" << mout.endl;
			}

			resources.palette.load(ifstr);
		}

		mout.debug(3) << "input properties" << resources.currentGrayImage->properties << mout.endl;
		mout.debug(2) << resources.palette << mout.endl;

		//mout.debug(5) << "--gain   " << resources.currentGrayImage->properties["what:gain"] << mout.endl;
		//mout.debug(5) << "--offset " << resources.currentGrayImage->properties["what:offset"] << mout.endl;

		apply();

		resources.select.clear();

	}

	void apply() const{

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();


		PaletteOp  op(resources.palette);
		// The intensities will  be mapped first: f' = gain*f + offset
		const FlexVariableMap  & props = resources.currentGrayImage->properties;

		/// Principally ODIM needed, but PolarODIM contains Nyquist velocity information, if needed.
		const PolarODIM imgOdim(props);
		mout.debug() << "input metadata: " << EncodingODIM(imgOdim) << mout.endl;


		if (imgOdim.quantity.substr(0,4) != "VRAD"){
			op.scale   = imgOdim.gain;   // props.get("what:gain", 1.0);
			op.offset  = imgOdim.offset; // props.get("what:offset",0.0);
		}
		else { // rescale relative to NI (percentage -100% ... +100%)

			const double NI = imgOdim.getNyquist(); //props["how:NI"];
			if (NI != 0.0){

				mout.info() << "Doppler speed (VRAD), using relative scale, NI=" << NI << " orig NI=" << props["how:NI"] << mout.endl;
				/*
					f(data_min) = scale*data_min + offset == -1.0
					f(data_max) = scale*data_max + offset == +1.0
					scale = (+1 - -1) / ( data_max-data_min )
					offset = scale*(data_min+data_max) / 2.0
				*/
				const double data_min = imgOdim.scaleInverse(-NI);
				const double data_max = imgOdim.scaleInverse(+NI);
				//const double data_mean = (data_max+data_min)/2.0;
				op.scale  =   2.0/(data_max-data_min);
				op.offset = - op.scale*(data_max+data_min)/2.0;
				//op.offset = -1.0;
				//op.scale = 2.0/(data_max-data_min);

				//mout.warn() << odim << mout.endl;
				mout.debug() << "expected storage value range: " << data_min << '-' << data_max << mout.endl;
				//mout.warn() << odim.scaleInverse(0) << mout.endl;
				//op.setSpecialCode("undetectValue", odim.scaleInverse(0.0));
			}
			else {
				mout.warn() << "No Nyquist velocity (NI) found in metadata." << mout.endl;
			}
		}

		op.setSpecialCode("nodata",   imgOdim.nodata);    // props["what:nodata"]);
		op.setSpecialCode("undetect", imgOdim.undetect); // props["what:undetect"]);

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

public:

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
		ODIMPathList paths;
		//std::map<std::string,std::string> m;
		DataSelector selector; //("data[0-9]+$");
		selector.setParameters(resources.select);
		resources.select.clear();
		mout.debug() << "selector: " << selector << mout.endl;
		//  selector.getPathsNEW(*resources.currentHi5, paths); // RE2 // todo getFirstData
		//DataSelector::getPathsByQuantity(*resources.currentHi5, selector, m); // key==quantity, so only one (last) path obtained
		selector.getPathsNEW(*resources.currentHi5, paths, ODIMPathElem::DATA);
		//size_t count = m.size();
		size_t count = paths.size();
		mout.info() << "found: " << count << " paths " << mout.endl;
		drain::image::ImageTray<Channel> tray;
		//size_t index = 0;
		//for (std::map<std::string,std::string>::const_iterator it = m.begin(); it != m.end(); ++it){
		for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); ++it){
			//mout.note() << "selected: " << it->first << '=' << it->second << mout.endl;
			mout.note() << "selected: " << *it << mout.endl;
			HI5TREE & dst = (*resources.currentHi5)(*it);
			//drain::image::Image & channel = (*resources.currentHi5)(it->second)["data"].data.dataSet;
			drain::image::Channel & channel = dst["data"].data.dataSet.getChannel(0);
			if (channel.isEmpty()){
				mout.warn() << "empty image in " << *it << "/data, skipping" << mout.endl;
				continue;
			}
			tray.appendImage(channel);
			mout.debug(1) << channel << mout.endl;
			//std::string path(*it);


			ODIMPath path;
			mout.warn() << "note: checking parallel quality unimplemented" << mout.endl;
			if (DataSelector().getPathNEW(dst, path, ODIMPathElem::QUALITY)){
			//if (DataSelector::getQualityPath(*resources.currentHi5, path)){
				path << ODIMPathElem(ODIMPathElem::ARRAY);
				mout.note() << "associated quality field for ["<< *it << "] found in " << path << mout.endl;
				if (tray.alpha.empty()){
					drain::image::Channel & quality = dst(path).data.dataSet.getChannel(0);
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
		if (!paths.empty()){
			const ODIMPathList::const_iterator it = paths.begin();
			mout.info() << "selected: " << *it << mout.endl;
		}
		 */

	}
};
static CommandEntry<CmdPlot> cmdPlot;
//static CommandEntry<CmdPaletteRefine> cmdPaletteRefine("paletteRefine");





} // namespace rack

