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

#ifndef RACK_RESOURCES_BASE
#define RACK_RESOURCES_BASE

#include <string>

#include <drain/util/Static.h>

#include "data/DataSelector.h"
#include "data/PolarODIM.h"
#include "hi5/Hi5.h"

// #include "graphics.h" // SVG

#include "resources-image.h"


namespace rack {



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
	std::string inputSelect;

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


	enum Hi5Role {
		CURRENT=1,   // Latest HDF5 processed
		INPUT=2,     // Latest input file
		POLAR=4,     // Current Polar volume or product
		CARTESIAN=8, // Accept Cartesian
		PRODUCT=16,  // To select polar product (POLAR,PRODUCT) instead of volume
		EMPTY=32,    // Also accept empty
		PRIVATE=64,  // File owned my this thread only
		SHARED=128,   // File shared by all the threads
	};

	typedef drain::EnumFlagger<drain::MultiFlagger<Hi5Role> > Hi5RoleFlagger;


	/// Pointer to the last HDF5 structure in Cartesian coordinates: input or Cartesian product.
	/**
	 *  - if CARTESIAN and POLAR are both set, either
	 *  - if neither CARTESIAN nor POLAR is set,
	 */
	Hi5Tree & getMyHi5(Hi5RoleFlagger::ivalue_t filter=(CARTESIAN|POLAR|INPUT|CURRENT));


	/// Derives the most relevant polar data (input or product) and returns it.
	/**
	 *  Returns in order of priority:
	 *  # local, if non-empty
	 *  # shared, if non-empty
	 *  # local (empty)
	 */
	Hi5Tree & getHi5Defunct(Hi5RoleFlagger::ivalue_t filter);

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

	void updateHdf5Status(drain::VariableMap & statusMap) const; // Not static, because checks

	static Hi5Tree empty;

};

template <>
const drain::Enum<Hdf5Context::Hi5RoleFlagger>::dict_t drain::Enum<Hdf5Context::Hi5RoleFlagger>::dict;

} /* namespace rack */

#endif /* RACKLET_H_ */

// Rack
