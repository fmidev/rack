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
#ifndef RACK_RESOURCES_IMG
#define RACK_RESOURCES_IMG

#include <string>

#include <drain/RegExp.h>
#include <drain/image/Image.h>
#include <drain/image/LayoutSVG.h>
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




// using namespace drain::image;



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
	void setCurrentImageColor(const drain::image::Image & src){
		currentImage     = &src;
	}

	inline
	void setCurrentImages(const drain::image::Image & src){
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
	int paletteResolution = 0;

	drain::image::Palette & getPalette();

	drain::image::Palette & getPalette(const std::string & key);

	void refinePalette(drain::image::Palette & palette);


	// New, returns also the actual key.
	//PaletteMap::iterator it getPaletteEntry(const std::string & key);

	/// Given image pointer (null ok), get properties.
	static
	void getImageInfo(const drain::image::Image *ptr, drain::Variable & entry);



	static
	std::string outputQuantitySyntax;

//protected:

	void updateImageStatus(drain::VariableMap & statusMap) const ;


};


}

#endif /* RACKLET_H_ */

// Rack
