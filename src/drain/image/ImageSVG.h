/*

MIT License

Copyright (c) 2023 FMI Open Development / Markus Peura, first.last@fmi.fi

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
/*
 * TreeSVG.h
 *
 *  Created on: Nov, 2024
 *      Author: mpeura
 */

#ifndef DRAIN_IMAGE_SVG
#define DRAIN_IMAGE_SVG

#include <string>

#include "Image.h"
#include "TreeSVG.h"
#include "TreeUtilsSVG.h" // FloaterSVG::

namespace drain {

namespace image {

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

	/*
	IMAGE_PANEL,     // Group containing and image and, potentially, title TEXTs.
	IMAGE_BORDER,    // RECT surrounding the image, potentially also a COORD_TRACKER
	BACKGROUND,
	OVERLAY,          // Standard subgroup name inside IMAGE_PANEL group.
	*/

	/// Group containing and image and, potentially, title TEXTs.
	static
	const ClassXML IMAGE_PANEL;

	/// RECT, by default invisible, used for aligning other objects, like titles.
	static
	const ClassXML BACKGROUND;

	/// Group for vector graphics
	static
	const ClassXML OVERLAY;

	// Interactive element illustrating a selection by the user. May also be a GROUP.
	static
	const ClassXML SELECTOR;

	/// Optional image serving as a data block.
	/**
	 *  \see MouseXML
	 */
	static
	const ClassXML DATA_ARRAY;

	/// Topmost layer, optionally with a visible border.
	/**
	 *   Optionally, this layer can be attached mouse event listener(s).
	 *   \see MouseXML
	 */
	static
	const ClassXML IMAGE_BORDER;


	/// Recommended polymorphic function, if Image is defined: stores also METADATA.
	/**
	 *
	 */
	drain::image::TreeSVG & getImageNew(const drain::image::Image & src, const drain::FilePath & filepath) const;


	drain::image::TreeSVG & getImage(const drain::FilePath & filePath = drain::FilePath(), const drain::Frame2D<drain::image::svg::coord_t> & geom = {0,0}) const;


	inline
	drain::image::TreeSVG & getMetadata() const {
		return getUniqueElem(imagePanelGroup, svg::METADATA);
	}

	drain::image::TreeSVG& getOverlayGroup() const;

	inline
	drain::image::TreeSVG& getBackGround() const {
		drain::image::TreeSVG & overlay = getOverlayGroup();
		return getUniqueElem(overlay, BACKGROUND, svg::RECT);
		// return getUniqueElem(imagePanelGroup, RackSVG::ElemClass::BACKGROUND, svg::RECT);
	};


	inline
	drain::image::TreeSVG & getImageBorder() const {
		drain::image::TreeSVG & imageBorder = getUniqueElem(imagePanelGroup, IMAGE_BORDER, svg::RECT);
		imageBorder->addClass(FloaterSVG::FLOATING);
		return imageBorder;
		//return getUniqueElem(imagePanelGroup, RackSVG::ElemClass::IMAGE_BORDER, svg::RECT);
	};

	// Not recommended (yet)
	// drain::image::TreeSVG & getSourceSpecificGroup(const std::string & source) const;

	/// For vector graphics (grids, sectors)
	/**
	 *   For
	 */
	drain::image::TreeSVG & getVectorOverlayGroup(const std::string & key="", const drain::Frame2D<drain::image::svg::coord_t> & geom = {0,0}) const;

	drain::image::TreeSVG & getMouseListenerElem() const;

	drain::image::TreeSVG & getDataImage(const drain::FilePath & filepath = drain::FilePath(), const drain::Frame2D<drain::image::svg::coord_t> & geom = {0,0}) const;

	/// Return a description element (of the image).
	inline
	drain::image::TreeSVG & getDescription(){
		return imagePanelGroup[svg::IMAGE][svg::DESC](svg::DESC);
	}

	/// Return a SELECTOR element or group.
	/**
	 *  Notice: the element type is not set by this function.
	 */
	inline
	drain::image::TreeSVG & getSelectionVisualisation(){
		drain::image::TreeSVG & select = getOverlayGroup()[SELECTOR]; // (svg::RECT);
		select->addClass(SELECTOR);
		return select;
	}


protected:

	drain::image::TreeSVG & imagePanelGroup;

	drain::image::TreeSVG & getUniqueElem(drain::image::TreeSVG & parent, const drain::ClassXML & cls, drain::image::svg::tag_t type = drain::image::svg::GROUP) const;

	drain::image::TreeSVG & getUniqueElem(drain::image::TreeSVG & parent, drain::image::svg::tag_t type = drain::image::svg::GROUP) const;

};


} // image::

DRAIN_TYPENAME(image::ImagePanel);

}  // drain::



#endif // DRAIN_IMAGE_SVG

