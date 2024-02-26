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
/*
#include <fstream>
#include <iostream>
#include <sys/stat.h>

#include "drain/util/Log.h"
#include "drain/util/FilePath.h"
#include "drain/util/Frame.h"
#include "drain/util/TreeOrdered.h"
#include "drain/util/TextDecorator.h"
#include "drain/util/Variable.h"
#include "drain/util/TreeHTML.h"
//#include <drain/image/ImageFile.h>
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
#include "file-hist.h"
#include "images.h"  // ImageSection
*/

#include "drain/util/Output.h"
#include "drain/util/StringMapper.h"
#include "drain/util/TreeXML.h"
#include "drain/util/TreeHTML.h"

#include "drain/image/FilePng.h"
#include "drain/image/TreeUtilsSVG.h"

#include "data/SourceODIM.h" // for NOD
#include "fileio-xml.h"  // ImageSection

namespace rack {


int MetaDataCollectorSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path){
	return 0;
}


int MetaDataCollectorSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	// std::cerr << __FUNCTION__ << ':' << path << std::endl;

	TreeSVG & current = tree(path);

	if (current->getType()==svg::METADATA){
		return 0;
	}

	// Experimental
	for (auto & entry: current.getChildren()){
		TreeSVG & child = entry.second;
		if (child.hasChild("metadata")){
			TreeSVG & metadata = current["metadata"](svg::METADATA);
			metadata->getAttributes().importCastableMap(entry.second["metadata"]->getAttributes());
			/*
			for (const auto & attr: entry.second["metadata"]->getAttributes()){
				metadata->set(attr.first, attr.second);
			}
			*/
		}
	}

	return 0;

}


int MetaDataPrunerSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path){

	// std::cerr << __FUNCTION__ << ':' << path << std::endl;

	return 0;
}


int MetaDataPrunerSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	TreeSVG & current = tree(path);

	//if (!((current->getType()==svg::GROUP) || (current->getType()==svg::IMAGE))){ // At least METADATA must be skipped...
	if (current->getType() != svg::GROUP){ // At least METADATA must be skipped...
		return 0;
	}

	typedef std::map<std::string, unsigned short> variableStat_t;
	variableStat_t stat;
	/// Number of children having (any) metadata.
	int count = 0;

	/// Check which attr(key,value) is shared by all the children.
	for (auto & entry: current.getChildren()){
		TreeSVG & child = entry.second;
		if (child.hasChild("metadata") && !child->hasClass("legend")){ // or has "data"?
			++count;
			TreeSVG & childMetadata = entry.second["metadata"](svg::METADATA);
			for (const auto & attr: childMetadata->getAttributes()){
				// tehty jo metadata->set(attr.first, attr.second);
				std::string s = drain::StringBuilder<>(attr.first,'=',attr.second);
				++stat[s];
			}
		}
	}

	TreeSVG & metadata = current["metadata"](svg::METADATA);
	/// Collective (union)
	// TreeSVG & metadata = current["metadataPlain"](svg::METADATA);
	// metadata->getAttributes().clear();
	// metadata->set("name", "simple");

	TreeSVG & debugAccepted = current["accepted"](svg::COMMENT);
	debugAccepted->ctext = "Accepted (shared): ";

	TreeSVG & debugRejected = current["rejected"](svg::COMMENT);
	debugRejected->ctext = "Rejected (specific): ";

	/// Collective (union)
	//  TreeSVG & metadataPruned = current["metadataPruned"](svg::METADATA);
	//  metadataPruned->set("name", "metadataPruned");
	drain::Logger mout(__FILE__, __FUNCTION__);

	// metadata->getAttributes().clear();
	mout.pending<LOG_DEBUG>("pruning: ", drain::sprinter(stat), path.str());
	for (const auto & e: stat){
		mout.pending<LOG_DEBUG>('\t', e.first, ':', e.second);

		// std::cerr << "\t vector " << e.first << ' ' << e.second << std::endl;
		std::string key, value;
		drain::StringTools::split2(e.first, key, value, '=');
		if (e.second == count){
			mout.accept<LOG_DEBUG>('\t', e.first, ' ', path.str());
			// debugAccepted->ctext += e.first; // set(key, value);
			debugAccepted->set(key, value);

			metadata->set(key, value); // NOTE: becoming strings (consider type dict?)
			for (auto & entry: current.getChildren()){
				TreeSVG & child = entry.second;
				if (child.hasChild("metadata")){
					TreeSVG & childMetadata = entry.second["metadata"](svg::METADATA);
					childMetadata -> remove(key);
				}
			}

		}
		else {
			mout.reject<LOG_DEBUG>('\t', e.first, ' ', path.str());
			//debugRejected->ctext += e.first; // set(key, value);
			debugRejected->set(key, value);
		}
	}


	return 0;

}




int TitleCreatorSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){


	TreeSVG & current = tree(path);

	/*
	if (current->getType() != svg::GROUP){ // At least METADATA must be skipped...
		return 0;
	}
	*/

	if (!((current->getType()==svg::GROUP) || (current->getType()==svg::IMAGE))){
		return 0;
	}

	if (!current.hasChild("metadata")){
		return 0;
	}

	drain::Logger mout(__FILE__, __FUNCTION__);


	TreeSVG & metadata = current["metadata"];

	if (metadata->getAttributes().empty()){
		mout.debug("title skipped, metadata empty under: ", path);
		return 0;
	}


	//if ((current->getType() == svg::GROUP) && (current->hasClass("imageSet"))){ // = row or column
	/*
	if (false){ // = row or column
		// Row / column stuff?
		int x = NodeSVG::xml_node_t::getCount() * 2;
		int y = x;

		TreeSVG & titleBox = current["titleBox"](NodeSVG::RECT);
		titleBox->set("name", "titleBox");
		titleBox->set("x", x);
		titleBox->set("y", y);
		titleBox->set("width", 200);
		titleBox->set("height", 30);
		titleBox->addClass("imageSetTitle");
		titleBox->setStyle("fill:darkblue; opacity:0.5;");

		TreeSVG & text = current["titleText"](svg::TEXT);
		text->set("name", "titleText");
		text->set("x", x + 2);
		text->set("y", y + 20);
		//text->set("text-anchor", "reijo");
		// text->setText(path.str());
		for (const auto & attr: metadata->getAttributes()){
			// consider str replace
			TreeSVG & tspan = text[attr.first](svg::TSPAN);
			tspan->addClass(attr.first);
			tspan->ctext = attr.second.toStr();
			tspan->ctext += "&#160;"; // space
			//entry.second->set(key, "*"); // TODO: remove attribute
		}

		return 0;
	}
	else */
	if (current->getType() == svg::IMAGE){

		TreeSVG::path_t parentPath;
		parentPath.append(path);
		parentPath.pop_back();
		if (parentPath.empty()){
			mout.warn("IMAGE elem directy under root?");
			return 1;
		}

		TreeSVG & parent = tree(parentPath);

		// Start from image coords.
		int x = current->get("x", 0);
		int y = current->get("y", 0);

		std::string name = current->get("name", "img?");
		name += ":Title";
		TreeSVG & text = parent[name](svg::TEXT);
		text->set("name", name);
		text->set("x", x + 2);
		text->set("y", y + 20);
		//text->setStyle("fill", "slateblue");
		text->setStyle("fill", "darkblue");
		// text->setStyle("stroke:white; stroke-width:0.5em; fill:black; paint-order:stroke; stroke-linejoin:round");
		text->addClass("imageTitle");

		for (const auto & attr: metadata->getAttributes()){
			// consider str replace
			TreeSVG & tspan = text[attr.first](svg::TSPAN);
			tspan->addClass(attr.first);
			tspan->ctext = attr.second.toStr();
			tspan->ctext += "&#160;"; //'_';
			//entry.second->set(key, "*"); // TODO: remove attribute
		}


	}

	return 0;

}




