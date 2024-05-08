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

//#include "drain/util/Convert.h"
#include "drain/util/Output.h"
#include "drain/util/StringMapper.h"
#include "drain/util/TreeXML.h"
#include "drain/util/TreeHTML.h"

#include "drain/image/FilePng.h"
#include "drain/image/TreeUtilsSVG.h"

#include "data/SourceODIM.h" // for NOD

#include "fileio-svg.h"


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

namespace rack {

/*
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
			for (const auto & attr: entry.second["metadata"]->getAttributes()){
				//metadata->set(attr.first, attr.second);
			}
		}
	}

	return 0;

}
*/


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

const std::string  & getTextClass(const std::string key){

	static
	const drain::ClassListXML timeClass = {"time", "date", "starttime"};

	static
	const drain::ClassListXML locationClass = {"site", "src", "lat", "lon", "PLC", "NOD", "WMO"};

	if (timeClass.has(key)){
		static const std::string s("TIME");
		return s;
	}
	else if (locationClass.has(key)){
		static const std::string s("LOCATION");
		return s;
	}
	else {
		static const std::string empty;
		return empty;
	}


}

static
const drain::ClassListXML timeClass = {"time", "date"};

static
const drain::ClassListXML locationClass = {"site", "src", "lat", "lon", "PLC", "NOD", "WMO"};


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
		text->set("ref", frame->getId());
		//text->set("ref", 0);
		text->set("x", x + 2);
		text->set("y", y + 20);
		text->addClass("FLOAT"); // "imageTitle" !
	}

	// TODO: align conf for TIME and LOCATION from svgConf
	/*
	if (timeClass.has(key)){
		text->addClass("TIME",  "BOTTOM", "LEFT"); // "FLOAT",
		text->set("y", y + 40); // temporary
	}

	if (locationClass.has(key)){
		text->addClass("LOCATION", "TOP", "RIGHT"); // "FLOAT",
		text->set("y", y + 60); // temporary
	}
	*/

	return text;
}







int TitleCreatorSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){


	TreeSVG & current = tree(path);

	if (current->getType() != svg::GROUP){ // At least METADATA must be skipped...
		return 0;
	}

	/*
	if (!((current->getType()==svg::GROUP) || (current->getType()==svg::IMAGE))){
		return 0;
	}
	*/


	if (!current.hasChild("metadata")){
		return 0;
	}

	drain::Logger mout(__FILE__, __FUNCTION__);


	TreeSVG & metadata = current["metadata"];


	if (!metadata->getAttributes().empty()){

		if (!current->classList.has("imageFrame")){
			TreeSVG & desc = current["debug"](svg::DESC);
			desc->ctext = drain::sprinter(metadata->getAttributes(), drain::Sprinter::plainLayout).str();
			desc->addClass("md_debug_noimg");
			return 0;
		}


		/*
		TreeSVG & text = current["bottom-right"](svg::TEXT);
		text->addClass("FLOAT BOTTOM imageTitle debug");
		text->setText(metadata->getAttributes());
		*/

		for (const auto & attr: metadata->getAttributes()){
			// consider str replace

			std::string key("title");
			if (timeClass.has(attr.first)){
				key += "-time";
			}

			if (locationClass.has(attr.first)){
				key += "-location";
			}

			//TreeSVG & text = getTextElem(current["image"], current, attr.first);
			TreeSVG & text = getTextElem(current["image"], current, key);
			text->addClass("imageTitle");

			if (timeClass.has(attr.first)){
				text->addClass("TIME",  "BOTTOM", "LEFT"); // "FLOAT",
				// text->set("y", y + 40); // temporary
				/* TODO:
				std::stringstream sstr;
				formatVariable2(attr.second, ":2:3", sstr);
				text->ctext = sstr.str();
				*/
			}

			if (locationClass.has(attr.first)){
				text->addClass("LOCATION", "TOP", "RIGHT"); // "FLOAT",
				// text->set("y", y + 60); // temporary
			}


			TreeSVG & tspan = text[attr.first](svg::TSPAN);
			//if (cls.empty())
			//	tspan->addClass(cls);
			tspan->addClass(attr.first); // allows user-specified style
			//std::string value = attr.second;
			std::string v, format;
			drain::StringTools::split2(attr.second.toStr(), v, format, '|');
			if (format.empty()){
				tspan->ctext = v;
			}
			else {
				std::stringstream sstr;
				drain::VariableFormatter<NodeSVG::map_t::value_t>::formatValue(v, format, sstr);
				tspan->ctext = sstr.str();
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

	return 	ctx.svgTrack["outputs"](NodeSVG::GROUP);

}

drain::image::TreeSVG & CmdBaseSVG::getCurrentGroup(RackContext & ctx){ // what about prefix?


	drain::image::TreeSVG & track = getMain(ctx);

	if (!track.hasChild(ctx.svgPanelConf.groupName)){
		drain::image::TreeSVG & group = track[ctx.svgPanelConf.groupName](NodeSVG::GROUP);
		// Ensure (repeatedly)
		group->addClass("imageSet");
		group->setId(ctx.svgPanelConf.groupName);
		// group->set("id", ctx.svgPanelConf.groupName);
		// group->set("name", ctx.svgPanelConf.groupName);
	}

	return track[ctx.svgPanelConf.groupName](NodeSVG::GROUP);

}


drain::image::TreeSVG & CmdBaseSVG::addImage(RackContext & ctx, const drain::image::Image & src, const drain::FilePath & filepath){ // what about prefix?

	// For each image an own group is created (for clarity, to contain also title TEXT's etc)
	const std::string name = drain::StringBuilder<'-'>(filepath.basename, filepath.extension);

	// drain::image::TreeSVG & group = getCurrentGroup(ctx)[filepath.basename+"_Group"](NodeSVG::GROUP);
	// drain::image::TreeSVG & image = group[name](NodeSVG::IMAGE); // +EXT!
	drain::image::TreeSVG & group = getCurrentGroup(ctx)[name](NodeSVG::GROUP);
	group->addClass("imageFrame");

	drain::image::TreeSVG & image = group["image"](NodeSVG::IMAGE); // +EXT!
	image->setId(); // autom.
	image->set("name", name);
	image->set("width", src.getWidth());
	image->set("height", src.getHeight());
	image->set("xlink:href", filepath);
	image["title"](drain::image::svg::TITLE) = filepath.basename;
	// image["description"](drain::image::svg::TITLE) = src.getProperties();

	drain::image::TreeSVG & rect = group["rect"](NodeSVG::RECT);
	rect->addClass("FLOAT", "imageFrame");
	rect->set("ref", image->getId());
	rect->set("width", src.getWidth());
	rect->set("height", src.getHeight());
	// rect->set("stroke", "black");
	rect->set("fill", "none"); // just to make sure...
	//rect->set("opacity", 0.25);


	/*
	drain::image::TreeSVG & text = group["img-text-ul"](NodeSVG::TEXT);
	text->addClass("FLOAT test");
	text->set("ref", image->getId());
	text->setText(filepath.basename);
	*/

	// Metadata:
	// drain::image::TreeSVG & metadata = image["metadata"](NodeSVG::METADATA);
	drain::image::TreeSVG & metadata    = group["metadata"](svg::METADATA);

	// Note assign: char * -> string  , "where:lat", "where:lon"
	if (src.properties.hasKey("what:source")){
		SourceODIM odim(src.properties.get("what:source",""));
		metadata->set("NOD", odim.NOD);
		metadata->set("PLC", odim.PLC);
	}

	// TODO: 1) time formatting 2) priority (startdate, starttime)
	for (const std::string key: {"what:date", "what:time", "what:quantity", "where:elangle", "prevCmdKey"}){
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

	drain::image::TreeSVG & description = group["description"](svg::DESC);
	description->getAttributes().importCastableMap(metadata->getAttributes());
	// todo: description  : prevCmdKey "what:product", "what:prodpar", "how:angles"

	return image;

}


drain::image::TreeSVG & CmdBaseSVG::addImage(RackContext & ctx, const drain::image::TreeSVG & svg, const drain::FilePath & filepath){ // what about prefix?

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

drain::image::TreeSVG & CmdBaseSVG::addImage(RackContext & ctx, const drain::FilePath & filepath, const drain::Frame2D<double> & frame){ // what about prefix?

	drain::image::TreeSVG & group = getCurrentGroup(ctx); //[filepath.basename+"_Group"](NodeSVG::GROUP);

	drain::image::TreeSVG & image = group[filepath.basename](NodeSVG::IMAGE); // +EXT!

	image->set("width", frame.width);
	image->set("height", frame.height);
	image->set("xlink:href", filepath);
	image["basename"](drain::image::svg::TITLE) = filepath.basename;
	return image;
}


// Re-align elements etc
void CmdBaseSVG::completeSVG(RackContext & ctx, const drain::FilePath & filepath){

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	drain::image::TreeSVG & mainGroup = getMain(ctx);

	if (!ctx.svgPanelConf.absolutePaths){
		drain::image::NodeSVG::path_list_t pathList;
		drain::image::NodeSVG::findByTag(mainGroup, drain::image::svg::IMAGE, pathList);
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

	drain::image::NodeSVG::path_list_t pathList;
	drain::image::NodeSVG::findByClass(mainGroup, "imageSet", pathList);

	drain::Frame2D<int> mainFrame;
	drain::Point2D<int> start(0,0);

	for (drain::image::NodeSVG::path_t & p: pathList){

		mout.debug("aligning: ", p);
		drain::image::TreeSVG & group = mainGroup[p](NodeSVG::GROUP);
		drain::Frame2D<int> frame;
		TreeUtilsSVG::getBoundingFrame(group, frame, ctx.svgPanelConf.orientation);

		mout.attention("aligning sequence: start:",  start.tuple(), ", frame: ", frame.tuple());
		drain::image::TreeUtilsSVG::alignSequence(group, frame, start, ctx.svgPanelConf.orientation, ctx.svgPanelConf.direction);

		// Recompute (detect marginal objs)
		//TreeUtilsSVG::getBoundingFrame(group, frame, ctx.svgPanelConf.orientation);
		/*
		drain::Box<double> rect;
		TreeUtilsSVG::getRect(group, rect);
		mout.attention("rect: ", rect, " for group: ", group.data);
		*/

		if (ctx.svgPanelConf.orientation == drain::image::PanelConfSVG::HORZ){
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
	if (mout.isLevel(LOG_INFO)){
		mout.special("dumping SVG tree");
		drain::TreeUtils::dump(ctx.svgTrack);
	}

	// drain::Output outfile("unpruned.svg");
	// drain::image::NodeSVG::toStream(outfile, ctx.svgTrack);

	MetaDataPrunerSVG metadataPruner;
	drain::TreeUtils::traverse(metadataPruner, ctx.svgTrack);

	if (mout.isLevel(LOG_INFO)){
		mout.special("dumping SVG tree");
		drain::TreeUtils::dump(ctx.svgTrack);
	}

	// REUSE:
	start = {0,0};

	if (ctx.svgPanelConf.title != "none"){

		TitleCreatorSVG titleCollector;
		drain::TreeUtils::traverse(titleCollector, ctx.svgTrack); // or mainTrack enough?


		// MAIN HEADER(s)
		if (mainGroup.hasChild("metadata") || (ctx.svgPanelConf.title != "false")){ // hmmm

			TreeSVG & headerGroup = ctx.svgTrack["headerGroup"](svg::GROUP);

			TreeSVG & headerRect = headerGroup["headerRect"](svg::RECT);
			//headerRect->setStyle("fill:slateblue");
			headerRect->setId();
			headerRect->setStyle("fill:darkblue");
			headerRect->setStyle("opacity:0.25");
			headerRect->set("x", 0);
			headerRect->set("y", -titleCollector.mainHeaderHeight);
			headerRect->set("width",  mainFrame.getWidth());
			headerRect->set("height", titleCollector.mainHeaderHeight);

			/*  Future extension
			TreeSVG & headerLeft = headerGroup["headerLeft"](svg::TEXT);
			headerLeft->addClass("FLOAT", "LEFT");
			headerLeft->set("ref", headerRect->getId());
			headerLeft->setText("left");

			TreeSVG & headerRight = headerGroup["headerRight"](svg::TEXT);
			headerRight->addClass("FLOAT", "RIGHT");
			headerRight->set("ref", headerRect->getId());
			headerRight->setText("right");
			*/

			TreeSVG & headerText = headerGroup["headerText"](svg::TEXT);
			headerText->addClass("FLOAT", "CENTER", "MIDDLE");
			headerText->set("ref", headerRect->getId());
			headerText->set("x", 51); // will be realigned
			headerText->set("y", 61); // will be realigned
			headerText->setStyle({
				{"font-size", "2.5em"},
				{"stroke", "none"},
				{"fill", "darkblue"}
			});

			// Automatic
			if (ctx.svgPanelConf.title.empty()){
				// TODO: vars aligned by class (time, etc)
				for (const auto & entry: mainGroup["metadata"]->getAttributes()){
					headerText->ctext += entry.second.toStr();
					headerText->ctext += ' ';
				}

			}
			else if (ctx.svgPanelConf.title != "false"){
				drain::StringMapper titleMapper(RackContext::variableMapper);
				titleMapper.parse(ctx.svgPanelConf.title);
				const drain::VariableMap & v = ctx.getStatusMap();
				headerText->ctext += titleMapper.toStr(v);
				headerText->ctext += ' ';
			}
			// else title == "false"

			// TODO: develop
			if (! headerText->ctext.empty() ){
				mainFrame.height +=  titleCollector.mainHeaderHeight; // why +=
				start.y           = -titleCollector.mainHeaderHeight;
			}

			drain::image::TreeUtilsSVG::alignText(headerGroup);

		}

		// IMAGE HEADER and MAIN HEADER positions
		// mout.attention("aligning texts of: ", group -> getTag());
		drain::image::TreeUtilsSVG::alignText(mainGroup);

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

// std::list<std::string> CmdOutputTreeConf::attributeList = {"quantity", "date", "time", "src", "elangle", "task_args", "legend"};
// See drain::TextDecorator::VT100
// std::map<std::string,std::string>


} // namespace rack
