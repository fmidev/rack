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
//CommandEntry<CmdPhysical> cmdPhysical("iPhysical");


void CmdImage::convertImage(const HI5TREE & src, const DataSelector & selector, const std::string & parameters, drain::image::Image &dst){

	drain::Logger mout("CmdImage", __FUNCTION__);

	//std::string path = "";
	//DataSelector::getPath(src, DataSelector("(data|quality)[0-9]+/?$", selector.quantity), path);
	ODIMPath path;
	selector.getPathNEW(src, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY);

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
		//const drain::image::Image &alpha = DataSelector::getData(*resources.currentHi5, iSelector);
		ODIMPath path;
		iSelector.getPathNEW(*resources.currentHi5, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY);  // ODIMPathElem::ARRAY
		path << ODIMPathElem::ARRAY;
		mout.debug() << "alpha path:"  <<  path << mout.endl;
		const drain::image::Image &alpha = (*resources.currentHi5)(path).data.dataSet;

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
		std::list<ODIMPath> paths;
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
		for (std::list<ODIMPath>::const_iterator it = paths.begin(); it != paths.end(); ++it){
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
			const std::list<ODIMPath>::const_iterator it = paths.begin();
			mout.info() << "selected: " << *it << mout.endl;
		}
		 */

	}
};
static CommandEntry<CmdPlot> cmdPlot;
//static CommandEntry<CmdPaletteRefine> cmdPaletteRefine("paletteRefine");





} // namespace rack

