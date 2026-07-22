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

//
#include <drain/image/MouseXML.h>
#include "data/SourceODIM.h"
//#include "resources-svg.h"

#include "graphics-base.h"

using namespace drain;
using namespace drain::image;


DRAIN_ENUM_DICT(rack::RackSVG::ElemClass) = {
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, NONE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MAIN_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MAIN_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, GROUP_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_TITLE),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, LOCATION),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, TIME),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, GENERAL),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_PANEL),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_BORDER),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, BACKGROUND_RECT),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, BORDER),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, SIDE_PANEL),
		// ---
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MOUSE),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MOUSE_TRACKER),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MONITOR),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, SELECTOR),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, DATA_ARRAY),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, SHARED_METADATA),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, OVERLAY),
};

namespace rack {

/**
 *   These mainly for PANELS
 *
 */

void RackSVG::addStyle(drain::image::TreeSVG & style){

	// Consider areas or frames: MAIN_FRAME, GROUP_FRAME, IMAGE_FRAME

	drain::Logger mout(__FILE__, __FUNCTION__);

	//TreeSVG & style = ctx.svgTrack[svg::STYLE]; // convention: main style section in the document
	if (!style->typeIs(svg::STYLE)){
		mout.suspicious("Changing ", style->getTag(), " to ", svg::STYLE);
		style->setType(svg::STYLE);
	}
	//drain::image::TreeSVG & style = root[svg::STYLE](svg::STYLE); // convention: main style section in the document

	using namespace drain;

	//style->setType(svg::STYLE);
	typedef SelectXML<svg::tag_t> Select;

	UtilsXML::ensureStyle(style, svg::TEXT, {
			{"font-family","Helvetica, Arial, sans-serif"},
			{"stroke", "none"},
	});

	UtilsXML::ensureStyle(style, svg::TEXT, {
			{"font-family","Helvetica, Arial, sans-serif"},
			{"stroke", "none"},
	});

	UtilsXML::ensureStyle(style, ClassXML(RackSVG::BACKGROUND_RECT), {
			{"stroke", "none"},
			{"fill", "none"},
	});

	UtilsXML::ensureStyle(style, Select(svg::TEXT, RackSVG::IMAGE_TITLE), {
			{"stroke", "white"},
			{"stroke-opacity", "0.75"},
			{"stroke-width", "0.3em"},
			{"stroke-linejoin", "round"},
			{"fill-opacity", "1"},
			{"paint-order", "stroke"},
	});


	UtilsXML::ensureStyle(style, Select(svg::RECT, RackSVG::GROUP_TITLE), {
			{"fill", "gray"},
			{"stroke", "white"},
			{"stroke-width", "1px"},
			{"opacity", 1.0},
	});

	UtilsXML::ensureStyle(style, Select(svg::TEXT, RackSVG::GROUP_TITLE), {
			// {"font-size", 20}, dynamic, see below
			{"fill", "black"},
			//{"stroke", "white"},
	});

	UtilsXML::ensureStyle(style, Select(svg::RECT, RackSVG::MAIN_TITLE), {
			{"fill", "darkblue"},
			//{"fill", "gray"},
			{"opacity", 1.0},
	});

	UtilsXML::ensureStyle(style, Select(svg::TEXT, RackSVG::MAIN_TITLE), {
			//
			{"fill", "white"},
			// {"stroke", "black"},
	});

	// Currently, image titles have no background RECT, but let's keep this for clarity.
	UtilsXML::ensureStyle(style, Select(svg::TEXT, RackSVG::IMAGE_TITLE), {
			{"font-size", "12px"},
	});

	// Date and time.
	UtilsXML::ensureStyle(style, ClassXML(RackSVG::TIME), {
			{"fill", "darkred"}
	});

	// Radar site
	UtilsXML::ensureStyle(style, ClassXML(RackSVG::LOCATION), {
			{"fill", "darkblue"}
	});

	// Option: set stroke to make borders appear. Future option: borders OUTSIDE the image.
	UtilsXML::ensureStyle(style, ClassXML(RackSVG::IMAGE_BORDER), {
			{"fill", "none"},
			{"stroke", "none"},
			// {"stroke-opacity", 0.0},
			{"stroke-width", 1.0},
			// {"stroke-dasharray", {2,5,3}},
	});

	// Overall image border
	UtilsXML::ensureStyle(style, ClassXML(RackSVG::BORDER), {
			{"fill", "none"},
			{"stroke", "none"},
			// {"stroke-opacity", 0.0},
			{"stroke-width", 1.0},
	});

	// User selection
	UtilsXML::ensureStyle(style, ClassXML(RackSVG::SELECTOR), {
			{"fill", "none"},
			{"stroke", "white"},
			// {"stroke-width", 1.0},
	});

	UtilsXML::ensureStyle(style, Select(svg::RECT, RackSVG::SELECTOR), {
				// {"stroke", "white"},
				{"stroke-width", "2px"},
	});

	UtilsXML::ensureStyle(style, Select(svg::TEXT, RackSVG::SELECTOR), {
			{"font-size", "large"},
			//{"stroke", "none"},
			{"stroke", "black"},
			{"stroke-opacity", "0.5"},
			{"stroke-width", "0.3em"},
			{"stroke-linejoin", "round"},
			{"fill", "white"},
			{"paint-order", "stroke"},
			//{"font-size", "x-large"},
			// {"stroke-width", 1.0},
	});


	UtilsXML::ensureStyle(style, Select(svg::RECT,RackSVG::SIDE_PANEL), { // TODO: add leading '.' ?
			{"fill", "black"},
			{"stroke", "white"},
			{"stroke-width", 2.0},
	});

	UtilsXML::ensureStyle(style, Select(svg::TEXT,RackSVG::SIDE_PANEL), { // TODO: add leading '.' ?
			{"font-size", "12px"},
			{"stroke", "none"},
			{"fill", "white"},
	});

	// }

	mout.revised("Not adjusting font sizes");
	/*
	 *
	mout.debug("Setting font sizes: ", svgPanelConf.fontSizes);
	style[ClassXML(RackSVG::MAIN_TITLE)] ->set("font-size", svgPanelConf.fontSizes[0]);
	style[ClassXML(RackSVG::GROUP_TITLE)]->set("font-size", svgPanelConf.fontSizes[1]);
	style[ClassXML(RackSVG::IMAGE_TITLE)]->set("font-size", svgPanelConf.fontSizes[2]);
	*/

	// Note: these can be accessed via:
	//    style[ClassXML(RackSVG::IMAGE_TITLE)]->get("font-size", 10);

	// return style;

}




} // rack::

