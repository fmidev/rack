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

//#include <exception>
#include <drain/image/ImageFile.h>
#include <fstream>
#include <iostream>
/*
#include <algorithm>
#include <limits>
#include <list>
#include <map>
#include <sstream>
#include <utility>
#include <vector>
 */
//#include <regex.h>
// #include <stddef.h>

#include "drain/util/Log.h"
#include "drain/util/FilePath.h"
#include "drain/util/Output.h"
#include "drain/util/StringMapper.h"
#include "drain/util/Tree.h"
#include "drain/util/Variable.h"
#include "drain/image/FilePng.h"
#include "drain/image/FilePnm.h"
#include "drain/image/FileGeoTIFF.h"
//#include "radar/FileGeoTIFF.h"

#include "drain/image/Image.h"
//#include "drain/image/Sampler.h"
#include "drain/imageops/ImageModifierPack.h"

#include "drain/prog/Command.h"
#include "drain/prog/CommandBankUtils.h"
#include "drain/prog/CommandInstaller.h"

#include "data/Data.h"
//#include "data/DataOutput.h"
#include "data/DataSelector.h"
#include "data/DataTools.h"
#include "data/ODIMPath.h"
#include "data/PolarODIM.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h"
#include "product/ProductOp.h"
//#include "radar/RadarDataPicker.h"

#include "resources.h"
#include "fileio.h"
#include "fileio-read.h"
#include "fileio-geotiff.h"
#include "file-hist.h"


