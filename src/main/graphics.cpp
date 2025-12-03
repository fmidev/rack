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

#include <string>

/*
  const bool IMAGE_PNG = drain::image::FilePng::fileInfo.checkPath(path);
  const bool IMAGE_PNM = drain::image::FilePnm::fileInfo.checkPath(path);
  const bool IMAGE_TIF = drain::image::FileTIFF::fileInfo.checkPath(path);
  const bool IMAGE_SVG = drain::image::NodeSVG::fileInfo.checkPath(path);
*/
#include <drain/image/FilePng.h>
#include <drain/image/TreeSVG.h>
#include <drain/prog/Command.h>
#include <drain/prog/CommandInstaller.h>
#include <drain/prog/CommandBank.h>
#include <drain/util/Output.h>
#include "data/SourceODIM.h" // for NOD

#include "resources.h"
//#include "fileio-svg.h"
#include "graphics.h"
#include "graphics-radar.h"


namespace drain {

/*
template <>
const drain::EnumDict<rack::RackSVG::ElemClass>::dict_t  drain::EnumDict<rack::RackSVG::ElemClass>::dict = {
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, NONE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MAIN),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MAIN_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, GROUP_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, LOCATION),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, TIME),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, GENERAL),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_PANEL),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_BORDER),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, SHARED_METADATA),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, IMAGE_SET)
};
 */


}

