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


		style[SelectorXMLcls(svg::RECT, PanelConfSVG::GROUP)] = {
				{"fill", "gray"},
				{"opacity", 1.0},
		};

		style[SelectorXMLcls(svg::TEXT, PanelConfSVG::GROUP)] = {
				// {"font-size", 20}, dynamic, see below
				{"fill", "black"},
		};


		style[SelectorXMLcls(svg::RECT, PanelConfSVG::MAIN)] = {
				{"fill", "darkblue"},
				{"opacity", 1.0},
		};

		style[SelectorXMLcls(svg::TEXT, PanelConfSVG::MAIN)] = {
				{"fill", "white"},
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

	}

	style[SelectorXMLcls(PanelConfSVG::MAIN)] ->set("font-size", ctx.svgPanelConf.fontSizes[0]);
	style[SelectorXMLcls(PanelConfSVG::GROUP)]->set("font-size", ctx.svgPanelConf.fontSizes[1]);
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

drain::image::TreeSVG & RackSVG::getMainGroup(RackContext & ctx){ // , const std::string & name

	//using namespace drain::image;
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// Ensure STYLE elem and definitions
	RackSVG::getStyle(ctx);
	// ctx.getStyle();
	// drain::image::TreeSVG & main = ctx.svgTrack[ctx.svgGroupNameSyntax]; <- this makes sense as well

	drain::image::TreeSVG & main = ctx.svgTrack[PanelConfSVG::MAIN];
	if (main -> isUndefined()){
		main->setType(svg::GROUP);
		main->addClass(PanelConfSVG::MAIN);
		// mout.attention("Created MAIN, ", PanelConfSVG::MAIN, ": ", main.data, " / ", main.data.getType());
	}
	// mout.attention("Providing MAIN, ", PanelConfSVG::MAIN, ": ", main.data, " / ", main.data.getType());

	return main;

}

/*
 *   This could be in GraphicsContext, but ctx.log should be virtualized first, like getLog():
 */
