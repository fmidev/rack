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

//#include <drain/util/Convert.h>
#include <drain/util/Output.h>
#include <drain/util/StringMapper.h>
#include <drain/util/TreeXML.h>
#include <drain/util/TreeHTML.h>

#include <drain/image/FilePng.h>
#include <drain/image/TreeUtilsSVG.h>

#include "data/SourceODIM.h" // for NOD

#include "fileio-svg.h"
#include "graphics.h"


/*
namespace drain {


template <>
template <class D>
void Convert2<FlexibleVariable>::convert(const FlexibleVariable &src, D & dst){
	dst = (const D &)src;
	std::cout << "CONV..." << src << " -> " << dst << '\n';
}


template <>
void Convert2<FlexibleVariable>::convert(const char *src, FlexibleVariable & dst){
	dst = src;
	std::cout << "CONV..." << src << " -> " << dst << '\n';
}



template <>
template <class S>
void Convert2<FlexibleVariable>::convert(const S &src, FlexibleVariable & dst){
	dst = src;
	std::cout << "CONV..." << src << " -> " << dst << '\n';
}


}
*/

//namespace drain {


//}

namespace rack {

typedef drain::image::TreeUtilsSVG tsvg;
// typedef drain::image::AlignSVG alignSvg;

template <>
const drain::EnumDict<RackSVG::TitleClass>::dict_t  drain::EnumDict<RackSVG::TitleClass>::dict = {
		DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, GENERAL),
		DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, LOCATION),
		DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, TIME),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, IMAGE_SET)
};

/// Group identifiers for elements which be automatically aligned (stacked horizontally or vertically)
const std::string RackSVG::IMAGE_FRAME("imageFrame");



//}


// NOTE: Part of SVG processing is  CmdOutputConf
/*
 * 		svgConf.link("group",       TreeUtilsSVG::defaultGroupName);
		svgConf.link("orientation", svgConf_Orientation, drain::sprinter(TreeUtilsSVG::defaultOrientation.getDict().getKeys()).str());
		svgConf.link("direction",   svgConf_Direction,   drain::sprinter(TreeUtilsSVG::defaultDirection.getDict().getKeys()).str());
 *
 */

/*
void CmdBaseSVG::createTitleBox(TreeSVG & tree){

	drain::Logger mout(__FILE__, __FUNCTION__);

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
}
	*/


drain::image::TreeSVG & RackSVG::getMain(RackContext & ctx){

	using namespace drain::image;

	if (!ctx.svgTrack.hasChild("style")){

		TreeSVG & style = ctx.svgTrack["style"](svg::STYLE);

		style["text"] = ("fill:black");
		//style["text"] = ("stroke:white; stroke-width:0.5em; stroke-opacity:0.25; fill:black; paint-order:stroke; stroke-linejoin:round");

		//style["group.imageFrame > rect"].data = {
		style["rect.imageFrame"].data = {
				{"stroke", "black"},
				{"stroke-opacity", "0.25"},
				{"stroke-width", "0.3em"}
		};

		style[".imageTitle"].data = {
				{"font-size", "1.5em"},
				{"stroke", "white"},
				{"stroke-opacity", "0.75"},
				{"stroke-width", "0.3em"},
				{"fill", "darkslateblue"},
				{"fill-opacity", "1"},
				{"paint-order", "stroke"},
				{"stroke-linejoin", "round"}
		};

		// Date and time.
		style[".TIME"].data = {
				{"fill", "darkred"}
		};

		style[".LOCATION"].data = {
				{"fill", "darkblue"}
		};

		// style[".imageTitle2"] = "font-size:1.5em; stroke:white; stroke-opacity:0.5; stroke-width:0.3em; fill:darkslateblue; fill-opacity:1; paint-order:stroke; stroke-linejoin:round";
		// drain::TreeUtils::dump(ctx.svgTrack);
	}
	drain::image::TreeSVG & main = ctx.svgTrack["outputs"];

	if (main -> isUndefined()){
		main->setType(NodeSVG::GROUP);
		main->setAlign(AlignSVG::OUTSIDE, AlignSVG::RIGHT); // AlignSVG::RIGHT);
		main->setAlign(AlignSVG::INSIDE,  AlignSVG::TOP);   // AlignSVG::TOP);
	}

	return main;

}

