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

#ifndef VERTICALProfileOp_H_
#define VERTICALProfileOp_H_

#include <string>

#include "data/Data.h"
#include "data/DataSelector.h"
//#include "data/PolarODIM.h"
#include "data/VerticalODIM.h"
#include "product/VolumeOp.h"


namespace rack {

/// Computes vertical profiles of reflectivity (DBZH), including raw and polarized reflectivity.
/**
 *   The resulting image is oriented "naturally", ie. minHeight and maxHeight mapped to the bottom and top rows, respectively.
 */
class VerticalProfileOp: public VolumeOp<VerticalProfileODIM> {

public:

	// Partially compliant to:
	//  4.3.4 where for vertical profiles

	// Todo: use Range double
	VerticalProfileOp(double minRange=10, double range=100,
			double minHeight=0, double maxHeight=10000,
			long int levels=100,
			double startaz=0.0, double stopaz=0.0, // default: full circle
			long int azSlots=1);

	virtual ~VerticalProfileOp(){};

protected:

	mutable
	double interval;

	void setGeometry(const PolarODIM & srcODIM, PlainData<VprDst> & dstData) const {}; // ???

	//virtual	void setGeometry(Data<VprDst> & dstData) const;

	virtual
	void computeSingleProduct(const DataSetMap<PolarSrc> & srcSweeps, DataSet<VprDst> & dstProduct) const;



};

}

#endif /* VERTICALProfileOp_H_ */

// Rack