namespace rack {

/// Group identifiers for elements which be automatically aligned (stacked horizontally or vertically)


const std::string RackSVG::BACKGROUND_RECT = "bgRect";

drain::image::TreeSVG & RackSVG::getStyle(RackContext & ctx){

	// Consider areas or frames: MAIN_FRAME, GROUP_FRAME, IMAGE_FRAME

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	TreeSVG & style = ctx.svgTrack[svg::STYLE];


	using namespace drain;

	if (style->isUndefined()){

		// mout.debug("initializing style");

		style->setType(svg::STYLE);

		style[svg::TEXT] = {
				{"font-family","Helvetica, Arial, sans-serif"},
				{"stroke", "none"},
		};


		style[SelectorXMLcls(svg::TEXT, PanelConfSVG::IMAGE)] = {
				{"stroke", "white"},
				{"stroke-opacity", "0.75"},
				{"stroke-width", "0.3em"},
				{"fill-opacity", "1"},
				{"paint-order", "stroke"},
				{"stroke-linejoin", "round"}
		};


		style[SelectorXMLcls(svg::RECT, PanelConfSVG::GROUP_TITLE)] = {
				{"fill", "gray"},
				{"stroke", "white"},
				{"stroke-width", "1px"},
				{"opacity", 1.0},
		};

		style[SelectorXMLcls(svg::TEXT, PanelConfSVG::GROUP_TITLE)] = {
				// {"font-size", 20}, dynamic, see below
				{"fill", "black"},
				//{"stroke", "white"},
		};


		style[SelectorXMLcls(svg::RECT, PanelConfSVG::MAIN_TITLE)] = {
				{"fill", "darkblue"},
				//{"fill", "gray"},
				{"opacity", 1.0},
		};

		style[SelectorXMLcls(svg::TEXT, PanelConfSVG::MAIN_TITLE)] = {
				//
				{"fill", "white"},
				// {"stroke", "black"},
		};

		// Currently, image titles have no background RECT, but let's keep this for clarity.
		style[SelectorXMLcls(svg::TEXT,PanelConfSVG::IMAGE)] = {
				{"font-size", 12.0},
		};

		// Date and time.
		style[SelectorXMLcls(PanelConfSVG::TIME)] = {
				{"fill", "darkred"}
		};

		// Radar site
		style[SelectorXMLcls(PanelConfSVG::LOCATION)] = {
				{"fill", "darkblue"}
		};

		// Option: set stroke to make borders appear. Future option: borders OUTSIDE the image.
		style[SelectorXMLcls(PanelConfSVG::IMAGE_BORDER)] = { // TODO: add leading '.' ?
				{"fill", "none"},
				{"stroke", "none"},
				// {"stroke-opacity", 0.0},
				{"stroke-width", 1.0},
				// {"stroke-dasharray", {2,5,3}},
		};

		style[SelectorXMLcls(svg::RECT,PanelConfSVG::SIDE_PANEL)] = { // TODO: add leading '.' ?
				{"fill", "black"},
				{"stroke", "white"},
				{"stroke-width", 2.0},
		};

		style[SelectorXMLcls(svg::TEXT,PanelConfSVG::SIDE_PANEL)] = { // TODO: add leading '.' ?
				{"font-size", 12.0},
				{"stroke", "none"},
				{"fill", "white"},
		};

		/*
		// These could be
		style[SelectorXMLcls("GRID")] = {
			{"stroke", "white"},
			{"stroke-width", 1.0},
			{"fill", "none"}, // debug
			//{"fill", "blue"}, // debug
			//{"fill-opacity", 0.35},
		};

		style[SelectorXMLcls(svg::TEXT, "GRID")] = {
				{"text-anchor", "middle"},
				{"font-size", ctx.svgPanelConf.fontSizes[1]},
				// {"font-size", 20.0},
				{"paint-order", "stroke"},
				{"stroke", "black"},
				{"stroke-opacity", "0.5"},
				{"stroke-width", "0.3em"},
				{"stroke-linejoin", "round"},
				{"fill", "white"}, // debug
				{"fill-opacity", "1"},
		};
		*/

	}

	style[SelectorXMLcls(PanelConfSVG::MAIN_TITLE)] ->set("font-size", ctx.svgPanelConf.fontSizes[0]);
	style[SelectorXMLcls(PanelConfSVG::GROUP_TITLE)]->set("font-size", ctx.svgPanelConf.fontSizes[1]);
	style[SelectorXMLcls(PanelConfSVG::IMAGE)]->set("font-size", ctx.svgPanelConf.fontSizes[2]);

	return style;
}
// const drain::StatusFlags::value_t RackContext::BBOX_STATUS =   StatusFlags::add("BBOX");

const std::string & RackSVG::guessFormat(const std::string & key){

		//v = attr.second.toStr();
		if (drain::StringTools::endsWith(key, "date")){
			static const std::string s("%Y/%m/%d");
			return s;
		}
		else if ((key == "time") || (key == "what:time")){
			//else if (drain::StringTools::endsWith(key, ":time")){
			static const std::string s("%H:%M UTC");
			return s;
		}
		else if (drain::StringTools::endsWith(key, "time")){
			static const std::string s("%H:%M:%S UTC");
			return s;
		}
		else {
			static const std::string s;
			return s;
		}
		// mout.accept<LOG_DEBUG>("TIME text format", format);
}


/// Apply an alignment, to next object only
/*
 *
 *  This could be in GraphicsContext, but ctx.log should be virtualized first, like getLog():
 */
//void RackSVG::applyAlignment(RackContext & ctx, drain::image::TreeSVG & group){
void RackSVG::consumeAlignRequest(RackContext & ctx, drain::image::NodeSVG & node){

	// TODO: return flags of set aligns?

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	if (ctx.anchorHorz.isSet()){
		node.setMyAlignAnchor<AlignBase::HORZ>(ctx.anchorHorz);
		ctx.anchorHorz.clear();
	}

	if (ctx.anchorVert.isSet()){
		node.setMyAlignAnchor<AlignBase::VERT>(ctx.anchorVert);
		ctx.anchorVert.clear();
	}

	//if (ctx.alignHorz.pos != AlignBase::UNDEFINED_POS){
	if (ctx.alignHorz.isSet()){ // experimental...
		node.setAlign(AlignBase::HORZ, ctx.alignHorz.pos, ctx.alignHorz.getOrDefault(AlignSVG::INSIDE));  // simplify
		node.addClass(LayoutSVG::INDEPENDENT); // why is this? hmm explicitly, specifically aligned element is "separate"
		// mout.attention("updated Horz align: ",  node); // , " -> all:", node.getAlignStr()
		ctx.alignHorz.reset();
		ctx.alignHorz.set(AlignBase::UNDEFINED_POS, AlignSVG::INSIDE);
		//mout.attention(" HORZ state now: ", ctx.alignHorz);
	}


	//if (ctx.alignVert.pos != AlignBase::UNDEFINED_POS){
	if (ctx.alignVert.isSet()){ // experimental...
		node.setAlign(AlignBase::VERT, ctx.alignVert.pos, ctx.alignVert.getOrDefault(AlignSVG::INSIDE)); // simplify
		node.addClass(LayoutSVG::INDEPENDENT); // why is this? hmm explicitly, specifically aligned element is "separate"
		// mout.attention("updated Vert align: ",  node); //  " -> all:", node.getAlignStr()
		ctx.alignVert.reset();
		ctx.alignVert.set(AlignBase::UNDEFINED_POS, AlignSVG::INSIDE);
		// mout.attention(" VERT state now: ", ctx.alignVert);
	}


}

//bool RackSVG::applyInclusion(RackContext & ctx, SvgInclude format){
bool RackSVG::applyInclusion(RackContext & ctx, const drain::FilePath & filepath){

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	//mout.special<LOG_WARNING>("NOW file: ", filepath, ", includes=", ctx.svgPanelConf.svgIncludes);
	mout.special<LOG_DEBUG>("NOW file: ", filepath, ", includes=", ctx.svgPanelConf.svgIncludes);

	SvgInclude format = UNKNOWN; // UNKNOWN
	if (drain::image::FilePng::fileInfo.checkExtension(filepath.extension)){
		format = PNG;
	}
	else if (drain::image::NodeSVG::fileInfo.checkExtension(filepath.extension)){
		format = SVG;
	}
	// + TXT ?


	// Main selection: format (PNG/SVG/TXT) is accepted
	//
	if (!ctx.svgPanelConf.svgIncludes.isSet(format)){
		mout.reject<LOG_INFO>("format=", format, ", skipped file: ", filepath);
		return false;
	}
	else if (ctx.svgPanelConf.svgIncludes.isSet(SvgInclude::SKIP)){
		mout.reject<LOG_INFO>("explicitly SKIPped file: ", filepath);
		ctx.svgPanelConf.svgIncludes.unset(SvgInclude::SKIP);
		return false;
	}
	else if (ctx.svgPanelConf.svgIncludes.isSet(SvgInclude::NEXT)){
		// Could warn, if also ON = "double-on"
		mout.info("explicitly included (", SvgInclude::NEXT, ") file: ", filepath);
		ctx.svgPanelConf.svgIncludes.unset(SvgInclude::NEXT);
		return true;
	}
	else {
		mout.pending<LOG_DEBUG>("considering file: ", filepath, ", format=", format);
		return ctx.svgPanelConf.svgIncludes.isSet(SvgInclude::ON);
	}

}


drain::image::TreeSVG & RackSVG::getMainGroup(RackContext & ctx){ // , const std::string & name

	//using namespace drain::image;
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// Ensure STYLE elem and definitions
	RackSVG::getStyle(ctx);
	// ctx.getStyle();
	// drain::image::TreeSVG & main = ctx.svgTrack[ctx.svgGroupNameSyntax]; <- this makes sense as well

	drain::image::TreeSVG & main = ctx.svgTrack["MAIN"]; //drain::image::LayoutSVG::
	if (main->isUndefined()){
		main->setType(svg::GROUP);
		main->addClass("MAIN"); // TitleCreatorSVG::visitPostfix
		main->addClass(LayoutSVG::ADAPTER);
		// request translate upon file write:?
		// mout.attention("Created MAIN, ", PanelConfSVG::MAIN, ": ", main.data, " / ", main.data.getType());
	}
	// mout.attention("Providing MAIN, ", PanelConfSVG::MAIN, ": ", main.data, " / ", main.data.getType());

	return main;

}

// Need ctx.

/// Return current row or column of image panels.
// Add later to PanelConf (or from there, LayoutSVG?)
//static
//const std::string ADAPTER("ADAPTER");

/** Intermediate group "hiding" translation that moves upper left corner of the object to the origin.
 *
 */
drain::image::TreeSVG & getAdapterGroup(drain::image::TreeSVG & group){

	// EnumDict<drain::image::LayoutSVG::GroupType>::dict::getKey(LayoutSVG::ADAPTER)

	drain::image::TreeSVG & adapterGroup = group["ADAPTER"];

	if (adapterGroup->isUndefined()){
		adapterGroup->setType(svg::GROUP);
		adapterGroup->addClass(LayoutSVG::ADAPTER);
		adapterGroup->transform.translate.set(0,0);

		// Parking lots!
		adapterGroup[svg::IMAGE]->setType(svg::GROUP);
		adapterGroup[svg::IMAGE]->addClass("RASTERS"); // set("data-debug", "IMAGEs");
		adapterGroup[RadarSVG::VECTOR_OVERLAY]; // parking lot ->setType(svg::GROUP);
		// adapterGroup[RadarSVG::VECTOR_OVERLAY]; //->set("data-debug", "VECTOR_OVERLAYs");

	}
	return adapterGroup;

	// group->addClass(LayoutSVG::ADAPTER);
	// return group;

}



drain::image::TreeSVG & RackSVG::getCurrentAlignedGroup(RackContext & ctx){ // what about prefix?

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	drain::image::TreeSVG & mainGroup = getMainGroup(ctx);

	//ctx.svgPanelConf.groupTitleFormatted = ctx.getFormattedStatus(ctx.svgPanelConf.groupIdentifier); // status updated upon last file save
	std::string groupId = ctx.getFormattedStatus(ctx.svgPanelConf.groupIdentifier);
	std::string groupTitleFormatted = ctx.getFormattedStatus(ctx.svgPanelConf.groupTitle); // status updated upon each PNG file save


	//drain::image::TreeSVG & alignedGroup = mainGroup[ctx.svgPanelConf.groupTitleFormatted];
	drain::image::TreeSVG & alignedGroup = mainGroup[groupId];

	if (alignedGroup -> isUndefined()){
		alignedGroup->setType(svg::GROUP);
		alignedGroup->set("data-id",    groupId);
		alignedGroup->set("data-title", groupTitleFormatted);
		alignedGroup->addClass(drain::image::LayoutSVG::STACK_LAYOUT);
	}

	//return alignedGroup;

	// Later, include perhaps here...
	drain::image::TreeSVG & adapterGroup = getAdapterGroup(alignedGroup);

	// TreeSVG & geoGroup =
	// alignedGroup[RadarSVG::VECTOR_OVERLAY]; // reserve a parking lot? Maybe unneeded.
	/*
	adapterGroup[svg::IMAGE]->setType(svg::GROUP);
	adapterGroup[svg::IMAGE]->addClass("RASTERS"); // set("data-debug", "IMAGEs");
	adapterGroup[RadarSVG::VECTOR_OVERLAY]; // parking lot ->setType(svg::GROUP);
	adapterGroup[RadarSVG::VECTOR_OVERLAY]; //->set("data-debug", "VECTOR_OVERLAYs");
	*/
	return adapterGroup;

}


drain::image::TreeSVG & RackSVG::getImagePanelGroup(RackContext & ctx){

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	const drain::VariableMap & map = ctx.getStatusMap();

	drain::FilePath filepath(map.get("outputPrefix", ""), map.get("outputFile", ""));

	mout.experimental("external image panel request: ", filepath);

	return getImagePanelGroup(ctx, filepath);
}


/// For each image an own group is created (for clarity, to contain also title TEXT's etc)
/**
 *
 */


drain::image::TreeSVG & RackSVG::getImagePanelGroup(RackContext & ctx, const drain::FilePath & filepath){

	// For each image an own group is created to contain also title TEXT's etc.
	const std::string name = drain::StringBuilder<'-'>(filepath.tail, filepath.extension);

	drain::image::TreeSVG & alignFrame = getCurrentAlignedGroup(ctx);

	// drain::image::TreeSVG & imagePanel = alignFrame[name];
	drain::image::TreeSVG & imageGroup = alignFrame[svg::IMAGE]; // (svg::GROUP);
	drain::image::TreeSVG & imagePanel = imageGroup[name];

	if (imagePanel->isUndefined()){

		imagePanel->setType(svg::GROUP);
		imagePanel->setId(name);
		imagePanel->setDefaultAlignAnchor(svg::IMAGE);

		drain::image::TreeSVG & image = imagePanel[svg::IMAGE](svg::IMAGE); // +EXT!

		image->setId(filepath.tail); // unneeded, as TITLE also has it?
		image->setUrl(filepath.str());
		image[drain::image::svg::TITLE](drain::image::svg::TITLE) = filepath.tail;
	}

	imagePanel->addClass(PanelConfSVG::IMAGE_PANEL);
	return imagePanel;

}


/// Add pixel image (PNG).
/**
 *   Idea: keep this minimal, add components only upon (eventual) saving of the SVG file.
 */
void RackSVG::addImage(RackContext & ctx, const drain::image::Image & src, const drain::FilePath & filepath){ // what about prefix?

	if (!applyInclusion(ctx, filepath)){
		return;
	}

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	mout.debug("file path:", filepath);

	//drain::image::TreeSVG & imagePanel = ctx.getImagePanelGroup(filepath); // getImagePanelGroup(ctx, filepath);
	drain::image::TreeSVG & imagePanel = getImagePanelGroup(ctx, filepath); // getImagePanelGroup(ctx, filepath);
	imagePanel->addClass(PanelConfSVG::IMAGE_PANEL); // Add elems ^ here ^ ?

	// problematic, as init value!
	consumeAlignRequest(ctx, imagePanel);

	drain::image::TreeSVG & image = imagePanel[svg::IMAGE](svg::IMAGE); // +EXT!
	image->addClass(LayoutSVG::FIXED);
	image->setLocation(0,0);
	image->setFrame(src.getGeometry().area);
	addImageBorder(imagePanel);


	// Metadata:
	drain::image::TreeSVG & metadata = imagePanel[svg::METADATA](svg::METADATA);

	// Note assign: char * -> string  , "where:lat", "where:lon"
	if (src.properties.hasKey("what:source")){
		SourceODIM odim(src.properties.get("what:source",""));
		metadata->set("NOD", odim.NOD);
		metadata->set("PLC", odim.PLC);
	}

	// TODO: 1) time formatting 2) priority (startdate, starttime)
	for (const std::string key: {
		"what:date", "what:time", "what:product", "what:prodpar", "what:quantity",
		"where:elangle", "where:lon", "where:lat", "where:EPSG", // "where:projdef",
		"how:camethod",
		"prevCmdKey"}){

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


	drain::image::TreeSVG & description = image[svg::DESC](svg::DESC);
	description->getAttributes().importCastableMap(metadata->getAttributes());
	// todo: description  : prevCmdKey "what:product", "what:prodpar", "how:angles"


}

/// Add SVG image
/**
 *
 */
// drain::image::TreeSVG &
void RackSVG::addImage(RackContext & ctx, const drain::image::TreeSVG & svg, const drain::FilePath & filepath){ // what about prefix?

	const drain::Frame2D<drain::image::svg::coord_t> frame(svg->getBoundingBox().getFrame());
	addImage(ctx, frame, filepath);

}

/// Add pixel image (PNG)
// drain::image::TreeSVG &
void RackSVG::addImage(RackContext & ctx, const drain::Frame2D<drain::image::svg::coord_t> & frame, const drain::FilePath & filepath, const std::string & styleClass){ // what about prefix?

	if (!applyInclusion(ctx, filepath)){
		return;
	}

	drain::image::TreeSVG & imagePanel = getImagePanelGroup(ctx, filepath);
	//imagePanel->addClass(PanelConfSVG::IMAGE_PANEL);
	consumeAlignRequest(ctx, imagePanel);

	drain::image::TreeSVG & image = imagePanel[svg::IMAGE](svg::IMAGE);
	image->addClass(LayoutSVG::FIXED);
	image->setLocation(0,0);
	image->setFrame(frame);

	if (!styleClass.empty()){
		imagePanel->addClass(styleClass);
	}

	return; //  imagePanel;
}

drain::image::TreeSVG & RackSVG::addImageBorder(drain::image::TreeSVG & imagePanelGroup){ //, const drain::Frame2D<drain::image::svg::coord_t> & frame){
	drain::image::TreeSVG & imageBorder = imagePanelGroup[PanelConfSVG::ElemClass::IMAGE_BORDER](svg::RECT); // +EXT!
	imageBorder->addClass(PanelConfSVG::ElemClass::IMAGE_BORDER); // style
	imageBorder->addClass(drain::image::LayoutSVG::INDEPENDENT);  // next object won't treat me as anchor (unless specifically called for)
	imageBorder->addClass(drain::image::LayoutSVG::INEFFECTIVE);  // does not expand COMPOUND bbox
	imageBorder->setAlign(drain::image::AlignSVG::HORZ_FILL, drain::image::AlignSVG::VERT_FILL);
	return imageBorder;
}



drain::image::TreeSVG & RackSVG::addTitleBox(const PanelConfSVG & conf, drain::image::TreeSVG & object, PanelConfSVG::ElemClass elemClass){

	drain::image::TreeSVG & backgroundRect = object[BACKGROUND_RECT](svg::RECT);
	backgroundRect->addClass(elemClass);
	//backgroundRect->setAlignAnchorHorz("*"); // only if HORZ-INCR?
	backgroundRect->setMyAlignAnchor(AnchorElem::Anchor::COLLECTIVE_CURRENT); // ("*");
	backgroundRect->setAlign(AlignSVG::HORZ_FILL);
	// backgroundRect->setHeight(40); // TODO!!

	// Lower... for GENERAL as well.
	switch (elemClass) {
	case PanelConfSVG::ElemClass::MAIN_TITLE:
		backgroundRect->setId(PanelConfSVG::ElemClass::MAIN_TITLE);
		backgroundRect->setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE);
		backgroundRect->setHeight(conf.boxHeights[0]);
		break;
	case PanelConfSVG::ElemClass::GROUP_TITLE:
		backgroundRect->setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE);
		backgroundRect->setHeight(conf.boxHeights[1]);
		break;
	default:
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.suspicious("Unhandled elemClass: ", elemClass);
		break;
	}