void RackSVG::applyAlignment(RackContext & ctx, drain::image::TreeSVG & group){

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	if (ctx.alignHorz.pos != AlignBase::UNDEFINED_POS){


		group->setAlign(AlignBase::HORZ, ctx.alignHorz.pos, ctx.alignHorz.get(AlignSVG::INSIDE));  // simplify
		mout.unimplemented<LOG_NOTICE>("Set: ", ctx.alignHorz, " -> ", group->getAlignStr());
		// ctx.alignHorz.pos  = AlignSVG::UNDEFINED_TOPOL;
		group->addClass(LayoutSVG::FLOAT); // what is this?
		mout.attention("updated align: ",  group.data); // , " -> all:", group->getAlignStr()

		ctx.alignHorz.reset();
		ctx.alignHorz.set(AlignBase::UNDEFINED_POS, AlignSVG::INSIDE);
		// ctx.alignHorz.pos == AlignBase::UNDEFINED_POS
		mout.attention(" HORZ state now: ", ctx.alignHorz);
	}
	/*
	else {
		group->setAlign(ctx.alignHorz);
		// group->setAlign(AlignSVG::RIGHT, AlignSVG::OUTSIDE); // AlignSVG::LEFT);
		mout.accept<LOG_NOTICE>("Using HORZ align: ", ctx.alignHorz, " -> ", group->getAlignStr());
	}
	 */

	if (ctx.alignVert.pos != AlignBase::UNDEFINED_POS){
		group->setAlign(AlignBase::VERT, ctx.alignVert.pos, ctx.alignVert.get(AlignSVG::INSIDE)); // simplify
		mout.unimplemented<LOG_NOTICE>("Set: ", ctx.alignVert, " -> ", group->getAlignStr());
		// ctx.alignVert.pos  = AlignSVG::UNDEFINED_TOPOL;
		group->addClass(LayoutSVG::FLOAT);
		mout.attention("updated align: ",  group.data); //  " -> all:", group->getAlignStr()
		ctx.alignVert.reset();
		ctx.alignVert.set(AlignBase::UNDEFINED_POS, AlignSVG::INSIDE);
		mout.attention(" VERT state now: ", ctx.alignVert);
		// ctx.alignVert.pos != AlignBase::UNDEFINED_POS
	}
	/*
	else {
		// group->setAlign(AlignSVG::TOP, AlignSVG::INSIDE); // AlignSVG::BOTTOM);
		group->setAlign(ctx.alignVert);
		// group->setAlign(AlignSVG::RIGHT, AlignSVG::OUTSIDE); // AlignSVG::LEFT);
		mout.accept<LOG_NOTICE>("Using VERT align: ", ctx.alignVert, " -> ", group->getAlignStr());
	}
	 */


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
// Need ctx.

/// Return current row or column of image panels.

drain::image::TreeSVG & RackSVG::getCurrentAlignedGroup(RackContext & ctx){ // what about prefix?

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	//drain::image::TreeSVG & mainGroup = ctx.getMainGroup();
	drain::image::TreeSVG & mainGroup = getMainGroup(ctx);

	// mout.attention(); drain::TreeUtils::dump(mainGroup, mout); mout.end();

	//drain::image::NodeSVG::toStream(ostr, tree, defaultTag, indent)

	//ctx.svgGroupNameFormatted = ctx.getFormattedStatus(ctx.svgGroupNameSyntax); // status updated upon last file save
	// drain::image::TreeSVG & alignedGroup = mainGroup[ctx.svgGroupNameFormatted];

	ctx.svgPanelConf.groupTitleFormatted = ctx.getFormattedStatus(ctx.svgPanelConf.groupTitleSyntax); // status updated upon last file save

	drain::image::TreeSVG & alignedGroup = mainGroup[ctx.svgPanelConf.groupTitleFormatted];

	if (alignedGroup -> isUndefined()){
		alignedGroup->setType(svg::GROUP);
		alignedGroup->setId(ctx.svgPanelConf.groupTitleFormatted);
		alignedGroup->addClass(drain::image::LayoutSVG::ALIGN_FRAME);
	}

	return alignedGroup;

}



/// For each image an own group is created (for clarity, to contain also title TEXT's etc)
/**
 *
 */

drain::image::TreeSVG & RackSVG::getImagePanelGroup(RackContext & ctx, const drain::FilePath & filepath){

	// For each image an own group is created to contain also title TEXT's etc.
	const std::string name = drain::StringBuilder<'-'>(filepath.basename, filepath.extension);

	drain::image::TreeSVG & alignFrame = getCurrentAlignedGroup(ctx);

	// drain::image::TreeSVG & comment = alignFrame[svg::COMMENT](svg::COMMENT);
	// comment->setText("start of ", LayoutSVG::ALIGN_FRAME, ' ', name, svg::GROUP);

	drain::image::TreeSVG & imagePanel = alignFrame[name];

	if (imagePanel->isUndefined()){

		imagePanel->setType(svg::GROUP);
		imagePanel->setId(name);

		drain::image::TreeSVG & image = imagePanel[svg::IMAGE](svg::IMAGE); // +EXT!
		image->setId(filepath.basename); // unneeded, as TITLE also has it?
		image->setUrl(filepath.str());
		/*
		image->set("data-dir", filepath.dir.str());
		if (filepath.dir.hasRoot()){
			image->setUrl(filepath.str());
		}
		else {
			// Append "relative" root, './' to skip support colon ':' in filenames
			//image->setUrl(drain::FilePath::path_t(".", filepath).str());
			image->setUrl(drain::StringBuilder<'/'>(".", filepath.str()));
		}
		*/
		image[drain::image::svg::TITLE](drain::image::svg::TITLE) = filepath.basename;
	}

	return imagePanel;

}


/// Add pixel image (PNG)
// drain::image::TreeSVG &
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
	applyAlignment(ctx, imagePanel);

	drain::image::TreeSVG & image = imagePanel[svg::IMAGE](svg::IMAGE); // +EXT!
	image->setFrame(src.getGeometry().area);
	addImageBorder(imagePanel); //, src.getGeometry().area);




	// Metadata:
	drain::image::TreeSVG & metadata = imagePanel[svg::METADATA](svg::METADATA);

	// Note assign: char * -> string  , "where:lat", "where:lon"
	if (src.properties.hasKey("what:source")){
		SourceODIM odim(src.properties.get("what:source",""));
		metadata->set("NOD", odim.NOD);
		metadata->set("PLC", odim.PLC);
	}

	// TODO: 1) time formatting 2) priority (startdate, starttime)
	for (const std::string key: {"what:date", "what:time", "what:product", "what:prodpar", "what:quantity", "where:elangle", "how:camethod", "prevCmdKey"}){
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
	imagePanel->addClass(PanelConfSVG::IMAGE_PANEL);
	applyAlignment(ctx, imagePanel);

	drain::image::TreeSVG & image = imagePanel[svg::IMAGE](svg::IMAGE);
	image->setFrame(frame);
	if (!styleClass.empty()){
		imagePanel->addClass(styleClass);
		// image->addClass(styleClass);
	}

	// return image;
}

drain::image::TreeSVG & RackSVG::addImageBorder(drain::image::TreeSVG & imagePanelGroup){ //, const drain::Frame2D<drain::image::svg::coord_t> & frame){
	drain::image::TreeSVG & imageBorder = imagePanelGroup[PanelConfSVG::ElemClass::IMAGE_BORDER](svg::RECT); // +EXT!
	imageBorder->addClass(PanelConfSVG::ElemClass::IMAGE_BORDER); // style
	imageBorder->addClass(drain::image::LayoutSVG::FLOAT);
	imageBorder->setAlign(drain::image::AlignSVG::HORZ_FILL, drain::image::AlignSVG::VERT_FILL);
	return imageBorder;
}



void RackSVG::addTitleBox(const PanelConfSVG & conf, drain::image::TreeSVG & object, PanelConfSVG::ElemClass elemClass){

	drain::image::TreeSVG & backgroundRect = object[BACKGROUND_RECT](svg::RECT);
	backgroundRect->addClass(elemClass);

	//backgroundRect->setAlignAnchorHorz("*"); // only if HORZ-INCR?
	backgroundRect->setAlignAnchor("*");
	backgroundRect->setAlign(AlignSVG::HORZ_FILL);
	// backgroundRect->setHeight(40); // TODO!!

	// Lower... for GENERAL as well.
	switch (elemClass) {
	case PanelConfSVG::ElemClass::MAIN:
		backgroundRect->setId(PanelConfSVG::ElemClass::MAIN);
		backgroundRect->setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE);
		backgroundRect->setHeight(conf.boxHeights[0]);
		break;
	case PanelConfSVG::ElemClass::GROUP:
		backgroundRect->setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE);
		backgroundRect->setHeight(conf.boxHeights[1]);
		// backgroundRect->setHeight(40);
		break;
		// case PanelConfSVG::ElemClass::IMAGE_TITLE:
		// backgroundRect->setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE);
		// backgroundRect->setHeight(40);
		// backgroundRect->setHeight(conf.boxHeights[2]);
		// break;
	default:
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.suspicious("Unhandled elemClass: ", elemClass);
		break;
	}

	addTitles(conf, object, BACKGROUND_RECT, elemClass);


}

