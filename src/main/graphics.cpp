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

#include <drain/prog/Command.h>
#include <drain/prog/CommandInstaller.h>
#include <drain/prog/CommandBank.h>
#include <drain/util/Output.h>

#include "data/SourceODIM.h" // for NOD

#include "resources.h"
//#include "fileio-svg.h"
#include "graphics.h"



namespace drain {

/// Automatic conversion of elem classes to STYLE selector, eg MAINTITLE -> ".MAINTITLE"
/**
 *  Notice leading '.'.
 *
 */

template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass &x){
	return (*this)[std::string(".")+EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(x, false)];
}


/// Automatic conversion of elem classes to strings.
/**
 *

template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const image::svg::tag_t & type){
	return (*this)[EnumDict<image::svg::tag_t>::dict.getKey(type, false)];
}
 */



/// Automatic conversion of elem classes to strings.
/**
 *
 */

// template <> // for T (Tree class)

/*
template <>
template <typename K> // for K (path elem arg)
const image::TreeSVG::key_t & image::TreeSVG::getKey(const K & key){
	return EnumDict<K>::dict.getKey(key, false);
}
*/




/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
inline
const image::TreeSVG & image::TreeSVG::operator[](const rack::RackSVG::TitleClass &x) const {
	// return (*this)["reijo"];
	return (*this)[EnumDict<rack::RackSVG::TitleClass>::dict.getKey(x, false)];
}
*/

}