drain::image::TreeSVG & RackSVG::getCurrentGroup(RackContext & ctx){ // what about prefix?


	drain::image::TreeSVG & track = getMain(ctx);

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// VariableFormatterODIM<drain::Variable> formatter;
	drain::StringMapper groupMapper(RackContext::variableMapper); // XXX
	groupMapper.parse(ctx.svgPanelConf.groupName);
	const drain::VariableMap & vmap = ctx.getStatusMap();
	const std::string groupName = groupMapper.toStr(vmap); // No ${variable|format} supported here (not needed - this is just for identifying, not style!)

	//mout.pending<LOG_WARNING>("considering group: '", groupName, "' <= ", groupMapper, vmap.get("what:quantity","??"));
	//mout.pending<LOG_WARNING>(__FUNCTION__, " quantity:", vmap.get("what:quantity","??"));
	drain::image::TreeSVG & group = track[groupName];

	if (group->isUndefined()){
		group->setType(NodeSVG::GROUP);
		group->setId(groupName);
		group->addClass(drain::image::LayoutSVG::ALIGN_FRAME);
		/*
		if (ctx.mainOrientation == drain::image::Align::Axis::VERT){
			group->setAlign(AlignSVG::OUTSIDE, Align::Axis::HORZ, (ctx.mainDirection==LayoutSVG::Direction::INCR) ? Align::MAX : Align::MIN);
			group->setAlign(AlignSVG::INSIDE,  Align::Axis::VERT, Align::MIN); // drain::image::AlignSVG::VertAlign::TOP);
		}
		else { // VERT  -> ASSERT? if (ctx.mainOrientation == drain::image::Align::Axis::VERT){
			group->setAlign(AlignSVG::INSIDE,  Align::Axis::HORZ, Align::MIN); // drain::image::AlignSVG::HorzAlign::LEFT);
			group->setAlign(AlignSVG::OUTSIDE, Align::Axis::VERT, (ctx.mainDirection==LayoutSVG::Direction::INCR) ? Align::MAX : Align::MIN);
		}
		*/
		// group->set("debug", groupName); // debug
		// if (!track.hasChild(groupName)){  // ctx.svgPanelConf.
		// drain::image::TreeSVG & group = track[groupName](NodeSVG::GROUP);  // ctx.svgPanelConf.
		// group->addClass(drain::image::LayoutSVG::ALIG NED); // not needed?
		// mout.accept<LOG_WARNING>("added group: '", groupName, "' <= ", groupMapper);
	}

	return track[groupName](NodeSVG::GROUP); // ctx.svgPanelConf.

}

drain::image::TreeSVG & RackSVG::getPanel(RackContext & ctx, const drain::FilePath & filepath){

	// For each image an own group is created (for clarity, to contain also title TEXT's etc)
	const std::string name = drain::StringBuilder<'-'>(filepath.basename, filepath.extension);
	drain::image::TreeSVG & group = getCurrentGroup(ctx)[name]; // (NodeSVG::GROUP);

	if (group->isUndefined()){
		group->setType(NodeSVG::GROUP);
		group->setId(filepath.basename + 'G');

		//bool FWD = (ctx.mainDirection==LayoutSVG::Direction::INCR);

		if (ctx.mainOrientation == drain::image::Align::Axis::HORZ){
			group->setAlign(AlignSVG::OUTSIDE, Align::Axis::HORZ, (ctx.mainDirection==LayoutSVG::Direction::INCR) ? Align::MAX : Align::MIN);
			group->setAlign(AlignSVG::INSIDE,  Align::Axis::VERT, Align::MIN); // drain::image::AlignSVG::VertAlign::TOP);
			/*
			group->setAlign(AlignSVG::ANCHOR,  Align::Axis::HORZ, Align::MAX); // LEFT
			group->setAlign(AlignSVG::OBJECT,  Align::Axis::HORZ, Align::MIN); // LEFT
			group->setAlign(AlignSVG::ANCHOR,  Align::Axis::VERT, Align::MIN); // drain::image::AlignSVG::VertAlign::TOP);
			group->setAlign(AlignSVG::OBJECT,  Align::Axis::VERT, Align::MIN); // TOP
			*/
		}
		else { // VERT  -> ASSERT? if (ctx.mainOrientation == drain::image::Align::Axis::VERT){
			group->setAlign(AlignSVG::INSIDE,  Align::Axis::HORZ, Align::MIN); // drain::image::AlignSVG::HorzAlign::LEFT);
			group->setAlign(AlignSVG::OUTSIDE, Align::Axis::VERT, (ctx.mainDirection==LayoutSVG::Direction::INCR) ? Align::MAX : Align::MIN);
			/*
			group->setAlign(AlignSVG::ANCHOR,  Align::Axis::VERT, Align::MAX); // LEFT
			group->setAlign(AlignSVG::OBJECT,  Align::Axis::VERT, Align::MIN); // LEFT
			group->setAlign(AlignSVG::ANCHOR,  Align::Axis::HORZ, Align::MIN); // drain::image::AlignSVG::VertAlign::TOP);
			group->setAlign(AlignSVG::OBJECT,  Align::Axis::HORZ, Align::MIN); // TOP
			*/
		}
		// group->addClass(LayoutSVG::ALI GNED);  // MUST!
	}

	return group;

}