void RackSVG::addTitles(const PanelConfSVG & conf,drain::image::TreeSVG & object, const std::string & anchor, PanelConfSVG::ElemClass elemClass){

	/** TODO
	const double fontSize = // getStyleValue(root, RackSVG::TITLE, "font-size", 12.5);
			root[drain::image::svg::STYLE][elemClass]->get("font-size", 12.5);
	 */

	drain::Logger mout(__FILE__, __FUNCTION__);
	// TODO: title area "filling order", by group class.

	TreeSVG & mainHeader = object[PanelConfSVG::ElemClass::GENERAL](svg::TEXT); // group[GENERAL](svg::TEXT);
	mainHeader->addClass(LayoutSVG::FLOAT);
	mainHeader->addClass(elemClass); // also GENERAL?
	mainHeader->setAlignAnchor(anchor);
	/*
	if (elemClass == PanelConfSVG::ElemClass::IMAGE_TITLE){
		mainHeader->setAlign(AlignSVG::BOTTOM, AlignSVG::LEFT); //AlignSVG::VertAlign::MIDDLE);
	}
	else {
		mainHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER); //AlignSVG::VertAlign::MIDDLE);
	}
	// mainHeader->setHeight(conf.boxHeights[0]);
	// mainHeader->setMargin(conf.boxHeights[0]*0.2); // ADJUST
	 */
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
	TreeSVG & timeHeader = object[PanelConfSVG::ElemClass::TIME](svg::TEXT);
	timeHeader->addClass(LayoutSVG::FLOAT);
	//timeHeader->addClass(RackSVG::TITLE, RackSVG::TIME);
	timeHeader->addClass(elemClass, PanelConfSVG::ElemClass::TIME);
	timeHeader->setAlignAnchor(anchor);
	timeHeader["date"](svg::TSPAN);
	timeHeader["date"]->addClass("date");
	timeHeader["time"](svg::TSPAN);
	timeHeader["time"]->addClass("time");
	// timeHeader["date"]->setText("date..."); //CTXX
	// timeHeader["time"]->setText("time"); //CTXX

	// Layout principle: there should be always time... so start/continue from left.
	TreeSVG & locationHeader = object[PanelConfSVG::ElemClass::LOCATION](svg::TEXT);
	locationHeader->addClass(LayoutSVG::FLOAT);
	//locationHeader->addClass(RackSVG::TITLE, RackSVG::LOCATION);
	locationHeader->addClass(elemClass, PanelConfSVG::ElemClass::LOCATION);
	locationHeader->setAlignAnchor(anchor);
	/*
	locationHeader->setAlign(AlignSVG::RIGHT);
	if (elemClass == PanelConfSVG::ElemClass::IMAGE_TITLE){
		locationHeader->setAlign(AlignSVG::TOP);
	}
	else {
		locationHeader->setAlign(AlignSVG::MIDDLE);
	}
	locationHeader->setHeight(16);
	locationHeader->setMargin(3); // adjust
	 */
	locationHeader["NOD"](svg::TSPAN);
	locationHeader["NOD"]->addClass("NOD");
	locationHeader["PLC"](svg::TSPAN);
	locationHeader["PLC"]->addClass("PLC");
	// locationHeader["NOD"]->setText("NOD"); //CTXX
	// locationHeader["PLC"]->setText("PLC"); //CTXX


	double textBoxHeight = 0; // conf.boxHeights[0]; // MAIN_HEADER
	double textBoxMargin = 0;

	switch (elemClass) {
	case PanelConfSVG::ElemClass::MAIN:
		textBoxHeight = conf.boxHeights[0]; // fontSizes
		mainHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER);
		timeHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::LEFT);
		locationHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::RIGHT);
		break;
	case PanelConfSVG::ElemClass::GROUP:
		textBoxHeight = conf.boxHeights[1]; // fontSizes
		mainHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER);
		timeHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::LEFT);
		locationHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::RIGHT);
		break;
	case PanelConfSVG::ElemClass::IMAGE:
		textBoxHeight = conf.boxHeights[2]; // fontSizes
		if (textBoxHeight > 0.0){
			mainHeader->setAlign(AlignSVG::BOTTOM, AlignSVG::LEFT);
			timeHeader->setAlign(AlignSVG::TOP, AlignSVG::LEFT);
			locationHeader->setAlign(AlignSVG::TOP, AlignSVG::RIGHT);
		}
		else {
			mainHeader->setComment("removed");
			timeHeader->setComment("removed");
			locationHeader->setComment("removed");
			return;
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



}