	return appendTitleElements(conf, object, BACKGROUND_RECT, elemClass);


}

drain::image::TreeSVG & RackSVG::appendTitleElements(const PanelConfSVG &conf, drain::image::TreeSVG &group, const std::string &anchor, PanelConfSVG::ElemClass elemClass) {

	/** TODO
	 const double fontSize = // getStyleValue(root, RackSVG::TITLE, "font-size", 12.5);
	 root[drain::image::svg::STYLE][elemClass]->get("font-size", 12.5);
	 */
	drain::Logger mout(__FILE__, __FUNCTION__);

	TreeSVG &mainHeader = group[PanelConfSVG::ElemClass::GENERAL](svg::TEXT);
	mainHeader->addClass(LayoutSVG::INEFFECTIVE);
	mainHeader->addClass(elemClass); // also GENERAL?
	mainHeader->setMyAlignAnchor(anchor);

	// Ensure order
	mainHeader["product"](svg::TSPAN);
	mainHeader["product"]->addClass("product");
	// mainHeader["product"]->setText("product"); //CTXX // debugging
	mainHeader["prodpar"](svg::TSPAN);
	mainHeader["prodpar"]->addClass("product"); // yes, same...
	// NEW
	mainHeader["quantity"](svg::TSPAN);
	mainHeader["quantity"]->addClass("product");
	mainHeader["camethod"](svg::TSPAN);
	mainHeader["camethod"]->addClass("product");

	// Layout principle: there should be always time... so start/continue from left.
	TreeSVG &timeHeader = group[PanelConfSVG::ElemClass::TIME](svg::TEXT);
	timeHeader->addClass(elemClass, PanelConfSVG::ElemClass::TIME);
	timeHeader->addClass(LayoutSVG::INEFFECTIVE);
	timeHeader->setMyAlignAnchor(anchor);
	timeHeader["date"](svg::TSPAN);
	timeHeader["date"]->addClass("date"); // PanelConfSVG::ElemClass::TIME);
	timeHeader["time"](svg::TSPAN);
	timeHeader["time"]->addClass("time"); // PanelConfSVG::ElemClass::TIME);
	// Layout principle: there should be always time... so start/continue from left.

	TreeSVG &locationHeader = group[PanelConfSVG::ElemClass::LOCATION](svg::TEXT);
	locationHeader->addClass(elemClass, PanelConfSVG::ElemClass::LOCATION);
	locationHeader->addClass(LayoutSVG::INEFFECTIVE); // testing LayoutSVG::INEFFECTIVE
	locationHeader->setMyAlignAnchor(anchor);
	locationHeader["NOD"](svg::TSPAN);
	locationHeader["NOD"]->addClass("NOD");
	locationHeader["PLC"](svg::TSPAN);
	locationHeader["PLC"]->addClass("PLC");



	double textBoxHeight = 0.0;
	double textBoxMargin = 0.0;
	switch (elemClass) {
	case PanelConfSVG::ElemClass::MAIN_TITLE:
		textBoxHeight = conf.boxHeights[0]; // fontSizes
		mainHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER);
		timeHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::LEFT);
		locationHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::RIGHT);
		break;
	case PanelConfSVG::ElemClass::GROUP_TITLE:
		textBoxHeight = conf.boxHeights[1]; // fontSizes
		mainHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER);
		timeHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::LEFT);
		locationHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::RIGHT);
		break;
	case PanelConfSVG::ElemClass::IMAGE:
		textBoxHeight = conf.boxHeights[2]; // fontSizes
		if (textBoxHeight > 0.0) {
			mainHeader->setAlign(AlignSVG::BOTTOM, AlignSVG::LEFT);
			timeHeader->setAlign(AlignSVG::TOP, AlignSVG::LEFT);
			locationHeader->setAlign(AlignSVG::TOP, AlignSVG::RIGHT);
		}
		else {
			mainHeader->setComment("removed");
			timeHeader->setComment("removed");
			locationHeader->setComment("removed");
			return mainHeader;
			// return;
		}
		break;
	default:
		mout.error(__LINE__, "unhandled ElemClass", elemClass);
		break;
	}
	textBoxMargin = textBoxHeight * 0.25;
	mainHeader->setHeight(textBoxHeight);
	mainHeader->setMargin(textBoxMargin); // adjust
	timeHeader->setHeight(textBoxHeight);
	timeHeader->setMargin(textBoxMargin); // adjust
	locationHeader->setHeight(textBoxHeight);
	locationHeader->setMargin(textBoxMargin); // adjust

	return mainHeader;
}




