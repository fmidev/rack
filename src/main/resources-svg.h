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
#ifndef RACK_RESOURCES_SVG
#define RACK_RESOURCES_SVG

#include <string>

// #include <drain/RegExp.h>
#include <drain/image/Image.h>
#include <drain/image/LayoutSVG.h>
#include <drain/image/TreeSVG.h>
#include <drain/image/TreeUtilsSVG.h>


namespace rack {


struct ConfSVG {

	typedef drain::EnumFlagger<drain::MultiFlagger<drain::image::FileSVG::IncludePolicy> > IncludeFlagger;

	IncludeFlagger svgIncludes;

	/// SVG file may contain several "modules", for example rows or columns of IMAGE:s. This is the name of the current module, contained in a GROUP.
	typedef drain::EnumFlagger<drain::MultiFlagger<drain::image::FileSVG::PathPolicy> > PathPolicyFlagger;

	PathPolicyFlagger pathPolicyFlagger = drain::image::FileSVG::ABSOLUTE;
	std::string pathPolicy = "ABSOLUTE";

	// Could be better here:
	// bool absolutePaths = true;

	std::string mainTitle = "AUTO";   // redesign?
	std::string groupIdentifier = ""; // NEW = "AUTO"; // redesign?
	std::string groupTitle = "AUTO";  // NEW
	// std::string groupTitleFormatted; dynamic, so better store in data-title attribute

	// On hold...
	/*
	typedef drain::EnumFlagger<drain::MultiFlagger<ElemClass> > TitleFlagger;
	TitleFlagger svgImageTitles; //  = ElemClass::TIME|ElemClass::LOCATION|ElemClass::GENERAL;
	*/

	/**
	 *   0 - mainTitle.main
	 *   1 - mainTitle.second and groupTile.main
	 *   2 - groupTitle.second
	 *   3 - imageTitle
	 */
	drain::UniTuple<double,3>  fontSizes = {15.0, 12.0, 10.0};

	/**
	 *   0 - mainTitle
	 *   1 - groupTitle
	 */
	drain::UniTuple<double,3>  boxHeights = {30.0, 25.0, 15.0};

	int debug = 0;

	inline  // maxPerGroup(10), layout(Alignment::HORZ, LayoutSVG::INCR), legend(LEFT, EMBED),
	ConfSVG() : svgIncludes(drain::image::FileSVG::IncludePolicy::ALL), pathPolicyFlagger(drain::image::FileSVG::PathPolicy::ABSOLUTE), pathPolicy("ABSOLUTE") { // absolutePaths(true){
	}


};


/*
/// SVG support for creating aligned image panels
class RackSVG {


public:

	//typedef drain::StyleSelectorXML<NodeSVG> Select;

	// Identifier for the anchor background
	// static const std::string BACKGROUND_RECT; //  = "mainRect";

	// \see rack::Interactive::ElemClass

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



	/// returns timeformat
	static
	const std::string & guessFormat(const std::string & key);



	/// Create a title from background rectangle and TEXT elements (MAINTITLE, LOCATION, TIME, GENERAL)
	//   \return TEXT element located at child(GENERAL)

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



protected:


};

*/


using namespace drain::image;

class GraphicsContext { // : public drain::BasicCommand {

	//

public:


	/// Default constructor
	GraphicsContext();

	/// Copy constructor
	GraphicsContext(const GraphicsContext & ctx);

	/// Destructor
	inline virtual
	~GraphicsContext(){};

	drain::image::TreeSVG svgTrack;

	// SVG output configuration (layout)
	ConfSVG svgPanelConf; // under constr

	AlignBase::Axis mainOrientation = AlignBase::Axis::HORZ;
	//LayoutSVG::Direction mainDirection = LayoutSVG::Direction::INCR;
	LayoutSVG::DirectionHorz mainDirectionHorz = LayoutSVG::DirectionHorz::RIGHT;
	LayoutSVG::DirectionVert mainDirectionVert = LayoutSVG::DirectionVert::DOWN;

	// Here AlignSVG::HorzAlign and AlignSVG::VertAlign unused, as they contain no Topol(ogy).
	CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::HORZ> alignHorz = {MutualAlign::Topol::INSIDE, AlignBase::Pos::UNDEFINED_POS};
	CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::VERT> alignVert = {MutualAlign::Topol::INSIDE, AlignBase::Pos::UNDEFINED_POS};

	AlignAnchorSVG::anchor_t anchorHorz;
	AlignAnchorSVG::anchor_t anchorVert;

	//int svgDebug = 0;
	std::string currentImagePanel;


	/// Some SVG style classes. Identifiers for IMAGE and RECT elements over which TEXT elements will be aligned
	/**
	 *  Initialize styles, if undone.
	 */
	inline
	drain::image::TreeSVG & getStyle(){
		return svgTrack[drain::image::svg::STYLE](drain::image::svg::STYLE);
	};


	drain::image::TreeSVG & getOnLoadScript();

	/**
	 * 	  // static
		     Rack = {
		     	 cls = {},

		     }
		     Rack.cls.SELECTOR = 'SELECTOR';
	 */
	drain::image::TreeSVG & getJavaScriptDefs(); //, const std::string & section="cls");

	/// key => Rack.cls.key = 'key';
	/**
	 *   TODO template
	 */
	drain::image::TreeSVG & addJavaScripsDef(const std::string & key, const std::string & section="cls");
	/// Some SVG style classes. Identifiers for IMAGE and RECT elements over which TEXT elements will be aligned





	// TODO: Move to graphicsCtx
	/**
	 *   \return \c true , if any alignment setting was given.
	 */
	bool consumeAlignRequest(drain::image::NodeSVG & node);

	// ?? TODO: Move to graphicsCtx?
	bool applyInclusion(const drain::FilePath & filepath); // , IncludePolicy fileFormat);

	/** Intermediate group "hiding" translation that moves upper left corner of the object to the origin.
	 *
	 */
	static
	drain::image::TreeSVG & ensureAdapterGroup(drain::image::TreeSVG & group);


	drain::image::TreeSVG & getCurrentAdapterGroup();

	/// Applicable for PNG images.
	drain::image::TreeSVG & getImagePanelGroup(const drain::FilePath & filepath, bool unique=false);

	/// Returns the last applicable
	drain::image::TreeSVG & getImagePanelGroup();

	drain::image::TreeSVG & getImagePanelGroup(const drain::Frame2D<int> & frame);

	/// Top-level GROUP used by Rack. All the graphic elements will be created inside this element.
	/**
	 *
	 *
	 */
	//static
	drain::image::TreeSVG & getMainGroup();

protected:

	//static
	virtual
	drain::image::TreeSVG & getStackedGroup() = 0;


};





} // rack::

#endif /* RACK */
