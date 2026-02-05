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



struct ConfSVG {

	typedef drain::EnumFlagger<drain::MultiFlagger<FileSVG::IncludePolicy> > IncludeFlagger;

	IncludeFlagger svgIncludes;

	/// SVG file may contain several "modules", for example rows or columns of IMAGE:s. This is the name of the current module, contained in a GROUP.
	typedef drain::EnumFlagger<drain::MultiFlagger<FileSVG::PathPolicy> > PathPolicyFlagger;

	PathPolicyFlagger pathPolicyFlagger = FileSVG::ABSOLUTE;
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
	ConfSVG() : svgIncludes(FileSVG::IncludePolicy::ALL), pathPolicyFlagger(FileSVG::PathPolicy::ABSOLUTE), pathPolicy("ABSOLUTE") { // absolutePaths(true){
	}


};

using namespace drain::image;

class GraphicsContext { // : public drain::BasicCommand {


public:



	/// Default constructor
	GraphicsContext();

	/// Copy constructor
	GraphicsContext(const GraphicsContext & ctx);

	TreeSVG svgTrack;

	// SVG output configuration (layout)
	ConfSVG svgPanelConf; // under constr

	AlignBase::Axis mainOrientation = AlignBase::Axis::HORZ;
	LayoutSVG::Direction mainDirection = LayoutSVG::Direction::INCR;

	// Here AlignSVG::HorzAlign and AlignSVG::VertAlign unused, as they contain no Topol(ogy).
	CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::HORZ> alignHorz = {AlignSVG::Topol::INSIDE, AlignBase::Pos::UNDEFINED_POS};
	CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::VERT> alignVert = {AlignSVG::Topol::INSIDE, AlignBase::Pos::UNDEFINED_POS};

	AlignAnchorSVG::anchor_t anchorHorz;
	AlignAnchorSVG::anchor_t anchorVert;

	//int svgDebug = 0;

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

	// New, returns also the actual key.
	//PaletteMap::iterator it getPaletteEntry(const std::string & key);

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
