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

// # pragma once
#ifndef RACK_SVG_IMAGEPANEL
#define RACK_SVG_IMAGEPANEL

#include <string>

// #include <drain/RegExp.h>
#include <drain/image/Image.h>
#include <drain/image/LayoutSVG.h>
#include <drain/image/TreeSVG.h>
#include <drain/image/TreeUtilsSVG.h>

#include "graphics-base.h"
#include "graphics-radar.h"

namespace rack {

/// using namespace drain::image;

/// Creator of SVG image elements located in a SVG group with fixed structure.
/**
 *   Preferred way to add images (and related vector overlays.
 *   Creates a structure accessible with methods.
 *
 *   The panel structure consists of a group (svg::GROUP.IMAGE_PANEL) with
 *   - one svg::IMAGE element, optionally removed (changed to DESC)
 *   - an overlay layer svg::GROUP.OVERLAY designed for vector graphics, initially containing a background svg::RECT.BACKGROUND_RECT applied for aligning subsequent elements.
 *   - image border (svg::RECT.IMAGE_BORDER), optionally with a visible outline style (.IMAGE_BORDER), and optionally with a mouse event listener
 *   - an extra overlay (svg::RECT.COVER) masking the image and vector elements
 */
class ImagePanel {


public:

	/// Recommended constructor, if Image is defined: stores also METADATA.
	/**
	 *
	 */
	ImagePanel(drain::image::TreeSVG & imagePanelGroup, const drain::image::Image & src, const drain::FilePath & filePath);

	ImagePanel(drain::image::TreeSVG & imagePanelGroup, const drain::FilePath & filePath, const drain::Frame2D<drain::image::svg::coord_t> & geom = {0,0});

	/// Retrieves the group defined last, or creates...
	ImagePanel(drain::image::TreeSVG & imagePanelGroup, const drain::Frame2D<int> & geom = {0,0}); // drain::image::svg::coord_t



	/// Recommended polymorphic function, if Image is defined: stores also METADATA.
	/**
	 *
	 */
	drain::image::TreeSVG & getImage(const drain::image::Image & src, const drain::FilePath & filepath) const;


	drain::image::TreeSVG & getImage(const drain::FilePath & filePath = drain::FilePath(), const drain::Frame2D<drain::image::svg::coord_t> & geom = {0,0}) const;


	inline
	drain::image::TreeSVG & getMetadata() const {
		return getUniqueElem(imagePanelGroup, svg::METADATA);
	}

	drain::image::TreeSVG& getOverlayGroup() const;


	inline
	drain::image::TreeSVG & getBackGround() const {
		drain::image::TreeSVG & overlay = getOverlayGroup();
		return getUniqueElem(overlay, RackSVG::ElemClass::BACKGROUND_RECT, svg::RECT);
	};


	inline
	drain::image::TreeSVG & getImageBorder() const {
		return getUniqueElem(imagePanelGroup, RackSVG::ElemClass::IMAGE_BORDER, svg::RECT);
	};

	// Not recommended (yet)
	drain::image::TreeSVG & getSourceSpecificGroup(const std::string & source) const;

	/// For vector graphics (grids, sectors)
	/**
	 *   For
	 */
	drain::image::TreeSVG & getVectorOverlayGroup(const std::string & key="", const drain::Frame2D<drain::image::svg::coord_t> & geom = {0,0}) const;

	drain::image::TreeSVG & getMouseListenerFrame() const;

	drain::image::TreeSVG & getDataImage(const drain::FilePath & filepath = drain::FilePath(), const drain::Frame2D<drain::image::svg::coord_t> & geom = {0,0}) const;



protected:

	drain::image::TreeSVG & imagePanelGroup;

	drain::image::TreeSVG & getUniqueElem(drain::image::TreeSVG & parent, RackSVG::ElemClass cls, drain::image::svg::tag_t type = drain::image::svg::GROUP) const;

	drain::image::TreeSVG & getUniqueElem(drain::image::TreeSVG & parent, drain::image::svg::tag_t type = drain::image::svg::GROUP) const;

};

} // rack::


namespace drain {

DRAIN_TYPENAME(rack::ImagePanel);

}  // drain

#endif /* RACK */
