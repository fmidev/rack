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
#ifndef RACK_RESOURCES_H_
#define RACK_RESOURCES_H_

#include <string>

#include "drain/image/Image.h"
#include "drain/imageops/ImageModifierPack.h"
#include "drain/imageops/PaletteOp.h"
#include "drain/prog/CommandBankUtils.h"
#include "drain/util/Rectangle.h"
#include "drain/util/RegExp.h"
#include "drain/util/StatusFlags.h"
#include "drain/util/Tree.h"
#include "drain/util/Variable.h"

#include "hi5/Hi5.h"
#include "data/DataSelector.h"
#include "data/PolarODIM.h"
#include "data/ProductConf.h"
#include "radar/Composite.h"
#include "radar/RadarAccumulator.h"

#include "resources-base.h"

namespace rack {



/// Resources provided separately for each thread.
/**
 *
 */
class RackContext : public ProductConf, public drain::SmartContext, public ImageContext, public AndreContext, public Hdf5Context {

public:



	/// Initializes data and image pointers to default targets or NULL.
	RackContext();

	/// Essential in cloning base context for threads (parallel execution of scripts).
	RackContext(const RackContext & ctx);

	virtual
	~RackContext(){};


	static
	RackContext & getSharedContext(){
		return drain::SuperContextual<RackContext>::baseCtx();
	}


	static
	const drain::StringMapper variableMapper;

	//static
	//const drain::RegExp variableMapperSyntax;


	/// Path prefix for input files. Move to resources?
	std::string inputPrefix;

	/// Path prefix for output files. Move to resources?
	std::string outputPrefix;


	/// TODO: inherit from ProductBase
	/*
	std::string targetEncoding; // consider encodingRequest !
	ODIMPathElem appendResults;
	/// Determines if also intermediate results (1) are saved. See --aStore
	int outputDataVerbosity;
	*/

	// Accumulator for data in Cartesian coordinates
	Composite composite;

	// Experimental
	bool guessDatasetGroup(const Hi5Tree & src, ODIMPathElem & pathElem) const;

	bool guessDatasetGroup(ODIMPathElem & pathElem){
		Hi5Tree & src = getHi5(CURRENT);
		return guessDatasetGroup(src, pathElem);
	}

	// Experimental
	// ODIMPath currentPath;

	/// Uses image selector to find 2D data in the curneent HDF5 structure
	//static
	ODIMPath findImage(const DataSelector & imageSelector);

	// static
	ODIMPath findImage();

	/// Light and lazy
	// sstatic
	const drain::image::Image & getCurrentGrayImage(); //RackContext & ctx);

	/// Light and lazy
	const drain::image::Image & getCurrentImage(); //RackContext & ctx);


	/// Ensures current image
	/**
	 *  # selection: find a new image, if requested.
	 *  # encoding: convert if needed	 *
	 */
	const drain::image::Image & updateCurrentImage(); //RackContext & ctx);

	/// Return current image, if modifiable. Else, copy (convert).
	drain::image::Image &  getModifiableImage(); // (RackContext & ctx); // const DataSelector & selector

	void convertGrayImage(const drain::image::Image & srcImage); // RackContext & ctx,

	virtual inline
	drain::VariableMap & getStatusMap(){
		Context::updateStatus();
		updateStatus();
		return statusMap;
	};

	// Story
	drain::image::TreeSVG svg;

//private:
protected:

	// Keep this class-specific.
	void updateStatus();


};

/// A container for shared resources applied by RackLets.
/**
 *
 */
class RackResources : public drain::SuperContextual<RackContext> { // ,drain::SmartContext : public RackContext {

public:

	RackResources(); // : inputOk(true), dataOk(true), currentHi5(&inputHi5), currentPolarHi5(&inputHi5), currentImage(NULL), currentGrayImage(NULL) {};


	/// Clears dst if source command varies. Keep for later reference...
	// void setSource(Hi5Tree & dst, const drain::Command & cmd);


	/// Standard (?) orientation of polar coords in radar data
	static
	const CoordinatePolicy polarLeft;

	/// Default coordinate policy; no wrapping or mirroring at edges.
	static
	const CoordinatePolicy limit;

	//static 	void updateCoordinatePolicy(Hi5Tree & src, const CoordinatePolicy & policy = CoordinatePolicy(CoordinatePolicy::LIMIT));



	// Accumulator for data in polar coordinates
	RadarAccumulator<Accumulator,PolarODIM> polarAccumulator;




};

/// Default instance
RackResources & getResources();




} /* namespace rack */

#endif /* RACKLET_H_ */

// Rack
