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



#include <drain/util/Debug.h>
#include <drain/util/RegExp.h>

#include <drain/image/Image.h>
#include <drain/image/TreeSVG.h>
#include <drain/image/ImageOpBank.h>

#include <drain/prog/CommandRegistry.h>
#include <drain/prog/Commands-ImageTools.h>
#include <drain/prog/Commands-ImageOps.h>

#include <drain/image/FastAverageOp.h>


#include "rack.h"
#include "product/DataConversionOp.h"
#include "data/ODIM.h"
//#include "hi5/Hi5.h"

#include "commands.h"
#include "images.h"

namespace rack {

CommandEntry<CmdImage> cmdImage("image");


void CmdImage::convertImage(const HI5TREE & src, const DataSelector & selector, const std::string & parameters, drain::image::Image &dst){

	drain::MonitorSource mout("CmdImage", __FUNCTION__);

	std::string path = "";
	DataSelector::getPath(src, DataSelector("(data|quality)[0-9]+/?$", selector.quantity), path);
	if (path.empty()){
		mout.warn() << "path empty" << mout.endl;
		return;
	}

	const Data<PolarSrc> srcData(src(path));  // limit?

	DataConversionOp op;
	// mout.warn() << "srcOdim:" << srcData.odim << mout.endl;
	op.odim.importMap(srcData.odim);
	// mout.note() << "dstOdim:" << op.odim << mout.endl;

	// mout.note() << "params :" << parameters << mout.endl;
	ProductOp::handleEncodingRequest(op.odim, parameters);
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

		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout; = resources.mout;

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

		DataConversionOp copier; //(copierHidden);
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
		copier.processImage(odimAlpha, alpha, copier.odim, img->getAlphaChannel());

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

		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout;

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

		drain::image::Image & imgAlpha = img.getAlphaChannel();

		const double alphaMax = imgAlpha.getMax<double>();
		if (alphaMax == 0) {
			mout.warn() << name << ": zero alphaMax" << mout.endl;
			return;
		}

		double alpha;

		Variable colorVector;
		colorVector.setType<size_t>();
		colorVector = value;

		size_t address;
		for (size_t k=0; k<img.getImageChannelCount(); ++k){
			drain::image::Image & channel = img.getChannel(k);
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

		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout;

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

		resources.palette.load(value);
		mout.debug(3) << "input properties" << resources.currentGrayImage->properties << mout.endl;
		mout.debug(2) << resources.palette << mout.endl;


		//mout.debug(5) << "--gain   " << resources.currentGrayImage->properties["what:gain"] << mout.endl;
		//mout.debug(5) << "--offset " << resources.currentGrayImage->properties["what:offset"] << mout.endl;

		apply();
	}

	void apply() const{

		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();


		PaletteOp  op(resources.palette);
		// The intensities will  be mapped first: f' = gain*f + offset
		const VariableMap  & props = resources.currentGrayImage->properties;
		op.scale   = props.get("what:gain", 1.0);
		op.offset  = props.get("what:offset",0.0);
		const std::string quantity = props["what:quantity"];
		if (quantity == "VRAD"){ // rescale relative to NI (percentage -100% ... +100%)
			const double NI = props["how:NI"];
			if (NI != 0.0){
				EncodingODIM odim;
				odim.gain   = op.scale;
				odim.offset = op.offset;
				odim.type = props.get("what:type","C");
				const double data_min = odim.scaleInverse(-NI);
				const double data_max = odim.scaleInverse(+NI);
				const double data_mean = (data_max+data_min)/2.0;
				op.scale = 2.0/(data_max-data_min);
				op.offset = - data_mean * op.scale;
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
		op.filter(*resources.currentGrayImage, resources.colorImage);
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
		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();
		resources.palette.refine(value);
		cmdPalette.apply();

	}
};
// static CommandEntry<CmdPaletteRefine> cmdPaletteRefine("paletteRefine");
static CommandEntry<CmdPaletteRefine> cmdPaletteRefine("paletteRefine");

class CmdImageOp : public BasicCommand {
    public: //re 
	//CmdImageOp() : SimpleCommand<std::string>(__FUNCTION__, "Process data with image operator", "op:parameters", ""){
	//};

	CmdImageOp() : BasicCommand(__FUNCTION__, "Run image operator. See --select and --help imageOps.") {
		this->parameters.reference("operator", opName);
		this->parameters.reference("parameters", opParameters);
	};

	std::string opName;
	std::string opParameters;

	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();

		DataSelector imageSelector(".*/data[0-9]+/?$","");   // Consider shared? Only for images. Not directly accessible.
		//DataSelector imageSelector(".*/data/?$","");   // Consider shared? Only for images. Not directly accessible.
		imageSelector.setParameters(resources.select);
		resources.select.clear();
		mout.debug(2) << imageSelector << mout.endl;

		std::list<std::string> l;
		DataSelector::getPaths(*resources.currentHi5, imageSelector, l); // todo getFirstData

		if (l.empty()){
			mout.warn() << "matching paths with selector: " << imageSelector << mout.endl;
			return;
		}

		Variable & v = (*resources.currentHi5)["what"].data.attributes["object"];
		if (!v.typeIsSet())
			v.setType<std::string>();
		const std::string object = v;
		const bool CARTESIAN = (object == "COMP");

		mout.info() << "data type " << (CARTESIAN?"CARTESIAN":"POLAR") << mout.endl;

		try {

			ImageOp & op = getImageOpBank().get(opName).get();

			try {
				op.setParameters(opParameters);
				mout.debug() << op << mout.endl;
			} catch (std::exception & e) {
				mout.error() << "invalid parameter(s) for '" << opName << "'" << mout.endl;
			}

			for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it){

				if (CARTESIAN)
					processDataSet<CartesianODIM>(op, *resources.currentHi5, *it, mout);
				else
					processDataSet<PolarODIM>(op, *resources.currentHi5, *it, mout);
			}

		} catch (std::exception & e) {
			mout.error() << "operator not found: " << opName << mout.endl;
		}



	}

protected:

	/*
	 *  \tparam T
	 */
	template <class T>
	void processDataSet(const ImageOp &op, HI5TREE & root, const std::string & path, MonitorSource & mout) const {

		RackResources & resources = getResources();

		Data<SrcType<T> > srcData(root(path));
		const drain::image::Image & src = srcData.data;

		if (!src.isEmpty()){
			mout.note() << " processing '" << srcData.odim.quantity << " (" << path << ')' << mout.endl;
			DataSetDst<DstType<T> > dstDataSet(root(DataSelector::getParent(path)));
			Data<DstType<T> > & dstData = dstDataSet.getData(srcData.odim.quantity+"_"+opName);
			drain::image::Image & dst = dstData.data;
			op.filter(src, dst);
			resources.currentGrayImage = & dst;
			resources.currentImage     = & dst;
		}
		else
			mout.note() << "skipping path: " << path << mout.endl;
	}

};
static CommandEntry<CmdImageOp> cmdImageOp("imageOp");



class CmdLegendOut : public SimpleCommand<> {
    public: //re 
	CmdLegendOut() : SimpleCommand<>(__FUNCTION__, "Save palette as a legend to a SVG file.", "filename", "", "<filename>.svg") {
		//parameters.separators.clear();
		//parameters.reference("filename", filename, "", "<filename>.svg");
	};

	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout;

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


std::string ImageOpRacklet::outputQuantity("{what:quantity}_{command}");

void ImageOpRacklet::exec() const {

	drain::MonitorSource mout(this->getName(), __FUNCTION__); // = resources.mout;

	RackResources & resources = getResources();

	DataSelector imageSelector(".*/data[0-9]+/?$","");   // Consider shared? Only for images. Not directly accessible.
	//DataSelector imageSelector(".*/data/?$","");   // Consider shared? Only for images. Not directly accessible.
	imageSelector.setParameters(resources.select);
	resources.select.clear();
	mout.debug(2) << imageSelector << mout.endl;

	std::list<std::string> l;
	DataSelector::getPaths(*resources.currentHi5, imageSelector, l); // todo getFirstData

	if (l.empty()){
		mout.warn() << "matching paths with selector: " << imageSelector << mout.endl;
		return;
	}

	Variable & v = (*resources.currentHi5)["what"].data.attributes["object"];
	if (!v.typeIsSet())
		v.setType<std::string>();
	const std::string object = v;
	const bool CARTESIAN = (object == "COMP");

	mout.info() << "data type " << (CARTESIAN?"CARTESIAN":"POLAR") << mout.endl;

	for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it){

		if (CARTESIAN)
			processDataSet<CartesianODIM>(*resources.currentHi5, *it, mout);
		else
			processDataSet<PolarODIM>(*resources.currentHi5, *it, mout);
	}


}


//ImageRackletModule::ImageRackletModule(CommandRegistry & registry){ //, const std::string & section, const std::string & prefix){
ImageRackletModule::ImageRackletModule(const std::string & section, const std::string & prefix){

	drain::MonitorSource mout(__FUNCTION__);

	CommandRegistry & registry = getRegistry();

	registry.setSection(section, prefix);

	ImageOpBank::map_t & ops = getImageOpBank().getMap();
	typedef std::list<ImageOpRacklet> list_t;
	static list_t l;
	//int n=0;
	for (ImageOpBank::map_t::iterator it=ops.begin(); it != ops.end(); ++it){

		const std::string & key = it->first;
		ImageOp & op = it->second.get();
		op.getParameters(); // check debug
		list_t::iterator lit = l.insert(l.end(), ImageOpRacklet(op, key));
		// mout.warn() << op.getName() << '\t' << adapter.getName() << mout.endl;
		registry.add(*lit, key);
		// ++n; if (n > 3) return;
	}


}


} // namespace rack

// Rack
