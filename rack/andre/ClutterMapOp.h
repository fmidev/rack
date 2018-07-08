/**

    Copyright 2010-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

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
	 *  Before applying this operator (by calling processVolume), \c cluttermap has to be read first. In \b rack , this is implemented with
	 *  AnDReClutterMapRead, "--aClutterMapRead" in command line.
	 *
	 *  This operator is \e universal , it is computed on DBZ but it applies also to other radar parameters measured (VRAD etc)
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

	/// NEW POLICY
	virtual
	void processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dst, DataSet<PolarDst> & aux) const;


	mutable HI5TREE clutterMap;

protected:




};


}

#endif