// NOTE: Part of SVG processing is  CmdOutputConf
/*
 * 		svgConf.link("group",       TreeUtilsSVG::defaultGroupName);
		svgConf.link("orientation", svgConf_Orientation, drain::sprinter(TreeUtilsSVG::defaultOrientation.getDict().getKeys()).str());
		svgConf.link("direction",   svgConf_Direction,   drain::sprinter(TreeUtilsSVG::defaultDirection.getDict().getKeys()).str());
 *
 */

void CmdBaseSVG::createTitleBox(TreeSVG & tree){

	drain::Logger mout(__FILE__, __FUNCTION__);

	/*
	// const std::string TITLE_BOX;
	if (tree.hasChild("titlebox")){
		return; //  tree;
	}

	TreeSVG & titleBox = tree["titlebox"](svg::RECT); // GROUP?
	titleBox->set("name", "titleBox");
	titleBox->set("x", 0);
	titleBox->set("y", 0);
	titleBox->set("width", 200);
	titleBox->set("height", 60);
	titleBox->addClass("imageSetTitle", "header");
	titleBox->setStyle("fill:darkblue; opacity:0.5;");
	titleBox->setText(tree->get("name", ""));
	*/
}

drain::image::TreeSVG & CmdBaseSVG::getMain(RackContext & ctx){

	if (!ctx.svgTrack.hasChild("style")){
		drain::image::TreeSVG & style = ctx.svgTrack["style"](svg::STYLE);
		//style["text"] = ("stroke:white; stroke-width:0.5em; stroke-opacity:0.25; fill:black; paint-order:stroke; stroke-linejoin:round");
		style["text"] = ("fill:black");
		style[".imageTitle"] = "font-size:1.5em; stroke:white; stroke-opacity:0.5; stroke-width:0.6em; fill:darkslateblue; fill-opacity:1; paint-order:stroke; stroke-linejoin:round";
		//style[".imageTitle"] = "font-size:1.5em; stroke:white; stroke-opacity:0.25; stroke-width:0.5em; fill:darkslateblue; fill-opacity:1; paint-order:stroke; stroke-linejoin:round";
		// style["text"] = ("stroke:white; stroke-width:1em; stroke-opacity:0.25; fill:black; paint-order:stroke; stroke-linejoin:round");
	}

	return 	ctx.svgTrack["outputs"](NodeSVG::GROUP);

}

drain::image::TreeSVG & CmdBaseSVG::getCurrentGroup(RackContext & ctx){ // what about prefix?


	drain::image::TreeSVG & track = getMain(ctx);

	if (!track.hasChild(ctx.svgGroup)){
		drain::image::TreeSVG & group = track[ctx.svgGroup](NodeSVG::GROUP);
		// Ensure (repeatedly)
		group->addClass("imageSet");
		group->set("id", ctx.svgGroup);
		group->set("name", ctx.svgGroup);
	}

	return track[ctx.svgGroup](NodeSVG::GROUP);

}


void CmdBaseSVG::addImage(RackContext & ctx, const drain::image::Image & src, const drain::FilePath & filepath){ // what about prefix?


	drain::image::TreeSVG & group = getCurrentGroup(ctx);

	drain::image::TreeSVG & image = group[filepath.basename](NodeSVG::IMAGE); // +EXT!
	image->set("name", filepath.basename);
	image->set("width", src.getWidth());
	image->set("height", src.getHeight());
	image->set("xlink:href", filepath);
	image["basename"](drain::image::svg::TITLE) = filepath.basename;

	// Metadata:
	// drain::FlexVariableMap & props = src.properties;
	drain::image::TreeSVG & metadata = image["metadata"](NodeSVG::METADATA);

	// Note assign: char * -> string  , "where:lat", "where:lon"
	if (src.properties.hasKey("what:source")){
		SourceODIM odim(src.properties.get("what:source",""));
		// metadata->set("source", odim.getSourceCode());
		metadata->set("NOD", odim.NOD);
		metadata->set("PLC", odim.PLC);
	}
	for (const std::string key: {"what:date", "what:time", "what:object", "what:quantity", "where:elangle", "where:elangles"}){
		if (src.properties.hasKey(key)){
			size_t i = key.find(':');
			if (i == std::string::npos){
				metadata->set(key, src.properties[key]);
			}
			else {
				metadata->set(key.substr(i+1), src.properties[key]);
			}

		}
	}

}