namespace rack {

/// Syntax for recognising hdf5 files.
//  Edited 2017/07 such that also files without extension are considered h5 files. BALTRAD bug

//const drain::RegExp h5FileExtension("^((.*\\.(h5|hdf5|hdf))|((.*/)?[\\w]+))$",  REG_EXTENDED | REG_ICASE);
//const drain::FileInfo fileInfoH5("tif|tiff");
//const drain::RegExp noExtension("^((.*/)?[\\w]+)$");


/// Syntax for recognising GeoTIFF files.
//const drain::RegExp tiffFileExtension(".*\\.(tif|tiff)$",  REG_EXTENDED | REG_ICASE);

//const drain::FileInfo fileInfoTIFF("tif|tiff");


/// Syntax for recognising image files (currently, png supported).
//const drain::RegExp pngFileExtension(".*\\.(png)$", REG_EXTENDED | REG_ICASE);

/// Syntax for recognising image files (currently, png supported).
//const drain::RegExp pnmFileExtension(".*\\.(p[nbgp]m)$", REG_EXTENDED | REG_ICASE); //[nbgp]

/// Syntax for recognising text files.
const drain::RegExp textFileExtension(".*\\.(txt)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising numeric array files (in plain text format anyway).
const drain::RegExp arrayFileExtension(".*\\.(mat)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for sparsely resampled data.
const drain::RegExp sampleFileExtension(".*\\.(dat)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising Graphviz DOT files.
const drain::RegExp dotFileExtension(".*\\.(dot)$",  REG_EXTENDED | REG_ICASE);


//static DataSelector imageSelector(".*/data/?$","");   // Only for images. Not directly accessible.
//static DataSelector imageSelector;  // Only images. Not directly accessible. Consider that of images.h








class CmdOutputConf : public drain::BasicCommand {

public:

	CmdOutputConf() : drain::BasicCommand(__FUNCTION__, "Format specific configurations") {

		parameters.separator = ':';
		parameters.link("format", format, "h5|png|tif");
		parameters.link("params", params, "<key>=<value>[,<key2>=<value2>,...]");

		gtiffConf.link("tilewidth", FileTIFF::defaultTile.width=256);
		gtiffConf.link("tileheight", FileTIFF::defaultTile.height=0);
		gtiffConf.link("compression", FileTIFF::defaultCompression, FileTIFF::getCompressionDict().toStr('|'));

	};

	CmdOutputConf(const CmdOutputConf & cmd) : drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	}

	std::string format;
	std::string params;

	/*
	virtual inline
	void set Parameters(const std::string & params, char assignmentSymbol=0) {
		 drain::BasicCommand::setParameters(params, 0); // sep = ':'
	}
	 */

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		mout.warn() << "Redesigned (setParameters dropped). Check results." << mout.endl;
		// mout.warn() << format << mout.endl;
		// mout.note() << params << mout.endl;
		// todo: shared resource for output conf:  refMap of refMaps...
		// todo recognize tif,TIFF
		if (format == "h5"){
			mout.unimplemented() << "(future extension)" << mout.endl;
		}
		else if (format == "png"){
			mout.unimplemented() << "(future extension)" << mout.endl;
		}
		else if (format == "tif"){
			// mout.note() <<  gtiffConf.getKeys() << mout.endl;
			if (!params.empty())
				gtiffConf.setValues(params);
			else
				mout.info() << gtiffConf << mout;
		}
		else {
			mout.warn() << "format '" << format << "' not recognized" << mout.endl;
		}

	}

	mutable
	drain::ReferenceMap gtiffConf;

};

/*
class CmdOutputTiffConf : public drain::BasicCommand {

public:

	CmdOutputTiffConf() : drain::BasicCommand(__FUNCTION__, "GeoTIFF configuration") {
		parameters.link("tilewidth", FileGeoTIFF::tileWidth=256);
		parameters.link("tileheight", FileGeoTIFF::tileHeight=0);
		parameters.link("compression", FileGeoTIFF::compression, FileGeoTIFF::getCompressionDict().toStr('|'));
	};


};
 */


// Cf. InputPrefix
class CmdOutputPrefix : public drain::SimpleCommand<std::string> {
	//class CmdOutputPrefix : public drain::BasicCommand {

public:

	CmdOutputPrefix() : drain::SimpleCommand<std::string> (__FUNCTION__, "Path prefix for output files."){
	};

	inline
	void exec() const {
		getContext<RackContext>().outputPrefix = value;
	}
};
//extern drain::CommandEntry<CmdOutputPrefix> cmdOutputPrefix;



void CmdOutputFile::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	if (value.empty()){
		mout.error() << "File name missing. (Use '-' for stdout.)" << mout.endl;
		return;
	}

	if (ctx.statusFlags.isSet(drain::StatusFlags::INPUT_ERROR)){
		mout.warn() << "input failed, skipping" << mout.endl;
		return;
	}

	if (ctx.statusFlags.isSet(drain::StatusFlags::DATA_ERROR)){
		mout.warn() << "data error, skipping" << mout.endl;
		return;
	}

	std::string filename;

	if (value != "-"){
		//mout.warn() << RackContext::variableMapper << mout.endl;
		drain::StringMapper mapper(RackContext::variableMapper);
		mapper.parse(ctx.outputPrefix + value);
		filename = mapper.toStr(ctx.getStatusMap());
		mout.note() << "writing: '" << filename << "'" << mout.endl;
	}
	else {
		filename = "-";
	}

	// mout.note() << "filename: " << filename << mout.endl;

	// TODO: generalize select
	// TODO: generalize image pick (current or str) for png/tif
	//drain::image::FilePng::
	/*
	const bool IMAGE_PNG = drain::image::FilePng::fileInfo.checkPath(value);  //pngFileExtension.test(value);
	const bool IMAGE_PNM = drain::image::FilePnm::fileInfo.checkPath(value);  // fileNameRegExp.test
	const bool IMAGE_TIF = drain::image::FileTIFF::fileInfo.checkPath(value); //tiffFileExtension.test(value);
	*/
	drain::FilePath path(value);
	const bool IMAGE_PNG = drain::image::FilePng::fileInfo.checkPath(path);
	const bool IMAGE_PNM = drain::image::FilePnm::fileInfo.checkPath(path);
	const bool IMAGE_TIF = drain::image::FileTIFF::fileInfo.checkPath(path);
	const bool NO_EXTENSION = path.extension.empty();


	Hi5Tree & src = ctx.getHi5(RackContext::CURRENT); // mostly shared (unneeded in image output, but fast anyway)

	//if (h5FileExtension.test(value)){
	if (hi5::fileInfo.checkPath(value) || NO_EXTENSION){
		if (NO_EXTENSION){
			mout.discouraged("No file extension! Assuming HDF5...");
		}
		mout.info() << "File format: HDF5" << mout.endl;
		src.data.attributes["Conventions"] = "ODIM_H5/V2_2"; // CHECK
		hi5::Writer::writeFile(filename, src); //*ctx.currentHi5);
		// ctx.outputPrefix + value

	}
	else if (IMAGE_PNG || IMAGE_PNM || IMAGE_TIF) {

		// This is the simple version. See image commands (--iXxxxx)

		mout.info() << "File format: image" << mout.endl;

		ctx.updateCurrentImage(); // ?

		// Use ctx.select and/or ctx.targetEncoding, if defined.
		const drain::image::Image & src = ctx.updateCurrentImage();

		if (src.isEmpty()){
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
			mout.warn() << "empty data, skipped" << mout.endl;
			return;
		}

		if (!ctx.formatStr.empty()){
			mout.special() << "formatting comments" << mout.endl;
			drain::StringMapper statusFormatter(RackContext::variableMapper);
			statusFormatter.parse(ctx.formatStr, true);
			drain::image::Image &dst = (drain::image::Image &)src; // violence...
			dst.properties[""] = statusFormatter.toStr(ctx.getStatusMap(), 0);
			ctx.formatStr.clear(); // OK?
		}

		if (IMAGE_PNG || IMAGE_PNM){
			mout.debug() << "PNG or PGM format" << mout.endl;
			drain::image::ImageFile::write(src, filename);
		}
		else if (IMAGE_TIF) {
#ifndef USE_GEOTIFF_NO
			// see FileGeoTiff::tileWidth
			CmdGeoTiff::write(src, filename);
			//FileGeoTIFF::write(filename, src); //, geoTIFF.width, geoTIFF.height);
			//
#else
			ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR || drain::StatusFlags::OUTPUT_ERROR);
			mout.error("No TIFF format support compiled");
#endif
		}
		else {
			ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR || drain::StatusFlags::OUTPUT_ERROR);
			mout.error() << "unknown file name extension" << mout.endl;
		}

	}
	else if (arrayFileExtension.test(filename)){

		/// Currently designed only for vertical profiles produced by VerticalProfileOp (\c --pVerticalProfile )
		/// TODO: modify DataSet such that the quantities appear in desired order.
		writeProfile(src, filename);

	}
	else if (sampleFileExtension.test(value)){

		writeSamples(src, filename);

	}
	else if (dotFileExtension.test(value)) {

		//mout.info() << "Dot/Graphviz file (.dot)" << mout.endl;
		/*
		DataSelector selector;
		if (selector.consumeParameters(ctx.select)){
		*/
		if (!ctx.select.empty()){
			mout.warn() << "no --select support for this command, use --delete and --keep instead" << mout.endl;
			ctx.select.clear();
		}

		writeDotGraph(src, filename, ODIMPathElem::ALL_GROUPS);

	}
	else {


		mout.info() << "File format: text" << mout.endl;

		drain::Output output(filename);

		if (ctx.formatStr.empty()){

			if (textFileExtension.test(filename) || (value == "-")){
				mout.info() << "Dumping HDF5 structure" << mout.endl;
			}
			else {
				mout.error() << "Text formatting --format unset, and unknown file format: " << value << mout.endl;
				return;
			}

			ODIMPathList paths;

			if (!ctx.select.empty()){
				DataSelector selector;
				selector.consumeParameters(ctx.select);
				// mout.warn() << selector << mout.endl;
				selector.getPaths(src, paths);
			}
			else {
				ctx.getHi5(RackContext::CURRENT).getPaths(paths);
				//ctx.currentHi5->getPaths(paths); // ALL
			}

			hi5::Hi5Base::writeText(src, paths, output);
		}
		else {
			mout.debug() << "formatting text output" << mout.endl;
			drain::StringMapper statusFormatter(RackContext::variableMapper);
			statusFormatter.parse(ctx.formatStr, true);
			statusFormatter.toStream(output, ctx.getStatusMap());
			// ctx.formatStr.clear(); // ?
		}

	}

};