void TitleCreatorSVG::formatTitle(TreeSVG & group, const NodeSVG::map_t & attributes) const {

	if (attributes.empty()){
		return;
	}

	drain::Logger mout(__FILE__, __FUNCTION__);

	VariableFormatterODIM<std::string> formatter; // (No instance properties used, but inheritance/overriding)

	for (const auto & attr: attributes){

		std::string key, format;
		drain::StringTools::split2(attr.first, key, format, '|');

		PanelConfSVG::ElemClass elemClass = PanelConfSVG::ElemClass::GENERAL;

		if ((ODIM::timeKeys.count(attr.first)>0) || (ODIM::dateKeys.count(attr.first)>0)){
			elemClass = PanelConfSVG::ElemClass::TIME;
		}
		else if (ODIM::locationKeys.count(attr.first)>0){
			elemClass = PanelConfSVG::ElemClass::LOCATION;
		}

		if (!group.hasChild(elemClass)){
			// getGroupTitle
			RackSVG::addTitleBox(this->svgConf, group, PanelConfSVG::GROUP_TITLE);
			TreeSVG & t = group[elemClass];
			t[svg::COMMENT]->setComment("added: ", attr.first, ' ', attr.second);
		}

		TreeSVG & text  = group[elemClass];
		if (text->isUndefined()){
			mout.suspicious("Expected TEXT elem, but undefined:", NodePrinter(group).str());
			// text->setType(svg::TEXT);
			// text[svg::COMMENT]->setComment("skipped: ", attr.first, ' ', attr.second);
		}

		TreeSVG & tspan = text[attr.first](svg::TSPAN);
		tspan->addClass(attr.first); // allows user-specified style

		if (elemClass == PanelConfSVG::ElemClass::TIME){
			if (format.empty()){
				format = RackSVG::guessFormat(key);
				text->set("data-format", format);
			}
			else {
				text->set("data-format", format); // DEBUG
			}
			// mout.accept<LOG_DEBUG>("TIME text format", format);
		}
		else if (elemClass == PanelConfSVG::ElemClass::LOCATION){
			// text->setAlign(AlignSVG::BOTTOM, AlignSVG::RIGHT);
		}
		else {
			// consider more classes later!
			// text->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER);
			// text->setText(drain::StringBuilder<'|'>(elemClass, attr.first, attr.second)); //CTXX
		}

		// mout.attention("handle: ", attr.first, " ", v, " + ", format);

		if (format.empty()){
			tspan->setTextSafe(attr.second, ' ');
		}
		else {
			//mout.attention("handle: ", attr.first, " ", v, " + ", format);
			std::stringstream sstr;
			formatter.formatVariable(key, attr.second, format, sstr);
			// tspan->setText(sstr.str(), " ");
			// tspan->setText(sstr.str(), drain::XML::entity_t::NONBREAKABLE_SPACE); // "&#160;",  non-breakable space
			tspan->setTextSafe(sstr.str(), ' '); // "&#160;",  non-breakable space
		}

	}



}


