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
#ifndef ClutterMapOP_H_
#define ClutterMapOP_H_

#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

/// Based on pre-computed clutter map, scales the clutter probability for desired sweeps.
/** Assuming that
 *
 */
class ClutterMapOp: public DetectorOp {

public:

	///	Default constructor.
	/**
	 *  \param decay - percentage of clutter probability remaining for each 1000m lift in altitude.
	 *
	 *  Before applying this operator (by calling processVolume), \c cluttermap has to be read first. In \b rack ,
	 *  this is implemented with AnDReClutterMapRead, "--aClutterMapRead" in command line.
	 *
	 *  This operator is \e universal , it is computed on dBZ but it applies also to str radar parameters measured (VRAD etc)
	 */
	ClutterMapOp(double decay=0.5) : //, const std::string & filename = "") :
		DetectorOp(__FUNCTION__, "Reads a ground clutter map and scales it to sweeps.", ECHO_CLASS_CLUTTER){ // Optional postprocessing: morphological closing.
		dataSelector.path = ".*/data[0-9]+/?$";
		dataSelector.quantity = "";  // or FREQ?
		REQUIRE_STANDARD_DATA = false;
		UNIVERSAL = true;
		dataSelector.count = 1;
		parameters.reference("decay", this->decay = decay);
		// parameters.reference("filename", this->filename, filename);
	};


	double decay;

	void setClutterMap(const std::string & filename) const;

	const Data<PolarSrc> & getClutterMap() const;


	virtual
	void processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dst, DataSet<PolarDst> & aux) const;


	mutable HI5TREE clutterMap;

protected:




};


}

#endif

// Rack
