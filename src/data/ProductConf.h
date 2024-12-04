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
/*
 * ProductOp.h
 *
 *  Created on: 2021/10
 *      Author: mpeura
 */

#ifndef RACK_PRODUCT_CONF
#define RACK_PRODUCT_CONF "ProductConf"

#include <string>

/*
#include <stdexcept>
#include <iostream>
#include <list>
#include <map>
#include <set>
*/

#include "drain/util/EnumFlags.h"
#include "data/DataSelector.h"
#include "data/ODIMPath.h"



namespace rack {


/// Shared properties for meteorological products and Cartesian products (composites and single-radar images)
/**
 *   TODO: separate member object
 */
class ProductConf {

public:

	/// Default constructor.
	inline
	ProductConf(){};

	/// Copy constructor.
	ProductConf(const ProductConf & conf);

	// INPUT:

	// Could be hidden (esp. if some quantity is definite?)
	DataSelector dataSelector;

	// OUTPUT

	/// How the (main) output is encoded
	std::string targetEncoding;

	/// If set, appends outputs in an hdf5 structure instead of overwriting. \see CmdAppend .
	/**
	 *   - empty: overwrite
	 *   - data: write to a new \c data group in \c dataset group with the largest index.
	 *   - data{n}: write to \c data group with index \i n in \c dataset group with the largest index.
	 *   - dataset: write to a new \c dataset
	 *   - dataset{n}: write to \c data1 group in \c dataset group with index \i n .
	 */
	ODIMPathElem appendResults;

	/// Determines if also intermediate results (1) are saved. See --store
	enum OutputDataVerbosity {DEFAULT=0, INTERMEDIATE=1, DEBUG=2, QUALITY=4};
	typedef drain::EnumFlagger<drain::MultiFlagger<OutputDataVerbosity> > OutputFlagger;
	OutputFlagger outputDataVerbosity = DEFAULT;
	//  int outputDataVerbosity = 0; // OLD
	/// Determines if also intermediate results (1) are saved. See --store
	/**
	 *
	 */

	// LegendFlagger legend = LEFT|RIGHT;


};

template <>
const drain::EnumDict<ProductConf::OutputDataVerbosity>::dict_t  drain::EnumDict<ProductConf::OutputDataVerbosity>::dict;

// template <>
// const ProductConf::OutputDataVerbosity drain::EnumDict<ProductConf::OutputDataVerbosity>::defaultValue; //= ProductConf::OutputDataVerbosity::DEFAULT;

}  // namespace rack


#endif /* RACK_PRODUCT_CONF */
