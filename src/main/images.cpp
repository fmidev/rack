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
//#include <stddef.h>

#include <drain/prog/CommandInstaller.h>
#include <main/palette-manager.h>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <string>

#include "drain/util/Log.h"
#include "drain/util/FilePath.h"
#include "drain/util/Output.h"
#include "drain/util/RegExp.h"
#include "drain/util/SmartMap.h"
#include "drain/util/TreeOrdered.h"
#include "drain/util/Type.h"
#include "drain/util/Variable.h"
#include "drain/util/VariableMap.h"
#include "drain/image/Image.h"
#include "drain/image/ImageChannel.h"
#include "drain/image/ImageLike.h"
#include "drain/image/ImageTray.h"
#include "drain/image/TreeSVG.h"
#include "drain/imageops/ImageModifierPack.h"
#include "drain/imageops/PaletteOp.h"
#include "drain/imageops/FunctorOp.h"

#include "drain/prog/Command.h"
#include "hi5/Hi5.h"
#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/ODIMPath.h"
#include "data/PolarODIM.h"
#include "radar/Analysis.h"
#include "product/DataConversionOp.h"
#include "resources.h"
#include "images.h"
//#include "image-kit.h"

#include "image-ops.h"


// #include <pthread.h>

namespace rack {


void CmdImage::exec() const {
	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, getName());

	drain::image::Image & dst = ctx.getModifiableImage();
	ctx.setCurrentImages(dst);

	mout.deprecating() << "This command may be removed in future versions." << mout;

	//ctx.updateCurrentImage();

	/*
	//if ctx.select...
	//ImageKit::findImage(ctx);
	ctx.currentPath = ctx.findImage();
	//if (ctx.currentPath)
	mout.debug() << "(Quality data not used alpha channel" << mout.endl;

	//ImageKit::convertGrayImage(ctx, *ctx.currentGrayImage);
	ctx.convertGrayImage(*ctx.currentGrayImage);
	*/

}




// Adds alpha channel containing current data.
class CmdImageAlphaBase : public drain::BasicCommand {

public:

	CmdImageAlphaBase(const std::string & name, const std::string & description) :  drain::BasicCommand(name, description){
	}


	/*
	// Source image (original data)
	ODIMPath path;
	// imageSelector.getPathNEW(*ctx.currentHi5, path, ODIMPathElem::DATA | ODIMPathElem::QUALITY);  // ODIMPathElem::ARRAY
	imageSelector.getPath3(*ctx.currentHi5, path);
	path << ODIMPathElem::ARRAY;
	mout.debug() << "alphaSrc path:"  <<  path << mout.endl;
	//hi5::NodeHi5 & node = (*ctx.currentHi5)(path).data;
	drain::image::Image &srcImg = (*ctx.currentHi5)(path).data.dataSet; // Yes non-const, see below
	EncodingODIM odim(srcImg);
	srcImg.setScaling(odim.scaling.scale, odim.scaling.offset);
	*/
	// TODO:PlainData<ODIM> dst((*ctx.currentHi5)(path));

	const Hi5Tree & getAlphaSrc(RackContext & ctx) const {

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		DataSelector imageSelector(ODIMPathElem::DATA|ODIMPathElem::QUALITY); // ImageS elector imageS elector;
		imageSelector.consumeParameters(ctx.select);
		imageSelector.ensureDataGroup();

		mout.debug() << "selector:"  << imageSelector << mout.endl;

		// Source image (original data)
		ODIMPath path;
		imageSelector.getPath(*ctx.currentHi5, path); //, ODIMPathElem::DATA | ODIMPathElem::QUALITY);  // ODIMPathElem::ARRAY

		if (path.empty()){
			mout.warn() << "no paths with selector: "  << imageSelector << mout.endl;
		}
		else {
			mout.debug() << "found path:"  <<  path << mout.endl;
		}


		//drain::image::Image &alphaSrc =
		return (*ctx.currentHi5)(path); //[ODIMPathElem::ARRAY].data.dataSet;
		//return alphaSrc;

	}

	drain::image::Image & getDstImage(RackContext & ctx) const {

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		drain::image::Image & dstImg = ctx.getModifiableImage();

		if (dstImg.isEmpty()){
			mout.warn() << "could not get ModifiableImage" << mout.endl;
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
			return dstImg;
		}
		/// Add empty alphaSrc channel
		mout.special() << "dst image/plain: " << dstImg << mout.endl;
		dstImg.setAlphaChannelCount(1);
		mout.special() << "dst image+alpha: " << dstImg << mout.endl;

		mout.debug() << "image:"       << dstImg << mout.endl;

		return dstImg;

	}

};



// Adds alpha channel containing current data.
class CmdImageAlpha : public CmdImageAlphaBase {

public:

	CmdImageAlpha() : CmdImageAlphaBase(__FUNCTION__, "Adds a transparency channel. Implies additional image, creates one if needed. See --encoding") {
	};

	CmdImageAlpha(const CmdImageAlpha & cmd) : CmdImageAlphaBase(cmd) {
		//parameters.copyStruct(cmd.parameters, cmd, *this);
	};

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = resources.mout; = resources.mout;