void CmdBaseSVG::addImage(RackContext & ctx, const drain::image::TreeSVG & svg, const drain::FilePath & filepath){ // what about prefix?

	drain::image::TreeSVG & group = getCurrentGroup(ctx);

	drain::image::TreeSVG & image = group[filepath.basename](NodeSVG::IMAGE); // +EXT!
	image->addClass("float", "legend");
	image->set("width", svg->get("width", 0));
	image->set("height", svg->get("height", 0));
	image->set("xlink:href", filepath);
	image["basename"](drain::image::svg::TITLE) = filepath.basename;

}

// Re-align elements etc
void CmdBaseSVG::completeSVG(RackContext & ctx){

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	drain::image::TreeSVG & track = getMain(ctx);

	drain::image::NodeSVG::path_list_t pathList;
	drain::image::NodeSVG::findByClass(track, "imageSet", pathList);

	/// Clumsy
	drain::image::TreeUtilsSVG::Orientation orientation = TreeUtilsSVG::defaultOrientation;
	drain::image::TreeUtilsSVG::Direction   direction   = TreeUtilsSVG::defaultDirection;

	drain::Frame2D<int> mainFrame;
	drain::Point2D<int> start(0,0);
	for (drain::image::NodeSVG::path_t & p: pathList){
		mout.debug("aligning: ", p);
		drain::image::TreeSVG & group = track[p](NodeSVG::GROUP);
		drain::Frame2D<int> frame;
		TreeUtilsSVG::determineBBox(group, frame);

		// METADATA -> title
		if (false){
			/*
			TreeSVG & titleBox = group["titleBox"](NodeSVG::RECT);
			titleBox->addClass("FIXED");
			titleBox->set("width", frame.width);
			titleBox->set("height", titleBoxHeight);
			titleBox->setStyle("fill:darkblue; opacity:0.25;");
			if (orientation == TreeUtilsSVG::HORZ){
				titleBox->set("x", 0);
				titleBox->set("y", start.y - 10);
			}
			else {
				titleBox->set("x", start.x);
				titleBox->set("y", 0);
			}
			TreeSVG & titleText = group["titleText"](NodeSVG::TEXT);
			titleText->addClass("FLOAT");
			// titleText->setStyle("stroke:darkgreen; font-size: 25");
			titleText = "Radar data";
			*/
		}

		/*
		TreeSVG & titleBox = group["titleBox"](NodeSVG::RECT);
		titleBox->addClass("FLOAT");
		titleBox->setStyle("fill:darkslateblue; opacity:75%;");
		if (orientation == TreeUtilsSVG::HORZ){
			titleBox->set("width", frame.width);
			titleBox->set("height", titleBoxHeight);
			TreeSVG & titleText = group["titleText"](NodeSVG::TEXT);
			titleText->addClass("FLOAT");
			titleText->setStyle("stroke", "lightblue");
			titleText = "Radar data";

		}
		*/


		drain::image::TreeUtilsSVG::align(group, frame, start, orientation, direction);

		/// Collect
		/*
		TitleCollectorSVG titleCollecor;
		drain::TreeUtils::traverse(titleCollecor, group);
		*/

		if (orientation == TreeUtilsSVG::HORZ){
			// Jump to the next "row"
			start.x = 0;
			start.y += frame.height;
			mainFrame.width   = std::max(mainFrame.width, frame.width);
			mainFrame.height += frame.height;
		}
		else {
			// Jump to the next "column"
			start.x += frame.width;
			start.y = 0;
			mainFrame.width  += frame.width;
			mainFrame.height  = std::max(mainFrame.height, frame.height);
		}
		// drain::TreeUtils::dump(group);



	}

	/// Collect
	/*
	MetaDataCollectorSVG metadataCollector;
	drain::TreeUtils::traverse(metadataCollector, ctx.svgTrack);
	drain::TreeUtils::dump(ctx.svgTrack);
	*/

	MetaDataPrunerSVG metadataPruner;
	drain::TreeUtils::traverse(metadataPruner, ctx.svgTrack);

	if (mout.isDebug()){
		mout.special("dumping SVG tree");
		drain::TreeUtils::dump(ctx.svgTrack);
	}

	// REUSE:
	start = {0,0};

	TitleCreatorSVG titleCollector;
	drain::TreeUtils::traverse(titleCollector, ctx.svgTrack);

	if (track.hasChild("metadata")){
		TreeSVG & headerRect = ctx.svgTrack["headerRect"](svg::RECT);
		headerRect->setStyle("fill:slateblue");
		headerRect->setStyle("opacity:0.25");
		headerRect->set("x", 0);
		headerRect->set("y", -titleCollector.mainHeaderHeight);
		headerRect->set("width",  mainFrame.getWidth());
		headerRect->set("height", titleCollector.mainHeaderHeight);

		TreeSVG & headerText = ctx.svgTrack["headerText"](svg::TEXT);
		headerText->set("x", 8);
		headerText->set("y", -8);
		headerText->setStyle({
			{"font-size", "2.5em"},
			{"stroke", "none"},
			{"fill", "black"}
		}); // fill:black;
		// headerText->setText("Radar image");
		for (const auto & entry: track["metadata"]->getAttributes()){
			headerText->ctext += entry.second.toStr();
			headerText->ctext += ' ';
		}

		mainFrame.height +=  titleCollector.mainHeaderHeight;
		start.y           = -titleCollector.mainHeaderHeight;
	}


	ctx.svgTrack->set("width",  mainFrame.width);
	ctx.svgTrack->set("height", mainFrame.height);
	const std::string viewBox = drain::StringBuilder<' '>(start.x, start.y, mainFrame.width, mainFrame.height);
	ctx.svgTrack->set("viewBox", viewBox);

}