namespace rack {

/// Group identifiers for elements which be automatically aligned (stacked horizontally or vertically)


const std::string RackSVG::BACKGROUND_RECT = "bgRect";


//}


// NOTE: Part of SVG processing is  CmdOutputConf
/*
 * 		svgConf.link("group",       TreeUtilsSVG::defaultGroupName);
		svgConf.link("orientation", svgConf_Orientation, drain::sprinter(TreeUtilsSVG::defaultOrientation.getDict().getKeys()).str());
		svgConf.link("direction",   svgConf_Direction,   drain::sprinter(TreeUtilsSVG::defaultDirection.getDict().getKeys()).str());

		drain::StyleSelectorXML<NodeSVG>("Elem");
		drain::StyleSelectorXML<NodeSVG>(svg::POLYGON);
		drain::StyleSelectorXML<NodeSVG>(svg::POLYGON,GraphicsContext::IMAGE_BORDER);

 */



/*
 *
 *  \see drain::XML::setStyle()
 *  \see drain::XML::xmlAssign()
 */
drain::image::TreeSVG & RackSVG::getStyle(RackContext & ctx){

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	TreeSVG & style = ctx.svgTrack[svg::STYLE];

	//if (!ctx.svgTrack.hasChild("style")){
	if (style->isUndefined()){

		mout.debug("initializing style");

		style->setType(svg::STYLE);

		style[svg::TEXT] = {
				{"font-family","Helvetica, Arial, sans-serif"}
		};

		// MODE 3: attribs of sub-element
		style[GraphicsContext::IMAGE_BORDER] = { // TODO: add leading '.' ?
				{"fill", "none"},
				{"stroke", "none"},
				// {"stroke-opacity", 0.0},
				{"stroke-width", "0.3em"},
				// {"stroke-dasharray", {2,5,3}},
		};
		//style["group.imageFrame > rect"].data = {

		typedef drain::StyleSelectorXML<NodeSVG> Select;

		// Text (new)
		// style["text.TITLE"] = {
		style[Select(svg::TEXT, GraphicsContext::TITLE)] = {
				{"font-size", "1.5em"},
				{"stroke", "none"},
				// {"fill", "blue"},
				/* {"fill", "black"}, */
				// {"fill-opacity", "1"},
				// {"paint-order", "stroke"},
				// {"stroke-linejoin", "round"}
		};


		// style["text.IMAGE_TITLE"] = {
		style[Select(svg::TEXT, GraphicsContext::IMAGE_TITLE)] = {
				{"font-size", 12},
				{"stroke", "white"},
				/* {"fill", "black"}, */
				{"stroke-opacity", "0.75"},
				{"stroke-width", "0.3em"},
				//{"fill", "darkslateblue"},
				{"fill-opacity", "1"},
				{"paint-order", "stroke"},
				{"stroke-linejoin", "round"}
		};

		style[GraphicsContext::GROUP_TITLE] = {
				{"font-size", 20},
		};

		style[Select(svg::RECT, GraphicsContext::GROUP_TITLE)] = {
		//style["rect.GROUP_TITLE"] = {
				{"fill", "gray"},
				{"opacity", 0.5},
		};

		style[Select(svg::TEXT, GraphicsContext::GROUP_TITLE)] = {
		// style["text.GROUP_TITLE"] = {
				{"fill", "black"},
		};

		style[GraphicsContext::MAIN_TITLE] = {
				{"font-size", "30"},
		};

		// style["rect.MAIN_TITLE"] = {
		style[Select(svg::RECT, GraphicsContext::MAIN_TITLE)] = {
				{"fill", "darkslateblue"},
				{"opacity", "0.5"},
		};

		// style["text.MAIN_TITLE"] = {
		style[Select(svg::TEXT, GraphicsContext::MAIN_TITLE)] = {
				{"fill", "white"},
		};

		// Date and time.
		style[GraphicsContext::TIME].data = {
				{"fill", "darkred"}
		};

		style[GraphicsContext::LOCATION].data = {
				{"fill", "darkblue"}
		};

		// style[".imageTitle2"] = "font-size:1.5em; stroke:white; stroke-opacity:0.5; stroke-width:0.3em; fill:darkslateblue; fill-opacity:1; paint-order:stroke; stroke-linejoin:round";
		// drain::TreeUtils::dump(ctx.svgTrack);

		// MODE 2: deprecated: text (CTEXT) of sub-element - note: child elem does not know its parent's type (STYLE)
		// style["text"] = "filler:black";
		// style["text"] = ("stroke:white; stroke-width:0.5em; stroke-opacity:0.25; fill:black; paint-order:stroke; stroke-linejoin:round");
		// style["text"].data = "stroker:white";

	}

	return style;
}


drain::image::TreeSVG & RackSVG::getMainGroup(RackContext & ctx){ // , const std::string & name

	using namespace drain::image;

	// Ensure STYLE elem and definitions
	RackSVG::getStyle(ctx);

	// drain::image::TreeSVG & main = ctx.svgTrack[ctx.svgGroupNameSyntax]; <- this makes sense as well
	drain::image::TreeSVG & main = ctx.svgTrack["rack-outputs"];

	if (main -> isUndefined()){
		main->setType(svg::GROUP);
	}

	return main;

}



/// Return current row or column of image panels.
drain::image::TreeSVG & RackSVG::getCurrentAlignedGroup(RackContext & ctx){ // what about prefix?

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	drain::image::TreeSVG & mainGroup = getMainGroup(ctx);

	/*
	mout.attention();
	drain::TreeUtils::dump(mainGroup, mout);
	mout.end();
	*/
	//drain::image::NodeSVG::toStream(ostr, tree, defaultTag, indent)

	ctx.svgGroupNameFormatted = ctx.getFormattedStatus(ctx.svgGroupNameSyntax); // status updated upon last file save

	drain::image::TreeSVG & alignedGroup = mainGroup[ctx.svgGroupNameFormatted];

	if (alignedGroup -> isUndefined()){
		alignedGroup->setType(svg::GROUP);
		alignedGroup->setId(ctx.svgGroupNameFormatted);
		alignedGroup->addClass(drain::image::LayoutSVG::ALIGN_FRAME);
	}

	return alignedGroup;

}


//static_cast<key_t>(key)

/**
 * const E & pathElem selector! like RackSVG::MAINTITLE
template <typename E>
const drain::image::TreeSVG & getStyle(const drain::image::TreeSVG & svg, const E & selector){
	const drain::image::TreeSVG & style = svg[drain::image::svg::STYLE]; // (drain::image::svg::STYLE);
	return style[selector]; // (drain::image::svg::STYLE_SELECT);
}

template <typename E, typename T>
T getStyleValue(const drain::image::TreeSVG & svg, const E & selector, const std::string & property, const T & defaultValue = T()){
	return getStyle(svg,selector)->get(property, defaultValue);
}
 */


/// For each image an own group is created (for clarity, to contain also title TEXT's etc)
/**
 *
 */
drain::image::TreeSVG & RackSVG::getImagePanelGroup(RackContext & ctx, const drain::FilePath & filepath){

	// For each image an own group is created to contain also title TEXT's etc.
	const std::string name = drain::StringBuilder<'-'>(filepath.basename, filepath.extension);

	drain::image::TreeSVG & group = getCurrentAlignedGroup(ctx)[name];

	if (group->isUndefined()){
		group->setType(svg::GROUP);
		group->setId(name + 'G');
		//group->setId(filepath.basename + 'G');
	}

	return group;

}

/// Add pixel image (PNG)
drain::image::TreeSVG & RackSVG::addImage(RackContext & ctx, const drain::image::Image & src, const drain::FilePath & filepath){ // what about prefix?

	using namespace drain::image;

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	mout.attention("filepath:", filepath);

	drain::image::TreeSVG & panelGroup = getImagePanelGroup(ctx, filepath);
	panelGroup->addClass(GraphicsContext::IMAGE_PANEL); // Add elems ^ here ^ ?

	// TEST
	// panelGroup->setAlignAnchor("image");

	drain::image::TreeSVG & image = panelGroup[svg::IMAGE](svg::IMAGE); // +EXT!
	image->setId(filepath.basename); // unneeded, as TITLE also has it:
	image[drain::image::svg::TITLE](drain::image::svg::TITLE) = filepath.basename;
	image->setFrame(src.getGeometry().area);
	image->set("xlink:href", filepath.str()); // 2025 FIX: without .str() error

	addImageBorder(panelGroup); //, src.getGeometry().area);

	/*
	if ( ctx.svgDebug > 0){  // TODO: move to --gDebug etc.
		image->set("opacity", 0.5);

		drain::image::TreeSVG & rect = panelGroup["rect"](svg::RECT);
		rect->set("width",  160); // debug
		rect->set("height", 100); // debug
		rect->set("fill", "none"); // just to make sure...
		rect->setStyle("stroke", "magenta");
		rect->setStyle("stroke-width", "2px");
		// rect->setStyle("border-style", "dotted");
		rect->setStyle("stroke-dasharray", {2,5});
		// rect->setAlign<AlignSVG::INSIDE>(AlignSVG::LEFT);
		rect->setAlign(AlignSVG::RIGHT, AlignSVG::INSIDE); // AlignSVG::RIGHT);
		rect->setAlign(AlignSVG::MIDDLE, AlignSVG::INSIDE); // AlignSVG::MIDDLE);

		drain::image::TreeSVG & rect2 = panelGroup["rect2"](svg::RECT);
		rect2->set("width",  150); // debug
		rect2->set("height", 120); // debug
		rect2->set("fill", "none"); // just to make sure...
		rect2->setStyle("stroke", "green");
		rect2->setStyle("stroke-width", "5px");
		rect2->setStyle("stroke-dasharray", {5,2,3});
		rect2->setAlign(AlignSVG::CENTER, AlignSVG::OUTSIDE); // prune arg
		rect2->setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE); //  AlignSVG::BOTTOM);
		//rect2->setAlign(AlignSVG::OBJECT, AlignSVG::CENTER);
		//rect2->setAlign(AlignSVG::ANCHOR, AlignSVG::BOTTOM);
		//rect2->setAlign(AlignSVG::OBJECT, AlignSVG::BOTTOM);
	}
	*/

	// Metadata:
	drain::image::TreeSVG & metadata = panelGroup[svg::METADATA](svg::METADATA);

	// Note assign: char * -> string  , "where:lat", "where:lon"
	if (src.properties.hasKey("what:source")){
		SourceODIM odim(src.properties.get("what:source",""));
		metadata->set("NOD", odim.NOD);
		metadata->set("PLC", odim.PLC);
	}

	// TODO: 1) time formatting 2) priority (startdate, starttime)
	for (const std::string key: {"what:date", "what:time", "what:product", "what:prodpar", "what:quantity", "where:elangle", "prevCmdKey"}){
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

	// SOLVED, by storeLastArguments()  ...
	// if (src.properties.hasKey("what:product")){
	/*
	drain::VariableMap & statusMap = ctx.getStatusMap();
	std::string cmdKey = statusMap.get("prevCmdKey", "");
	mout.warn("prevCmdKey: ",  cmdKey);
	mout.warn("prevCmdArgs: ", statusMap.get("prevCmdArgs", ""));

	if (cmdKey.size() >= 2){ // actually larger...
		// metadata->set("cmdArgs", statusMap.get("cmdArgs", ""));
	}
	*/
	// std::string cmd = statusMap.get("cmd", "");
	// if (cmd.size() >= 2){prevCmdKey
	// metadata->set("cmd", statusMap.get("cmd", ""));

	drain::image::TreeSVG & description = panelGroup["description"](svg::DESC);
	description->getAttributes().importCastableMap(metadata->getAttributes());
	// todo: description  : prevCmdKey "what:product", "what:prodpar", "how:angles"

	return image;

}

/// Add SVG image
/**
 *
 */
drain::image::TreeSVG & RackSVG::addImage(RackContext & ctx, const drain::image::TreeSVG & svg, const drain::FilePath & filepath){ // what about prefix?

	drain::image::TreeSVG & group = getCurrentAlignedGroup(ctx); //[filepath.basename+"_Group"](svg::GROUP);

	drain::image::TreeSVG & image = group[filepath.basename](svg::IMAGE); // +EXT!
	image->setFrame(svg->getBoundingBox().getFrame());
	// image->set("width", svg->get("width", 0));
	// image->set("height", svg->get("height", 0));
	image->set("xlink:href", filepath.str());
	// image["basename"](drain::image::svg::TITLE) = filepath.basename;
	image[drain::image::svg::TITLE](drain::image::svg::TITLE) = filepath.basename;

	// TODO: align ?

	return image;

}

/// Add pixel image (PNG)
drain::image::TreeSVG & RackSVG::addImage(RackContext & ctx, const drain::FilePath & filepath, const drain::Frame2D<double> & frame){ // what about prefix?

	drain::image::TreeSVG & group = getCurrentAlignedGroup(ctx); //[filepath.basename+"_Group"](svg::GROUP);

	drain::image::TreeSVG & image = group[filepath.basename](svg::IMAGE); // +EXT!
	image->setFrame(frame);
	image->set("xlink:href", filepath.str());
	image[drain::image::svg::TITLE](drain::image::svg::TITLE) = filepath.basename;

	return image;
}

drain::image::TreeSVG & RackSVG::addImageBorder(drain::image::TreeSVG & imagePanelGroup){ //, const drain::Frame2D<double> & frame){
	// DEBUG: (may be fatal for input.sh etc.)
	// drain::image::svg::toStream(std::cout, image);
	drain::image::TreeSVG & imageBorder = imagePanelGroup[GraphicsContext::IMAGE_BORDER](svg::RECT); // +EXT!
	imageBorder->addClass(GraphicsContext::IMAGE_BORDER); // style
	imageBorder->addClass(drain::image::LayoutSVG::FLOAT);
	imageBorder->setAlign(drain::image::AlignSVG::HORZ_FILL, drain::image::AlignSVG::VERT_FILL);
	return imageBorder;
}



void RackSVG::addTitleBox(drain::image::TreeSVG & object, GraphicsContext::ElemClass elemClass){

	drain::image::TreeSVG & backgroundRect = object[BACKGROUND_RECT](svg::RECT);
	backgroundRect->addClass(elemClass);
	// Lower... for GENERAL as well.
	switch (elemClass) {
		case GraphicsContext::ElemClass::MAIN_TITLE:
			backgroundRect->setId(GraphicsContext::ElemClass::MAIN_TITLE);
			backgroundRect->setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE);
			break;
		case GraphicsContext::ElemClass::GROUP_TITLE:
			backgroundRect->setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE);
			break;
		case GraphicsContext::ElemClass::IMAGE_TITLE:
			backgroundRect->setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE);
			break;
		default:
			break;
	}

	backgroundRect->setAlignAnchorHorz("*"); // only if HORZ-INCR?
	backgroundRect->setAlign(AlignSVG::HORZ_FILL);
	backgroundRect->setHeight(40);

	addTitles(object, BACKGROUND_RECT, elemClass);
}

