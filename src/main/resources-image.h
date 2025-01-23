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
#ifndef RACK_RESOURCES_IMG_H_
#define RACK_RESOURCES_IMG_H_

#include <string>

#include <drain/RegExp.h>
#include <drain/image/Image.h>
#include <drain/image/TreeSVG.h>
#include <drain/image/Image.h>
#include <drain/image/TreeUtilsSVG.h>
#include <drain/imageops/PaletteOp.h>

#include <drain/imageops/ImageModifierPack.h>
#include <drain/imageops/PaletteOp.h>
#include <drain/prog/CommandBankUtils.h>
#include <drain/util/Rectangle.h>
#include <drain/util/StatusFlags.h>
// #include <drain/util/TreeOrdered.h>

//#include <drain/util/Variable.h>

// #include "data/ProductConf.h"
#include "data/VariableFormatterODIM.h"

//#include "resources-base.h"


namespace rack {


using namespace drain::image;

/*
class FontSizes : public drain::UniTuple<double,4> {


public:

	typedef drain::UniTuple<double,4> base_t;

	double & titles;
	double & header;
	double & leftHeader;
	double & rightHeader;

	inline
	FontSizes() : base_t(12,10,8,6), titles(next()), header(next()), leftHeader(next()), rightHeader(next()) {
	}

	inline
	FontSizes(const FontSizes & fs) : base_t(fs), titles(next()), header(next()), leftHeader(next()), rightHeader(next()) {
	}

};
*/

struct PanelConfSVG {

	/// SVG file may contain several "modules", for example rows or columns of IMAGE:s. This is the name of the current module, contained in a GROUP.
	bool absolutePaths = true;

	std::string mainTitle = "auto";

	std::string groupNameSyntax = "group";
	std::string groupNameFormatted;

	/**
	 *   0 - mainTitle.main
	 *   1 - mainTitle.second and groupTile.main
	 *   2 - groupTitle.second
	 *   3 - imageTitle
	 */
	drain::UniTuple<double,4>  fontSizes = {15.0, 12.0, 10.0, 8.0};

	/**
	 *   0 - mainTitle
	 *   1 - groupTitle
	 */
	drain::UniTuple<double,2>  boxHeights = {20.0, 15.0};


	// Currently, applications are recommended to handle "false" and "none". Or "auto"?
	// std::string title;
	// FontSizes fontSize;

	inline  // maxPerGroup(10), layout(Alignment::HORZ, LayoutSVG::INCR), legend(LEFT, EMBED),
	PanelConfSVG() : absolutePaths(true){
	}

};

class GraphicsContext { // : public drain::BasicCommand {

public:


	/// Default constructor
	GraphicsContext();

	/// Copy constructor
	GraphicsContext(const GraphicsContext & ctx);

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


	// SVG output configuration (layout)
	TreeSVG svgTrack;
	PanelConfSVG svgPanelConf; // under constr - consider embedding these to PanelConfSVG:
	// std::string svgGroupNameSyntax = "group";
	// std::string svgGroupNameFormatted;

	AlignBase::Axis mainOrientation = AlignBase::Axis::HORZ;
	LayoutSVG::Direction mainDirection = LayoutSVG::Direction::INCR;
	int svgDebug = 0;

	/*
	typedef drain::EnumFlagger<drain::MultiFlagger<ElemClass> > TitleFlagger;
	TitleFlagger svgTitles = ElemClass::NONE;
	*/
	// std::string svgTitles = "";

	// For the NEXT graphic object
	// AlignSVG::HorzAlign
	CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::HORZ> alignHorz = {AlignSVG::Topol::INSIDE}; // (AlignSVG::Topol::INSIDE, AlignBase::Axis::HORZ); // = {AlignSVG::Topol::INSIDE, AlignBase::Axis::HORZ};
	CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::VERT> alignVert = {AlignSVG::Topol::INSIDE, AlignBase::Pos::MIN};

};

//DRAIN_ENUM_OSTREAM(GraphicsContext::ElemClass);

}


namespace drain {

/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const rack::GraphicsContext::ElemClass & key) const;


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass & key);

template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass & key) const ;
*/

}






namespace rack {


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
		currentImage     = NULL;
		currentGrayImage = NULL;
	}

	void getImageStatus(drain::FlexVariableMap & statusMap) const;


	/// Pointer to the last 2D data (image) processed
	const Image *currentImage; // = NULL;

	/// Pointer to the last single-channel 2D data (image) processed
	const Image *currentGrayImage; // = NULL;  // data or grayImage


	Image grayImage;
	Image colorImage;

	std::string paletteKey;


	// New
	inline
	Palette & getPalette(){
		return PaletteOp::getPalette(paletteKey);
	}

	// New
	inline
	Palette & getPalette(const std::string & key){
		paletteKey = key;
		return PaletteOp::getPalette(key);
	}

	/*
	struct {
		drain::Point2D<unsigned short> tilesize = 256;
	} geotiff;
	*/
	/// Protected:

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