		const PlainData<PolarSrc> src(getAlphaSrc(ctx));

		drain::image::Image & dstImg = getDstImage(ctx);
		/*
		drain::image::Image & dstImg = ctx.getModifiableImage();
		if (dstImg.isEmpty()){
			mout.warn() << "could not get ModifiableImage" << mout.endl;
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
			return;
		}
		/// Add empty alphaSrc channel
		dstImg.setAlphaChannelCount(1);
		mout.debug() << "image:"       << dstImg << mout.endl;
		//mout.debug() << "imageAlpha:"  << img.getAlphaChannel() << mout.endl;
		*/

		DataConversionOp<ODIM> copier; //(copierHidden);
		copier.odim.addShortKeys();
		copier.odim.setValues(ctx.targetEncoding);
		ctx.targetEncoding.clear();

		//const std::string type(1, drain::Type::getTypeChar(img.getType())); // rewrite
		const std::string type = drain::Type(dstImg.getType()); // rewrite
		if (copier.odim.type != type){
			mout.note() << " using the type of base image: " << type << mout.endl;
		}
		copier.odim.type = type; //drain::Type::getTypeChar(img->getType());

		//mout.debug3() << "alphaSrcODIM:  "  <<  alphaSrcODIM << mout.endl;
		mout.debug3() << "odimOut: " << copier.odim << mout.endl;
		mout.debug3() << "copier: " << copier << mout.endl;
		copier.traverseImageFrame(src.odim,  src.data, copier.odim, dstImg.getAlphaChannel());

		//copier.traverseImageFrame(srcODIM,  srcAlpha, copier.odim, dstImg.getAlphaChannel());

		// ???
		dstImg.properties["what.scaleAlpha"]  = copier.odim.scaling.scale;
		dstImg.properties["what.offsetAlpha"] = copier.odim.scaling.offset;

	}
};

// Adds alpha channel containing current data.
class CmdImageTransp : public CmdImageAlphaBase {

public:

	//drain::Range<double> range;
	std::string ftor;
	double undetect;
	double nodata;
	// std::vector<double> kokkeilu;

	CmdImageTransp() : CmdImageAlphaBase(__FUNCTION__, "Adds a transparency channel. Uses copied image, creating one if needed.") {
		//parameters.link("range", range.tuple(), "min:max").fillArray = true;
		parameters.link("ftor", ftor, "min:max or Functor_p1_p2_p3_...");
		parameters.link("undetect", undetect=0, "opacity of 'undetect' pixels");
		parameters.link("nodata",   nodata=1, "opacity of 'nodata' pixels"); // std::numeric_limits<double>::max()
		// range.min =  -std::numeric_limits<double>::max();
		// range.max =  +std::numeric_limits<double>::max();
		// std::cerr << __FUNCTION__ << ' '; parameters.dump(std::cerr);
	};

	CmdImageTransp(const CmdImageTransp & cmd) : CmdImageAlphaBase(cmd) {
		parameters.copyStruct(cmd.parameters, cmd, *this);
		//std::cerr << __FUNCTION__ << " (copy)"; parameters.dump(std::cerr);
	};