void RackSVG::addTitles(drain::image::TreeSVG & object, const std::string & anchor, GraphicsContext::ElemClass elemClass){

	/** TODO
	const double fontSize = // getStyleValue(root, RackSVG::TITLE, "font-size", 12.5);
			root[drain::image::svg::STYLE][elemClass]->get("font-size", 12.5);
	*/

	TreeSVG & mainHeader = object[GraphicsContext::GENERAL](svg::TEXT); // group[GENERAL](svg::TEXT);
	mainHeader->addClass(LayoutSVG::FLOAT);
	mainHeader->addClass(elemClass); // also GENERAL?
	mainHeader->setAlignAnchor(anchor);
	if (elemClass == GraphicsContext::ElemClass::IMAGE_TITLE){
		mainHeader->setAlign(AlignSVG::BOTTOM, AlignSVG::LEFT); //AlignSVG::VertAlign::MIDDLE);
	}
	else {
		mainHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER); //AlignSVG::VertAlign::MIDDLE);
	}
	mainHeader->setHeight(20);
	mainHeader->setMargin(4);
	// Ensure order
	mainHeader["product"](svg::TSPAN);
	mainHeader["product"]->addClass("product");
	mainHeader["prodpar"](svg::TSPAN);
	// mainHeader["product"]->ctext = "product";
	// mainHeader["prodpar"]->ctext = "prodpar";


	TreeSVG & timeHeader = object[GraphicsContext::TIME](svg::TEXT);
	timeHeader->addClass(LayoutSVG::FLOAT);
	timeHeader->addClass(GraphicsContext::TITLE, GraphicsContext::TIME);
	timeHeader->setAlignAnchor(anchor);
	timeHeader->setAlign(AlignSVG::TOP, AlignSVG::RIGHT); // , AlignSVG::INSIDE);
	timeHeader->setHeight(16);
	timeHeader->setMargin(3);
	timeHeader["date"](svg::TSPAN);
	timeHeader["date"]->addClass("date");
	timeHeader["time"](svg::TSPAN);
	timeHeader["time"]->addClass("time");
	// timeHeader["date"]->ctext = "date...";
	// timeHeader["time"]->ctext = "time";

	TreeSVG & locationHeader = object[GraphicsContext::LOCATION](svg::TEXT);
	locationHeader->addClass(LayoutSVG::FLOAT);
	locationHeader->addClass(GraphicsContext::TITLE, GraphicsContext::LOCATION);
	locationHeader->setAlignAnchor(anchor);
	if (elemClass == GraphicsContext::ElemClass::IMAGE_TITLE){
		locationHeader->setAlign(AlignSVG::TOP, AlignSVG::RIGHT);
	}
	else {
		locationHeader->setAlign(AlignSVG::MIDDLE, AlignSVG::LEFT);
	}
	locationHeader->setHeight(16);
	locationHeader->setMargin(3);
	locationHeader["NOD"](svg::TSPAN);
	locationHeader["NOD"]->addClass("NOD");
	locationHeader["PLC"](svg::TSPAN);
	locationHeader["PLC"]->addClass("PLC");
	// locationHeader["NOD"]->ctext = "NOD";
	// locationHeader["PLC"]->ctext = "PLC";

}


