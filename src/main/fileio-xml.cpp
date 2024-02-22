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

#include "drain/image/FilePng.h"
#include "drain/image/TreeUtilsSVG.h"

#include "fileio-xml.h"  // ImageSection

namespace rack {


// NOTE: Part of SVG processing is  CmdOutputConf
/*
 * 		svgConf.link("group",       TreeUtilsSVG::defaultGroupName);
		svgConf.link("orientation", svgConf_Orientation, drain::sprinter(TreeUtilsSVG::defaultOrientation.getDict().getKeys()).str());
		svgConf.link("direction",   svgConf_Direction,   drain::sprinter(TreeUtilsSVG::defaultDirection.getDict().getKeys()).str());
 *
 */


void CmdBaseSVG::addImage(RackContext & ctx, const drain::image::Image & src, const drain::FilePath & filepath){ // what about prefix?

	drain::image::TreeSVG & track = ctx.svgTrack["outputs"](NodeSVG::GROUP);

	// Ensure (repeatedly)
	/*
	drain::image::TreeSVG & mainHeader = track["mainHeader"](NodeSVG::RECT);
	mainHeader->setStyle("fill:beige;opacity:0.5");
	mainHeader->set("width", "200");
	mainHeader->set("height", "30");
	*/
	//mainHeader->addClass("FLOAT");


	drain::image::TreeSVG & group = track[ctx.svgGroup](NodeSVG::GROUP);
	// Ensure (repeatedly)
	group->addClass("imageSet");
	group->set("id", ctx.svgGroup);



	// group = filepath.basename + "Group";

	drain::image::TreeSVG & image = group[filepath.basename](NodeSVG::IMAGE); // +EXT!
	image->set("width", src.getWidth());
	image->set("height", src.getHeight());
	image->set("xlink:href", filepath);
	image["basename"](drain::image::svg::TITLE) = filepath.basename;

	// Metadata:
	for (const auto key: {"what:site", "what:object", "what:date", "what:time", "where:lat", "where:lon", "where:elangle", "where:elangles"}){
		if (src.properties.hasKey(key)){
			drain::image::TreeSVG & metadata = image["metadata"](NodeSVG::METADATA);
			metadata->set(key, src.properties.get(key, ""));
			/*
			drain::image::TreeSVG & metadata = image[key](NodeSVG::METADATA);
			metadata->set("key", key);
			metadata = src.properties.get(key, "");
			*/
		}
	}

	TreeSVG & titleBox = group[filepath.basename+"-title"](NodeSVG::RECT);
	titleBox->addClass("FLOAT");
	titleBox->set("width", src.getWidth()/4);
	titleBox->set("height", 30);
	titleBox->setStyle("fill:darkblue; opacity:0.5;");


	// Ensure (repeatedly). This will be on top of the FIRST image of row/column?
	/*
	drain::image::TreeSVG & imageHeader = group["imageHeader"](NodeSVG::RECT);
	imageHeader->addClass("FLOAT");
	imageHeader->setStyle("fill:pink;opacity:0.75");
	imageHeader->set("width", "200");
	imageHeader->set("height", "10");
	*/
}


void CmdBaseSVG::addImage(RackContext & ctx, const drain::image::TreeSVG & svg, const drain::FilePath & filepath){ // what about prefix?

	drain::image::TreeSVG & track = ctx.svgTrack["outputs"](NodeSVG::GROUP);

	drain::image::TreeSVG & group = track[ctx.svgGroup](NodeSVG::GROUP);
	group->set("id", ctx.svgGroup);
	group->set("class", "imageSet");

	drain::image::TreeSVG & image = group[filepath.basename](NodeSVG::IMAGE); // +EXT!
	image->addClass("float legend");
	image->set("width", svg->get("width", 0));
	image->set("height", svg->get("height", 0));
	image->set("xlink:href", filepath);
	image["basename"](drain::image::svg::TITLE) = filepath.basename;

}

// Re-align elements etc
void CmdBaseSVG::completeSVG(RackContext & ctx){

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	drain::image::TreeSVG & track = ctx.svgTrack["outputs"](NodeSVG::GROUP);

	drain::image::NodeSVG::path_list_t pathList;
	drain::image::NodeSVG::findByClass(track, "imageSet", pathList);

	drain::image::TreeUtilsSVG::Orientation orientation = TreeUtilsSVG::defaultOrientation;
	drain::image::TreeUtilsSVG::Direction   direction   = TreeUtilsSVG::defaultDirection;

	int titleBoxHeight = 30;
	drain::Frame2D<int> mainFrame;
	drain::Point2D<int> start(0,0);
	for (drain::image::NodeSVG::path_t & p: pathList){
		mout.debug("aligning: ", p);
		drain::image::TreeSVG & group = track[p](NodeSVG::GROUP);
		drain::Frame2D<int> frame;
		TreeUtilsSVG::determineBBox(group, frame);

		// METADATA -> title
		if (true){
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
			titleText->setStyle("stroke:green");
			titleText = "Radar data";
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

	ctx.svgTrack->set("width",  mainFrame.getWidth());
	ctx.svgTrack->set("height", mainFrame.getHeight());
	ctx.svgTrack->set("viewBox", (const std::string &)drain::StringBuilder<' '>(0, 0, mainFrame.width, mainFrame.height));

}





void CmdBaseSVG::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);


	drain::image::TreeSVG & track = ctx.svgTrack["outputs"]; //["file"];
	ctx.svgTrack["outputs"](NodeSVG::GROUP);
	//track->setType(NodeSVG::GROUP); // ensureType?
	track->set("name", "outputs");


};


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




} // namespace rack