	void exec() const {

		// parameters.dump();

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = resources.mout; = resources.mout;

		mout.warn() << getName() << mout.endl;

		//const PlainData<PolarSrc> src(getAlphaSrc(ctx));
		// TODO: const Data<PolarSrc>, mixing data with quality
		const drain::image::Image & srcAlpha = ctx.getCurrentGrayImage();
		ODIM srcODIM(srcAlpha); // NOTE: perhaps no odim data in props?

		mout.special() << "alpha src image: " << srcAlpha << mout.endl;

		// Dst image (typically, an existing coloured image)
		drain::image::Image & dstImg = getDstImage(ctx);
		/*
		 * OLD
		 *
		drain::image::Image & dstImg = ctx.getModifiableImage(); //ImageKit::getModifiableImage(ctx);
		if (dstImg.isEmpty()){
			mout.warn() << "could not get ModifiableImage" << mout.endl;
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
			return;
		}
		mout.special() << "dst image/plain: " << dstImg << mout.endl;
		dstImg.setAlphaChannelCount(1);
		mout.special() << "dst image+alpha: " << dstImg << mout.endl;
		// dstImg.getAlphaChannel().fill(128);
		//dstImg.setAlphaChannelCount(0);
		//mout.special() << "dst image+alpha: " << dstImg << mout.endl;
		//return;
		*/


		RadarFunctorBase radarFtor;
		//radarFtor.LIMIT = true;
		//fu zzyStep.odimSrc.updateFromMap(src.data.getProperties());
		radarFtor.odimSrc.updateFromMap(srcAlpha.getProperties());

		const double dstMax = drain::Type::call<drain::typeNaturalMax>(dstImg.getType()); // 255, 65535, or 1.0
		drain::typeLimiter<double>::value_t limit = dstImg.getConf().getLimiter<double>();  // t is type_info, char or std::string.
		//fuzzyStep.nodataValue   = limit(dstMax*nodata);
		//fuzzyStep.undetectValue = limit(dstMax*undetect);
		radarFtor.nodataValue   = limit(dstMax*nodata);
		radarFtor.undetectValue = limit(dstMax*undetect);


		std::string s1;
		std::string s2;
		drain::StringTools::split2(ftor, s1, s2, "_");
		//std::list<std::string> l;
		//drain::StringTools::split(ftor, l, "_");

		const drain::FunctorBank & functorBank = drain::getFunctorBank();

		if (functorBank.has(s1)){

			drain::UniCloner<drain::UnaryFunctor> localBank(functorBank);

			//mout.special() << "list " << drain::sprinter(l) << mout;

			drain::UnaryFunctor & ftor2 = localBank.getCloned(s1);
			//l.pop_front();

			ftor2.setParameters(s2, '=', '_');
			mout.special() << "functor: " << ftor2 << mout;

			radarFtor.apply(srcAlpha.getChannel(0), dstImg.getAlphaChannel(), ftor2, true);

		}
		else {

			if (!s2.empty()) {
				mout.warn()  << functorBank << mout;
				mout.warn()  << "use range <min>:<max> or functors above, params separated with '_'" << mout;
				mout.error() << "unknown functor: " << s1 << mout;
				return;
			}
			//std::list<std::string> params;

			drain::Range<double> range;
			//drain::Referencer r(range.tuple());
			//r.setSeparator(':').setFill() = ftor;
			drain::Referencer(range.tuple()).setSeparator(':').setFill().assignString(ftor);

			mout.special() << "range: " << range << mout;

			drain::FuzzyStep<double> fuzzyStep;

			//fuzzyStep.setParameters(p, assignmentSymbol, separatorSymbol)
			/*
			RadarFunctorOp<drain::FuzzyStep<double> > fuzzyStep;
			fuzzyStep.LIMIT = true;
			fuzzyStep.odimSrc.updateFromMap(srcAlpha.getProperties());
			*/

			if (ctx.imagePhysical){
				mout.info() << "using physical scale " << mout;
				//fuzzyStep.functor.set(range.min, range.max, 1.0);
				//fuzzyStep.setParameters(range, 1.0);
				//fuzzyStep.set(range.min, range.max, 1.0);
			}
			else {
				//const drain::ValueScaling & scaling = srcImg.getScaling();
				// EncodingODIM(src.odim)
				mout.info() << "no physical scaling, using raw values of gray source: "  <<  srcAlpha.getConf()  << mout;
				//const double max = drain::Type::call<drain::typeNaturalMax>(src.data.getType()); // 255, 65535, or 1.0
				const double max = drain::Type::call<drain::typeNaturalMax>(srcAlpha.getType()); // 255, 65535, or 1.0
				//fuzzyStep.functor.set(srcAlpha.getConf().fwd(max*range.min), srcAlpha.getConf().fwd(max*range.max), 1.0);
				const ImageConf & ac = srcAlpha.getConf();
				// fuzzyStep.functor.set(ac.fwd(max*range.min), ac.fwd(max*range.max), 1.0);
				//fuzzyStep.set(ac.fwd(max*range.min), ac.fwd(max*range.max), 1.0);
				range.min = ac.fwd(max*range.min);
				range.max = ac.fwd(max*range.max);
				// fuzzyStep.set(ac.fwd(max*range.min), ac.fwd(max*range.max), 1.0);
				// mout.info() << "new range: " << fuzzyStep.range << mout;
				// mout.info() << "range: " << fuzzyStep.functor.range << mout.endl;
			//	mout.info() << "new range: " << fuzzyStep.range << mout;
			}

			//fuzzyStep.setParameters(range, 1.0);
			fuzzyStep.set(range, 1.0);

			mout.debug() << "fuzzy: "  << fuzzyStep << mout.endl;

			radarFtor.apply(srcAlpha.getChannel(0), dstImg.getAlphaChannel(), fuzzyStep, true);

		}

		// dstImg has unknowns source -> encoding will not be written in metadata

		//fuzzyStep.traverseChannel(srcAlpha.getChannel(0), dstImg.getAlphaChannel());

		//mout.warn() = this->getParameters();

	}

};



class CmdImageFlatten : public drain::SimpleCommand<std::string> {

public:

	CmdImageFlatten() : drain::SimpleCommand<>(__FUNCTION__, "Removes a alpha (transparency) channel. Adds a new background of given color.",
			"bgcolor", "0", "<gray>|<red>,<green>,<blue>") {
	};

