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


namespace rack {

//typedef drain::image::AlignSVG alignSvg;



template <>
const drain::EnumDict<RackSVG::TitleClass>::dict_t  drain::EnumDict<RackSVG::TitleClass>::dict = {
		DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, GENERAL),
		DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, LOCATION),
		DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, TIME),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, IMAGE_SET)
};

/// Group identifiers for elements which be automatically aligned (stacked horizontally or vertically)
const std::string RackSVG::IMAGE_PANEL("imageFrame");



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

drain::image::TreeSVG & RackSVG::getStyle(RackContext & ctx){

	TreeSVG & style = ctx.svgTrack["style"];

	//if (!ctx.svgTrack.hasChild("style")){
	if (style->isUndefined()){

		style->setType(svg::STYLE);
		//TreeSVG & style = ctx.svgTrack["style"](svg::STYLE);

		// MODE 2: text (CTEXT) of sub-element
		style["text"] = "fill:black";
		//style["text"] = ("stroke:white; stroke-width:0.5em; stroke-opacity:0.25; fill:black; paint-order:stroke; stroke-linejoin:round");
		style["text"].data = "stroke:white";

		// MODE 3: attribs of sub-element
		style["rect.imageFrame"].data = {
				{"stroke", "black"},
				{"stroke-opacity", "0.25"},
				{"stroke-width", "0.3em"}
		};
		//style["group.imageFrame > rect"].data = {

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

	return style;
}


drain::image::TreeSVG & RackSVG::getMain(RackContext & ctx){

	using namespace drain::image;

	//TreeSVG & style = ctx.svgTrack["style"](svg::STYLE);
	RackSVG::getStyle(ctx);

	/*
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
	*/

	drain::image::TreeSVG & main = ctx.svgTrack["outputs"];

	if (main -> isUndefined()){
		main->setType(NodeSVG::GROUP);
		main->setId("outputs");
		main->setAlign(AlignSVG::RIGHT, AlignSVG::OUTSIDE); // AlignSVG::RIGHT);
		main->setAlign(AlignSVG::TOP);   // AlignSVG::TOP);
	}

	return main;

}

drain::image::TreeSVG & RackSVG::getCurrentGroup(RackContext & ctx){ // what about prefix?


	drain::image::TreeSVG & track = getMain(ctx);

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	/*
	drain::StringMapper groupMapper(RackContext::variableMapper); // XXX
	groupMapper.parse(ctx.svgPanelConf.groupName);
	const drain::VariableMap & vmap = ctx.getStatusMap();
	const std::string groupName = groupMapper.toStr(vmap, '_', RackContext::variableFormatter);
	*/
	///
	const std::string groupName = ctx.getFormattedStatus(ctx.svgPanelConf.groupName); // status updated upon last file save
	//const std::string groupName = groupMapper.toStr(vmap); // No ${variable|format} supported here (not needed - this is just for identifying, not style!)

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
	drain::image::TreeSVG & group = getCurrentGroup(ctx)[name];

	if (group->isUndefined()){
		group->setType(NodeSVG::GROUP);
		group->setId(filepath.basename + 'G');
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
	panelGroup->addClass(IMAGE_PANEL); // Add elems ^ here ^ ?

	// TEST
	panelGroup->setAlignAnchor("image");

	drain::image::TreeSVG & image = panelGroup["image"](NodeSVG::IMAGE); // +EXT!
	image->setId(filepath.basename); // autom.
	// image->addClass(drain::image::AlignSVG::ANCHOR); -> setAlignAnchor
	image->set("name", filepath.basename); // Note: without extension
	image->set("width", src.getWidth());
	image->set("height", src.getHeight());
	image->set("xlink:href", filepath);
	image["title"](drain::image::svg::TITLE) = filepath.basename;

	// DEBUG: (may be fatal for input.sh etc.)
	// drain::image::NodeSVG::toStream(std::cout, image);


	if ( ctx.svgDebug > 0){
		image->set("opacity", 0.5);

		drain::image::TreeSVG & rect = panelGroup["rect"](NodeSVG::RECT);
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

		drain::image::TreeSVG & rect2 = panelGroup["rect2"](NodeSVG::RECT);
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


// Consider changing this to visitor.

// Re-align elements etc
void RackSVG::completeSVG(RackContext & ctx){ //, const drain::FilePath & filepath){


	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	drain::image::TreeSVG & mainGroup = getMain(ctx);

	if (mout.isLevel(LOG_DEBUG)){
		// mout.special("dumping SVG tree");
		// drain::TreeUtils::dump(ctx.svgTrack);
		drain::Output outfile("svg-0-unaligned.svg");
		drain::image::NodeSVG::toStream(outfile.getStream(), ctx.svgTrack);
	}


	// drain::Point2D<drain::image::svg::coord_t> start(0,0);
	//TreeUtilsSVG::superAlign(mainGroup, Align::HORZ, LayoutSVG::INCR, start);superAlign
	mout.attention("next superAlign ", ctx.mainOrientation, '|', ctx.mainDirection);
	//TreeUtilsSVG::superAlign(mainGroup, ctx.mainOrientation, ctx.mainDirection);

	if (mout.isLevel(LOG_DEBUG)){
		// mout.special("dumping SVG tree");
		// drain::TreeUtils::dump(ctx.svgTrack);
		drain::Output outfile("svg-0-unpruned.svg");
		drain::image::NodeSVG::toStream(outfile.getStream(), ctx.svgTrack);
	}


	MetaDataPrunerSVG metadataPruner;
	drain::TreeUtils::traverse(metadataPruner, ctx.svgTrack);

	if (mout.isLevel(LOG_DEBUG)){
		mout.special("dumping SVG tree");
		drain::Output outfile("svg-1-pruned.svg");
		drain::image::NodeSVG::toStream(outfile.getStream(), ctx.svgTrack);
		// drain::TreeUtils::dump(ctx.svgTrack);
	}

	if (ctx.svgPanelConf.title != "none"){ // also "false" !?

		TitleCreatorSVG titleCreator;
		drain::TreeUtils::traverse(titleCreator, ctx.svgTrack); // or mainTrack enough?


		// MAIN HEADER(s)
		if (mainGroup.hasChild("metadata") || (ctx.svgPanelConf.title != "false")){ // hmmm

			TreeSVG & group = RackSVG::getMain(ctx); // ctx.svgTrack["headerGroup"](svg::GROUP);

			TreeSVG & headerRect = group["headerRect"](svg::RECT);
			//headerRect->setStyle("fill:slateblue");
			headerRect->setId("headerRect");
			headerRect->setStyle("fill", "blue");
			headerRect->setStyle("opacity", 0.25);

			TreeSVG & mainHeader = group["GENERAL"](svg::TEXT);
			mainHeader -> setAlign(AlignSVG::RIGHT, AlignSVG::OUTSIDE); //
			mainHeader -> setAlign(AlignSVG::MIDDLE, AlignSVG::INSIDE); //AlignSVG::VertAlign::MIDDLE);
			mainHeader->setStyle({
				{"font-size", "20"},
				{"stroke", "none"},
				{"fill", "darkslateblue"}
			});
			// Ensure order
			mainHeader["product"](svg::TSPAN);
			mainHeader["prodpar"](svg::TSPAN);


			TreeSVG & timeHeader = group["TIME"](svg::TEXT);
			timeHeader->addClass(TIME);
			timeHeader["date"](svg::TSPAN);
			timeHeader["time"](svg::TSPAN);


			TreeSVG & locationHeader = group["LOCATION"](svg::TEXT);
			locationHeader->addClass(LOCATION);
			locationHeader -> setAlign(AlignSVG::RIGHT, AlignSVG::INSIDE); // AlignSVG::HorzAlign::RIGHT);
			//tsvg::markAligned(headerRect, locationHeader, alignSvg::LEFT, alignSvg::MIDDLE);
			locationHeader["NOD"](svg::TSPAN);
			locationHeader["PLC"](svg::TSPAN);



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

					TreeSVG & subHeader = group[key](svg::TEXT);
					TreeSVG & sh = subHeader.hasChild(entry.first) ? subHeader[entry.first] : subHeader;
					sh->ctext += sstr.str();
					//sh->set("XXX", key);
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


			// TODO: develop
			/*
			if (mainHeader.hasChildren() || !mainHeader->ctext.empty()){
				mainFrame.height +=  titleCreator.mainHeaderHeight; // why +=
				start.y           = -titleCreator.mainHeaderHeight;
			}
			*/

			// tsvg::alignText(headerGroup);

		}

		// IMAGE HEADER and MAIN HEADER positions
		// mout.attention("aligning texts of: ", group -> getTag());
		// tsvg::alignText(mainGroup);
	}

	TreeUtilsSVG::superAlign(mainGroup, ctx.mainOrientation, ctx.mainDirection);

	drain::image::TreeUtilsSVG::finalizeBoundingBox(ctx.svgTrack);


}




int MetaDataPrunerSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path){
	// std::cerr << __FUNCTION__ << ':' << path << std::endl;
	return 0;
}



int MetaDataPrunerSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	drain::Logger mout(__FILE__, __FUNCTION__);

	TreeSVG & current = tree(path);

	//if (!((current->getType()==svg::GROUP) || (current->getType()==svg::IMAGE))){ // At least METADATA must be skipped...
	if (current->getType() != svg::GROUP){ // At least METADATA must be skipped...
		return 0;
	}

	/// Statistics: computer count for each (key,value> pair.
	typedef std::map<std::string, unsigned short> variableStat_t;
	variableStat_t stat;
	/// Number of children having (any) metadata.
	int count = 0;

	/// Iterate children and their attributes: check which attributes (key and value) are shared by all the children.
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


	if (count > 0){

		TreeSVG & metadata = current["metadata"](svg::METADATA);
		metadata->addClass("md_shared");

		TreeSVG & debugSharedBase = current["shared"](svg::DESC);
		debugSharedBase->set("type", "SHARED");
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

		for (const auto & e: stat){

			mout.pending<LOG_DEBUG>('\t', e.first, ':', e.second);

			// std::cerr << "\t vector " << e.first << ' ' << e.second << std::endl;
			std::string key, value;
			drain::StringTools::split2(e.first, key, value, '=');
			if (e.second == count){

				mout.accept<LOG_DEBUG>('\t', e.first, ' ', path.str());

				debugSharedBase->ctext += ' ';
				debugSharedBase->ctext += e.first;
				// debugShared->set(key, value);

				metadata->set(key, value); // NOTE: becoming strings (consider type dict?)

				for (auto & entry: current.getChildren()){
					TreeSVG & child = entry.second;
					if (child.hasChild("metadata")){
						TreeSVG & childMetadata = entry.second["metadata"](svg::METADATA);
						childMetadata -> remove(key);
						childMetadata -> addClass("md_pruned");
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
 *  \param frame - IMAGE or RECT inside which the text will be aligned
 */
TreeSVG & getTextElem(const TreeSVG & frame, TreeSVG & current, const std::string key){


	/*
	std::string name = current->get("name", "unknown-image");
	if (timeClass.has(key)){
		name += "_TIME";
	}
	if (locationClass.has(key)){
		name += "_LOC";
	}
	*/


	TreeSVG & text = current[key+"_title"];

	// Temporary (until aligned)
	const int x = frame->get("x", 0);
	const int y = frame->get("y", 0);

	if (text -> isUndefined()){
		text -> setType(svg::TEXT);
		//text->set("ref", current["image"]->getId());
		//text->set("ref", 0);
		text->set("x", x + 52);
		text->set("y", y + 30);
		// drain::image::TreeUtilsSVG::markAligned(frame, text);
		// text->addClass(svgAlignBase::FLOAT); // "imageTitle" !
		// text->set("ref", frame->getId());
	}

	// TODO: align conf for TIME and LOCATION from svgConf
	/*
	if (timeClass.has(key)){
		text->addClass("TIME",  "BOTTOM", CmdBaseSVG::LEFT); // CmdBaseSVG::FLOAT,
		text->set("y", y + 40); // temporary
	}

	if (locationClass.has(key)){
		text->addClass("LOCATION", "TOP", "RIGHT"); // CmdBaseSVG::FLOAT,
		text->set("y", y + 60); // temporary
	}
	*/

	return text;
}



int TitleCreatorSVG::visitPostfix(TreeSVG & root, const TreeSVG::path_t & path){


	TreeSVG & group = root(path);

	if (!group->typeIs(svg::GROUP)){ // At least METADATA must be skipped...
		return 0;
	}

	if (!group.hasChild("metadata")){
		return 0;
	}

	if (!group->hasClass(RackSVG::IMAGE_PANEL)){
		return 0;
	}


	drain::Logger mout(__FILE__, __FUNCTION__);

	group->setAlignAnchor("image");


	TreeSVG & metadata = group["metadata"];


	if (!metadata->getAttributes().empty()){

		/*
		TreeSVG & text = current["bottom-right"](svg::TEXT);
		text->addClass("FLOAT BOTTOM imageTitle debug");
		text->setText(metadata->getAttributes());
		*/

		VariableFormatterODIM<std::string> formatter; // (No instance properties used, but inheritance/overriding)
		// mout.attention("handle: ", current.data);
		// Note: these are "subtitles", not the main title

		for (const auto & attr: metadata->getAttributes()){

			// This is a weird (old code)?  Can metadata have formatting, like time|%Y=201408171845 ??
			// Anyway, after split, key contains attrib key.
			std::string key, format;
			drain::StringTools::split2(attr.first, key, format, '|');

			bool IS_TIME     = (ODIM::timeKeys.count(attr.first)>0) || (ODIM::dateKeys.count(attr.first)>0);
			bool IS_LOCATION =  ODIM::locationKeys.count(attr.first)>0;

			mout.warn(key, '/', attr.first, '=', attr.second, " => ", IS_TIME, '/', IS_LOCATION);

			std::string elemKey("title");
			if (IS_TIME){
				elemKey += "-time";
			}

			if (IS_LOCATION){
				elemKey += "-location";
			}

			//TreeSVG & text = getTextElem(current["image"], current, key);
			TreeSVG & text = group[elemKey](svg::TEXT);  // +"_title"
			text->addClass("imageTitle"); // style class (only)

			TreeSVG & tspan = text[attr.first](svg::TSPAN);
			tspan->addClass(attr.first); // allows user-specified style
			//drain::StringTools::split2(attr.second.toStr(), v, format, '|');

			if (IS_TIME){
				text->addClass(RackSVG::TIME); // ,  alignSvg::BOTTOM, alignSvg::LEFT); // CmdBaseSVG::FLOAT,
				text->setAlign(AlignSVG::TOP); //, AlignSVG::INSIDE);
				text->setAlign(AlignSVG::LEFT); //, AlignSVG::INSIDE);
				if (format.empty()){
					//v = attr.second.toStr();
					if (drain::StringTools::endsWith(key, "date")){
						format = "%Y/%m/%d";
					}
					else if (drain::StringTools::endsWith(key, ":time")){
						format = "%H:%M UTC";
					}
					else if (drain::StringTools::endsWith(key, "time")){
						format = "%H:%M:%S UTC";
					}
					text->set("format", format);
				}
				mout.accept<LOG_NOTICE>("TIME text format", format);
				/* TODO:
				std::stringstream sstr;
				formatVariable2(attr.second, ":2:3", sstr);
				text->ctext = sstr.str();
				*/
				// text->ctext = " "; // drain::StringBuilder<':'>(attr.first, attr.second);
			}
			else if (IS_LOCATION){
				text->addClass(RackSVG::LOCATION); // , alignSvg::TOP, alignSvg::RIGHT); // CmdBaseSVG::FLOAT,
				text->setAlign(AlignSVG::RIGHT); //, AlignSVG::INSIDE); // AlignSVG::HorzAlign::RIGHT);  // text->setAlignInside(LayoutSVG::Axis::HORZ, AlignSVG::MAX); // = RIGHT
				text->setAlign(AlignSVG::BOTTOM); //, AlignSVG::INSIDE); // AlignSVG::VertAlign::BOTTOM); // text->setAlignInside(LayoutSVG::Axis::VERT, AlignSVG::MAX); // = BOTTOM
				// text->set("y", y + 60); // temporary
				text->ctext = " "; //drain::StringBuilder<'@'>(attr.first, attr.second);
			}
			else {
				text->setAlign(AlignSVG::CENTER); //, AlignSVG::INSIDE); // AlignSVG::HorzAlign::RIGHT);  // text->setAlignInside(LayoutSVG::Axis::HORZ, AlignSVG::MAX); // = RIGHT
				text->setAlign(AlignSVG::MIDDLE); //, AlignSVG::INSIDE); // AlignSVG::VertAlign::BOTTOM); // text->setAlignInside(LayoutSVG::Axis::VERT, AlignSVG::MAX); // = BOTTOM
				text->ctext = drain::StringBuilder<'+'>(attr.first, attr.second);
			}

			text->setWidth(10); // margin
			text->setHeight(16); // row height
			text->addClass(LayoutSVG::FLOAT); // Anchor defined, but need for proper bbox computation! (Yet text should be discarded)


			// mout.attention("handle: ", attr.first, " ", v, " + ", format);

			if (format.empty()){
				tspan->ctext += attr.second.toStr();
			}
			else {
				//mout.attention("handle: ", attr.first, " ", v, " + ", format);
				std::stringstream sstr;
				//VariableFormatterODIM<NodeSVG::map_t::value_t>::formatVariable(attr.first, v, format, sstr);
				formatter.formatVariable(key, attr.second, format, sstr);
				// drain::VariableFormatter<NodeSVG::map_t::value_t>::formatValue(v, format, sstr);
				tspan->ctext += sstr.str();
				tspan->ctext += "?";
			}
			//tspan->ctext = attr.second.toStr();
			tspan->ctext += "&#160;"; //'_';
			//entry.second->set(key, "*"); // TODO: remove attribute
		}
	}
	else {
		mout.debug("title skipped, metadata empty under: ", path);
		// TreeSVG & text = current["mark"](svg::TEXT);
		// text->setText(path.str() + " empty metadata...");
		return 0;
	}

	return 0;

}



} // namespace rack