// Consider changing this to visitor.

// Re-align elements etc
void RackSVG::completeSVG(RackContext & ctx){ //, const drain::FilePath & filepath){


	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	drain::image::TreeSVG & mainGroup = getMainGroup(ctx);


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

	if (ctx.svgTitles){ // also "false" !?
		// Create titles for each image panel
		TitleCreatorSVG titleCreator(ctx.svgTitles.getValue());
		drain::TreeUtils::traverse(titleCreator, ctx.svgTrack); // or mainTrack enough?
	}


	if (false && ctx.svgTitles.isSet(GraphicsContext::ElemClass::MAIN_TITLE)){ // also "false" !?

		// MAIN HEADER(s)
		// if (mainGroup.hasChild("metadata") || (ctx.svgPanelConf.title != "false")){ // hmmm
		// if (ctx.svgTrack.hasChild("mainRect")){ // check
		if (!ctx.svgTrack.hasChild(BACKGROUND_RECT)){ // AUTO
		// if (false){ // AUTO

			// TreeSVG & group = RackSVG::getMainGroup(ctx); // ctx.svgTrack["headerGroup"](svg::GROUP);

			addTitleBox(ctx.svgTrack, GraphicsContext::ElemClass::MAIN_TITLE);

			// Automatic
			//if ((ctx.svgPanelConf.title == "auto") || ctx.svgPanelConf.title.empty()){
			if (true){
				// TODO: vars aligned by class (time, etc)
				VariableFormatterODIM<drain::FlexibleVariable> formatter; // (No instance properties used, but inheritance/overriding)
				for (const auto & entry: mainGroup["metadata"]->getAttributes()){
					// Here, class is also the element key:
					// Consider separate function, recognizing date/time, and formatting?

					// std::string key;
					GraphicsContext::ElemClass key;
					std::stringstream sstr;
					/*
					const std::string & key = getTextClass(entry.first, "GENERAL");
					TreeSVG & subHeader = headerGroup[key](svg::TEXT);
					TreeSVG & sh = subHeader.hasChild(entry.first) ? subHeader[entry.first] : subHeader;
					std::stringstream sstr(sh->ctext);
					*/

					// In these, the selection is based on entry.first (eg. what:endtime)
					if (formatter.formatDate(sstr, entry.first, entry.second, "%Y/%m/%d")){
						mout.special("Recognized  DATE: ", entry.first, '=', entry.second);
						key = GraphicsContext::TIME; //"TIME"; // str + enum??
					}
					else if (formatter.formatTime(sstr, entry.first, entry.second, "%H:%M UTC")){
						mout.special("Recognized  TIME: ", entry.first, '=', entry.second);
						key = GraphicsContext::TIME; //"TIME"; // str + enum??
					}
					else if (formatter.formatPlace(sstr, entry.first, entry.second)){
						mout.special("Recognized PLACE: ", entry.first, '=', entry.second);
						key = GraphicsContext::LOCATION; // "LOCATION"; // str + enum??
					}
					else {
						sstr << entry.second.toStr();
						key = GraphicsContext::GENERAL; // "GENERAL";
					}
					sstr << "&#160;";

					TreeSVG & subHeader = ctx.svgTrack[key](svg::TEXT);
					if (subHeader.hasChild(entry.first)){
						subHeader[entry.first]->setType(svg::TSPAN); // needed?
						subHeader[entry.first]->ctext += sstr.str(); // +"%";
					}
					else {
						subHeader->ctext += sstr.str();
					}

				}

			}

			/* FREE format? */
			//if (!ctx.svgTitles.empty()){
			if (false){
				VariableFormatterODIM<drain::Variable> formatter; // (No instance properties used, but inheritance/overriding)
				drain::StringMapper titleMapper(RackContext::variableMapper); // XXX
				titleMapper.parse(ctx.svgTitles);
				const drain::VariableMap & v = ctx.getStatusMap();
				//mainHeader->ctext += titleMapper.toStr(v);
				ctx.svgTrack[GraphicsContext::GENERAL]->ctext += titleMapper.toStr(v, -1, formatter);
				ctx.svgTrack[GraphicsContext::GENERAL]->ctext += ' ';
			}



		}

		// IMAGE HEADER and MAIN HEADER positions
		// mout.attention("aligning texts of: ", group -> getTag());
		// tsvg::alignText(mainGroup);
	}

	//TreeUtilsSVG::superAlign(mainGroup, ctx.mainOrientation, ctx.mainDirection);

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
		// debugShared->ctext = "SHARED: ";

		if (mout.isLevel(LOG_DEBUG)){
			TreeSVG & debugAll = current["description"](svg::DESC);
			debugAll->set("COUNT", count);
			debugAll->ctext = "All";
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
			if (e.second == count){ // Shared by all the children.

				mout.accept<LOG_DEBUG>('\t', e.first, ' ', path.str());

				debugSharedBase->ctext += ' ';
				debugSharedBase->ctext += e.first;
				// debugShared->set(key, value);

				metadata->set(key, value); // NOTE: becoming strings (consider type dict?)

				if (count > 1){
					for (auto & entry: current.getChildren()){
						TreeSVG & child = entry.second;
						if (child.hasChild(svg::METADATA)){
							TreeSVG & childMetadata = entry.second[svg::METADATA](svg::METADATA);
							childMetadata -> removeAttribute(key);
							childMetadata -> addClass("md_pruned");
						}
					}
				}

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

	TreeSVG & group = root(path);

	if (!group->typeIs(svg::GROUP, svg::SVG)){ // At least METADATA must be skipped...
		return 0;
	}

	/// Group title
	if (group->hasClass(LayoutSVG::ALIGN_FRAME) && titles.isSet(GraphicsContext::ElemClass::GROUP_TITLE)){
		// Problem: VERT layout?
		RackSVG::addTitleBox(group, GraphicsContext::ElemClass::GROUP_TITLE);
		group[GraphicsContext::ElemClass::GENERAL](svg::TEXT); // undeeded, debugging
		// group[GraphicsContext::ElemClass::GENERAL]->setText(path.back()); // Keep! Good for debugging, as last elem serves as an ID
	}
	else if (group->typeIs(svg::SVG) && titles.isSet(GraphicsContext::ElemClass::MAIN_TITLE)){ // At least METADATA must be skipped...
		RackSVG::addTitleBox(group, GraphicsContext::ElemClass::MAIN_TITLE);
	}



	if (!group.hasChild("metadata")){
		return 0;
	}

	/*
	if (!(group->hasClass(LayoutSVG::ALIGN_FRAME) || group->hasClass(GraphicsContext::IMAGE_PANEL))){
		return 0;
	}
	*/


	if (group->hasClass(GraphicsContext::IMAGE_PANEL)){
		// return 0;
		group->setAlignAnchor(svg::IMAGE);
	}


	TreeSVG & metadata = group[svg::METADATA];

	if (!metadata->getAttributes().empty()){

		VariableFormatterODIM<std::string> formatter; // (No instance properties used, but inheritance/overriding)
		// mout.attention("handle: ", current.data);
		// Note: these are "subtitles", not the main title



		for (const auto & attr: metadata->getAttributes()){


			// This is a weird (old code)?  Can metadata have formatting, like time|%Y=201408171845 ??
			// Anyway, after split, key contains attrib key.
			std::string key, format;
			drain::StringTools::split2(attr.first, key, format, '|');

			GraphicsContext::ElemClass elemClass = GraphicsContext::GENERAL;

			if ((ODIM::timeKeys.count(attr.first)>0) || (ODIM::dateKeys.count(attr.first)>0)){
				elemClass = GraphicsContext::TIME;
			}
			else if (ODIM::locationKeys.count(attr.first)>0){
				elemClass = GraphicsContext::LOCATION;
			}

			if (group->hasClass(GraphicsContext::IMAGE_PANEL) && titles.isSet(GraphicsContext::ElemClass::IMAGE_TITLE)){
				RackSVG::addTitles(group, "image", GraphicsContext::ElemClass::IMAGE_TITLE);
			}
			else {
				// return 0;
			}


			TreeSVG & text = group[elemClass](svg::TEXT);  // +"_title"
			text->addClass(elemClass);
			//text->set("name", elemClass);


			// text->addClass("imageTitle"); // style class (only)
			if (group->hasClass(GraphicsContext::IMAGE_PANEL)){
				text->addClass(GraphicsContext::IMAGE_TITLE);
			}

			TreeSVG & tspan = text[attr.first](svg::TSPAN);
			tspan->addClass(attr.first); // allows user-specified style
			//drain::StringTools::split2(attr.second.toStr(), v, format, '|');

			if (elemClass == GraphicsContext::TIME){
				text->setAlign(AlignSVG::TOP, AlignSVG::LEFT);
				if (format.empty()){
					//v = attr.second.toStr();
					if (drain::StringTools::endsWith(key, "date")){
						format = "%Y/%m/%d";
					}
					else if (drain::StringTools::endsWith(key, ":time")){
						format = key+"%H:%M UTC";
					}
					else if (drain::StringTools::endsWith(key, "time")){
						format = " %H:%M:%S UTC";
					}
					text->set("format", format);
				}
				mout.accept<LOG_DEBUG>("TIME text format", format);
			}
			else if (elemClass == GraphicsContext::LOCATION){
				text->setAlign(AlignSVG::BOTTOM, AlignSVG::RIGHT);
			}
			else {
				text->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER);
				// text->ctext = drain::StringBuilder<'|'>(elemClass, attr.first, attr.second);
			}

			// Explicit (instead of style-derived) font size needed for bounding box (vertical height)
			/*
			const double fontSize = // getStyleValue(root, RackSVG::TITLE, "font-size", 12.5);
					root[drain::image::svg::STYLE][GraphicsContext::MAINTITLE]->get("font-size", 12.5);
			*/
			// text->setStyle("font-size", fontSize); //
			text->setHeight(14);     // row height
			text->setMargin(4); // margin
			text->addClass(LayoutSVG::FLOAT); // Anchor defined, but need for proper bbox computation! (Yet text should be discarded)


			// mout.attention("handle: ", attr.first, " ", v, " + ", format);

			if (format.empty()){
				tspan->ctext += attr.second.toStr();
			}
			else {
				//mout.attention("handle: ", attr.first, " ", v, " + ", format);
				std::stringstream sstr;
				formatter.formatVariable(key, attr.second, format, sstr);
				tspan->ctext += sstr.str();
				// tspan->ctext += "?";
			}
			tspan->ctext += "&#160;"; //'_';
		}

	}
	else {
		mout.debug("title skipped, metadata empty under: ", path);
		metadata->setType(drain::XML::COMMENT);
	}

	return 0;

}



} // namespace rack