	void exec() const {

		drain::Logger mout(__FILE__, __FUNCTION__); // = resources.mout;

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::image::Image & img = ctx.getModifiableImage(); //ImageKit::getModifiableImage(ctx);
		if (img.isEmpty()){
			mout.warn() << "could not get ModifiableImage" << mout.endl;
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
			return;
		}


		if (img.getAlphaChannelCount() == 0) {
			mout.warn() << name << ": no alpha channel" << mout.endl;
			return;
		}

		drain::image::ImageFrame & imgAlpha = img.getAlphaChannel();

		const double alphaMax = imgAlpha.getConf().getTypeMax<double>();
		if (alphaMax == 0) {
			mout.warn() << name << ": zero alphaMax" << mout.endl;
			return;
		}

		double alpha;

		drain::Variable colorVector(typeid(size_t));
		// colorVector.setType(typeid(size_t));
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
////static drain::CommandEntry<CmdImageFlatten> cmdImageFlatten("imageFlatten");

// See also CmdPaletteIn and CmdPaletteOut in imageOps?
class CmdPalette : public drain::SimpleCommand<std::string> {

public:

	CmdPalette() : drain::SimpleCommand<std::string>(__FUNCTION__, "Load and apply palette.", "filename", "", "<filename>.[txt|json]") {
	};


	static
	void retrieveQuantity(RackContext & ctx, std::string & quantity){

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		const drain::image::Image & srcImage = ctx.getCurrentGrayImage();
		if (!srcImage.isEmpty()){
			quantity = srcImage.getProperties().get("what:quantity", "");
			if (!quantity.empty())
				mout.special() << "ctx.currentGrayImage: quantity=" << quantity << mout.endl;
		}

		if (quantity.empty()){
			drain::VariableMap & statusMap = ctx.getStatusMap(); // Note: no image created
			//ctx.updateImageStatus(statusMap);
			quantity = statusMap["what:quantity"].toStr();
			if (!quantity.empty())
				mout.ok() << "ctx.statusMap quantity=" << quantity << mout.endl;
		}

	}


	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = resources.mout;

		if (ctx.statusFlags.isSet(drain::StatusFlags::INPUT_ERROR)){
			mout.warn() << "input failed, skipping" << mout.endl;
			return;
		}

		if (ctx.statusFlags.isSet(drain::StatusFlags::DATA_ERROR)){   // !resources.dataOk){
			mout.warn("data error, skipping");
			return;
		}

		// No explicit quantity issued
		const bool DEFAULT_PALETTE = (value.empty() || (value == "default"));

		if (DEFAULT_PALETTE && !ctx.paletteKey.empty()){
			mout.info("Using current palette: ", ctx.paletteKey);
		}
		else {
			// Read (get) a palette? (Ie. change current palette
			//if ((!value.empty()) || ctx.paletteKey.empty()){
			//if (!value.empty() || ctx.palette.empty()){

			if (DEFAULT_PALETTE){

				// Guess label (quantity) Allow load even if no image
				std::string quantity;
				/// Minimum total effort... try selecting image already.
				retrieveQuantity(ctx, quantity);

				// NEW 2023/05/10 : consider DBZH:RESIZE
				std::string processing;
				drain::StringTools::split2(quantity, quantity, processing, "|:>");
				// if ..
				mout.special("src [", quantity, "] : processing(", processing, ")");

				if (!quantity.empty()){
					mout.info("loading palette: ", quantity);
					//ctx.palette.load(quantity, true);
					ctx.getPalette(quantity);
				}
				else {
					mout.fail("could not derive data quantity (quantity empty)");
				}
			}
			else {
				ctx.getPalette(value);
				// ctx.palette.load(value, true);
			}


			/// TODO: store palette in dst /dataset, or better add palette(link) to Image ?
			/*
			mout.warn() << "palette will be also saved to: " << ctx.currentPath << mout.endl;
			Hi5Tree & dst = (*ctx.currentHi5);
			dst(ctx.currentPath).data.attributes["IMAGE_SUBCLASS"] = "IMAGE_INDEXED";
			*/
		}

		apply(ctx);

	}

	static
	void apply(RackContext & ctx){

		drain::Logger mout(ctx.log, __FUNCTION__, "Palette"); // = resources.mout;

		//drain::image::Palette & palette = ctx.palette; // FAKE old (fallback)
		drain::image::Palette & palette = ctx.getPalette(); // NEW

		if (palette.empty()){
			mout.warn("empty palette, giving up");
			return;
		}

		const drain::image::Image & graySrc = ctx.getCurrentGrayImage(); // ImageKit::getCurrentGrayImage(ctx);  // no conversion
		ctx.select = "";

		if (graySrc.isEmpty()){
			if (!ctx.currentHi5->empty()){
				mout.fail("hdf5 data exists, but selected gray image is empty, skipping.");
				ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR); // selector failed
			}
			else {
				mout.info("hdf5 data empty");
			}
			return;
		}
		//graySrc.getConf().getElementSize()


		if (palette.refinement > 0){
			// TODO: Refine for PaletteBank::geCloned()
			mout.note("Applying refinement: ", palette.refinement);
			palette.refine();
		}


		PaletteOp  op(palette);

		// The intensities will  be mapped first: f' = gain*f + offset
		const drain::FlexVariableMap  & props = graySrc.getProperties();
		mout.debug() << "input properties: " << props << mout.endl;
		/*
		const drain::FlexVariable & nodata = props["what:nodata"];
		double d = nodata;
		mout.special() << "props: ";
		mout.precision(20);
		mout << nodata << '=';
		mout.precision(20);
		mout << d << ':' <<  drain::Type::call<drain::complexName>(nodata.getType()) << mout;
		*/

		/// Principally ODIM needed, but PolarODIM contains Nyquist velocity information, if needed.
		//const PolarODIM imgOdim(props);
		const PolarODIM imgOdim(graySrc); // Uses Castable, so type-consistent
		mout.info("input encoding: ", EncodingODIM(imgOdim));


		if (imgOdim.quantity.substr(0,4) != "VRAD"){
			op.scale   = imgOdim.scaling.scale;   // props.get("what:gain", 1.0);
			op.offset  = imgOdim.scaling.offset; // props.get("what:offset",0.0);
		}
		else { // rescale relative to NI (percentage -100% ... +100%)

			const double NI = imgOdim.getNyquist(); //props["how:NI"];
			if (NI != 0.0){
				mout.special() << "Doppler speed (VRAD), using relative scale, NI=" << NI << " orig NI=" << props["how:NI"] << mout.endl;
				/*
					f(data_min) = scale*data_min + offset == -1.0
					f(data_max) = scale*data_max + offset == +1.0
					scale = (+1 - -1) / ( data_max-data_min )
					offset = scale*(data_min+data_max) / 2.0
				*/
				const double data_min = imgOdim.scaleInverse(-NI);
				const double data_max = imgOdim.scaleInverse(+NI);
				op.scale  =   2.0/(data_max-data_min);
				op.offset = - op.scale*(data_max+data_min)/2.0;
				mout.debug() << "expected storage value range: " << data_min << '-' << data_max << mout.endl;
			}
			else {
				mout.fail() << "No Nyquist velocity (NI) found in metadata." << mout.endl;
			}
		}

		// mout.debug() <<  imgOdim << mout.endl;
		op.registerSpecialCode("nodata",   imgOdim.nodata);    // props["what:nodata"]);
		op.registerSpecialCode("undetect", imgOdim.undetect); // props["what:undetect"]);

		// mout.note() << imgOdim << mout.endl;
		/*
		mout.warn() << "OP Special codes: \n";
		mout.precision(20);
		mout << op.specialCodes << mout.endl;

		mout.warn() << "imgOdim.nodata: \n";
		mout.precision(20);
		mout << imgOdim.nodata << mout.endl;

		mout.warn() << "First pixel: \n";
		mout.precision(20);
		mout << graySrc.get<double>(0) << mout.endl;
		*/

		ODIM encoding;
		//encoding.type = "C";
		encoding.setTypeDefaults("C");
		if (!ctx.targetEncoding.empty()){ // does not check if an encoding change requested, preserving quantity?
			encoding.addShortKeys();
			encoding.updateValues(ctx.targetEncoding); // do not clear yet
			mout.note() << "target quantity: " << encoding.quantity << mout.endl;
			//mout.note() << "target type:     " << encoding.type << mout.endl;
			if (encoding.type != "C"){
				mout.warn() << "currently, only char type [C] supported, rejecting: " << encoding.type << mout.endl;
				encoding.setTypeDefaults("C");
			}
			//ctx.targetEncoding.clear();
		}


		mout.note("params: ", op.getParameters());

		// If user has not explicitly set a quantity, assume RGB image temporary. (Excluded in HDF write)
		const bool NO_SAVE = encoding.quantity.empty();

		drain::VariableMap & statusMap = ctx.getStatusMap();
		//statusMap["command"] = op.getName();
		//ctx.lastCmd? args?
		//statusMap["commandArgs"] = ctx.palette.title; // would better be: bevalue;

		// NOTE: this should be similar with imageOps remapping. Consider shared function?
		std::string dstQuantity; // NEW 2023
		std::string srcQuantity;
		retrieveQuantity(ctx, srcQuantity);

		// NEW 2023/05/10
		/*
		std::string processing;
		drain::StringTools::split2(srcQuantity, srcQuantity, processing,':');
		mout.special("src [", srcQuantity, "] : processing(", processing, ")");
		*/

		// NEW 2023
		drain::StringMapper quantitySyntaxMapper(RackContext::variableMapper);
		if (!encoding.quantity.empty()){
			quantitySyntaxMapper.parse(encoding.quantity); // Can be literal ("DBZH") or variable ("{what:quantity}_NEW")
		}
		else {
			quantitySyntaxMapper.parse(ImageContext::outputQuantitySyntax);
		}
		statusMap["what:quantity"] = srcQuantity; // override...
		dstQuantity = quantitySyntaxMapper.toStr(statusMap);
		if (srcQuantity != dstQuantity)
			mout.special("src[", srcQuantity, "] -> dst[", dstQuantity, "]");

		/*
		if (encoding.quantity.empty()){
			std::string srcQuantity;
			retrieveQuantity(ctx, srcQuantity);
			// New ...
			drain::StringMapper quantitySyntaxMapper(RackContext::variableMapper); // copy constr, ensures accepting variables with ':', like {what:quantity}
			quantitySyntaxMapper.parse(ImageContext::outputQuantitySyntax);
			statusMap["what:quantity"] = srcQuantity; // override...
			encoding.quantity = quantitySyntaxMapper.toStr(statusMap);
			mout.special() << "automatic quantity: " <<  encoding.quantity << mout;
			//dstQuantity = quantity + "/Palette";
		}
		 */

		//if (encoding.quantity.empty()){ // NOW: never true
		if (false){

			mout.warn("Using separate image (ctx.colorImage)");

			/// MAIN
			op.process(graySrc, ctx.colorImage);
			ctx.colorImage.properties = props;
			//File::write(ctx.colorImage, "color.png");
			ctx.setCurrentImageColor(ctx.colorImage);
			// ctx.currentImage = & ctx.colorImage;
		}
		else {
			/// Create new quantity in data structure

			Hi5Tree & dst = ctx.getHi5(RackContext::CURRENT);

			ODIMPathElem elem;
			ctx.guessDatasetGroup(dst, elem);


			DataSet<BasicDst> dstProduct(dst[elem]);
			//Data<BasicDst> & data = dstProduct.getData(encoding.quantity);
			const size_t origSize = dstProduct.size();
			Data<BasicDst> & data = dstProduct.getData(dstQuantity); // NEW 2023/03 !
			data.setNoSave(NO_SAVE);

			if (origSize == dstProduct.size()){
				mout.note("Storing processed image in: ", elem, "/data?/ [", dstQuantity, ']'); // NEW 2023
			}
			else {
				mout.note("Storing processed image in: ", elem, "/data", dstProduct.size(),"/ [", dstQuantity, ']'); // NEW 2023
			}

			if (NO_SAVE){
				mout.advice("use explicit --encoding quantity={...} (before) or --keep ", elem, ":/data: (after) to include colour image in saving HDF5.");
			}

			mout.debug("target encoding: ", EncodingODIM(encoding));

			data.odim.updateFromMap(graySrc.getProperties());
			// data.odim.updateFromCastableMap(encoding);
			data.odim.quantity = dstQuantity; // NEW 2023 encoding.quantity;
			data.odim.type = "C";
			data.setEncoding("C");
			//data.odim.scaling.setScale(1, 0);
			data.data.setScaling(data.odim.scaling);
			// mout.note() << "target encoding:     " << EncodingODIM(data.odim) << mout.endl;
			// data.setEncoding("C");
			// if (data.odim.type)


			// This step only for storage type (quantity (other easily discarded).

			//ProductBase::completeEncoding(data.odim, ctx.targetEncoding);
			//data.setEncoding(data.odim.type);
			//data.data.setScaling(data.odim.scaling);
			//data.data.setScaling(128.0, 120); // RGB = unitless.
			//data.odim.quantity = encoding.quantity;

			mout.debug() << "target: " << data << mout;

			/// MAIN
			op.process(graySrc, data.data);

			// Currently, copying is possible only afterwards
			if (graySrc.hasAlphaChannel()){
				mout.experimental() << "copying alpha channel " << mout;
				//data.data.setGeometry(graySrc.getGeometry(), 3, 1);
				data.data.setAlphaChannelCount(1);
				data.data.getAlphaChannel().copyData(graySrc.getAlphaChannel());
			}



			size_t channels = data.data.getChannelCount();
			if (channels == 0){
				mout.warn() << "operation failed, result has 0 channels" << mout.endl;
				ctx.statusFlags.set(drain::StatusFlags::COMMAND_ERROR);
				ctx.unsetCurrentImages();
			}
			else if (channels >= 3){
				//mout.note() << "stored color image in HDF5 structure: " << elem << '[' << dstQuantity << ']' << mout.endl;

				if (channels > 3){
					mout.warn() << "only 3 channels (RGB) of " << channels << " will be stored in HDF5 files" << mout.endl;
				}
				ctx.setCurrentImageColor(data.data);
			}
			else { //if (channels >= 1){
				if (channels > 1){
					mout.warn() << "only first channel of " << channels << " stored" << mout.endl;
				}
				//mout.note() << "also currentGrayImage updated " << mout.endl;
				ctx.setCurrentImages(data.data);
			}

			//ctx.currentImage = & data.data;
			// d.odim.setGeometry(d.data.getWidth(), d.data.getHeight());
		}

		ctx.targetEncoding.clear();
	}
};