drain::image::TreeSVG & RackSVG::addImage(RackContext & ctx, const drain::image::Image & src, const drain::FilePath & filepath){ // what about prefix?

	using namespace drain::image;

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// For each image an own group is created (for clarity, to contain also title TEXT's etc)
	/*
	const std::string name = drain::StringBuilder<'-'>(filepath.basename, filepath.extension);
	drain::image::TreeSVG & group = getCurrentGroup(ctx)[name](NodeSVG::GROUP);
	group->addClass(IMAGE_FRAME);
	*/
	mout.attention("filepath:", filepath);

	drain::image::TreeSVG & panelGroup = getPanel(ctx, filepath);

	// TEST
	panelGroup->setAlignAnchor("image");

	drain::image::TreeSVG & image = panelGroup["image"](NodeSVG::IMAGE); // +EXT!
	image->setId(); // autom.
	// image->addClass(drain::image::AlignSVG::ANCHOR); -> setAlignAnchor
	image->set("name", filepath.basename); // Note: without extension
	image->set("width", src.getWidth());
	image->set("height", src.getHeight());
	image->set("opacity", 0.5);
	image->set("xlink:href", filepath);
	image["title"](drain::image::svg::TITLE) = filepath.basename;

	// DEBUG: (may be fatal for input.sh etc.)
	// drain::image::NodeSVG::toStream(std::cout, image);
	drain::image::TreeSVG & rect = panelGroup["rect"](NodeSVG::RECT);
	rect->set("width",  160); // debug
	rect->set("height", 100); // debug
	rect->set("fill", "none"); // just to make sure...
	rect->setStyle("stroke", "black");
	rect->setStyle("stroke-width", "2px");
	// rect->setStyle("border-style", "dotted");
	rect->setStyle("stroke-dasharray", {2,5});
	// rect->setAlign<AlignSVG::INSIDE>(AlignSVG::LEFT);
	rect->setAlign(AlignSVG::INSIDE, AlignSVG::RIGHT); // AlignSVG::RIGHT);
	rect->setAlign(AlignSVG::INSIDE, AlignSVG::MIDDLE); // AlignSVG::MIDDLE);
	// rect->setAlign(AlignSVG::OBJECT, AlignSVG::RIGHT);
	//rect->setAlignInside(LayoutSVG::Axis::HORZ, AlignSVG::MAX);
	//rect->setAlignInside(LayoutSVG::Axis::VERT, AlignSVG::MID);

	drain::image::TreeSVG & rect2 = panelGroup["rect2"](NodeSVG::RECT);
	rect2->set("width",  150); // debug
	rect2->set("height", 120); // debug
	rect2->set("fill", "none"); // just to make sure...
	rect2->setStyle("stroke", "green");
	rect2->setStyle("stroke-width", "5px");
	rect2->setStyle("stroke-dasharray", {5,2,3});
	rect2->setAlign(AlignSVG::OUTSIDE, AlignSVG::CENTER); // AlignSVG::CENTER);
	rect2->setAlign(AlignSVG::OUTSIDE, AlignSVG::BOTTOM); //  AlignSVG::BOTTOM);
	//rect2->setAlign(AlignSVG::OBJECT, AlignSVG::CENTER);
	//rect2->setAlign(AlignSVG::ANCHOR, AlignSVG::BOTTOM);
	//rect2->setAlign(AlignSVG::OBJECT, AlignSVG::BOTTOM);

	// Metadata:
	drain::image::TreeSVG & metadata = panelGroup["metadata"](svg::METADATA);

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


drain::image::TreeSVG & RackSVG::addImage(RackContext & ctx, const drain::image::TreeSVG & svg, const drain::FilePath & filepath){ // what about prefix?

	drain::image::TreeSVG & group = getCurrentGroup(ctx); //[filepath.basename+"_Group"](NodeSVG::GROUP);

	//drain::image::TreeSVG & imageGroup = group[filepath.basename](NodeSVG::GROUP);

	drain::image::TreeSVG & image = group[filepath.basename](NodeSVG::IMAGE); // +EXT!
	//image->addClass("float", "legend");
	//image->addClass("MARGINAL", "legend"); MOVED TO: images.cpp
	image->set("width", svg->get("width", 0));
	image->set("height", svg->get("height", 0));
	image->set("xlink:href", filepath);
	image["basename"](drain::image::svg::TITLE) = filepath.basename;
	return image;
}

drain::image::TreeSVG & RackSVG::addImage(RackContext & ctx, const drain::FilePath & filepath, const drain::Frame2D<double> & frame){ // what about prefix?

	drain::image::TreeSVG & group = getCurrentGroup(ctx); //[filepath.basename+"_Group"](NodeSVG::GROUP);

	drain::image::TreeSVG & image = group[filepath.basename](NodeSVG::IMAGE); // +EXT!

	image->set("width", frame.width);
	image->set("height", frame.height);
	image->set("xlink:href", filepath);
	image["basename"](drain::image::svg::TITLE) = filepath.basename;
	return image;
}



// Re-align elements etc
void RackSVG::completeSVG(RackContext & ctx, const drain::FilePath & filepath){

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	drain::image::TreeSVG & mainGroup = getMain(ctx);

	// TODO: add explanation
	if (!ctx.svgPanelConf.absolutePaths){
		drain::image::NodeSVG::path_list_t pathList;
		drain::image::NodeSVG::findByTag(mainGroup, drain::image::svg::IMAGE, pathList);
		if (!filepath.dir.empty()){
			const std::string dir = filepath.dir.str()+'/';  // <- consider plain, and remove leading slashes, or add only if non-empty.
			for (drain::image::NodeSVG::path_t & p: pathList){
				drain::image::TreeSVG & image = mainGroup(p);
				//drain::FilePath fp(image->get("xlink:href"));
				std::string imagePath = image->get("xlink:href");
				if (drain::StringTools::startsWith(imagePath, dir)){
					image->set("xlink:href", imagePath.substr(dir.size()));
				}
				else {
					mout.attention("could not set relative path: ", p, " href:", imagePath);
				}
				//mout.attention("path: ", p, " href:", image->get("xlink:href"));
			}
		}
	}


	drain::Point2D<drain::image::svg::coord_t> start(0,0);
	//TreeUtilsSVG::superAlign(mainGroup, Align::HORZ, LayoutSVG::INCR, start);superAlign
	mout.attention("next superAlign ", ctx.mainOrientation, '|', ctx.mainDirection);
	TreeUtilsSVG::superAlign(mainGroup, ctx.mainOrientation, ctx.mainDirection, start);

	/*
	/// Search for PANEL's: all the containers insider which elements will be aligned.
	drain::image::NodeSVG::path_list_t pathList;
	drain::image::NodeSVG::findByClass(mainGroup, drain::image::AlignSVG::ALIGN_GROUP, pathList);


	// NEW
	for (const drain::image::NodeSVG::path_t & p: pathList){

		mout.debug("aligning: ", p);
		drain::image::TreeSVG & group = mainGroup[p](NodeSVG::GROUP);
		TreeUtilsSVG::superAlign(group, start);
	}
	*/


	drain::Frame2D<int> mainFrame; // OLD: remove this later



	/// Collect
	/*
	MetaDataCollectorSVG metadataCollector;
	drain::TreeUtils::traverse(metadataCollector, ctx.svgTrack);
	drain::TreeUtils::dump(ctx.svgTrack);
	*/
	if (mout.isLevel(LOG_DEBUG)){
		mout.special("dumping SVG tree");
		drain::TreeUtils::dump(ctx.svgTrack);
	}

	// drain::Output outfile("unpruned.svg");
	// drain::image::NodeSVG::toStream(outfile, ctx.svgTrack);

	MetaDataPrunerSVG metadataPruner;
	drain::TreeUtils::traverse(metadataPruner, ctx.svgTrack);

	if (mout.isLevel(LOG_DEBUG)){
		mout.special("dumping SVG tree");
		drain::TreeUtils::dump(ctx.svgTrack);
	}

	// REUSE:
	start = {0,0};

	if (ctx.svgPanelConf.title != "none"){ // also "false" !?

		TitleCreatorSVG titleCreator;
		drain::TreeUtils::traverse(titleCreator, ctx.svgTrack); // or mainTrack enough?


		// MAIN HEADER(s)
		if (mainGroup.hasChild("metadata") || (ctx.svgPanelConf.title != "false")){ // hmmm

			TreeSVG & headerGroup = ctx.svgTrack["headerGroup"](svg::GROUP);

			TreeSVG & headerRect = headerGroup["headerRect"](svg::RECT);
			//headerRect->setStyle("fill:slateblue");
			headerRect->setId("headerRect");
			headerRect->setStyle("fill:darkblue");
			headerRect->setStyle("opacity:0.25");
			headerRect->set("x", -20);
			headerRect->set("y", -titleCreator.mainHeaderHeight);
			headerRect->set("width",  35 + mainFrame.getWidth());
			headerRect->set("height", 15 + titleCreator.mainHeaderHeight);

			/*  Future extension
			TreeSVG & headerLeft = headerGroup["headerLeft"](svg::TEXT);
			headerLeft->addClass(CmdBaseSVG::FLOAT, CmdBaseSVG::LEFT);
			headerLeft->set("ref", headerRect->getId());
			headerLeft->setText("left");

			TreeSVG & headerRight = headerGroup["headerRight"](svg::TEXT);
			headerRight->addClass(CmdBaseSVG::FLOAT, CmdBaseSVG::RIGHT);
			headerRight->set("ref", headerRect->getId());
			headerRight->setText("right");
			*/

			//TreeSVG & mainHeader = headerGroupsubHeadert"](svg::TEXT);
			TreeSVG & mainHeader = headerGroup["GENERAL"](svg::TEXT);
			mainHeader -> setAlign(AlignSVG::OUTSIDE, AlignSVG::RIGHT); //
			mainHeader -> setAlign(AlignSVG::INSIDE, AlignSVG::MIDDLE); //AlignSVG::VertAlign::MIDDLE);
			// mainHeader -> setAlignInside(Align::Axis::HORZ, Align::Position::MID);
			//tsvg::markAligned(headerRect, mainHeader, alignSvg::CENTER, alignSvg::MIDDLE);
			// mainHeader->set("x", 51); // will be realigned
			// mainHeader->set("y", 61); // will be realigned
			mainHeader->setStyle({
				{"font-size", "2.5em"},
				{"stroke", "none"},
				{"fill", "darkblue"}
			});
			// Ensure order
			mainHeader["product"](svg::TSPAN);
			mainHeader["prodpar"](svg::TSPAN);


			TreeSVG & timeHeader = headerGroup["TIME"](svg::TEXT);
			timeHeader->addClass(TIME);
			timeHeader -> setAlign(AlignSVG::OUTSIDE, AlignSVG::TOP); // AlignSVG::VertAlign::TOP);    // Outside(Align::Axis::VERT, Align::Position::MIN);
			timeHeader -> setAlign(AlignSVG::OUTSIDE, AlignSVG::RIGHT); // AlignSVG::HorzAlign::RIGHT);  // Inside(Align::Axis::HORZ, Align::Position::MAX);
			//tsvg::markAligned(headerRect, timeHeader, alignSvg::RIGHT, alignSvg::MIDDLE);
			// Ensure order
			timeHeader["date"](svg::TSPAN);
			timeHeader["time"](svg::TSPAN);


			TreeSVG & locationHeader = headerGroup["LOCATION"](svg::TEXT);
			locationHeader->addClass(LOCATION);
			locationHeader -> setAlign(AlignSVG::INSIDE, AlignSVG::RIGHT); // AlignSVG::HorzAlign::RIGHT);
			//tsvg::markAligned(headerRect, locationHeader, alignSvg::LEFT, alignSvg::MIDDLE);



			// Automatic
			if ((ctx.svgPanelConf.title == "auto") || ctx.svgPanelConf.title.empty()){
				// TODO: vars aligned by class (time, etc)
				VariableFormatterODIM<drain::FlexibleVariable> formatter; // (No instance properties used, but inheritance/overriding)
				for (const auto & entry: mainGroup["metadata"]->getAttributes()){
					// Here, class is also the element key:
					// Consider separate function, recognizing date/time, and formatting?

					std::string key;
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
						key = "TIME"; // str + enum??
					}
					else if (formatter.formatTime(sstr, entry.first, entry.second, "%H:%M UTC")){
						mout.special("Recognized  TIME: ", entry.first, '=', entry.second);
						key = "TIME"; // str + enum??
					}
					else if (formatter.formatPlace(sstr, entry.first, entry.second)){
						mout.special("Recognized PLACE: ", entry.first, '=', entry.second);
						key = "LOCATION"; // str + enum??
					}
					else {
						sstr << entry.second.toStr();
						key = "GENERAL";
					}
					sstr << "&#160;";

					TreeSVG & subHeader = headerGroup[key](svg::TEXT);
					TreeSVG & sh = subHeader.hasChild(entry.first) ? subHeader[entry.first] : subHeader;
					sh->ctext = sstr.str();
					sh->set("XXX", key);
					// sh->ctext += entry.second.toStr();
					// sh->ctext += "&#160;"; // ' ';

				}

			}
			else if (ctx.svgPanelConf.title != "false"){
				VariableFormatterODIM<drain::Variable> formatter; // (No instance properties used, but inheritance/overriding)
				drain::StringMapper titleMapper(RackContext::variableMapper); // XXX
				titleMapper.parse(ctx.svgPanelConf.title);
				const drain::VariableMap & v = ctx.getStatusMap();
				//mainHeader->ctext += titleMapper.toStr(v);
				mainHeader->ctext += titleMapper.toStr(v, -1, formatter);
				mainHeader->ctext += ' ';
			}
			// else title == "false"

			// TODO: develop
			if (mainHeader.hasChildren() || !mainHeader->ctext.empty()){
				mainFrame.height +=  titleCreator.mainHeaderHeight; // why +=
				start.y           = -titleCreator.mainHeaderHeight;
			}

			// tsvg::alignText(headerGroup);

		}

		// IMAGE HEADER and MAIN HEADER positions
		// mout.attention("aligning texts of: ", group -> getTag());
		// tsvg::alignText(mainGroup);

	}

	/*
	for (const drain::image::NodeSVG::path_t & p: pathList){

		mout.attention("Would like to align: ", p);
		drain::image::TreeSVG & group = mainGroup[p](NodeSVG::GROUP);

	}
	*/
	// drain::image::TreeUtilsSVG::alignDomains(mainGroup);

	{
		drain::image::BBoxSVG bb;
		drain::image::TreeUtilsSVG::computeBoundingBox(ctx.svgTrack, bb);

		//ctx.svgTrack->set("width",  bb.width);
		//ctx.svgTrack->set("height", bb.height);
		/*
		const std::string viewBox = drain::StringBuilder<' '>(start.x, start.y, bb.width, bb.height);
		ctx.svgTrack->set("viewBox", viewBox);
		*/
		ctx.svgTrack->setBoundingBox(bb);
	}
	/*
	ctx.svgTrack->set("width",  mainFrame.width);
	ctx.svgTrack->set("height", mainFrame.height);
	const std::string viewBox = drain::StringBuilder<' '>(start.x, start.y, mainFrame.width, mainFrame.height);
	ctx.svgTrack->set("viewBox", viewBox);
	*/
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

	if (ctx.statusFlags.isSet(drain::Status::INPUT_ERROR)){
		mout.warn("input failed, skipping");
		return;
	}

	if (ctx.statusFlags.isSet(drain::Status::DATA_ERROR)){
		mout.warn("data error, skipping");
		return;
	}

	//TreeSVG &  // = svg["bg"];
	drain::image::TreeSVG svg(svg::SVG);
	// TreeSVG svg; // (svg::SVG); REDO this, check copy constr!
	svg->setType(svg::SVG);

	drain::image::TreeSVG & main = svg["main"];
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



} // namespace rack
