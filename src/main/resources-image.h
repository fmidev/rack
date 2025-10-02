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
#ifndef RACK_RESOURCES_IMG_H
#define RACK_RESOURCES_IMG_H

#include <string>

#include <drain/RegExp.h>
#include <drain/image/Image.h>
#include <drain/image/TreeSVG.h>
#include <drain/image/TreeUtilsSVG.h>
#include <drain/imageops/PaletteOp.h>
#include <drain/imageops/ImageModifierPack.h>
#include <drain/imageops/PaletteOp.h>
#include <drain/prog/CommandBankUtils.h>
#include <drain/util/Rectangle.h>

#include "data/QuantityMap.h"
#include "data/VariableFormatterODIM.h"


namespace rack {


//using namespace drain::image;

enum SvgInclude {
	NONE = 0,
	PNG = 1,  // Main title in SVG image
	SVG = 2,
	TXT = 4,
	SKIP = 16,
	NEXT = 32,
	ON   = 64,
	OFF  = 128,
	ALL = ON|(PNG|SVG|TXT),
	UNKNOWN = 255,
	// --- unused ? ---
	// TITLE,      // Default title
};


/*
enum TitleClass {
	NONE = 0,
	MAIN = 1,  // Main title in SVG image
	GROUP = 2,
	IMAGE = 4,  // Small title in a corner of radar image (time, location)
	TIME = 8,       // Date and time attributes
	LOCATION = 16,   // Place (coordinates, municipality)
	GENERAL = 32,    // Default type
};
*/


struct PanelConfSVG {

	typedef drain::EnumFlagger<drain::MultiFlagger<SvgInclude> > IncludeFlagger;
	IncludeFlagger svgIncludes;

	enum ElemClass {
		NONE = 0,
		MAIN = 1,  // Main title in SVG image
		GROUP = 2, // Group title
		IMAGE = 4,  // Image title: small text (time, location) in corners of radar images
		TIME = 8,       // Date and time attributes
		LOCATION = 16,   // Place (coordinates, municipality)
		GENERAL = 32,    // Default type
		ALL = (63),
		// MAIN,
		IMAGE_PANEL,
		IMAGE_BORDER, // RECT surrounding the image
		SIDE_PANEL,
		SHARED_METADATA, // Something that should not be repeated in panels.
		// --- unused ? ---
		// TITLE,      // Default title
	};
	typedef drain::EnumFlagger<drain::MultiFlagger<ElemClass> > TitleFlagger;
	TitleFlagger svgImageTitles; //  = ElemClass::TIME|ElemClass::LOCATION|ElemClass::GENERAL;

	std::string imageTitle = "TIME,LOCATION,GENERAL";

	enum PathPolicy {
		ABSOLUTE = 1,  // Else is relative, stripped away using inputPrefix?
		PREFIXED = 2,  // file:// is appended
	};

	/// SVG file may contain several "modules", for example rows or columns of IMAGE:s. This is the name of the current module, contained in a GROUP.
	typedef drain::EnumFlagger<drain::MultiFlagger<PathPolicy> > PathPolicyFlagger;

	PathPolicyFlagger pathPolicyFlagger = ABSOLUTE;
	std::string pathPolicy = "ABSOLUTE";
	// bool absolutePaths = true;

	std::string mainTitle = "AUTO"; // redesign?
	std::string groupTitleSyntax = "AUTO"; // redesign?
	std::string groupTitleFormatted;


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

	inline  // maxPerGroup(10), layout(Alignment::HORZ, LayoutSVG::INCR), legend(LEFT, EMBED),
	PanelConfSVG() : svgIncludes(SvgInclude::ALL), pathPolicyFlagger(PathPolicy::ABSOLUTE), pathPolicy("ABSOLUTE") { // absolutePaths(true){
	}


};

}


namespace drain {

template <>
const drain::EnumDict<rack::SvgInclude>::dict_t  drain::EnumDict<rack::SvgInclude>::dict;
DRAIN_ENUM_OSTREAM(rack::SvgInclude);

template <>
const drain::EnumDict<rack::PanelConfSVG::ElemClass>::dict_t  drain::EnumDict<rack::PanelConfSVG::ElemClass>::dict;
DRAIN_ENUM_OSTREAM(rack::PanelConfSVG::ElemClass);


template <>
const drain::EnumDict<rack::PanelConfSVG::PathPolicy>::dict_t  drain::EnumDict<rack::PanelConfSVG::PathPolicy>::dict;
DRAIN_ENUM_OSTREAM(rack::PanelConfSVG::PathPolicy);


template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const rack::PanelConfSVG::ElemClass & key) const ;