class CmdPaletteConf : public drain::BasicCommand  {

public:

	CmdPaletteConf() : drain::BasicCommand(__FUNCTION__, "Check status of palette(s)."), lenient(true) {
		parameters.link("key", key="list");
		parameters.link("code", code);
		parameters.link("lenient", lenient);
		// parameters.link("quantity", quantity = "");
	};

	CmdPaletteConf(const CmdPaletteConf & cmd) : drain::BasicCommand(cmd), lenient(true){
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	};

	void reset(){
		key = "list";
		code.clear();
		lenient = true;

	}

	mutable
	drain::image::PaletteEntry paletteEntry;

	std::string key;
	std::string code;
	bool lenient;


	virtual
	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		if (key == "list"){
			for (const auto & entry: drain::image::PaletteOp::getPaletteMap()){
				mout.note(entry.first, " <=> ", entry.second.title);
			}
		}
		else {
			mout.note("Palette: [", key, "]");
			drain::image::Palette & palette = ctx.getPalette(key);
			mout.note("  title:   '", palette.title, "'");
			if (code.empty()){
				mout.note("  title:   '", palette.title, "'");
				mout.note("  entries: ", palette.size(), "");
				mout.note("  specials: ", palette.specialCodes.size(), "");
			}
			else {
				drain::image::PaletteEntry::value_t v = 0.0;
				try {
					v = palette.getValueByCode(code, lenient);
				}
				catch (std::exception & e) {
					mout.warn("  no such code: ", code, "");
					if (!lenient){
						try {
							v = palette.getValueByCode(code, true);
							mout.advice("  however, lenient matching found: ", code);
						}
						catch (std::exception & e) {
							return;
						}
					}
				}
				mout.note("arg: [", code, "] => value=", v, " => ", palette[v].code);
			}
		}

	};

};

