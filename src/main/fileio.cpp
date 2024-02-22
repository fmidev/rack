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
#include <fstream>
#include <iostream>
#include <sys/stat.h>

#include "drain/util/Log.h"
#include "drain/util/FilePath.h"
#include "drain/util/Frame.h"
#include "drain/util/Output.h"
#include "drain/util/StringMapper.h"
#include "drain/util/TreeOrdered.h"
#include "drain/util/TextDecorator.h"
#include "drain/util/Variable.h"
#include "drain/util/TreeHTML.h"
//#include <drain/image/ImageFile.h>
#include "drain/image/FilePng.h"
#include "drain/image/FilePnm.h"
#include "drain/image/FileGeoTIFF.h"
#include "drain/image/TreeSVG.h"
#include "drain/image/TreeUtilsSVG.h"

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
#include "data/SourceODIM.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h"
#include "product/ProductOp.h"
//#include "radar/RadarDataPicker.h"

#include "resources.h"
#include "fileio.h"
#include "fileio-read.h"
#include "fileio-geotiff.h"
#include "fileio-xml.h"
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

		// TreeUtilsSVG::defaultOrientation.value
		// svgConf.link("group",       TreeUtilsSVG::defaultGroupName);
		RackContext & ctx = getContext<RackContext>();
		svgConf.link("group", ctx.svgGroup); // consider struct for svgConf, one for defaults, in TreeUtilsSVG::defaultConf etc...
		svgConf.link("orientation", svgConf_Orientation, drain::sprinter(TreeUtilsSVG::defaultOrientation.getDict().getKeys()).str());
		svgConf.link("direction",   svgConf_Direction,   drain::sprinter(TreeUtilsSVG::defaultDirection.getDict().getKeys()).str());
		// TreeUtilsSVG::defaultDirection.value);



#ifndef USE_GEOTIFF_NO
		gtiffConf.link("tile", FileTIFF::defaultTile.tuple(), "<width>[:<height>]");
		// Compression METHOD
		gtiffConf.link("compression", FileTIFF::defaultCompression, drain::sprinter(FileTIFF::getCompressionDict(), "|", "<>").str());
		// gtiffConf.link("level", FileTIFF::defaultCompressionLevel, "1..10");
		gtiffConf.link("compliancy", FileGeoTIFF::compliancy = FileGeoTIFF::compliancyFlagger.str(), drain::sprinter(FileGeoTIFF::compliancyFlagger.getDict(), "|", "<>").str()); // drain::sprinter(FileGeoTIFF::flagger.getDict(), "|", "<>").str());