// Consider changing this to visitor.

// Re-align elements etc
void RackSVG::completeSVG(RackContext & ctx){ //, const drain::FilePath & filepath){


	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// drain::image::TreeSVG & mainGroup = getMainGroup(ctx);

	if (mout.isLevel(LOG_DEBUG)){
		// mout.special("dumping SVG tree");
		// drain::TreeUtils::dump(ctx.svgTrack);
		drain::Output outfile("svg-0-unaligned.svg");
		drain::image::NodeSVG::toStream(outfile.getStream(), ctx.svgTrack);
	}

	mout.debug("next superAlign ", ctx.mainOrientation, '|', ctx.mainDirection);

	if (mout.isLevel(LOG_DEBUG+2)){
		// mout.special("dumping SVG tree");
		// drain::TreeUtils::dump(ctx.svgTrack);
		drain::Output outfile("svg-0-unpruned.svg");
		drain::image::NodeSVG::toStream(outfile.getStream(), ctx.svgTrack);
	}


	MetaDataPrunerSVG metadataPruner;
	drain::TreeUtils::traverse(metadataPruner, ctx.svgTrack);

	if (mout.isLevel(LOG_DEBUG+2)){
		mout.special("dumping SVG tree");
		drain::Output outfile("svg-1-pruned.svg");
		drain::image::NodeSVG::toStream(outfile.getStream(), ctx.svgTrack);
		// drain::TreeUtils::dump(ctx.svgTrack);
	}

	// TODO
	/*
	if (ctx.svgTitles){ // also "false" !?
		// Create titles for each image panel
		TitleCreatorSVG titleCreator(ctx.svgTitles.getValue());
		drain::TreeUtils::traverse(titleCreator, ctx.svgTrack); // or mainTrack enough?
	}
	 */
	//TitleCreatorSVG titleCreator(0xff);
	ctx.svgPanelConf.mainTitle = ctx.getFormattedStatus(ctx.svgPanelConf.mainTitle);

	TitleCreatorSVG titleCreator(ctx.svgPanelConf);
	drain::TreeUtils::traverse(titleCreator, ctx.svgTrack); // or mainTrack enough?


	TreeUtilsSVG::superAlign(ctx.svgTrack, ctx.mainOrientation, ctx.mainDirection);

	TreeUtilsSVG::finalizeBoundingBox(ctx.svgTrack);


}




int MetaDataPrunerSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path){
	// std::cerr << __FUNCTION__ << ':' << path << std::endl;
	return 0;
}



int MetaDataPrunerSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	drain::Logger mout(__FILE__, __FUNCTION__);

	TreeSVG & current = tree(path);

	//if (!((current->getType()==svg::GROUP) || (current->getType()==svg::IMAGE))){ // At least METADATA must be skipped...
	if (!current->typeIs(svg::GROUP, svg::SVG)){ // At least METADATA must be skipped...
		return 0;
	}

	/// Statistics: computer count for each (key,value) pair, for example (what:time, "20240115")
	typedef std::map<std::string, unsigned short> variableStat_t;
	variableStat_t stat;
	/// Number of children having (any) metadata.
	int count = 0;

	/// Iterate children and their attributes: check which attributes (key and value) are shared by all the children.
	for (auto & entry: current.getChildren()){
		TreeSVG & child = entry.second;
		if (child.hasChild(svg::METADATA) && !child->hasClass("legend")){ // or has "data"?
			++count;
			TreeSVG & childMetadata = entry.second[svg::METADATA](svg::METADATA);
			for (const auto & attr: childMetadata->getAttributes()){
				// tehty jo metadata->set(attr.first, attr.second);
				std::string s = drain::StringBuilder<>(attr.first,'=',attr.second);
				++stat[s];
			}
		}
	}


	if (count > 0){

		TreeSVG & metadata = current[svg::METADATA](svg::METADATA);
		metadata->addClass("md_shared");

		TreeSVG & debugSharedBase = current[svg::DESC](svg::DESC);
		debugSharedBase->set("data-type", "SHARED"); // ??
		// TreeSVG & debugShared = debugSharedBase["cmt"](svg::COMMENT);
		// debugShared->setText("SHARED: "); //CTXX

		if (mout.isLevel(LOG_DEBUG)){
			TreeSVG & debugAll = current["description"](svg::DESC);
			debugAll->set("COUNT", count);
			debugAll->setText("All"); //CTXX
		}

		/*
		TreeSVG & debugExplicit = current["rejected"](svg::DESC);
		debugExplicit->addClass("EXPLICIT");
		debugAll->ctext += drain::sprinter(stat).str();
		 */

		// metadata->getAttributes().clear();
		mout.pending<LOG_DEBUG>("pruning: ", drain::sprinter(stat), path.str());

		// If this group level has variable entries ABC=123, DEF=456, ... , prune them from the lower level
		for (const auto & e: stat){

			mout.pending<LOG_DEBUG>('\t', e.first, ':', e.second);

			// std::cerr << "\t vector " << e.first << ' ' << e.second << std::endl;
			std::string key, value;
			drain::StringTools::split2(e.first, key, value, '=');
			if (e.second == count){ // = ALL,  shared by all the children.

				mout.accept<LOG_DEBUG>('\t', e.first, ' ', path.str());

				//debugSharedBase->set(e.first, 1);
				// debugSharedBase->ctext += ' ';
				// debugSharedBase->ctext += e.first;
				debugSharedBase.addChild() = e.first;
				// debugShared->set(key, value);

				// Update/extend, "upwards".
				metadata->set(key, value); // NOTE: becoming strings (consider type dict?)

				// Actual pruning, "downwards"
				// if (count > 1){
				//if (true){
				for (auto & entry: current.getChildren()){
					TreeSVG & child = entry.second;
					if (child.hasChild(svg::METADATA)){
						TreeSVG & childMetadata = entry.second[svg::METADATA]; //(svg::METADATA);
						childMetadata->removeAttribute(key);
						childMetadata->addClass("md_pruned");
						TreeSVG & childMetadata2 = entry.second[PanelConfSVG::ElemClass::SHARED_METADATA](svg::METADATA);
						childMetadata2->addClass("md_general");
						childMetadata2->set(key, value);
					}
				}
				// }

			}
			else {
				mout.reject<LOG_DEBUG>('\t', e.first, ' ', path.str());
				// debugExplicit->ctext += e.first;
			}
		}

	}

	return 0;

}