class CmdPaletteIn : public drain::SimpleCommand<std::string> {

public:

	CmdPaletteIn() : drain::SimpleCommand<std::string>(__FUNCTION__, "Load palette.", "filename", "", "<filename>.[txt|json]") {
	};

	virtual
	void exec() const {
		load(value);
	};

	void load(const std::string &s) const {

		static
		const drain::RegExp re("^[A-Z][A-Z_0-9\\-]+[A-Z0-9]$"); // Nearly duplicate code, see Palette::load

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		//ctx.palette.load(s);

		// NEW
		if (re.test(s)){
			mout.note("Checking palette [", s,"]");
			ctx.getPalette(s).load(s, true);
		}
		else {
			mout.unimplemented("No quantity extraction yet... (filename: ", s, ")");
			mout.warn("Reading generic palette (quantity unset)");
			// drain::image::PaletteOp::paletteMap["USER");
			ctx.getPalette("").load(s); // ctx.paletteKey = "USER";
		}
	}

};

class CmdPaletteOut : public drain::SimpleCommand<> {

public:

	CmdPaletteOut() : drain::SimpleCommand<>(__FUNCTION__, "Save palette as TXT, JSON or SVG.", "filename", "") {
	};

	void exec() const {

		drain::Logger mout(__FILE__, __FUNCTION__); // = resources.mout;

		RackContext & ctx = getContext<RackContext>();

		mout.attention("Palette key: ", ctx.paletteKey);

		const drain::image::Palette & palette = ctx.getPalette();

		if (ctx.formatStr.empty()){
			palette.write(ctx.outputPrefix + value);
		}
		else {
			mout.warn("user defined format, extension not checked: ", ctx.formatStr);
			drain::Output output(ctx.outputPrefix + value);
			palette.exportFMT(output, ctx.formatStr);
		}

	}
};