#endif

	};

	CmdOutputConf(const CmdOutputConf & cmd) : drain::SimpleCommand<std::string>(cmd) { // drain::BasicCommand(cmd) {
		//parameters.copyStruct(cmd.getParameters(), cmd, *this);
		hdf5Conf.copyStruct(cmd.hdf5Conf,   cmd, *this, drain::ReferenceMap::LINK);
		pngConf.copyStruct(cmd.pngConf,     cmd, *this, drain::ReferenceMap::LINK);
		gtiffConf.copyStruct(cmd.gtiffConf, cmd, *this, drain::ReferenceMap::LINK);
		svgConf.copyStruct(cmd.svgConf, cmd, *this,     drain::ReferenceMap::LINK); //,

		RackContext & ctx = getContext<RackContext>();
		svgConf.link("group", ctx.svgGroup); // otherwise could be other/static ctx, because of LINK above ?
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
		else if (drain::image::NodeSVG::fileInfo.checkExtension(format)){
			// read params
			handleParams(svgConf, params);
			// write params
			TreeUtilsSVG::defaultOrientation.set(svgConf_Orientation);
			TreeUtilsSVG::defaultDirection.set(svgConf_Direction);
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
			drain::VariableMap & vmap = DataTools::getAttributeStyles();
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
	std::string svgConf_Orientation;
	std::string svgConf_Direction;


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

	// const std::string fileKey = STD_OUTPUT ? "stdout" : path.basename;
	// drain::image::TreeSVG & xmlOutputGroup = ctx.xmlTrack["group"];

	// xmlOutputGroup.hasChild(key)
	// ctx.xmlTrack->setType(svg::UNDEFINED);
	// ctx.xmlTrack->setType(svg::SVG);

	drain::image::TreeSVG & track = ctx.svgTrack["outputs"]; //["file"];
	ctx.svgTrack["outputs"](NodeSVG::GROUP);
	//track->setType(NodeSVG::GROUP); // ensureType?
	track->set("name", "outputs");

	/*
	const std::string key = STD_OUTPUT ? "stdout" : path.basename;
	if (!track.hasChild(key)){
		drain::image::TreeSVG & b = track[key](NodeSVG::GROUP);
		b->set("name", path.basename);
		// b->setType;
	}
	drain::image::TreeSVG & baseGroup = track[key]; // track.retrieveChild(key);
	*/

	if (!STD_OUTPUT){
		track.data.set("id", path.basename);
		if (!ctx.outputPrefix.empty())
			track.data.set("outputPrefix", ctx.outputPrefix);
		// TODO: outDir
		//	track.data.set("outputPrefix", ctx.outputPrefix);
		//track.data.set("prefix", path.basename);
	}
	//track.data.set("id", STD_OUTPUT ? "stdout" : path.basename);
	std::list<std::string> keys = {"what:lon", "here"};

	//if (h5FileExtension.test(value)){
	if (hi5::fileInfo.checkPath(value) || NO_EXTENSION){
		if (NO_EXTENSION){
			mout.discouraged("No file extension! Assuming HDF5...");
		}
		mout.info("File format: HDF5");
		src.data.attributes["Conventions"] = "ODIM_H5/V2_2"; // CHECK
		hi5::Writer::writeFile(filename, src); //*ctx.currentHi5);
		/*
		drain::image::TreeSVG & h5 = baseGroup["h5"](svg::TEXT);
		h5->set("object", src.data.attributes["object"]);
		h5->set("base", path.basename);
		*/
	}
	else if (IMAGE_PNG || IMAGE_PNM || IMAGE_TIF) {

		// This is the simple version. See image commands (--iXxxxx)

		mout.info("File format: image");

		// Optional on-the-fly conversions: handle ctx.select and ctx.targetEncoding, if defined.
		const drain::image::Image & srcImage = ctx.updateCurrentImage();

		if (IMAGE_PNG){
			CmdBaseSVG::addImage(ctx, srcImage, path.str());
		}

		mout.info("Retrieved image: ", srcImage, " [", srcImage.properties.get("what:quantity", ""), "]");

		if (srcImage.isEmpty()){
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
			mout.warn("empty data, skipped");
			return;
		}

		drain::StringMapper dataIDSyntax(RackContext::variableMapper);
		// drain::StringMapper dataIDSyntax("${what:date}_${what:time} ${what:product}", "^[A-Za-z0-9_:]*$");
		dataIDSyntax.parse("${what:date}_${what:time}_");
		//std::string dataID = dataIDSyntax.toStr(ctx.getStatusMap(true), 'x');
		std::string dataID = TreeUtilsSVG::defaultGroupName;
		// dataIDSyntax.toStr(src.properties, 'x') + SourceODIM(src.properties.get("what:source","")).getSourceCode();


		//drain::image::TreeSVG & baseGroup = track[dataID](svg::GROUP); // track.retrieveChild(key);
		//baseGroup->addClass("imagecol");

		drain::StringMapper imageIDSyntax(RackContext::variableMapper);
		//imageIDSyntax.parse("${what:product}_${what:quantity}");
		imageIDSyntax.parse("${what:product}"); // _${where:elangle}
		//imageIDSyntax.parse("${what:product}_");
		std::string imageID = imageIDSyntax.toStr(srcImage.properties);

		if (!ctx.formatStr.empty()){
			mout.special("formatting comments");
			drain::StringMapper statusFormatter(RackContext::variableMapper);
			statusFormatter.parse(ctx.formatStr, true);
			drain::image::Image &dst = (drain::image::Image &)srcImage; // violence...
			dst.properties[""] = statusFormatter.toStr(ctx.getStatusMap(), 0);
			ctx.formatStr.clear(); // OK?
		}

		if (IMAGE_PNG || IMAGE_PNM){
			mout.debug("PNG or PGM format, using ImageFile::write");
			drain::image::ImageFile::write(srcImage, filename);
		}
		else if (IMAGE_TIF) {
#ifndef USE_GEOTIFF_NO
			// see FileGeoTiff::tileWidth
			//imageGroup->comment();
			CmdGeoTiff::write(srcImage, filename);
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
	else if (drain::image::NodeSVG::fileInfo.checkPath(path)) {
		mout.experimental("writing SVG file: ", path);
		drain::Output ofstr(value);
		CmdBaseSVG::completeSVG(ctx);

		drain::image::NodeSVG::toStream(ofstr, ctx.svgTrack);
		// ofstr << ctx.xmlTrack << '\n';
		// mout.unimplemented("not support yet, use --outputPanel / dumpXML");
	}
	else if (drain::NodeHTML::fileInfo.checkPath(path)) {
		mout.special("writing HTML file: ", path);

		if (!path.dir.empty()){
			mout.special("making dir: ", path.dir);
			drain::FilePath::mkdir(path.dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}


		if (!ctx.select.empty()){
			// Initially, mark all paths excluded.
			DataTools::markExcluded(src, true);

			DataSelector selector;
			selector.setParameters(ctx.select);
			mout.special("selector for saved paths: ", selector);

			ODIMPathList savedPaths;
			selector.getPaths(src, savedPaths); //, ODIMPathElem::DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY);

			for (const ODIMPath & path: savedPaths){
				DataTools::markExcluded(src, path, false);
				// ARRAY's: compare with CmdKeep
			}
		}


		class TreeXMLvisitor {

		public:

			drain::TreeHTML html;

			//template <class T>
			int visit(const Hi5Tree & tree, const Hi5Tree::path_t & odimPath){

				drain::Logger submout(__FILE__, __FUNCTION__);

				// Yes.
				const Hi5Tree & t = tree(odimPath);

				// std::cout << path << ':'  << '\n'; // << tree(path).data

				if (t.data.exclude){
					submout.warn("excluding ", odimPath);
					return 1;
				}

				if (!odimPath.empty()){
				}

				submout.special("visiting ", odimPath);

				if (html->isUndefined()){

					// submout.ok("initializing HTML");
					html(drain::BaseHTML::HTML);

					// submout.ok("adding HEAD");
					drain::TreeHTML & head = html["head"](drain::BaseHTML::HEAD); //  << drain::NodeHTML::entry<drain::NodeHTML::HEAD>();

					// submout.ok("adding TITLE");
					head["title"](drain::BaseHTML::TITLE) = "HDF5 file";

					drain::TreeHTML & style = head["style"](drain::BaseHTML::STYLE);
					style->set("ul, #myUL", "list-style-type: none");
					style->set("#myUL", "margin:0;padding:0");
					style[".caret-down::before"]->set({{"transform", "rotate(90deg)"}, {"margin",0}, {"padding",0}});
					style["#myUL"]->set("margin:0;padding:0");
					style["#myUL"]->set("fargin", 1.0);
					style["#myUL"]->set("fadding", 1.0);

					//head.ensureChild(drain::NodeHTML::entry<drain::NodeHTML::TITLE>()) = "HDF5 file";
					//head.ensureChild(drain::NodeHTML::entry<drain::NodeXML<>::COMMENT >()) = RACK_BASE;
					//html(body)(drain::NodeHTML::BODY);
					//html << std::pair<hp_elem,drain::NodeHTML>("body", drain::NodeHTML::BODY);

					html["body"](drain::BaseHTML::BODY);

					submout.accept(html);

				}

				drain::TreeHTML & body = html["body"];

				// ensureChild(drain::NodeHTML::entry<drain::NodeHTML::BODY>());

				drain::TreeHTML::path_t htmlPath;
				// drain::TreeHTML::path_t htmlParentPath;

				// Expand the path to html path UL->LI->UL-> by adding an element (LI) after each
				for (const ODIMPathElem & e: odimPath){

					htmlPath.appendElem("ul");
					// submout.ok("checking path: ", htmlPath);

					const std::string & estr = e.str();

					drain::TreeHTML & group = body(htmlPath)(drain::BaseHTML::UL);
					// group->setType(drain::NodeHTML::UL);
					if (group->isUndefined()){
						group->setType(drain::BaseHTML::UL);
						//group->set("name", htmlPath);
						group->set("hdf5", odimPath);
						group->addClass(estr+"Parent");
					}

					htmlPath.appendElem(estr);
					// htmlPath << e;  // hdf5 elem -> html elem >> html path
					// submout.ok("extending path: ", htmlPath);
					bool EXISTS = body.hasPath(htmlPath);
					drain::TreeHTML & item = body(htmlPath); // (drain::NodeHTML::LI);
					if (item->isUndefined()){
						item->setType(drain::BaseHTML::LI);
						item->set("name", estr);
						item->addClass(e.getPrefix());
					}
					else {
						submout.attention("LI group existed=", (EXISTS?"yes":"no"), ", ", item.data, " at ", htmlPath);
					}
				}


				return 0;

				submout.accept("traversing path: ", odimPath, " -> html: ", htmlPath);

				drain::TreeHTML & current = body(htmlPath);

				// submout.attention("stripping... ", htmlPath);
				//htmlPath.pop_back();
				// submout.attention("       to... ", htmlPath);
				// drain::TreeHTML & parent  = body(htmlPath);

				drain::TreeHTML & parent  = current;

				if (body->isComment()){
					submout.error(__LINE__, " BODY elem became comment: at ", odimPath);
					return 1;
				}
				else {
					submout.special(__LINE__, " BODY elem  ", body->getType());
				}

				const ODIMPathElem & elem = odimPath.back();

				current->set("h5", odimPath);

				if (elem.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
					drain::TreeHTML & table = parent["metadata"](drain::BaseHTML::TABLE);
					// NOTE: shared table for what, where, how
					for (const auto & attr: t.data.attributes){
						drain::TreeHTML & row = table[attr.first](drain::BaseHTML::TR);
						row["key"](drain::BaseHTML::TH)->setText(attr.first);
						row["value"](drain::BaseHTML::TD) = (attr.second);
						std::cout << '\t' << attr.first << ':' << attr.second << '\n'; // << tree(path).data
					}
					// return 1;
				}
				else if (elem.belongsTo(ODIMPathElem::DATA_GROUPS)){

					drain::TreeHTML & caret = current["ul"](drain::BaseHTML::UL);
					caret->setText(odimPath.str());
					caret->addClass("caret");
					return 0;
					//current["m"]->setComment(elem);
				}
				else if (elem.is(ODIMPathElem::ARRAY)){
					if (!t.data.empty()){
						drain::TreeHTML & img = parent["i"](drain::BaseHTML::IMG);
						//img.set("width", ...)
						std::string filename = elem;
						filename += ".png";
						img->set("href", filename);
					}
					//return 1;
				}
				else {
					// current["skip"]->setText("skipped elem" );
				}

				if (body->isComment()){
					submout.error(__LINE__, " BODY elem became comment: at ", odimPath);
					return 1;
				}




				for (const auto & attr: t.data.attributes){
					std::cout << '\t' << attr.first << ':' << attr.second << '\n'; // << tree(path).data
				}

				return 0;
			}

		};

		TreeXMLvisitor handler;
		drain::TreeUtils::traverse(handler, src);

		mout.warn(handler.html.data);
		drain::TreeUtils::dump(handler.html);

		drain::Output outfile("-");
		drain::NodeHTML::toStream(outfile, handler.html, "html");

		if (!ctx.select.empty()){
			mout.debug("resetting selector");
			ctx.select.clear();
			mout.debug("marking data structure fully included");
			DataTools::markExcluded(src, false);
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
		drain::TreeUtils::dump<Hi5Tree,true>(src, output, DataTools::treeToStream);
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

			// OLD statusFormatter.toStream(output, ctx.getStatusMap());

			// NEW
			ODIMPathList paths;
			DataSelector selector(ODIMPathElem::DATASET,ODIMPathElem::DATA);
			// selector.setPathMatcher(ODIMPathElem::DATASET,ODIMPathElem::DATA);
			selector.consumeParameters(ctx.select);
			mout.debug(selector);
			selector.getPaths(src, paths);
			for (const ODIMPath & path: paths){
				//output << path << ':' << src(path).data.attributes << '\n';
				statusFormatter.toStream(output, src(path).data.image.properties);
			}
		}

	}

};


/**
 *
 */
/*
void CmdOutputPanel::appendImage(TreeSVG & group, const std::string & label, drain::VariableMap & variables,
		const drain::Point2D<double> & upperLeft, const drain::image::Image & image, drain::BBox & bbox) const {

	static const drain::StringMapper basename("${outputPrefix}${PREFIX}-${NOD}-${what:product}-${what:quantity}-${LABEL}", "[a-zA-Z0-9:_]+");

	variables["LABEL"] = label;
	std::string fn = basename.toStr(variables,'X') + ".png";

	basename.toStream(std::cout, variables, 0); std::cout << '\n';
	basename.toStream(std::cout, variables, 'X'); std::cout << '\n';
	basename.toStream(std::cout, variables, -1); std::cout << '\n';


	//drain::Point2D<double> upperRight(upperLeft.x + image.getWidth(), upperLeft.y + image.getWidth(), );
	double w = image.getWidth();
	double h = image.getHeight();

	bbox.lowerLeft.x = std::min(bbox.lowerLeft.x,   upperLeft.x);
	bbox.lowerLeft.y = std::max(bbox.lowerLeft.y,   upperLeft.y + h);

	bbox.upperRight.x = std::max(bbox.upperRight.x, upperLeft.x + w);
	bbox.upperRight.y = std::min(bbox.upperRight.y, upperLeft.y);



	drain::image::TreeSVG & imageElem = group[label];
	imageElem->setType(svg::IMAGE);
	imageElem->set("x", upperLeft.x);
	imageElem->set("y", upperLeft.y);
	imageElem->set("width",  w);
	imageElem->set("height", h);
	//imageElem->set("xlink:href", fn);
	imageElem->set("href", fn);
	drain::image::FilePng::write(image, fn);

	drain::image::TreeSVG & title = imageElem["title"];
	title->setType(svg::TITLE);
	title->ctext = label + " (experimental) ";

	//title->setType(NodeSVG:);
	drain::image::TreeSVG & comment = imageElem["comment"];
	comment->setComment("label:" + label);

	// comment->setType(NodeXML::COMM)

}
*/

/**
 *
 *   \see Palette::exportSVGLegend()
void CmdOutputPanel::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// mout.attention(ctx.getName());
	// mout.warn("ctx.select=", ctx.select);


	if (ctx.statusFlags.isSet(drain::StatusFlags::INPUT_ERROR)){
		mout.warn("input failed, skipping");
		return;
	}

	if (ctx.statusFlags.isSet(drain::StatusFlags::DATA_ERROR)){
		mout.warn("data error, skipping");
		return;
	}

	//TreeSVG &  // = svg["bg"];
	TreeSVG svg(svg::SVG);
	// TreeSVG svg; // (svg::SVG); REDO this, check copy constr!
	svg->setType(svg::SVG);

	TreeSVG & main = svg["main"];
	main->setType(svg::GROUP);
	// main->set("style", "fill:green");
	// main->set("jimbo", 126);
	// main->set("jimboz", true);

	drain::VariableMap & variables = ctx.getStatusMap();
	variables["PREFIX"] = "PANEL";


	// drain::StringMapper basename("${PREFIX}-${NOD}-${what:product}-${what:quantity}");
	// drain::BBox bboxAll;
	drain::BBox bbox;
	drain::Point2D<double> upperLeft(0,0);

	//ctx.updateCurrentImage();
	const drain::image::Image & src = ctx.getCurrentImage();
	appendImage(main, "color", variables, upperLeft, src, bbox);
	mout.attention("prev. BBOX: ", bbox);
	// bboxAll.extend(bbox);

	// variables["what:product"] = "prod";

	const drain::image::Image & src2 = ctx.getCurrentGrayImage();
	upperLeft.set(bbox.upperRight.x, 0);
	appendImage(main, "gray", variables, upperLeft, src2, bbox);
	// bboxAll.extend(bbox);
	mout.attention("prev. BBOX: ", bbox);

	// mout.attention("final BBOX: ", bboxAll);

	svg->set("viewboxFOO", bbox.tuple());
	svg->set("width",  +bbox.getWidth());
	svg->set("height", -bbox.getHeight());

	// svg->set("width",  src.getWidth());
	// svg->set("height", src.getHeight());
	ctx.getCurrentGrayImage();



	if (layout.empty() || layout == "basic"){
		//TreeSVG & radar = image["radar"];
		//radar->set("foo", 123);
	}
	else {
		mout.error("Unknown layout '", layout, "'");
	}

	const std::string s = filename.empty() ? layout+".svg" : filename;

	if (!NodeSVG::fileInfo.checkPath(s)){ // .svg
		mout.fail("suspicious extension for SVG file: ", s);
		mout.advice("extensionRegexp: ", NodeSVG::fileInfo.extensionRegexp);
	}

	drain::Output ofstr(s);
	mout.note("writing SVG file: '", s, "");
	// ofstr << svg;
	NodeSVG::toStream(ofstr, svg);


}
*/

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
		mout.note("writing tree: '", filename, "'");
	}

	drain::Output output(filename);
	drain::TreeUtils::dump(ctx.getHi5(RackContext::CURRENT), output, DataTools::treeToStream);

}

//std::list<std::string> CmdOutputTreeConf::attributeList = {"quantity", "date", "time", "src", "elangle", "task_args", "legend"};
// See drain::TextDecorator::VT100
// std::map<std::string,std::string>




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
