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
#include <drain/Log.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>


#include <drain/util/FilePath.h>
#include <drain/util/Frame.h>
#include <drain/util/Output.h>
#include <drain/util/StringMapper.h>
#include <drain/util/TreeOrdered.h>
#include <drain/util/TreeUtilsXML.h> // pruner
// #include <drain/util/TextDecorator.h>
#include <drain/util/TreeHTML.h>
#include <drain/image/FilePng.h>
#include <drain/image/FilePnm.h>
#include <drain/image/FileGeoTIFF.h>
#include <drain/image/TreeSVG.h>
#include <drain/image/TreeElemUtilsSVG.h>
#include <drain/image/TreeUtilsSVG.h>
#include <drain/image/Image.h>
#include <drain/imageops/ImageModifierPack.h>
#include <drain/prog/Command.h>
#include <drain/prog/CommandBankUtils.h>
#include <drain/prog/CommandInstaller.h>
#include <product/RadarProductOp.h>

#include "data/Data.h"
//#include "data/DataOutput.h"
#include "data/DataSelector.h"
#include "data/DataTools.h"
#include "data/ODIMPath.h"
#include "data/PolarODIM.h"
#include "data/SourceODIM.h"
#include "data/VariableFormatterODIM.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h"
#include "resources.h"
#include "fileio.h"
#include "fileio-read.h"
#include "fileio-geotiff.h"
#include "fileio-svg.h"
#include "graphics.h"
#include "fileio-html.h"
#include "file-hist.h"
#include "images.h"  // ImageSection


namespace rack {

/// Syntax for recognising hdf5 files.
//  Edited 2017/07 such that also files without extension are considered h5 files. BALTRAD bug

/// Syntax for command files, typically used reading HDF5 input lists.
const drain::RegExp listFileExtension(".*\\.(lst|vol)$",  REG_EXTENDED); //  | REG_ICASE);

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

		RackContext & ctx = getContext<RackContext>();

		hdf5Conf.link("compression", hi5::Writer::compressionLevel, "0...6 recommended in ODIM");

		pngConf.link("compression", drain::image::FilePng::compressionLevel);

		//svgConf.link("absolutePaths", ctx.svgPanelConf.absolutePaths);
		svgConf.link("paths", ctx.svgPanelConf.pathPolicy);
		// svgConf.link("fontSize", ctx.svgPanelConf.fontSize.tuple());
		svgConf.link("debug", ctx.svgPanelConf.debug); // consider struct for svgConf, one for defaults, in TreeUtilsSVG::defaultConf etc...

#ifndef USE_GEOTIFF_NO

		gtiffConf.link("tile", FileTIFF::defaultTile.tuple(), "<width>[:<height>]");
		gtiffConf.link("compression", FileTIFF::defaultCompression, drain::sprinter(FileTIFF::compressionDict, "|", "<>").str());
		gtiffConf.link("compliancy", FileGeoTIFF::compliancy = "EPSG:STRICT", drain::sprinter(FileGeoTIFF::compliancyFlagger.getDict(), "|", "<>").str());
		// gtiffConf.link("level", FileTIFF::defaultCompressionLevel, "1..10");
		// gtiffConf.link("compliancy", FileGeoTIFF::compliancy = FileGeoTIFF::compliancyFlagger.str(), drain::sprinter(FileGeoTIFF::compliancyFlagger.getDict(), "|", "<>").str()); // drain::sprinter(FileGeoTIFF::flagger.getDict(), "|", "<>").str());
		// gtiffConf.link("compliancy", FileGeoTIFF::compliancy = FileGeoTIFF::compliancyFlagger.getKeysNEW2(FileGeoTIFF::EPSG|FileGeoTIFF::STRICT, ':'), drain::sprinter(FileGeoTIFF::compliancyFlagger.getDict(), "|", "<>").str());
#endif

	};

	CmdOutputConf(const CmdOutputConf & cmd) : drain::SimpleCommand<std::string>(cmd) { // drain::BasicCommand(cmd) {
		//getParameters().copyStruct(cmd.getParameters(), cmd, *this);
		hdf5Conf.copyStruct(cmd.hdf5Conf,   cmd, *this, drain::ReferenceMap::LINK);
		pngConf.copyStruct(cmd.pngConf,     cmd, *this, drain::ReferenceMap::LINK);
		gtiffConf.copyStruct(cmd.gtiffConf, cmd, *this, drain::ReferenceMap::LINK);
		svgConf.copyStruct(cmd.svgConf,     cmd, *this, drain::ReferenceMap::LINK); //,
	}


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
			mout.error("no format (h5,png,svg,tif) given");
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
		else if (drain::image::FileSVG::fileInfo.checkExtension(format)){
			handleParams(svgConf, params);
			ctx.svgPanelConf.pathPolicyFlagger.set(ctx.svgPanelConf.pathPolicy);
		}
#ifndef USE_GEOTIFF_NO
		else if (drain::image::FileGeoTIFF::fileInfo.checkExtension(format)){ // "tif"
			handleParams(gtiffConf, params);
			std::string s; // commas for Flagger
			drain::StringTools::replace(FileGeoTIFF::compliancy, ":", ",", s); //FileGeoTIFF::compliancy);
			FileGeoTIFF::compliancyFlagger.set(s);
		}