class CmdPaletteRefine : public drain::SimpleCommand<int> {

public:

	CmdPaletteRefine() : drain::SimpleCommand<int>(__FUNCTION__, "Refine colors", "count", 0){
	};

	void exec() const {

		drain::Logger mout(__FILE__, __FUNCTION__); // = resources.mout;

		RackContext & ctx = getContext<RackContext>();

		/* OLD
		if (ctx.palette.empty()){
			mout.note("Palette not (yet) loaded, ok");
			ctx.palette.refinement = value;
			return;
		}

		ctx.palette.refine(value);
		*/

		// NEW
		if (ctx.paletteKey.empty()){
			mout.warn("Palette not loaded, refine will ne discarded!");
			return;
		}

		ctx.getPalette().refine(value);

		// cmdPalette.apply();
		// mout.unimplemented()= "refine";

		CmdPalette::apply(ctx);

	}

};


/// Quality groups accepted as alpha channels
/**
 *
 */



class CmdImageQuality : public drain::BasicCommand { // public drain::SimpleCommand<std::string> {

public:

	//  drain::SimpleCommand<std::string>
	CmdImageQuality() :  drain::BasicCommand(__FUNCTION__, "Applied parent groups for quality: dataset:data"){
		// , "groups", "None"
		parameters.link("groups", groups = "dataset:data");
		// parameters.link("quantity", quantity = "");
	};

	CmdImageQuality(const CmdImageQuality & cmd) : drain::BasicCommand(cmd){
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	}

	std::string groups;
	//std::string quantity; // future option? but Data<src> needs redesign


	virtual
	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		// ctx.qualitySelector.quantity = quantity;
		// ctx.qualityGroups.flags.reset(); // TODO reset() .clear();
		ctx.qualityGroups = 0;

		/// Clumsy assignment of /dataset<n> and /data<n> groups.
		std::list<std::string> l;
		drain::StringTools::split(groups, l, ",:"); // future: maybe , needed for second arg (quantity=QIND)

		for (const std::string & s: l){
			ODIMPathElem elem;
			elem.extractPrefix(s, true);

			ctx.qualityGroups = ctx.qualityGroups | elem.getType();

		}
		// matcher.flags.assign(value);
		// matcher.flags.value = v; // OK!
		mout.special(ctx.qualityGroups, '#' );
		//mout.special() << ctx.qualityGroups << '#' << ctx.qualityGroups.flags << '=' << ctx.qualityGroups.flags.value << mout;
		//mout.special() << matcher << '=' << matcher.flags << '=' << matcher.flags.value << mout;
		//ctx.qualitySelector.pathMatcher.setElems(matcher);
		//mout.warn() << ctx.qualitySelector << mout;

