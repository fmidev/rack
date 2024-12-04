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

#include <drain/RegExp.h>
#include <drain/image/Image.h>
#include <drain/imageops/ImageModifierPack.h>
#include <drain/imageops/PaletteOp.h>
#include <drain/prog/CommandBankUtils.h>
#include <drain/util/Rectangle.h>
#include <drain/util/StatusFlags.h>
#include <drain/util/TreeOrdered.h>
//#include <drain/util/Variable.h>

#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"
#include "data/DataSelector.h"
#include "data/PolarODIM.h"
#include "data/ProductConf.h"
#include "data/VariableFormatterODIM.h"
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

	// static
	// const std::string validVariableChars;

	// Consider validVariableRE as general init parameter?
	static
	const drain::StringMapper variableMapper;

	// DataSelector superSelector;

	//static
	//const drain::RegExp variableMapperSyntax;inputFilter
	//drain::SingleFlagger<Reader::Mode>
	// drain::SingleFlagger<hi5::Reader::Mode>
	hi5::Reader::ModeFlagger inputFilter;

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



	/**
	 *  \param filter - flags marking the allowed/desired properties (POLAR, INPUT, ...)
	 *  \return Requested data object, with properties defined by filter.
	 */
	//inline
	//Hi5Tree & getHi5(h5_role::value_t filter);
	//	return getHi5Full(filter);
	// }

	/**
	 *  \param filter - flags marking the allowed/desired properties (POLAR, INPUT, ...) in order of preference
	 *  \return Requested data object, with properties defined by filter.
	 */
	template <typename T, typename ...TT>
	Hi5Tree & getHi5(const T &filter, const TT &... filters){

		T f(filter);
		//Hi5Tree & dst = getHi5Full(f);
		if (f != 0){
			// return dst;
			//Hi5Tree & dst = getHi5Full(f);
			return getHi5Full(f);
		}
		else
			return getHi5(filters...);
	}



	//Hi5Tree & getHi5Full(h5_role::ivalue_t & filter);
	Hi5Tree & getHi5Full(Hi5RoleFlagger::ivalue_t filter);

	inline
	Hi5Tree & getHi5(){
		return empty; // or current?
	}

	/** Get composite, preferring either shared or private (if defined).
	 *
	 *  Both ways, a defined composite is preferred.
	 *
	 *  \return Object containing compositing arrays.
	 */
	Composite & getComposite(Hi5RoleFlagger::ivalue_t filter);


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

	/*
	virtual inline
	drain::VariableMap & getStatusMap(bool update=true){
		if (update){
			Context::updateStatus(); // check if this should be moved under this->updateStatus() ? So this function could be removed.
			updateStatus();
		}
		return statusMap;
	};
	*/

	static
	const VariableFormatterODIM<drain::Variable> variableFormatter;

	static
	const VariableFormatterODIM<drain::FlexibleVariable> flexVariableFormatter;

//private:
protected:

	virtual
	void updateStatus() const final;
	// Keep this class-specific.
	//void updateStatus();

	// typedef drain::Cloner<Context,RackContext> ctx_cloner_t;

	// EXPERIMENTAL
	static inline
	RackContext & baseCtx() {
		//return getContextCloner().getSourceOrig();
		return drain::Static::get<RackContext>();
	}

	/*
	static inline
	ctx_cloner_t & getContextCloner(){
		return getCloner<RackContext>();
	}
	*/

};

/// A container for shared resources applied by RackLets.
/**
 *
 */
class RackResources : public drain::SuperContextual<RackContext> {

public:

	RackResources(); // : inputOk(true), dataOk(true), currentHi5(&inputHi5), currentPolarHi5(&inputHi5), currentImage(NULL), currentGrayImage(NULL) {};

	// Accumulator for data in polar coordinates
	RadarAccumulator<Accumulator,PolarODIM> polarAccumulator;


};

/// Default instance
RackResources & getResources();




} /* namespace rack */

#endif /* RACKLET_H_ */

// Rack