#endif
		else if ((format == "tre")||(format == "dot")){
			static drain::SprinterLayout layout("[|]", ",", "=", "");
			drain::VariableMap & vmap = DataTools::getAttributeStyles();
			if (!params.empty()){
				mout.info("current values: ", drain::sprinter(vmap, layout));
				vmap.clear();
				vmap["format"] = "vt100"; // keep as default
				//vmap.importEntries<false>(p, '='); and others fail...
				drain::MapTools::setValues(vmap, params, ',', '=');
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
		if (!params.empty()){
			rmap.setValues(params);
		}
		else {
			const drain::ReferenceMap::unitmap_t & umap = rmap.getUnitMap();
			// drain::Sprinter::toStream(std::cout, gtiffConf, layout);
			//for (const auto & entry: rmap){
			for (const std::string & key: rmap.getKeyList()){
				//std::cout << entry.first << '=' << entry.second;
				std::cout << key << '=' << rmap.get(key, "");
				// if (rmap) UNITMAP
				drain::ReferenceMap::unitmap_t::const_iterator it = umap.find(key);
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

	mutable
	drain::ReferenceMap svgConf;
	std::string svgConfOrientation;
	std::string svgConfDirection;
	std::string svgConfLegend;


};

/*
class CmdOutputTiffConf : public drain::BasicCommand {

public:

	CmdOutputTiffConf() : drain::BasicCommand(__FUNCTION__, "GeoTIFF configuration") {
		getParameters().link("tilewidth", FileGeoTIFF::tileWidth=256);
		getParameters().link("tileheight", FileGeoTIFF::tileHeight=0);
		getParameters().link("compression", FileGeoTIFF::compression, FileGeoTIFF::getCompressionDict().toStr('|'));
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


// Expands ODIM variables to other
/*
 *
 */
class ODIMVariableHandler {

public:

        drain::IosFormat iosFormat;

        /// Default
        /**
         *  \return true if handles.
         */
        template <class T>
        bool handle(const std::string & key, const std::map<std::string,T> & variables, std::ostream & ostr) const {
                typename std::map<std::string,T>::const_iterator it = variables.find(key);
                if (it != variables.end()){
                        iosFormat.copyTo(ostr);
                        return true;
                }
                else {
                        return false;
                }
        }

};

/** A "private" dictionary, distinguished with the 2nd template argument referring to the owner.
 *
 */
template <>
const drain::EnumDict<rack::ODIM::Version,CmdOutputFile>::dict_t drain::EnumDict<rack::ODIM::Version,CmdOutputFile>::dict = {
		{"ODIM_H5/V2_2",  rack::ODIM::ODIM_2_2 },
		{"ODIM_H5/V2_3",  rack::ODIM::ODIM_2_3 },
		{"ODIM_H5/V2_4",  rack::ODIM::ODIM_2_4 },
};

void CmdOutputFile::exec() const {

	RackContext & ctx = getContext<RackContext>();

	// drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
	drain::Logger mout(ctx.log, __FILE__, getName());

	// mout.attention(ctx.getName());
	// mout.warn("ctx.select=", ctx.select);
	/*
	if (value.empty()){
		mout.error("File name missing. (Use '-' for stdout.)" );
		return;
	}
	*/

	if (ctx.statusFlags.isSet(drain::Status::INPUT_ERROR)){
		mout.warn("input failed, skipping");
		return;
	}

	if (ctx.statusFlags.isSet(drain::Status::DATA_ERROR)){
		mout.warn("data error, skipping");
		return;
	}

	std::string filepath = "-";

	const bool STD_OUTPUT = value.empty() || (value == "-");

	// New 2025 (expensive?)
	const drain::VariableMap & statusMap = ctx.getUpdatedStatusMap();

	if (!STD_OUTPUT){
		drain::StringMapper mapper(RackContext::variableMapper);
		mapper.parse(ctx.outputPrefix + value);
		// VariableFormatterODIM<drain::Variable> odimHandler;
		filepath = mapper.toStr(statusMap, -1, RackContext::variableFormatter); // odimHandler);
		/*
		if (!ctx.outputPrefix.empty()){
			mapper.parse(ctx.outputPrefix);
			prefixFinal = mapper.toStr(statusMap);
		}
		mapper.parse(value);
		relativePathFinal = mapper.toStr(statusMap);
		*/
		mout.note("writing: '" , filepath , "'" );
	}
	// mout.note("filename: " , filename );

	// TODO: generalize select
	// TODO: generalize image pick (current or str) for png/tif
	//drain::FilePath path(value);
	drain::FilePath path(filepath);
	const bool DATA_HDF5 = hi5::fileInfo.checkPath(path);
	const bool IMAGE_PNG = drain::image::FilePng::fileInfo.checkPath(path);
	const bool IMAGE_PNM = drain::image::FilePnm::fileInfo.checkPath(path);
	const bool IMAGE_TIF = drain::image::FileTIFF::fileInfo.checkPath(path);
	const bool IMAGE_SVG = drain::image::FileSVG::fileInfo.checkPath(path);
	const bool DATA_HTML = drain::NodeHTML::fileInfo.checkPath(path);

	const bool NO_EXTENSION = path.extension.empty() && !STD_OUTPUT;


	Hi5Tree & src = ctx.getHi5(RackContext::CURRENT); // mostly shared (unneeded in image output, but fast anyway)

	// drain::image::TreeSVG & svgGroup = ctx.getMainGroup();//  RackSVG::getMainGroup(ctx); // , path.tail  //  Note: repeatedly called for svg and png files?
	// drain::image::TreeSVG & svgGroup = RackSVG::getMainGroup(ctx); // , path.tail  //  Note: repeatedly called for svg and png files?

	//track.data.set("id", STD_OUTPUT ? "stdout" : path.tail);
	// std::list<std::string> keys = {"what:lon", "here"};

	//if (h5FileExtension.test(value)){
	// hi5::fileInfo.checkPath(path)
	if (DATA_HDF5 || NO_EXTENSION){
		if (NO_EXTENSION){
			mout.discouraged("No file extension! Assuming HDF5...");
		}
		mout.info("File format: HDF5");

		//src.data.attributes["Conventions"] = "ODIM_H5/V2_2"; // CHECK
		drain::Variable & conventions = src.data.attributes["Conventions"];
		// conventions = "";
		// conventions.setSeparator(0);

		for (const auto & entry: drain::EnumDict<rack::ODIM::Version,CmdOutputFile>::dict){
			if (ODIM::versionFlagger.isSet(entry.second)){
				conventions = entry.first;
				break;
			}
		}

		if (conventions.empty()){
			conventions = "ODIM_H5/V2_3";
			mout.warn("Unrecognized ODIM version: '", ODIM::versionFlagger, "' , setting: ", conventions);
		}
		else {
			mout.revised<LOG_NOTICE>("setting: ", DRAIN_LOG(conventions));
		}


		// const std::string & v = drain::EnumDict<rack::ODIM::Version>::dict.getKey((rack::ODIM::Version)ODIM::versionFlagger.getValue());
		// mout.revised<LOG_WARNING>("Simplifying ODIM version: ", ODIM::versionFlagger, " => ", v);

		/*
		std::string version;
		drain::StringTools::replace(ODIM::versionFlagger, ".", "_", version);

		ODIM::VersionFlagger odimOnly = ODIM::versionFlagger;
		odimOnly.unset();

		mout.revised("Simplifying ODIM version: ", ODIM::versionFlagger);
		*/

		//conventions = "ODIM_H5/";

		/*
		for (const auto & entry: drain::EnumDict<rack::ODIM::Version>::dict){
			if (static_cast<ODIM::VersionFlagger::storage_t>(entry.second) == ODIM::versionFlagger.getValue()){
				conventions << entry.second;
				break;
			}
		}

		for (ODIM::Version v: {ODIM::Version::ODIM_2_4, ODIM::Version::ODIM_2_3, ODIM::Version::ODIM_2_2}){
			if (ODIM::versionFlagger.isSet(v)){
				conventions << "ODIM_H5/" << v;
			}
		}
		*/



		/*
		if (ODIM::versionFlagger.isSet(ODIM::Version::ODIM_2_4)){
			conventions << "2_4";
		}
		else if (ODIM::versionFlagger.isSet(ODIM::Version::ODIM_2_3)){
			conventions << "V2_3";
		}
		else if (ODIM::versionFlagger.isSet(ODIM::Version::ODIM_2_2)){
			conventions << "V2_2";
		}
		else {
			mout.warn("Unrecognized ODIM version: ", ODIM::versionFlagger);
			conventions = "ODIM_H5/ODIM_2_3";
		}
		*/

		// mout.revised("Setting ODIM version: ", );
		// conventions = std::string("ODIM_H5/") + version;

		hi5::Writer::writeFile(filepath, src); //*ctx.currentHi5);
		/*
		drain::image::TreeSVG & h5 = baseGroup["h5"](svg::TEXT);
		h5->set("object", src.data.attributes["object"]);
		h5->set("base", path.tail);
		*/
	}
	else if (IMAGE_PNG || IMAGE_PNM || IMAGE_TIF) {

		// This is the simple version. See image commands (--iXxxxx)

		mout.info("File format: image");

		//const drain::VariableMap & vmap = ctx.getStatusMap();
		// mout.pending<LOG_WARNING>(__FUNCTION__, " quantity : ", ctx.getStatusMap().get("what:quantity","??"));
		// mout.pending<LOG_WARNING>(__FUNCTION__, " quantity1: ", ctx.getStatusMap().get("what:quantity","??"));

		// Optional on-the-fly conversions: handle ctx.select and ctx.targetEncoding, if defined.
		const drain::image::Image & srcImage = ctx.updateCurrentImage();

		//mout.pending<LOG_WARNING>(__FUNCTION__, " quantity2: ", ctx.getStatusMap().get("what:quantity","??"));

		if (IMAGE_PNG){
			RackSVG::addImage(ctx, srcImage, filepath);
			/*
			std::cout << "\nMain:\n";
			drain::TreeUtils::dump(RackSVG::getMainGroup(ctx), std::cout);
			drain::image::NodeSVG::toStream(std::cout, RackSVG::getMainGroup(ctx));

			std::cout << "\nTrack:\n";
			drain::TreeUtils::dump(ctx.svgTrack, std::cout);
			drain::image::NodeSVG::toStream(std::cout, ctx.svgTrack);
			*/
		}

		mout.info("Retrieved image: ", srcImage, " [", srcImage.properties.get("what:quantity", ""), "]");

		if (srcImage.isEmpty()){
			ctx.statusFlags.set(drain::Status::DATA_ERROR);
			mout.warn("empty data, skipped");
			return;
		}

		if (!ctx.formatStr.empty()){
			mout.special("formatting comments");
			drain::StringMapper statusFormatter(RackContext::variableMapper);

			statusFormatter.parse(ctx.formatStr, true);
			drain::image::Image &dst = (drain::image::Image &)srcImage; // violence...
			dst.properties[""] = statusFormatter.toStr(ctx.getStatusMap(), 0, RackContext::variableFormatter); // XXX
			ctx.formatStr.clear(); // OK?
		}

		if (IMAGE_PNG || IMAGE_PNM){
			mout.debug("PNG or PGM format, using ImageFile::write");
			drain::image::ImageFile::write(srcImage, filepath);
		}
		else if (IMAGE_TIF) {
#ifndef USE_GEOTIFF_NO
			// see FileGeoTiff::tileWidth
			//imageGroup->comment();
			CmdGeoTiff::write(ctx, srcImage, filepath);
			//FileGeoTIFF::write(filename, src); //, geoTIFF.width, geoTIFF.height);
			//
#else
			ctx.statusFlags.set(drain::Status::PARAMETER_ERROR, drain::Status::OUTPUT_ERROR);
			mout.error("No TIFF format support compiled");
#endif
		}
		else {
			// This should be impossible
			ctx.statusFlags.set(drain::Status::PARAMETER_ERROR, drain::Status::OUTPUT_ERROR);
			mout.error("unknown file name extension: ", filepath);
		}

		// mout.revised<LOG_WARNING>("CLEARING SELECTOR 1");
		// ctx.select.clear(); // 2025/03
		// ctx.formatStr.clear();

	}
	else if (IMAGE_SVG){ // drain::image::NodeSVG::fileInfo.checkPath(path)) {

		drain::image::TreeSVG & svgGroup = RackSVG::getMainGroup(ctx);

		svgGroup->set("id", path.tail);
		if (!ctx.outputPrefix.empty()){
			svgGroup->set("data-outputPrefix", ctx.outputPrefix); // add "data:..."
		}

		mout.experimental("writing SVG file: ", path);

		if (!ctx.svgPanelConf.pathPolicyFlagger.isSet(drain::image::FileSVG::PathPolicy::ABSOLUTE)){
			// mout.attention("svg: RELATIVE paths, stripping: ", path.dir);
			const std::string prefix = ctx.svgPanelConf.pathPolicyFlagger.isSet(drain::image::FileSVG::PathPolicy::PREFIXED) ? "./" : "";
			drain::image::RelativePathSetterSVG psetter(path.dir, prefix); // consider "file://"
			drain::TreeUtils::traverse(psetter, ctx.svgTrack);
			// TreeUtilsSVG::setRelativePaths(RackSVG::getMain(ctx), path.dir);
		}
		else {
			// mout.attention("svg: ABSOLUTE paths");
		}

		if (ctx.svgTrack->get("data-version") == 2){
			mout.attention("skipping alignment");

			const drain::image::BBoxSVG & bb = RackSVG::getMainGroup(ctx)->getBoundingBox();
			ctx.svgTrack->setFrame(bb.getFrame()); // width, height
				// Finalize view box
			ctx.svgTrack->setViewBox(bb);
			// const std::string viewBox = drain::StringBuilder<' '>(bb.x, bb.y, bb.width, bb.height);
			// ctx.svgTrack->set("viewBox", viewBox);
		}
		else {
			/*
			TreeUtilsSVG::detectBoxNEW(ctx.svgTrack, true);
			TreeUtilsSVG::addStackLayout(ctx.svgTrack, AlignBase::Axis::HORZ, LayoutSVG::Direction::INCR);
			TreeUtilsSVG::superAlignNEW(ctx.svgTrack);
			*/
			//
			drain::image::MetaDataCollectorSVG metadataPruner;
			drain::TreeUtils::traverse(metadataPruner, ctx.svgTrack);

			//  RackSVG::completeSVG(ctx);

			ctx.svgPanelConf.mainTitle = ctx.getFormattedStatus(ctx.svgPanelConf.mainTitle);

			TitleCreatorSVG titleCreator(ctx.svgPanelConf);
			drain::TreeUtils::traverse(titleCreator, ctx.svgTrack); // or mainTrack enough?

			// NEW:

			TreeUtilsSVG::addStackLayout(ctx.svgTrack, ctx.mainOrientation, ctx.mainDirection);
			TreeUtilsSVG::superAlign(ctx.svgTrack);

			// OLD
			// TreeUtilsSVG::superAlign(ctx.svgTrack, ctx.mainOrientation, ctx.mainDirection);

			const drain::image::BBoxSVG & bb = RackSVG::getMainGroup(ctx)->getBoundingBox();
			ctx.svgTrack->setFrame(bb.getFrame()); // width, height
			ctx.svgTrack->setViewBox(bb);

			{
				using namespace drain::image;
				typedef svg::tag_t tag_t;
				TreeSVG & subGroup = ctx.svgTrack["BORDER"];
				NodeSVG::Elem<tag_t::RECT> frame(subGroup);
				frame.width  = bb.width;
				frame.height = bb.height;
				frame.node.addClass("BORDER");
				// frame.node.setStyle("fill", "none");
				frame.node.setStyle({
						{"fill", "none"},
						// {"stroke", "red"},
						//{"stroke-width", "1px"},
				});

			}


			// mout.attention("View BBOX: ", bb);
		}


		drain::TreePruner<drain::image::TreeSVG> textPruner;
		// Remove all UNDEFINED elements (Todo: optional, through outputConf svg:xxx
		textPruner.tagSelector[svg::UNDEFINED] = 0;
		// Remove all G elements which have no children.
		textPruner.tagSelector[svg::GROUP] = drain::XmlEmptiness::CHILDREN;
		// Remove all TEXT elements which have neither children neither own text.
		textPruner.tagSelector[svg::TEXT]  = drain::XmlEmptiness::TEXT | drain::XmlEmptiness::CHILDREN;
		// Remove all TSPAN elements which have no text
		textPruner.tagSelector[svg::TSPAN] = drain::XmlEmptiness::TEXT;
		drain::TreeUtils::traverse(textPruner, ctx.svgTrack);


		drain::image::ClipperSVG clipper(ctx.svgTrack);
		drain::TreeUtils::traverse(clipper, ctx.svgTrack);


		drain::Output ofstr(filepath);

		drain::image::NodeSVG::toStream(ofstr, ctx.svgTrack);
		mout.hint<LOG_DEBUG>("For converting to PNG, consider: ");
		mout.hint<LOG_DEBUG>("\t inkscape -o out.png ", path, "  # Relative paths ok");
		mout.hint<LOG_DEBUG>("\t convert ", path, " out.png # Use full system paths");
		// ofstr << ctx.xmlTrack << '\n';
		// mout.unimplemented("not support yet, use --outputPanel / dumpXML");
		mout.debug(DRAIN_LOG(ctx.svgPanelConf.groupIdentifier));
		mout.debug(DRAIN_LOG(ctx.svgPanelConf.groupTitle));
		// mout.attention(DRAIN_LOG(ctx.svgPanelConf.groupTitleFormatted));
	}
	else if (DATA_HTML) { // drain::NodeHTML::fileInfo.checkPath(path)

		mout.special<LOG_DEBUG>("writing HTML file: ", path);

		if (!path.dir.empty()){
			//mout.special("making dir: ", path.dir);
			//drain::FilePath::mkdir(path.dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}


		if (!ctx.select.empty()){
			// Initially, mark all paths excluded.
			DataTools::markExcluded(src, true);

			DataSelector selector;
			selector.setParameters(ctx.select);
			mout.special<LOG_DEBUG>("selector for saved paths: ", selector);

			ODIMPathList savedPaths;
			selector.getPaths(src, savedPaths); //, ODIMPathElem::DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY);

			for (const ODIMPath & path: savedPaths){
				DataTools::markExcluded(src, path, false);
				// ARRAY's: compare with CmdKeep
			}
		}


		ExtractorH5toHTML extractor;
		//extractor.setBaseDir(drain::FilePath::path_t(path.dir, path.tail));
		extractor.setBaseDir(path.dir);
		drain::TreeHTML & html = extractor.getHtml();
		html[drain::Html::HEAD][drain::Html::TITLE](drain::Html::TITLE) = path.tail;
		html[drain::Html::BODY]["header"](drain::Html::H1) = path.tail;
		html->setId(path.tail); // Needed in relativePath by ExtractorH5toHTML::visitPrefix().

		drain::TreeUtils::traverse(extractor, src);

		if (mout.isDebug(1)){
			mout.attention("Source data:");
			drain::TreeUtils::dump(src);
			mout.attention("Selected HTML data:");
			// mout.warn(html.data);
			drain::TreeUtils::dump(html);
		}
		else {
			mout.hint("Run with --debug to see tree structures");
		}

		drain::Output outfile(path.str());
		//drain::NodeHTML::toStream(outfile, html);
		outfile << html;

		if (!ctx.select.empty()){
			mout.debug("resetting selector");
			ctx.select.clear();
			mout.debug("marking data structure fully included");
			DataTools::markExcluded(src, false);
		}

	}
	else if (arrayFileExtension.test(value)){

		/// Currently designed only for vertical profiles produced by VerticalProfileOp (\c --pVerticalProfile )
		/// TODO: modify DataSet such that the quantities appear in desired order.
		writeProfile(src, filepath);

	}
	else if (sampleFileExtension.test(value)){

		writeSamples(src, filepath);

	}
	else if (dotFileExtension.test(value)) {

		// DataSelector selector;
		// if (selector.consumeParameters(ctx.select))
		if (!ctx.select.empty()){
			mout.unimplemented("no --select support for Graphviz DOT output, use --delete and --keep instead");
			ctx.select.clear();
		}

		writeDotGraph(src, filepath, ODIMPathElem::ALL_GROUPS);

	}
	else if (path.extension == "tre"){
		drain::Output output(path.str());
		//drain::TreeUtils::dumpContents(src, output);
		drain::TreeUtils::dump(src, output);
	}
	else if (path.extension == "TRE"){
		mout.advice("Use dedicated --outputTree to apply formatting");
		drain::Output output(path.str());
		drain::TreeUtils::dump<Hi5Tree,true>(src, output, DataTools::treeToStream);
	}
	else if (path.extension == "inf"){
		// mout.advice("Use dedicated --outputTree to apply formatting");
		drain::Output output(path.str());

		// TODO: consider Data<polar> object, with PolarODIM



		ODIM odim;
		odim.date = src[ODIMPathElem::WHAT].data.attributes.get("date", "");
		odim.time = src[ODIMPathElem::WHAT].data.attributes.get("time", "");
		drain::Time time;
		odim.getTime(time);
		std::cerr << "base TIME: " << time.getTime() << '\n';

		for (const auto & datasetGroup: src){
			if (datasetGroup.first.is(ODIMPathElem::DATASET)){
				// datasetGroup.second[ODIMPathElem::WHAT].data.attributes.get("date", "");
				// datasetGroup.second[ODIMPathElem::WHAT].data.attributes.get("time", "");
				DataSet<PolarSrc> dataSet(datasetGroup.second);
				std::cerr << dataSet << '\n';

				output << datasetGroup.first << '=';
				std::set<std::string> quantities; // keep ordered
				for (const auto & dataGroup: datasetGroup.second){
					if (dataGroup.first.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
						quantities.insert(dataGroup.second[ODIMPathElem::WHAT].data.attributes.get("quantity", "?"));
						// output << dataGroup.second[ODIMPathElem::WHAT].data.attributes.get("quantity", "?") << ':';
					}
				}
				//output.getStream() << ((drain::StringBuilder<','>(quantities2)).str());
				output << drain::sprinter(quantities, ",");
				output << '\n';
			}
		}
		//drain::TreeUtils::dump<Hi5Tree,true>(src, output, DataTools::treeToStream);
	}
	else {

		mout.info("File format: text");

		drain::Output output(filepath);

		if (ctx.formatStr.empty()){

			if (textFileExtension.test(filepath) || STD_OUTPUT){ // (value == "-")){
				mout.info("Dumping HDF5 structure");
			}
			else {
				mout.error("Text formatting --format unset, and unknown file format: ", value );
				return;
			}

			ODIMPathList paths;

			if (!ctx.select.empty()){
				DataSelector selector;
				selector.consumeParameters(ctx.select); // special<LOG_DEBUG>
				mout.revised("always using selector in --format'ted output, current selector=", selector);
				// mout.debug(selector);
				selector.getPaths(src, paths);
			}
			else {
				drain::TreeUtils::getPaths(ctx.getHi5(RackContext::CURRENT), paths);
			}


			hi5::Hi5Base::writeText(src, paths, output);
		}
		else {
			mout.debug("formatting text output: >", ctx.formatStr, '<');
			drain::StringMapper statusMapper(RackContext::variableMapper);
			statusMapper.parse(ctx.formatStr, true);

			// mout.attention("Debugging FALSE: ");
			// statusFormatter.parse(ctx.formatStr, false);
			// statusFormatter.toStream(std::cerr);
			// OLD statusFormatter.toStream(output, ctx.getStatusMap());

			// NEW
			ODIMPathList paths;
			DataSelector selector; //("dataset1/data1/");
			// selector.setQuantities("DBZH:DBZ.*");
			// DataSelector selector(ODIMPathElem::DATASET,ODIMPathElem::DATA);
			// selector.setPathMatcher(ODIMPathElem::DATASET,ODIMPathElem::DATA);
			if (ctx.select.empty()){
				// special<LOG_DEBUG>
				selector.setPathMatcher(ODIMPathElem::DATA); //   "data1");
				selector.setMaxCount(1);
				// mout.special("Revised code: always using selector in --format'ted output, default selector=", selector); // <LOG_DEBUG>
				mout.revised<LOG_NOTICE>("Revised code: default selector=", selector, ", pathMatcher=", selector.getPathMatcher()); // <LOG_DEBUG>
			}
			else {
				selector.consumeParameters(ctx.select);
			}
			//mout.debug(selector);
			selector.getPaths(src, paths);

			// ctx.getUpdatedStatusMap();
			//
			const drain::VariableMap & vmapShared = ctx.getStatusMap(); // updated above

			mout.debug("VMAP (", vmapShared.size(), ") where:EPSG=", vmapShared.get("where:EPSG", -1));

			switch (paths.size()){
			case 0:
				mout.warn("no paths returned with by Selector: ", selector);
				// no break
			case 1:
				break;
			default:
				mout.hint("several datasets found, use --select dataset1 or --select dataset:,count=1 to limit");
			}

			for (const ODIMPath & path: paths){
				const drain::image::Image & img = src(path).data.image;
				drain::FlexVariableMap vmap;
				vmap.importCastableMap(vmapShared);
				vmap.importCastableMap(img.properties);
				// TODO version flags
				if (ODIM::versionFlagger.isSet(ODIM::RACK_EXTENSIONS)){
					vmap["what:typesize"] = drain::Type::call<drain::sizeGetter>(img.getType());
					vmap["what:typename"] = drain::Type::call<drain::compactName>(img.getType());
				}
				vmap["path"] = path.str();
				if (path.front().is(ODIMPathElem::DATASET)){
					vmap["dataset"] = path.front().getIndex();
					if (path.size() > 1){
						vmap["data"] = (++path.begin())->getIndex();
					}
					else {
						vmap["data"] = 0; //.setType(typeid(std::string));
					}
				}
				else {
					vmap["dataset"] = 0; //.setType(typeid(std::string)); // empty ""
					vmap["data"] = 0;
				}
				// output << path << ':' << src(path).data.attributes << '\n';
				// const drain::FlexVariableMap & vmap = src(path).data.image.properties;

				// mout.special<LOG_DEBUG+1>('\t', path, ": attr: ", vmap);
				// statusFormatter.toStream(output, src(path).data.image.properties);
				statusMapper.toStream(output, vmap, 0, RackContext::flexVariableFormatter); // odimHandler);
			}
		}

	}

	// mout.revised<LOG_WARNING>("CLEARING SELECTOR");
	// ctx.select.clear(); // 2025/03

};


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
		filename = mapper.toStr(ctx.getStatusMap(), 0, RackContext::variableFormatter);
		mout.note("writing tree: '", filename, "'");
	}

	drain::Output output(filename);
	drain::TreeUtils::dump(ctx.getHi5(RackContext::CURRENT), output, DataTools::treeToStream);

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
		mout.note("Writing multiple image files: " , fp.dir , ' ' , fp.tail , "???_*." , fp.extension );

		std::string filenameOut;
		int i=0; // Overall index (prefix)

		Hi5Tree & src = ctx.getHi5(RackContext::CURRENT);

		ODIMPathList paths;
		imageSelector.getPaths(src, paths); //, ODIMPathElem::DATA | ODIMPathElem::QUALITY); // RE2

		//for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); it++) {
		for (const ODIMPath & path: paths) {

			//const ODIMPath & path = (*it); // modified below
			hi5::NodeHi5 & node = src(path)[ODIMPathElem::ARRAY].data;
			drain::image::Image & img = node.image;

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
			sstr << fp.dir << fp.tail;
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



class CmdImageSampler : public drain::BeanCommand<drain::image::ImageSampler> {

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


/// "Overrides" drain::CmdFormat
//class CmdFormat : public drain::BasicCommand {
class CmdFormat : public drain::SimpleCommand<> {

public:


	/// Default constructor.
	//CmdFormat() : drain::BasicCommand(__FUNCTION__,"Set format for data dumps (see --sample or --outputFile)") {  // SimpleCommand<std::string>(getResources().generalCommands, name, alias, "Sets a format std::string.") {
	CmdFormat() : drain::SimpleCommand<>(__FUNCTION__,"Set format for data dumps (see --sample or --outputFile)", "syntax") {  // SimpleCommand<std::string>(getResources().generalCommands, name, alias, "Sets a format std::string.") {
	};

	/// Copy constructor.
	/*
	CmdFormat(const CmdFormat & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK );
	}
	*/

	static
	const std::map<std::string,std::string> presets;

	virtual
	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		ctx.formatStr = value;
		/*
		mout.attention("ctx.id = ", ctx.getId());
		mout.attention("ctx.formatStr: '", ctx.formatStr,"'");
		mout.attention("params: '", this->getParameters());
		mout.attention("syntax: '", this->getParameters()["syntax"]);
		*/
		for (const auto & entry: presets){
			// mout.attention("now ", entry.first);
			if (entry.first == ctx.formatStr){
				mout.info("using preset: ", entry.first, "=>",  entry.second);
				ctx.formatStr = entry.second;
				return;
			}
		}
	}

};

const std::map<std::string,std::string> CmdFormat::presets = {
		{"OSM", "wms?service=WMS&version=1.1.0&request=GetMap&format=image/png&layers=osm:osm&srs=EPSG:${where:EPSG}&bbox=${where:BBOX_native}&width=${where:xsize}&height=${where:ysize}"},
		{"FMI-OSM", "http://map.fmi.fi/geoserver/wms?wms?service=WMS&version=1.1.0&request=GetMap&format=image/png&layers=osm:osm&srs=EPSG:${where:EPSG}&bbox=${where:BBOX_native}&width=${where:xsize}&height=${where:ysize}"},
		{"metadata", {
				"SOURCE='${what:source}'\n"
				"NOD='${NOD}'\n"
				"WMO='${WMO}'\n"
				"PLC='${PLC}'\n"
				"LAT='${where:lat}'\n"
				"LON='${where:lon}'\n"
				"SITEHEIGHT='${where:height}'\n"
				"PROJDEF='${where:projdef}'\n"
				"EPSG='${where:EPSG}'\n"
				"BBOX='${where:BBOX_native}'\n"
				"WIDTH='${where:xsize}'\n"
				"HEIGHT='${where:ysize}'\n"
				//"BBOX='${where:LL_lon},${where:LL_lat},${where:UR_lon},${where:UR_lat}'\n"
		}
		}
};


class CmdFormatOut : public drain::SimpleCommand<std::string> {

public:

	CmdFormatOut() : drain::SimpleCommand<std::string>(__FUNCTION__, "Dumps the formatted std::string to a file or stdout.", "filename","","std::string") {
		//getParameters().separators.clear();
		//getParameters().link("filename", filename, "");
	};


	virtual
	void exec() const override {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = resources.mout;


		drain::StringMapper statusFormatter(RackContext::variableMapper);
		statusFormatter.parse(ctx.formatStr, true);

		mout.deprecating("Use  -o / --outputFile [file|'-'] instead of  ", getName());

		if (value == "log"){
			mout.unimplemented("Logging: future option" );
			//statusFormatter.toStream(ctx.log.getOstr, ctx.getStatus());
			//std::cout << statusFormatter;
		}
		else if (value == "image"){
			//resources.
			mout.deprecating("this command is unneed (--format is sufficient)" );
		}
		else {
			std::string outFileName;
			drain::VariableMap & statusMap = ctx.getUpdatedStatusMap();
			if ((value == "")||(value == "-")){
				outFileName = "-";
			}
			else {
				drain::StringMapper filenameFormatter(RackContext::variableMapper);
				filenameFormatter.parse(ctx.outputPrefix + value, false);  //filename = mapper.toStr(ctx.getStatusMap());
				// Consider here or shared: VariableFormatterODIM<drain::Variable> odimHandler;
				outFileName = filenameFormatter.toStr(statusMap, -1, RackContext::variableFormatter);
				//outFileName = ctx.outputPrefix + value;
			}
			mout.info("writing " , outFileName );
			drain::Output ofstr(outFileName);
			//mout.warn(ctx.getStatus() );
			//std::ofstream ofstr(outFileName.c_str(), std::ios::out);
			if (ofstr){
				// VariableFormatterODIM<drain::Variable> odimHandler;
				statusFormatter.toStream(ofstr, statusMap, 0, RackContext::variableFormatter); // odimHandler);
			}
			else
				mout.warn("write error: " , outFileName );
			//strm.toStream(ofstr, cmdStatus.statusMap.exportMap());
			//ofstr.close();
		}

		//mout.warn("after expansion: " , r.statusFormatter );
		//r.statusFormatter.debug(std::cerr, r.getStatusMap());

	};

};



FileModule::FileModule(drain::CommandBank & bank) : module_t(bank) { // :(){ // : drain::CommandSection("general") {

	const drain::FlagResolver::ivalue_t TRIGGER = drain::Static::get<drain::TriggerSection>().index;
	const drain::FlagResolver::ivalue_t IMAGES  = drain::Static::get<ImageSection>().index;

	install<CmdInputFile>('i').addSection(TRIGGER);

	//CmdOutputFile mika;
	install<CmdOutputFile>('o');
	install<CmdOutputTree>('t');
	// install<CmdOutputPanel>();

	install<CmdInputPrefix>();
	install<CmdInputSelect>();
	install<CmdOutputPrefix>();
	install<CmdOutputRawImages>('O').addSection(IMAGES);
	install<CmdOutputConf>();

	// install<CmdFormat>(); // rack version, with presets
	// install<CmdFormatOut>();
	install<drain::CmdFormatFile<RackContext> >();

	DRAIN_CMD_INSTALL(Cmd,Format)();
	DRAIN_CMD_INSTALL(Cmd,FormatOut)();
	linkRelatedCommands(Format,FormatOut);

	DRAIN_CMD_INSTALL(Cmd,ImageSampler)("sample");
	ImageSampler.linkRelated("format");
	// ImageSampler.relatedCommands.insert("format");

	// install<CmdOutputTreeConf>();

	install<CmdGeoTiff>().addSection(IMAGES);

	//DRAIN_CMD_INSTALL(ImageSampler)("sample");
	// drain::Command & imageSampler =  install<CmdImageSampler>("sample");
	// imageSampler.relatedCommands.insert("format");

	install<CmdHistogram>();

	//installer<CmdInputValidatorFile> cmdInputValidatorFile;

}


} // namespace rack
