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
 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */

#ifndef VOLTRAVOP_H_
#define VOLTRAVOP_H_

#include "PolarProductOp.h"


namespace rack {

using namespace drain::image;

/// Class designed for "sequential" "parallel" processing of Sweeps.
/**
 *  Basically, there are two kinds of polar processing
 *  - Cumulative: the volume is traversed, each sweep contributing to a single accumulation array, out of which the product layer(s) is extracted.
 *  - Sequential: each sweep generates new layer (/dataset) in the product; typically, the lowest only is applied.
 *
 */
class VolumeTraversalOp : public PolarProductOp {

public:

	VolumeTraversalOp(const std::string & name, const std::string &description="") : PolarProductOp(name, description){
	};

	~VolumeTraversalOp(){};

	// For AnDRe, only as quality/ but not as a product!
	virtual
	void traverseVolume(const Hi5Tree &src, Hi5Tree &dst) const;


	// Shadowing, because template hereby "specialized" ie limited to (PolarSrc + PolarDst)?
	virtual
	void computeProducts(const DataSetMap<PolarSrc> & srcVolume, DataSetMap<PolarDst> & dstVolume) const;


	// Already in PolarProductOp.h
	/**
	virtual	inline
	void setGeometry(const PolarODIM & srcODIM, PlainData<PolarDst> & dstData) const {
		copyPolarGeometry(srcODIM, dstData);
	}
	*/


protected:

	// Routine for both product and AnDRe param selection
	void collect(const Hi5Tree &src, Hi5Tree &dst) const;


};


}  // namespace rack


#endif /* RACKOP_H_ */

// Rack
