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
#ifndef RACK_RESOURCES_BASE_H_
#define RACK_RESOURCES_BASE_H_

#include <string>

#include "drain/image/Image.h"
//#include "drain/imageops/ImageModifierPack.h"
#include "drain/imageops/PaletteOp.h"
//#include "drain/prog/CommandPack.h"
//#include "drain/prog/CommandRegistry.h" // OLD
//#include "drain/prog/CommandBankUtils.h"     // NEW

//#include "drain/util/Rectangle.h"
//#include "drain/util/RegExp.h"
//#include "drain/util/StatusFlags.h"
//#include "drain/util/Tree.h"
#include "drain/util/Variable.h"
#include "drain/util/Static.h"

#include "data/DataSelector.h"
#include "data/PolarODIM.h"
#include "hi5/Hi5.h"

//#include "radar/Composite.h"
//#include "radar/RadarAccumulator.h"


namespace rack {

typedef std::map<std::string, std::map<unsigned short, drain::image::Palette> >  PaletteMap;


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
	//DataSelector qualitySelector;
	//ODIMPathElemMatcher qualityGroups;
	ODIMPathElem::group_t qualityGroups;

	// Accessed by CmdSampler
	drain::image::ImageSampler imageSampler;


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
		currentImage     = NULL;
		currentGrayImage = NULL;
	}

	void getImageStatus(drain::FlexVariableMap & statusMap) const;


	/// Pointer to the last 2D data (image) processed
	const drain::image::Image *currentImage; // = NULL;

	/// Pointer to the last single-channel 2D data (image) processed
	const drain::image::Image *currentGrayImage; // = NULL;  // data or grayImage


	drain::image::Image grayImage;
	drain::image::Image colorImage;



	drain::image::Palette palette;

	/*
	Palette & getPalette(const std::string & quantity, unsigned short colors){
		PaletteMap & pMap = drain::Static::get<PaletteMap>();

		Palette & p = pMap[quantity][colors];

		if (p.empty()){
			p.load(quantity, true);
		}
		return p;
	}
	*/

	struct {
		drain::Point2D<unsigned short> tilesize = 256;
	} geotiff;
	/// Protected:

	/// Given image pointer (null ok), get properties.
	static
	void getImageInfo(const drain::image::Image *ptr, drain::Variable & entry);



	static std::string outputQuantitySyntax;

//protected:

	void updateImageStatus(drain::VariableMap & statusMap) const ;


};




class AndreContext {
public:

	AndreContext() : andreSelect("path=dataset1:3,elangle=0:2.0"), defaultQuality(0.9) {
		// defaultQuality initialized in respective cmd CmdQuality
	}

	AndreContext(const AndreContext & ctx) : andreSelect(ctx.andreSelect), defaultQuality(ctx.defaultQuality) {
		// defaultQuality initialized in respective cmd CmdQuality
	}

	std::string andreSelect;

	float defaultQuality;
};





class Hdf5Context {

public:

	Hdf5Context();

	Hdf5Context(const Hdf5Context &ctx);

	std::string select;



	/// The last input file read, typically a volume. May be concatenated ie. read in incrementally.
	Hi5Tree polarInputHi5;

	/// The polar product that has been generated from the volume.
	Hi5Tree polarProductHi5;

	/// A single-radar Cartesian product or a multi-radar composite (mosaic).
	Hi5Tree cartesianHi5;

	/// Pointer to the last HDF5 structure read or generated.
	Hi5Tree *currentHi5; // = &inputHi5;


	/// Pointer to the last HDF5 structure in polar coordinates: input volume or polar product.
	/**
	 *   The data pointed to will be applied as input to a Cartesian product (single-radar or composite).
	 */
	Hi5Tree *currentPolarHi5; // = &inputHi5;

	typedef drain::GlobalFlags<Hi5Tree> h5_role;

	static const h5_role::value_t CURRENT; //,    **< Link also external targets *
	static const h5_role::value_t INPUT; // ,      **< No not link, but add entry (void) *
	// static const h5_role::value_t PRODUCT;
	static const h5_role::value_t POLAR; // =4,      **< No action *
	static const h5_role::value_t CARTESIAN; // =8,  **< Throw exception *
	static const h5_role::value_t EMPTY; // =16,     **< Also accept empty  *
	static const h5_role::value_t PRIVATE; // =32,
	static const h5_role::value_t SHARED; // =64     **< Try shared first  *


	/// Pointer to the last HDF5 structure in Cartesian coordinates: input or Cartesian product.
	/**
	 *  - if CARTESIAN and POLAR are both set, either
	 *  - if neither CARTESIAN nor POLAR is set,
	 */
	Hi5Tree & getMyHi5(h5_role::value_t filter=(CARTESIAN|POLAR|INPUT|CURRENT));

	/// Derives the most relevant polar data (input or product) and returns it.
	/**
	 *  Returns in order of priority:
	 *  # local, if non-empty
	 *  # shared, if non-empty
	 *  # local (empty)
	 */
	Hi5Tree & getHi5Defunct(h5_role::value_t filter);

	/// Derives the most relevant polar input data and returns it.
	/**
	 *  Returns in order of priority:
	 *  # local, if non-empty
	 *  # shared, if non-empty
	 *  # local (empty)
	 */
	/*
	inline
	Hi5Tree & getCartesianHi5(h5_role::value_t filter=(PRIVATE|SHARED)){
		return getHi5(filter | CARTESIAN);
	}

	inline
	Hi5Tree & getCurrentInputHi5(h5_role::value_t filter=(PRIVATE|SHARED)){
		return getHi5(filter | CURRENT|INPUT);
	}

	static inline
	Hdf5Context & getStaticContext(){
		return drain::Static::get<Hdf5Context>();
	}
	*/

protected:

	void updateHdf5Status(drain::VariableMap & statusMap); // Not static, because checks

	static Hi5Tree empty;

};


} /* namespace rack */

#endif /* RACKLET_H_ */

// Rack