/**
 */
int TitleCreatorSVG::visitPostfix(TreeSVG & root, const TreeSVG::path_t & path){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// Apply to groups only.
	TreeSVG & group = root(path);
	if (!group->typeIs(svg::GROUP)){ //
		return 0;
	}

	if (!group.hasChild(svg::METADATA)){
		mout.attention("skipping, group has no METADATA element: ", group.data);
		return 0;
	}

	// TreeSVG & metadata = group[svg::METADATA];

	const NodeSVG::map_t & attributesPrivate = group[svg::METADATA]->getAttributes();
	const NodeSVG::map_t & attributesShared  = group[PanelConfSVG::ElemClass::SHARED_METADATA]->getAttributes();

	const bool WRITE_PRIVATE_METADATA = !attributesPrivate.empty();
	bool WRITE_SHARED_METADATA = !attributesShared.empty(); // SHARED_METADATA_EXISTS;

	const bool MAIN_AUTO  = (svgConf.mainTitle == "AUTO");
	const bool GROUP_AUTO = (svgConf.groupTitleFormatted.substr(0,4) == "AUTO");
	const bool GROUP_NONE = (svgConf.groupTitleFormatted.substr(0,4) == "NONE");
	const bool GROUP_USER = !(svgConf.groupTitleFormatted.empty() || GROUP_AUTO || GROUP_NONE);


	if (group->hasClass(PanelConfSVG::ElemClass::MAIN)){
		if (MAIN_AUTO){
			if (WRITE_PRIVATE_METADATA || WRITE_SHARED_METADATA){
				RackSVG::addTitleBox(svgConf, group, PanelConfSVG::ElemClass::MAIN);
			}
		}
		else if (!svgConf.mainTitle.empty()){
			RackSVG::addTitleBox(svgConf, group, PanelConfSVG::ElemClass::MAIN);
			group[PanelConfSVG::ElemClass::GENERAL]->setText(svgConf.mainTitle);
			return 0;
		}
	}
	else if (group->hasClass(LayoutSVG::ALIGN_FRAME)){
		if (GROUP_AUTO){
			// If no higher element will write meta data, write it here (perhaps repeatedly)
			WRITE_SHARED_METADATA &= (svgConf.mainTitle.empty()); // explicitly set main title MAY still  rewrite some metadata.
			if (WRITE_PRIVATE_METADATA || WRITE_SHARED_METADATA){
				RackSVG::addTitleBox(svgConf, group, PanelConfSVG::ElemClass::GROUP);
			}
		}
		else if (GROUP_USER){
			RackSVG::addTitleBox(svgConf, group, PanelConfSVG::ElemClass::GROUP);
			// group[PanelConfSVG::ElemClass::GENERAL]->setText(group[svg::TITLE]);
			group[PanelConfSVG::ElemClass::GENERAL]->setText(group->getId());
			// group[PanelConfSVG::ElemClass::GENERAL]->setText(svgConf.groupTitleFormatted+ "..dynamic=temporary WRONG!");
			return 0;
		}
		else if (GROUP_NONE){
			return 0;
		}
		else {
			return 0;
			mout.suspicious("could not interpret title of group of class ALIGN_FRAME :", svgConf.groupTitleFormatted);
		}
	}
	else if (group->hasClass(PanelConfSVG::ElemClass::IMAGE_PANEL)){
		// WRITE_SHARED_METADATA = ! (titles.isSet(PanelConfSVG::ElemClass::GROUP_TITLE) && titles.isSet(PanelConfSVG::ElemClass::MAIN_TITLE));
		WRITE_SHARED_METADATA &=  !(MAIN_AUTO || GROUP_AUTO);
		if (WRITE_PRIVATE_METADATA || WRITE_SHARED_METADATA){
			RackSVG::addTitles(svgConf, group, "image", PanelConfSVG::ElemClass::IMAGE);
		}
		// If no higher element will write metadata, write it here (perhaps repeatedly)
	}
	else {
		// CHECK when?
		return 0;
	}


	/*
	if (!group.hasChild(svg::METADATA)){
		mout.attention("group has no METADATA element: ", group.data);
		return 0;
	}
	 */

	// Always
	writeTitles(group, attributesPrivate);

	if (WRITE_SHARED_METADATA){
		writeTitles(group, attributesShared);
	}

	/*
	if (WRITE_SHARED_METADATA && group.hasChild(PanelConfSVG::ElemClass::SHARED_METADATA)){ // explicit request: GROUP
		writeTitles(group, group[PanelConfSVG::ElemClass::SHARED_METADATA]->getAttributes());
	}
	 */

	return 0;

}