class CmdOutputRawImages : public drain::SimpleCommand<std::string> {

public:

	CmdOutputRawImages() : drain::SimpleCommand<std::string>(__FUNCTION__, "Output datasets to png files named filename[NN].ext.",
			"filename", "", "string") {
	};


	void exec() const {

		RackContext & ctx  = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
		//mout.note() << "Writing multiple image files" << mout.endl;

		//DataSelector imageSelector(ODIMPathElem::DATA | ODIMPathElem::QUALITY); //("/data$");
		DataSelector imageSelector(ODIMPathElem::DATA | ODIMPathElem::QUALITY); // ImageS elector imageS elector;
		imageSelector.consumeParameters(ctx.select);
		imageSelector.ensureDataGroup();
		mout.debug() << imageSelector << mout.endl;


		/// Split filename to dir+basename+extension.
		drain::FilePath fp(ctx.outputPrefix + value);
		mout.note() << "outputPrefix='" << ctx.outputPrefix << "'" << mout;
		mout.note() << "Writing multiple image files: " << fp.dir << ' ' << fp.basename << "???_*." << fp.extension << mout.endl;

		std::string filenameOut;
		int i=0; // Overall index (prefix)

		Hi5Tree & src = ctx.getHi5(RackContext::CURRENT);

		ODIMPathList paths;
		imageSelector.getPaths(src, paths); //, ODIMPathElem::DATA | ODIMPathElem::QUALITY); // RE2

		//for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); it++) {
		for (const ODIMPath & path: paths) {

			//const ODIMPath & path = (*it); // modified below
			hi5::NodeHi5 & node = src(path)[ODIMPathElem::ARRAY].data;
			drain::image::Image & img = node.dataSet;

			mout.debug() << "testing: " << path << " => /data" <<mout.endl;

			if (img.isEmpty()){
				mout.warn() << "empty data array: " << path << " => /data" <<mout.endl;
				continue;
			}

			DataTools::getAttributes(src, path, img.properties); // may be unneeded

			//if (path.front().isRoot()) // typically is, string started with slash '/'
			//path.pop_back(); // strip /data
			mout.debug3() << "constructing filename for : " << path <<mout.endl;

			std::stringstream sstr;
			sstr << fp.dir << fp.basename;
			sstr.width(3);
			sstr.fill('0');
			sstr << ++i << '_';
			//for (ODIMPath::const_iterator pit=path.begin(); pit!=path.end(); ++pit){
			for (const ODIMPathElem & elem: path){
				if (elem.isRoot())
					continue;
				sstr << elem.getCharCode();
				if (elem.isIndexed()){
					sstr << elem.getIndex();
				}
				sstr << '-';
			}
			sstr << img.properties["what:quantity"];
			sstr << '.' << fp.extension;
			filenameOut = sstr.str();

			mout.info() << "Writing image file: " << filenameOut << '\t' << path << mout.endl;
			//mout.debug() << "  data :" << *it << mout.endl;

			drain::image::ImageFile::write(img, filenameOut);
			//getResources().currentImage = & img; NEW 2017

		}
		// options[key] = value;  // save last successful filename, see --writeText


	};

};



