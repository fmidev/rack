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


#include "resources-image.h"


namespace drain {

template <>
const drain::EnumDict<rack::GraphicsContext::ElemClass>::dict_t  drain::EnumDict<rack::GraphicsContext::ElemClass>::dict = {
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, NONE),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, MAIN),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, MAIN_TITLE),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, GROUP_TITLE),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, IMAGE_TITLE),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, TITLE),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, LOCATION),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, TIME),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, GENERAL),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, IMAGE_PANEL),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, IMAGE_BORDER),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, SHARED_METADATA),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, IMAGE_SET)
};


template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const rack::GraphicsContext::ElemClass & key) const {
	// std::string(".")+
	return hasChild(EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(key, true)); // no error on non-existent dict entry
}


/// Automatic conversion of elem classes to strings.
/**
 *
 */
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass & value) const {
	// std::string(".")+
	return (*this)[EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(value, false)];
}


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass & key){
	// std::string(".")+
	return (*this)[EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(key, false)];
}

/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass &x){
	return (*this)[std::string(".")+EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(x, false)];
}
 */

}



using namespace drain;
using namespace drain::image;

namespace rack {




GraphicsContext::GraphicsContext() {
}

GraphicsContext::GraphicsContext(const GraphicsContext & ctx) {
	svgPanelConf.absolutePaths = ctx.svgPanelConf.absolutePaths;
	svgGroupNameSyntax = ctx.svgGroupNameSyntax;
	svgTitles    = ctx.svgTitles;
}

drain::image::TreeSVG & GraphicsContext::getStyle(){

	// drain::Logger mout(log, __FILE__, __FUNCTION__);

	TreeSVG & style = svgTrack[svg::STYLE];

	//if (!ctx.svgTrack.hasChild("style")){
	if (style->isUndefined()){

		// mout.debug("initializing style");

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

		const Select clsTITLE('.', GraphicsContext::TITLE);
		const Select clsIMAGE_TITLE('.', GraphicsContext::IMAGE_TITLE);
		const Select clsGROUP_TITLE('.', GraphicsContext::GROUP_TITLE);
		const Select clsMAIN_TITLE('.', GraphicsContext::MAIN_TITLE);
		const Select clsTIME('.', GraphicsContext::TIME);
		const Select clsLOCATION('.', GraphicsContext::LOCATION);

		// Text (new)
		// style["text.TITLE"] = {
		style[Select(svg::TEXT, clsTITLE)] = {
				{"font-size", "1.5em"},
				{"stroke", "none"},
				// {"fill", "blue"},
				/* {"fill", "black"}, */
				// {"fill-opacity", "1"},
				// {"paint-order", "stroke"},
				// {"stroke-linejoin", "round"}
		};


		// style["text.IMAGE_TITLE"] = {
		style[Select(svg::TEXT, clsIMAGE_TITLE)] = {
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

		style[clsGROUP_TITLE] = {
				{"font-size", 20},
		};

		style[Select(svg::RECT, clsGROUP_TITLE)] = {
		//style["rect.GROUP_TITLE"] = {
				{"fill", "gray"},
				{"opacity", 0.5},
		};

		style[Select(svg::TEXT, clsGROUP_TITLE)] = {
		// style["text.GROUP_TITLE"] = {
				{"fill", "black"},
		};

		style[clsMAIN_TITLE] = {
				{"font-size", "30"},
		};

		// style["rect.MAIN_TITLE"] = {
		style[Select(svg::RECT, clsMAIN_TITLE)] = {
				{"fill", "darkslateblue"},
				{"opacity", "0.5"},
		};

		// style["text.MAIN_TITLE"] = {
		style[Select(svg::TEXT, clsMAIN_TITLE)] = {
				{"fill", "white"},
		};

		// Date and time.
		style[clsTIME].data = {
				{"fill", "darkred"}
		};

		style[clsLOCATION].data = {
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
// const drain::StatusFlags::value_t RackContext::BBOX_STATUS =   StatusFlags::add("BBOX");










ImageContext::ImageContext(): imagePhysical(true), qualityGroups(ODIMPathElem::NONE),
		currentImage(nullptr), currentGrayImage(nullptr) {
}

ImageContext::ImageContext(const ImageContext & ctx):
		imagePhysical(ctx.imagePhysical),
		qualityGroups(ctx.qualityGroups),
		currentImage(ctx.currentImage),
		currentGrayImage(ctx.currentGrayImage){
}

// Note: for example Palette uses this to extract actual quantity
/**
 *  Note: for example Palette uses this to extract actual quantity
 *
 */
// std::string ImageContext::outputQuantitySyntax("${what:quantity}|${cmdKey}(${cmdArgs})");
std::string ImageContext::outputQuantitySyntax("${what:quantity}/${cmdKey}(${cmdArgs})");



void ImageContext::getImageInfo(const drain::image::Image *ptr, Variable & entry) {
	std::stringstream sstr;
	if (ptr != nullptr){
		ptr->toStream(sstr);
	}
	else {
		sstr << "NULL";
	}
	entry = sstr.str();
}

void ImageContext::updateImageStatus(drain::VariableMap & statusMap) const {

	getImageInfo(&colorImage, statusMap["img:colorImage"]);
	getImageInfo(&grayImage, statusMap["img:grayImage"]);
	getImageInfo(currentImage, statusMap["img:currentImage"]);
	getImageInfo(currentGrayImage, statusMap["img:currentGrayImage"]);

}






} // rack::

