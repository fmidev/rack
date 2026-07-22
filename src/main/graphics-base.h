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

#ifndef RACK_GRAPHICS_BASE_SVG
#define RACK_GRAPHICS_BASE_SVG


#include <drain/Enum.h>
#include <drain/image/TreeSVG.h>


namespace rack {

class ConfSVG;


/// SVG support for creating aligned image panels
class RackSVG {


public:

	//typedef drain::StyleSelectorXML<NodeSVG> Select;

	// Identifier for the anchor background
	// static const std::string BACKGROUND_RECT; //  = "mainRect";

	/**
	 *  \see rack::Interactive::ElemClass
	 */
	enum ElemClass {
		NONE = 0,
		MAIN_TITLE  = 1, // Main title in SVG image
		GROUP_TITLE = 2, // Group title
		IMAGE_TITLE = 4, // Image title: small text (time, location) in corners of radar images
		// Topical
		TIME = 8,        // Date and time attributes
		LOCATION = 16,   // Place (coordinates, municipality)
		GENERAL = 32,    // Default type
		ALL = (63),
		// MAIN,
		IMAGE_PANEL,     // Group containing and image and, potentially, title TEXTs.
		IMAGE_BORDER,    // RECT surrounding the image, potentially also a COORD_TRACKER
		BACKGROUND_RECT, // invisible RECT used for aligning titles.
		SIDE_PANEL,
		BORDER,          // Overall border (RECT) around the SVG document, invisible by default
		// --
		MOUSE,			 // A group marked for mouse event listeners
		// MOUSE_TRACKER,   // Area inside which mouse events will be tracked.
		// MONITOR,         // Display of interactive operations
		SELECTOR,        // Interactive element illustrating a selection by the user
		DATA_ARRAY,      // Image used as data array only, not to be displayed
		// SHARED_METADATA, // Something that should not be repeated in panels.
		// --- unused ? ---
		// TITLE,      // Default title
		OVERLAY,          // Standard subgroup name inside IMAGE_PANEL group.
	};

	static
	void addStyle(drain::image::TreeSVG & style);

	/// returns timeformat
	static
	const std::string & guessFormat(const std::string & key);


	/// Applicable for PNG images.
	/*
	static
	drain::image::TreeSVG & getImagePanelGroup(RackContext & ctx, const drain::FilePath & filepath, bool unique=false);

	static
	drain::image::TreeSVG & getImagePanelGroupPlain(RackContext & ctx);

	/// Applicable for vector graphics.
	static
	drain::image::TreeSVG & getVectorImagePanelGroup(RackContext & ctx);

	static
	drain::image::TreeSVG & getOverlayGroup(TreeSVG & imagePanelGroup);

	static
	drain::image::TreeSVG & getFloatingGroupFOO(TreeSVG & imagePanel);
	// drain::image::TreeSVG & getFloatingGroup(RackContext & ctx);
	*/

	// static
	// drain::image::TreeSVG & getSourceSpecificGroup(RackContext & ctx, drain::image::TreeSVG & panel);

	//static
	// void addImage(RackContext & ctx, const drain::image::Image & src, const drain::FilePath & filepath);

	// static
	// void addImageNEW(RackContext & ctx, const drain::FilePath & filepath, const drain::Frame2D<drain::image::svg::coord_t> & frame);

	/// Currently, uses file link (does not embed)
	// static
	// void addImage(RackContext & ctx, const drain::image::TreeSVG & svg, const drain::FilePath & filepath);

	/// Add external image from a file path.
	/**
	 *   \param styleClass - optional style class, possibly FLOAT
	 */
	// static
	// void addExternalImage(RackContext & ctx, const drain::Frame2D<drain::image::svg::coord_t> & frame, const drain::FilePath & filepath, const std::string & styleClass = "");


	/// Create a title from background rectangle and TEXT elements (MAINTITLE, LOCATION, TIME, GENERAL)
	/**
	 *  \return TEXT element located at child(GENERAL)
	 */
	static
	drain::image::TreeSVG & addTitleBox(const ConfSVG & conf, drain::image::TreeSVG & object, RackSVG::ElemClass elemClass);

	// TODO: title area "filling order", by group class.
	/// Add title elements in given group, to "reserve slots" for actual text content to be added later.

	static
	drain::image::TreeSVG & appendTitleElements(const ConfSVG &conf, drain::image::TreeSVG &group, const std::string &anchor, RackSVG::ElemClass elemClass);

	static inline
	drain::image::TreeSVG & appendTitleElements(const ConfSVG &conf, drain::image::TreeSVG &group, const char * anchor, RackSVG::ElemClass elemClass){
		return appendTitleElements(conf, group, std::string(anchor), elemClass);
	}

	template <class T>
	static inline
	drain::image::TreeSVG & appendTitleElements(const ConfSVG &conf, drain::image::TreeSVG &group, const T & anchorId, RackSVG::ElemClass elemClass){
		//const drain::StringWrapper<T> anchor(anchorId);
		const drain::EnumWrapper<T> anchor(anchorId);
		// const std::string & s = anchor;
		return appendTitleElements(conf, group, (const std::string &)anchor, elemClass);
	}


	/// Add rectangle
	// static
	// drain::image::TreeSVG & addRectangleGroup(RackContext & ctx, const drain::Frame2D<double> & frame = {200,200});


	/// Add
	/**
	 *  \param imagePanel
	 */
	// static
	// drain::image::TreeSVG & getImageBorder(drain::image::TreeSVG & imagePanel); // , const drain::Frame2D<double> & frame = {200,200});


protected:


};


} // rack::


namespace drain {

DRAIN_ENUM_DICT(rack::RackSVG::ElemClass);
DRAIN_ENUM_OSTREAM(rack::RackSVG::ElemClass);

DRAIN_XML_ENUM_KEY(image::TreeSVG, rack::RackSVG::ElemClass);


}
#endif