int TitleCreatorSVG::visitPostfix(TreeSVG &root, const TreeSVG::path_t &path){

	drain::Logger mout(__FILE__, __FUNCTION__);
	// Apply to groups only.

	TreeSVG &group = root(path);

	if (!group->typeIs(svg::GROUP)) {
		return 0;
	}

	if (!group.hasChild(svg::METADATA)) {
		mout.reject<LOG_DEBUG>("skipping, group has no METADATA element: ", group.data);
		return 0;
	}

	if (group->hasClass(LayoutSVG::ADAPTER)) {
	// 	return 0;
	}

	const NodeSVG::map_t &attributesPrivate = group[svg::METADATA]->getAttributes();
	const NodeSVG::map_t &attributesShared  = group[MetaDataCollectorSVG::SHARED]->getAttributes();
	// const bool WRITE_PRIVATE_METADATA = !attributesPrivate.empty();

	// bool WRITE_SHARED_METADATA = !attributesShared.empty();

	if (svgConf.groupTitle == "NONE") {
		mout.obsolete("groupTitle 'NONE'");
	}

	// const bool MAIN_AUTO  =  (svgConf.mainTitle == "AUTO");
	// const bool GROUP_AUTO =  (svgConf.groupTitle == "AUTO"); // (svgConf.groupTitleFormatted.substr(0,4) == "AUTO");
	// const bool GROUP_NONE =  (svgConf.groupTitle.empty()); // (svgConf.groupTitleFormatted.substr(0,4) == "NONE");
	// const bool GROUP_USER = !(svgConf.groupTitleFormatted.empty() || GROUP_AUTO || GROUP_NONE);
	// const bool GROUP_USER = !(GROUP_AUTO || GROUP_NONE);

	//if (group->hasClass(PanelConfSVG::ElemClass::MAIN_TITLE)) {
	if (group->hasClass("MAIN")) {
		if (svgConf.mainTitle.empty()){
			group[PanelConfSVG::ElemClass::MAIN_TITLE]->setComment("explicitly empty MAIN_TITLE – skipped");
		}
		else {
			TreeSVG & mainText = RackSVG::addTitleBox(svgConf, group, PanelConfSVG::ElemClass::MAIN_TITLE);
			if (svgConf.mainTitle == "AUTO"){
				formatTitle(group, attributesPrivate);
				formatTitle(group, attributesShared);
			}
			else {
				mainText->setTextSafe(svgConf.mainTitle);
			}
		}
	}
	else if (group->hasClass(LayoutSVG::STACK_LAYOUT)) {
		if (svgConf.groupTitle.empty()){
			group[PanelConfSVG::ElemClass::GROUP_TITLE]->setComment("explicitly empty GROUP_TITLE – skipped");
		}
		else {
			TreeSVG & adapterGroup = getAdapterGroup(group);
			TreeSVG & mainText = RackSVG::addTitleBox(svgConf, adapterGroup, PanelConfSVG::ElemClass::GROUP_TITLE);
			if (svgConf.groupTitle == "AUTO"){
				formatTitle(adapterGroup, attributesPrivate);
				if (svgConf.mainTitle.empty()){
					// Flush the attributes not displayed this far. As AUTO was requested...
					formatTitle(adapterGroup, attributesShared);
				}
			}
			else {
				// group->get("data-title", "");
				mainText->setTextSafe(group->get("data-title", "- - -"));
			}
		}
	}
	else if (group->hasClass(PanelConfSVG::ElemClass::IMAGE_PANEL)) {
		// Add elements directly on the image (corners), skip creating a background rectangle.
		// Use IMAGE element "image" as anchor.
		RackSVG::appendTitleElements(svgConf, group, "image", PanelConfSVG::ElemClass::IMAGE);
		formatTitle(group, attributesPrivate);
	}
	else {
		// "Neutral groups go here"
		mout.debug("Skipped group:", NodePrinter(group).str());
	}


	/*
	 if (!group.hasChild(svg::METADATA)){
	 mout.attention("group has no METADATA element: ", group.data);
	 return 0;
	 }
	 */
	// Always
	/*
	formatTitle(group, attributesPrivate);
	if (WRITE_SHARED_METADATA && !group->hasClass(PanelConfSVG::ElemClass::IMAGE_PANEL)) {
		formatTitle(group, attributesShared);
	}
	*/
	/*
	 if (WRITE_SHARED_METADATA && group.hasChild(PanelConfSVG::ElemClass::SHARED_METADATA)){ // explicit request: GROUP
	 writeTitles(group, group[PanelConfSVG::ElemClass::SHARED_METADATA]->getAttributes());
	 }
	 */
	return 0;
}



} // rack


