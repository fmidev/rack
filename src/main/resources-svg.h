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

	typedef drain::UniTuple<double,3> sizeConf;

	/**
	 *   0 - mainTitle.main
	 *   1 - mainTitle.second and groupTile.main
	 *   2 - groupTitle.second
	 *   3 - imageTitle
	 */
	sizeConf fontSizes = {15.0, 12.0, 10.0};

	/**
	 *   0 - mainTitle
	 *   1 - groupTitle
	 */
	sizeConf boxHeights = {30.0, 25.0, 15.0};

	// int debug = 0;

	inline  // maxPerGroup(10), layout(Alignment::HORZ, LayoutSVG::INCR), legend(LEFT, EMBED),
	ConfSVG() : svgIncludes(drain::image::FileSVG::IncludePolicy::ALL), pathPolicyFlagger(drain::image::FileSVG::PathPolicy::ABSOLUTE), pathPolicy("ABSOLUTE") { // absolutePaths(true){
	}


};




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

	// To be protected:
	drain::image::TreeSVG svgTrack;

	inline
	drain::image::TreeSVG & getSVG(){
		return svgTrack;
	}

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