/// Automatic conversion of elem classes to strings.
/**
 *
 */
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const rack::PanelConfSVG::ElemClass & value) const;


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::PanelConfSVG::ElemClass & key);

/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const rack::PanelConfSVG::ElemClass & key) const;


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::PanelConfSVG::ElemClass & key);

template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const rack::PanelConfSVG::ElemClass & key) const;
*/


}

// using namespace drain::image;


namespace rack {

using namespace drain::image;

class GraphicsContext { // : public drain::BasicCommand {


public:



	/// Default constructor
	GraphicsContext();

	/// Copy constructor
	GraphicsContext(const GraphicsContext & ctx);

	TreeSVG svgTrack;

	// SVG output configuration (layout)
	PanelConfSVG svgPanelConf; // under constr

	AlignBase::Axis mainOrientation = AlignBase::Axis::HORZ;
	LayoutSVG::Direction mainDirection = LayoutSVG::Direction::INCR;

	// Here AlignSVG::HorzAlign and AlignSVG::VertAlign unused, as they contain no Topol(ogy).
	CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::HORZ> alignHorz = {AlignSVG::Topol::INSIDE, AlignBase::Pos::UNDEFINED_POS};
	CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::VERT> alignVert = {AlignSVG::Topol::INSIDE, AlignBase::Pos::UNDEFINED_POS};

	AlignAnchorSVG::anchor_t anchorHorz;
	AlignAnchorSVG::anchor_t anchorVert;

	int svgDebug = 0;

	/// Some SVG style classes. Identifiers for IMAGE and RECT elements over which TEXT elements will be aligned
	/**
	 *  Initialize styles, if undone.
	 */
	// drain::image::TreeSVG & getStyle();

	/// Top-level GROUP used by Rack. All the graphic elements will be created inside this element.
	/**
	 *
	 *
	 */
	// drain::image::TreeSVG & getMainGroup();

	//drain::image::TreeSVG & getCurrentAlignedGroup();

	// virtual
	// std::string getFormattedStatus(const std::string & format) const = 0;

	// drain::image::TreeSVG & getCurrentAlignedGroup();

	// drain::image::TreeSVG & getImagePanelGroup(const drain::FilePath & filepath);

	/*
	typedef drain::EnumFlagger<drain::MultiFlagger<ElemClass> > TitleFlagger;
	TitleFlagger svgTitles = ElemClass::NONE;
	*/
	// std::string svgTitles = "";

	/// Apply and reset alignHorz and alignVert.
	// static
	// void applyAlignment(drain::image::TreeSVG & group);

};




// Consider moving ImageKit here?
class ImageContext {
public:


	/// Default constr
	ImageContext();

	/// Copy constr
	ImageContext(const ImageContext & ctx);

	/// Defines if the next image processing operation uses scaled intentsites instead of raw byte values.
	bool imagePhysical;

	/// Defines which quality data, if any, is used as input weight.
	/**
	 *   Practically, only the following properties are actually used:
	 *   - path: dataset, data or both
	 *   - quantity: (typically QIND), future option
	 */
	ODIMPathElem::group_t qualityGroups;

	// Accessed by CmdSampler
	ImageSampler imageSampler;


	inline
	void setCurrentImageColor(const Image & src){
		currentImage     = &src;
	}

	inline
	void setCurrentImages(const Image & src){
		currentImage     = &src;
		currentGrayImage = &src;
	}

	inline
	void unsetCurrentImages(){
		currentImage     = nullptr;
		currentGrayImage = nullptr;
	}

	void getImageStatus(drain::FlexVariableMap & statusMap) const;


	/// Pointer to the last 2D data (image) processed
	const Image *currentImage = nullptr; // = NULL;

	/// Pointer to the last single-channel 2D data (image) processed
	const Image *currentGrayImage = nullptr; // = NULL;  // data or grayImage


	Image grayImage;
	Image colorImage;

	std::string paletteKey;


	inline
	Palette & getPalette(){
		return PaletteOp::getPalette(paletteKey);
	}

	Palette & getPalette(const std::string & key);


	/// Given image pointer (null ok), get properties.
	static
	void getImageInfo(const Image *ptr, drain::Variable & entry);



	static std::string outputQuantitySyntax;

//protected:

	void updateImageStatus(drain::VariableMap & statusMap) const ;


};








} /* namespace rack */

#endif /* RACKLET_H_ */

// Rack