		// DEBUG
		//for (const std::string & s: {"", "dataset1", "dataset5", "data2", "data5"}) {
		for (const char *s: {"", "dataset1", "dataset5", "data2", "data5"}) {
			//mout.special() << s << "?\t" << ctx.quality....Matcher.match(s) << mout;
			ODIMPathElem elem(s);
			mout.special(s, "?\t", elem.belongsTo(ctx.qualityGroups));
		}

		//}

	}


};


class CmdImageBox : public drain::BeanCommand<drain::image::ImageBox> {
public:

	CmdImageBox(){
	};

	void exec() const {
		drain::Logger mout(__FILE__, __FUNCTION__); // = resources.mout;

		RackContext & ctx = getContext<RackContext>();

		if (!ctx.select.empty())
			ctx.unsetCurrentImages();

		drain::image::Image & dst = ctx.getModifiableImage(); //ImageKit::getModifiableImage(ctx);
		if (dst.isEmpty()){
			mout.warn() << "could not get ModifiableImage" << mout.endl;
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
			return;
		}

		bean.process(dst);
	}
};








class CmdPlot : public drain::SimpleCommand<std::string> {
public:

	CmdPlot() : drain::SimpleCommand<std::string>(__FUNCTION__, "Plot", "i,j,...", "0,0"){
	};

	void exec() const {

		drain::Logger mout(__FILE__, __FUNCTION__); // = resources.mout;

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		ODIMPathList paths;
		//std::map<std::string,std::string> m;
		DataSelector selector; //("data[0-9]+$");
		selector.consumeParameters(ctx.select);
		selector.pathMatcher.set(ODIMPathElem::DATA); // | ODIMPathElem::QUALITY);
		mout.debug() << "selector: " << selector << mout.endl;

		//  selector.getPaths(*ctx.currentHi5, paths); // RE2 // todo getFirstData
		//DataSelector::getPathsByQuantity(*ctx.currentHi5, selector, m); // key==quantity, so only one (last) path obtained
		Hi5Tree & src = ctx.getHi5(RackContext::CURRENT);
		//selector.getPaths(*ctx.currentHi5, paths); //, ODIMPathElem::DATA);
		selector.getPaths(src, paths); //, ODIMPathElem::DATA);

		//size_t count = m.size();
		size_t count = paths.size();
		mout.info() << "found: " << count << " paths " << mout.endl;
		drain::image::ImageTray<Channel> tray;
		//size_t index = 0;
		//for (std::map<std::string,std::string>::const_iterator it = m.begin(); it != m.end(); ++it){
		for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); ++it){
			//mout.note() << "selected: " << it->first << '=' << it->second << mout.endl;
			mout.debug() << "dst image: " << *it << '|' << ODIMPathElem::ARRAY << mout.endl;
			Hi5Tree & dst = src(*it);
			//drain::image::Image & channel = (*ctx.currentHi5)(it->second)["data"].data.dataSet;
			drain::image::Channel & channel = dst[ODIMPathElem::ARRAY].data.dataSet.getChannel(0);
			if (channel.isEmpty()){
				mout.warn() << "empty image in " << *it << "/data, skipping" << mout.endl;
				continue;
			}
			tray.appendImage(channel);
			mout.debug2() << channel << mout.endl;
			//std::string path(*it);


			ODIMPath path;
			mout.unimplemented() << "note: checking parallel quality unimplemented" << mout.endl;
			DataSelector qualityDataSelector;
			qualityDataSelector.pathMatcher.set(ODIMPathElem::QUALITY, ODIMPathElem::ARRAY);
			if (qualityDataSelector.getPath(dst, path)){
			//if (DataSelector::getQualityPath(*ctx.currentHi5, path)){
				//path << ODIMPathElem(ODIMPathElem::ARRAY);
				mout.special() << "associated quality field for ["<< *it << "] found in " << path << mout.endl;
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




ImageModule::ImageModule(drain::CommandBank & bank) : module_t(bank) { // :{ //  : CommandSection("images"){

	//drain::CommandBank::trimWords().insert("Op");

	/// Legacy commands with plain names
	// drain::CommandInstaller<0,ImageSection> installer(drain::getCommandBank());
	install<CmdImage>();
	install<CmdPalette>();
	install<CmdImageAlpha>();
	install<CmdImageTransp>();
	install<CmdPaletteOut>("legendOut"); // Same as --iPaletteOut below
	install<CmdImageFlatten>();
	install<CmdImagePhysical>("iPhysical"); // perhaps should be imagePhysical!
	install<CmdImagePhysical>("imagePhysical");
	install<CmdImageQuality>();

	/// WAS: with prefix 'i', like image operators
	// drain::CommandInstaller<'i',ImageSection> installer2(drain::getCommandBank());
	install<CmdPaletteConf>();
	install<CmdPaletteIn>();
	install<CmdPaletteOut>();
	install<CmdPaletteRefine>();
	install<CmdPlot>();
	install<CmdImageBox>();


	PaletteManager::getMap();





};


} // namespace rack