// BeanCommand<drain::image::ImageSampler>
// BeanerCommand<drain::image::ImageSampler>
//class CmdSample : public drain::SimpleCommand<> {
//class CmdImageSampler : public drain::BeanCommand<drain::image::ImageSampler> {
class CmdImageSampler : public drain::BeanerCommand<drain::image::ImageSampler> {

public:

	// Main
	virtual
	const bean_t & getBean() const {
		RackContext & ctx  = getContext<RackContext>();
		return ctx.imageSampler;
		//return getResources().sampler;
	};


	// Main
	virtual
	bean_t & getBean(){
		RackContext & ctx  = getContext<RackContext>();
		return ctx.imageSampler;
		//return getResources().sampler;
	};

};



FileModule::FileModule(drain::CommandBank & bank) : module_t(bank) { // :(){ // : drain::CommandSection("general") {

	//drain::CommandBank & cmdBank = drain::getCommandBank();
	const drain::Flagger::value_t TRIGGER = drain::Static::get<drain::TriggerSection>().index;

	//drain::CommandInstaller<> installer(drain::getCommandBank());

	install<CmdInputFile>('i').addSection(TRIGGER);
	install<CmdOutputFile>('o'); //  cmdOutputFile('o');

	install<CmdInputPrefix>(); //  cmdInputPrefix;
	install<CmdOutputPrefix>(); //  cmdOutputPrefix;
	install<CmdOutputRawImages>('O'); //  cmdOutputRawImages('O');
	install<CmdOutputConf>(); // cmdOutputConf;
	install<CmdGeoTiff>(); //  geoTiffTile;
	install<CmdImageSampler>("sample"); //cmdSample("sample");
	install<CmdHistogram>(); // hist;

	//installer<CmdInputValidatorFile> cmdInputValidatorFile;

}


} // namespace rack
