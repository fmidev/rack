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

#include "drain/util/Log.h"
#include "drain/util/FilePath.h"
#include "drain/util/Output.h"
#include "drain/util/StringMapper.h"
#include "drain/util/TreeOrdered.h"
#include "drain/util/TextDecorator.h"
#include "drain/util/Variable.h"
#include "drain/image/FilePng.h"
#include "drain/image/FilePnm.h"
#include "drain/image/FileGeoTIFF.h"

#include "drain/image/Image.h"
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
#include "images.h"  // ImageSection


namespace rack {

/// Syntax for recognising hdf5 files.
//  Edited 2017/07 such that also files without extension are considered h5 files. BALTRAD bug

/// Syntax for recognising text files.
const drain::RegExp textFileExtension(".*\\.(txt)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising numeric array files (in plain text format anyway).
const drain::RegExp arrayFileExtension(".*\\.(mat)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for sparsely resampled data.
const drain::RegExp sampleFileExtension(".*\\.(dat)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising Graphviz DOT files.
const drain::RegExp dotFileExtension(".*\\.(dot)$",  REG_EXTENDED | REG_ICASE);




//class CmdOutputConf : public drain::BasicCommand {
class CmdOutputConf : public drain::SimpleCommand<std::string> {

public:

	CmdOutputConf() : drain::SimpleCommand<std::string>(__FUNCTION__, "Format (h5|tif|png|tre|dot) specific configurations", "value", "<format>:<key>=value>,conf...") {

		hdf5Conf.link("compression", hi5::Writer::compressionLevel);

		pngConf.link("compression", drain::image::FilePng::compressionLevel);

#ifndef USE_GEOTIFF_NO
		gtiffConf.link("tile", FileTIFF::defaultTile.tuple(), "<width>[:<height>]");
		// Compression METHOD
		gtiffConf.link("compression", FileTIFF::defaultCompression, drain::sprinter(FileTIFF::getCompressionDict(), "|", "<>").str());
		// gtiffConf.link("level", FileTIFF::defaultCompressionLevel, "1..10");
		// gtiffConf.link("strict", FileGeoTIFF::strictCompliance, "stop on GeoTIFF incompliancy");
		gtiffConf.link("compliancy", FileGeoTIFF::compliancy = FileGeoTIFF::compliancyFlagger.str(), drain::sprinter(FileGeoTIFF::compliancyFlagger.getDict(), "|", "<>").str()); // drain::sprinter(FileGeoTIFF::flagger.getDict(), "|", "<>").str());
		// gtiffConf.link("compliance", FileGeoTIFF::compliance, drain::sprinter(drain::EnumDict<FileGeoTIFF::TiffCompliance>::dict, "|", "<>").str()); // "raw" object (segfaul risk)
		// gtiffConf.link("plainEPSG", FileGeoTIFF::plainEPSG, "use EPSG only, if code supported");
#endif

	};

	CmdOutputConf(const CmdOutputConf & cmd) : drain::SimpleCommand<std::string>(cmd) { // drain::BasicCommand(cmd) {
		//parameters.copyStruct(cmd.getParameters(), cmd, *this);
		hdf5Conf.copyStruct(cmd.hdf5Conf,   cmd, *this, drain::ReferenceMap::LINK);
		pngConf.copyStruct(cmd.pngConf,     cmd, *this, drain::ReferenceMap::LINK);
		gtiffConf.copyStruct(cmd.gtiffConf, cmd, *this, drain::ReferenceMap::LINK);

	}

	// std::string format;
	// std::string params;

	/*
	virtual inline
	void set Parameters(const std::string & params, char assignmentSymbol=0) {
		 drain::BasicCommand::setParameters(params, 0); // sep = ':'
	}
	 */

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		std::string format;
		std::string params;

		drain::StringTools::split2(value, format, params, ":");

		//mout.warn("Redesigned (setParameters dropped). Check results.");

		// mout.note("format: ", format);
		// mout.note("params: ", params);

		if (format.empty()){
			mout.error("no format (h5,png,tif) given");
			return;
		}

		// mout.warn(format );
		// mout.note(params );
		// todo: shared resource for output conf:  refMap of refMaps...
		// todo recognize tif,TIFF
		mout.debug("Current conf [", format, "]");

		// const drain::SprinterLayout & layout = drain::Sprinter::lineLayout; // cppLayout;

		if (hi5::fileInfo.checkExtension(format)){ // "h5", "hdf", "hdf5"
			handleParams(hdf5Conf, params);
		}
		else if (drain::image::FilePng::fileInfo.checkExtension(format)){
			handleParams(pngConf, params);
		}
		else if (drain::image::FilePnm::fileInfo.checkExtension(format)){
			mout.unimplemented("(no parameters supported for PPM/PGM )");
		}
#ifndef USE_GEOTIFF_NO
		else if (drain::image::FileGeoTIFF::fileInfo.checkExtension(format)){ // "tif"
			handleParams(gtiffConf, params);
			drain::StringTools::replace(FileGeoTIFF::compliancy, ":", ",", FileGeoTIFF::compliancy);
			FileGeoTIFF::compliancyFlagger.assign(FileGeoTIFF::compliancy);
		}
#endif
		else if ((format == "tre")||(format == "dot")){
			static drain::SprinterLayout layout("[|]", ",", "=", "");
			drain::VariableMap & vmap = CmdOutputTree::getAttributes();
			if (!params.empty()){
				mout.info("current values: ", drain::sprinter(vmap, layout));
				vmap.clear();
				vmap["format"] = "vt100"; // keep as default
				//vmap.importEntries<false>(p, '='); and others fail...
				drain::SmartMapTools::setValues(vmap, params, ',', '=');
				// drain::Sprinter::toStream(std::cout, vmap, drain::Sprinter::pythonLayout);
				// std::cout << '\n';
				mout.info("    new values: ", drain::sprinter(vmap, layout));
			}
			else {
				drain::Sprinter::toStream(std::cout, vmap, layout);
				std::cout << '\n';
			}
		}
		else {
			mout.warn("format '", format, "' not recognized");
		}

	}


	void handleParams(drain::ReferenceMap & rmap, const std::string & params) const {
		if (!params.empty())
			rmap.setValues(params);
		else {
			const drain::ReferenceMap::unitmap_t & umap = rmap.getUnitMap();
			// drain::Sprinter::toStream(std::cout, gtiffConf, layout);
			for (const auto & entry: rmap){
				std::cout << entry.first << '=' << entry.second;
				// if (rmap) UNITMAP
				drain::ReferenceMap::unitmap_t::const_iterator it = umap.find(entry.first);
				if (it != umap.end()){
					std::cout << ' ' << '(' << it->second << ')';
				}
				std::cout << '\n';
			}
		}
	}

	mutable
	drain::ReferenceMap hdf5Conf;

	mutable
	drain::ReferenceMap pngConf;

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

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// mout.attention(ctx.getName());
	// mout.warn("ctx.select=", ctx.select);

	/*
	if (value.empty()){
		mout.error("File name missing. (Use '-' for stdout.)" );
		return;
	}
	*/

	if (ctx.statusFlags.isSet(drain::StatusFlags::INPUT_ERROR)){
		mout.warn("input failed, skipping");
		return;
	}

	if (ctx.statusFlags.isSet(drain::StatusFlags::DATA_ERROR)){
		mout.warn("data error, skipping");
		return;
	}

	std::string filename;

	const bool STD_OUTPUT = value.empty() || (value == "-");

	if (STD_OUTPUT){
		filename = "-";
	}
	else {
		//mout.warn(RackContext::variableMapper );
		drain::StringMapper mapper(RackContext::variableMapper);
		mapper.parse(ctx.outputPrefix + value);
		filename = mapper.toStr(ctx.getStatusMap());
		mout.note("writing: '" , filename , "'" );
	}
	// mout.note("filename: " , filename );

	// TODO: generalize select
	// TODO: generalize image pick (current or str) for png/tif
	drain::FilePath path(value);
	const bool IMAGE_PNG = drain::image::FilePng::fileInfo.checkPath(path);
	const bool IMAGE_PNM = drain::image::FilePnm::fileInfo.checkPath(path);
	const bool IMAGE_TIF = drain::image::FileTIFF::fileInfo.checkPath(path);
	const bool NO_EXTENSION = path.extension.empty() && !STD_OUTPUT;


	Hi5Tree & src = ctx.getHi5(RackContext::CURRENT); // mostly shared (unneeded in image output, but fast anyway)

	//if (h5FileExtension.test(value)){
	if (hi5::fileInfo.checkPath(value) || NO_EXTENSION){
		if (NO_EXTENSION){
			mout.discouraged("No file extension! Assuming HDF5...");
		}
		mout.info("File format: HDF5");
		src.data.attributes["Conventions"] = "ODIM_H5/V2_2"; // CHECK
		hi5::Writer::writeFile(filename, src); //*ctx.currentHi5);
		// ctx.outputPrefix + value

	}
	else if (IMAGE_PNG || IMAGE_PNM || IMAGE_TIF) {

		// This is the simple version. See image commands (--iXxxxx)

		mout.info("File format: image");

		// Optional on-the-fly conversions: handle ctx.select and ctx.targetEncoding, if defined.
		const drain::image::Image & src = ctx.updateCurrentImage();

		if (src.isEmpty()){
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
			mout.warn("empty data, skipped");
			return;
		}

		if (!ctx.formatStr.empty()){
			mout.special("formatting comments");
			drain::StringMapper statusFormatter(RackContext::variableMapper);
			statusFormatter.parse(ctx.formatStr, true);
			drain::image::Image &dst = (drain::image::Image &)src; // violence...
			dst.properties[""] = statusFormatter.toStr(ctx.getStatusMap(), 0);
			ctx.formatStr.clear(); // OK?
		}

		if (IMAGE_PNG || IMAGE_PNM){
			mout.debug("PNG or PGM format, using ImageFile::write");
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
			// This should be impossible
			ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR || drain::StatusFlags::OUTPUT_ERROR);
			mout.error("unknown file name extension: ", filename);
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

		// DataSelector selector;
		// if (selector.consumeParameters(ctx.select))
		if (!ctx.select.empty()){
			mout.unimplemented("no --select support for Graphviz DOT output, use --delete and --keep instead");
			ctx.select.clear();
		}

		writeDotGraph(src, filename, ODIMPathElem::ALL_GROUPS);

	}
	else if (path.extension == "tre"){
		drain::Output output(path.str());
		//drain::TreeUtils::dumpContents(src, output);
		drain::TreeUtils::dump(src, output);
	}
	else if (path.extension == "TRE"){
		mout.advice("Use dedicated --outputTree to apply formatting");
		drain::Output output(path.str());
		drain::TreeUtils::dump<Hi5Tree,true>(src, output, CmdOutputTree::dataToStream);
	}
	else {

		mout.info("File format: text");

		drain::Output output(filename);

		if (ctx.formatStr.empty()){

			if (textFileExtension.test(filename) || STD_OUTPUT){ // (value == "-")){
				mout.info("Dumping HDF5 structure");
			}
			else {
				mout.error("Text formatting --format unset, and unknown file format: ", value );
				return;
			}

			ODIMPathList paths;

			//mout.attention(ctx.getName());
			//mout.warn("ctx.select=", ctx.select);

			if (!ctx.select.empty()){
				DataSelector selector;
				selector.consumeParameters(ctx.select);
				mout.debug(selector);
				selector.getPaths(src, paths);
			}
			else {
				drain::TreeUtils::getPaths(ctx.getHi5(RackContext::CURRENT), paths);
				// ctx.getHi5(RackContext::CURRENT).getPaths(paths);
				//ctx.currentHi5->getPaths(paths); // ALL
			}

			hi5::Hi5Base::writeText(src, paths, output);
		}
		else {
			mout.debug("formatting text output" );
			drain::StringMapper statusFormatter(RackContext::variableMapper);
			statusFormatter.parse(ctx.formatStr, true);
			statusFormatter.toStream(output, ctx.getStatusMap());
			// ctx.formatStr.clear(); // ?
		}

	}

};

/**
 *
 *   \see Palette::exportSVGLegend()
 */
void CmdOutputPanel::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// mout.attention(ctx.getName());
	// mout.warn("ctx.select=", ctx.select);

	/*
	if (value.empty()){
		mout.error("File name missing. (Use '-' for stdout.)" );
		return;
	}
	*/

	if (ctx.statusFlags.isSet(drain::StatusFlags::INPUT_ERROR)){
		mout.warn("input failed, skipping");
		return;
	}

	if (ctx.statusFlags.isSet(drain::StatusFlags::DATA_ERROR)){
		mout.warn("data error, skipping");
		return;
	}

	//TreeSVG &  // = svg["bg"];
	TreeSVG svg(NodeSVG::SVG);
	// TreeSVG svg; // (NodeSVG::SVG); REDO this, check copy constr!
	svg->setType(NodeSVG::SVG);

	TreeSVG & main = svg["main"];
	main->setType(NodeSVG::GROUP);
	main->set("style", "fill:green");
	main->set("jimbo", 126);
	main->set("jimboz", true);

	const drain::image::Image & src = ctx.updateCurrentImage();
	// svg->set("width",  src.getWidth());
	// svg->set("height", src.getHeight());

	/*
	TreeSVG::node_data_t & rect = svg["main"];
	main.setType(NodeSVG::RECT);
	main.set("x", 0);
	main.set("y", 0);
	//main.set("style", "fill:white opacity:0.8"); // not supported by some SVG renderers
	main.set("fill", "white");
	main.set("opacity", 0.8);
	*/
	TreeSVG & image = main["image1"];
	image->setType(NodeSVG::IMAGE);
	image->set("x", 0);
	image->set("y", 0);
	image->set("width",  src.getWidth());
	image->set("height", src.getHeight());
	image->set("xlink:href", "image.png");

	/*
	TreeSVG & header = svg["title"];
	header->setType(NodeSVG::TEXT);
	header->set("x", lineheight/4);
	header->set("y", (headerHeight * 9) / 10);
	header->ctext = title;
	header->set("style","font-size:20");
	*/

	if (layout.empty() || layout == "basic"){

	}
	else {
		mout.error("Unknown layout '", layout, "'");
	}

	std::string s = filename.empty() ? layout+".svg" : filename;

	if (NodeSVG::fileinfo.checkExtension("svg")){ // .svg
		drain::Output ofstr(s);
		mout.note("writing SVG file: '", s, "");
		ofstr << svg;
	}


}

void CmdOutputTree::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// mout.info("File format: tree");

	const bool STD_OUTPUT = value.empty() || (value == "-");

	std::string filename;
	if (STD_OUTPUT){
		filename = "-";
	}
	else {
		drain::StringMapper mapper(RackContext::variableMapper);
		mapper.parse(ctx.outputPrefix + value);
		filename = mapper.toStr(ctx.getStatusMap());
		mout.note("writing: '", filename, "'");
	}

	drain::Output output(filename);
	drain::TreeUtils::dump(ctx.getHi5(RackContext::CURRENT), output, CmdOutputTree::dataToStream);


}

//std::list<std::string> CmdOutputTreeConf::attributeList = {"quantity", "date", "time", "src", "elangle", "task_args", "legend"};
// See drain::TextDecorator::VT100
// std::map<std::string,std::string>
drain::VariableMap CmdOutputTree::attributes = {
		{"format", "vt100"}, // "txt", "html" ?
		{"image", "BLUE"},
		{"data", "BOLD"},
		{"object", "WHITE"},
		{"quantity", "BOLD:GREEN"},
		{"date", "RED:UNDERLINE"},
		{"time", "RED"},
		{"source", "YELLOW:DIM"},
		{"lon", "YELLOW:DIM"},
		{"lat", "YELLOW:DIM"},
		{"xsize", "YELLOW:DIM"},
		{"ysize", "YELLOW:DIM"},
		{"elangle", "ITALIC:YELLOW"},
		{"gain", "ITALIC:YELLOW"},
		{"offset", "ITALIC:YELLOW"},
		{"nodata", "DIM:YELLOW"},
		{"undetect", "DIM:YELLOW"},
		{"task_args", "CYAN"},
		{"legend", "PURPLE"}
};




bool CmdOutputTree::dataToStream(const Hi5Tree::node_data_t & data, std::ostream &ostr){

	// Shared TextDecorator!
	// RackContext & ctx = getResources().getContext<RackContext>();

	// drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	//mout.unimplemented("Future option... ");

	bool empty = true;

	drain::VariableMap & attrs = CmdOutputTree::getAttributes();

	drain::TextDecorator noDeco;
	drain::TextDecoratorVt100 vt100Deco;

	drain::TextDecorator & decorator = attrs.get("format", "") == "vt100" ? vt100Deco : noDeco;
	decorator.setSeparator(":");

	if (data.exclude){
		ostr << "~";
		return false;
	}

	const drain::image::ImageFrame & img = data.dataSet;
	if (!img.isEmpty()){
		// if (data.attributes.hasKey("image")){
		ostr << img.getWidth() << ',' << img.getHeight() << ' ';
		ostr << drain::Type::call<drain::compactName>(img.getType());
		ostr << '[' << (8*drain::Type::call<drain::sizeGetter>(img.getType())) << ']';
		//<< drain::Type::call<drain::complexName>(img.getType());
		empty = false;
		//}
	}
	// else ...
	char sep = 0;

	for (const auto & entry: attrs){
		if (data.attributes.hasKey(entry.first)){
			if (sep)
				ostr << sep << ' ';
			else
				sep = ',';
			//decorator.set(entry.second);
			decorator.begin(ostr, entry.second);
			ostr << entry.first << '=' << data.attributes[entry.first];
			decorator.end(ostr);
			//decorator.reset();
			empty = false;
		}
	}

	return empty;

}

class CmdOutputRawImages : public drain::SimpleCommand<std::string> {

public:

	CmdOutputRawImages() : drain::SimpleCommand<std::string>(__FUNCTION__, "Output datasets to png files named filename[NN].ext.",
			"filename", "", "string") {
	};


	void exec() const {


		// Shared
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		//mout.note("Writing multiple image files" );

		//DataSelector imageSelector(ODIMPathElem::DATA | ODIMPathElem::QUALITY); //("/data$");
		DataSelector imageSelector(ODIMPathElem::DATA | ODIMPathElem::QUALITY); // ImageS elector imageS elector;
		imageSelector.consumeParameters(ctx.select);
		imageSelector.ensureDataGroup();
		mout.debug(imageSelector );


		/// Split filename to dir+basename+extension.
		drain::FilePath fp(ctx.outputPrefix + value);
		mout.note("outputPrefix='" , ctx.outputPrefix , "'" );
		mout.note("Writing multiple image files: " , fp.dir , ' ' , fp.basename , "???_*." , fp.extension );

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

			mout.debug("testing: " , path , " => /data" );

			if (img.isEmpty()){
				mout.warn("empty data array: " , path , " => /data" );
				continue;
			}

			DataTools::getAttributes(src, path, img.properties); // may be unneeded

			//if (path.front().isRoot()) // typically is, string started with slash '/'
			//path.pop_back(); // strip /data
			mout.debug3("constructing filename for : " , path );

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

			mout.info("Writing image file: " , filenameOut , '\t' , path );
			//mout.debug("  data :" , *it );

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

	const drain::Flagger::value_t TRIGGER = drain::Static::get<drain::TriggerSection>().index;
	const drain::Flagger::value_t IMAGES  = drain::Static::get<ImageSection>().index;

	install<CmdInputFile>('i').addSection(TRIGGER);
	install<CmdOutputFile>('o');
	install<CmdOutputTree>('t');
	install<CmdOutputPanel>();

	install<CmdInputPrefix>();
	install<CmdInputSelect>();
	install<CmdOutputPrefix>();
	install<CmdOutputRawImages>('O').addSection(IMAGES);
	install<CmdOutputConf>();

	// install<CmdOutputTreeConf>();

	install<CmdGeoTiff>().addSection(IMAGES);
	install<CmdImageSampler>("sample");
	install<CmdHistogram>();

	//installer<CmdInputValidatorFile> cmdInputValidatorFile;

}


} // namespace rack
