/*

    Copyright 2001 - 2016  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
/*
 * ProductOp.h
 *
 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */

#ifndef VOLTRAVOP_H_
#define VOLTRAVOP_H_

#include "PolarProductOp.h"


namespace rack {

using namespace drain::image;

/// Base class for radar data processors.

/// Base class for radar data processors.
/** Input and output as HDF5 data, which has been converted to internal structure, drain::Tree<NodeH5>.
 *
 *  Basically, there are two kinds of polar processing
 *  - Cumulative: the volume is traversed, each sweep contributing to a single accumulation array, out of which the product layer(s) is extracted.
 *  - Sequential: each sweep generates new layer (/dataset) in the product; typically, the lowest only is applied.
 *
 *  TODO: Raise to RackOp
 */




///
/**
 *   \tparam M - ODIM type corresponding to products type (polar, vertical)
 */
class VolumeTraversalOp : public PolarProductOp { //VolumeOp<PolarODIM> {

public:

	VolumeTraversalOp(const std::string & name, const std::string &description="") : PolarProductOp(name, description){
	};

	~VolumeTraversalOp(){};

	virtual
	void processVolume(const HI5TREE &src, HI5TREE &dst) const;

	virtual
	void processDataSets(const DataSetMap<PolarSrc> & srcVolume, DataSetMap<PolarDst> & dstVolume) const;

	virtual
	inline
	void setGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		copyPolarGeometry(srcODIM, dstData);
	}

};


}  // namespace rack


#endif /* RACKOP_H_ */