/**
 *
 */
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
	/*
	imageElem->x = upperLeft.x;
	imageElem->y = upperLeft.y;
	imageElem->width  = image.getWidth();
	imageELem->height = image.getHeight();
	*/
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


	/*
	TreeSVG::node_data_t & rect = svg["main"];
	main.setType(svg::RECT);
	main.set("x", 0);
	main.set("y", 0);
	//main.set("style", "fill:white opacity:0.8"); // not supported by some SVG renderers
	main.set("fill", "white");
	main.set("opacity", 0.8);
	*/
	/*
	TreeSVG & image = main["image1"];
	image->setType(svg::IMAGE);
	image->set("x", 0);
	image->set("y", 0);
	image->set("width",  src.getWidth());
	image->set("height", src.getHeight());
	//image->set("xlink:href", "image.png");
	 * image->set("href", "image.png");
	*/

	/*
	TreeSVG & header = svg["title"];
	header->setType(svg::TEXT);
	header->set("x", lineheight/4);
	header->set("y", (headerHeight * 9) / 10);
	header->ctext = title;
	header->set("style","font-size:20");
	*/

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
//std::list<std::string> CmdOutputTreeConf::attributeList = {"quantity", "date", "time", "src", "elangle", "task_args", "legend"};
// See drain::TextDecorator::VT100
// std::map<std::string,std::string>



int H5HTMLvisitor::visitPrefix(const Hi5Tree & tree, const Hi5Tree::path_t & odimPath){

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



} // namespace rack