void TitleCreatorSVG::writeTitles(TreeSVG & group, const NodeSVG::map_t & attributes){

	if (attributes.empty()){
		return;
	}


	VariableFormatterODIM<std::string> formatter; // (No instance properties used, but inheritance/overriding)

	for (const auto & attr: attributes){

		// This is a weird (old code)?  Can metadata have formatting, like time|%Y=201408171845 ??
		// Anyway, after split, key contains attrib key.
		std::string key, format;
		drain::StringTools::split2(attr.first, key, format, '|');

		PanelConfSVG::ElemClass elemClass = PanelConfSVG::ElemClass::GENERAL;

		if ((ODIM::timeKeys.count(attr.first)>0) || (ODIM::dateKeys.count(attr.first)>0)){
			elemClass = PanelConfSVG::ElemClass::TIME;
		}
		else if (ODIM::locationKeys.count(attr.first)>0){
			elemClass = PanelConfSVG::ElemClass::LOCATION;
		}

		TreeSVG & text  = group[elemClass]; // (svg::TEXT);
		if (text->isUndefined()){
			text->setType(svg::COMMENT); // only test...
			text->setText("skipped elem");
		}
		TreeSVG & tspan = text[attr.first](svg::TSPAN);
		tspan->addClass(attr.first); // allows user-specified style

		if (elemClass == PanelConfSVG::ElemClass::TIME){
			if (format.empty()){
				format = RackSVG::guessFormat(key);
				text->set("format", format);
			}
			// mout.accept<LOG_DEBUG>("TIME text format", format);
		}
		else if (elemClass == PanelConfSVG::ElemClass::LOCATION){
			// text->setAlign(AlignSVG::BOTTOM, AlignSVG::RIGHT);
		}
		else {
			// text->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER);
			// text->setText(drain::StringBuilder<'|'>(elemClass, attr.first, attr.second)); //CTXX
		}

		// Explicit (instead of style-derived) font size needed for bounding box (vertical height)
		/*
			const double fontSize = // getStyleValue(root, PanelConfSVG::TITLE, "font-size", 12.5);
					root[drain::image::svg::STYLE][PanelConfSVG::MAINTITLE]->get("font-size", 12.5);
		 */
		// text->setStyle("font-size", fontSize); //
		// text->setHeight(14);     // row height
		// text->setMargin(4); // margin
		// text->addClass(LayoutSVG::FLOAT); // Anchor defined, but need for proper bbox computation! (Yet text should be discarded)


		// mout.attention("handle: ", attr.first, " ", v, " + ", format);

		if (format.empty()){
			// tspan->setText(attr.second, "&#160;"); // non-b.sp
			tspan->setText(attr.second, " "); // escape code & in non-b.sp caused problems...
		}
		else {
			//mout.attention("handle: ", attr.first, " ", v, " + ", format);
			std::stringstream sstr;
			formatter.formatVariable(key, attr.second, format, sstr);
			// tspan->ctext += sstr.str();
			// tspan->setText(sstr.str(), "&#160;"); // non-b.sp
			tspan->setText(attr.second, " "); // escape code & in non-b.sp caused problems...
		}

	}



}




} // namespace rack

