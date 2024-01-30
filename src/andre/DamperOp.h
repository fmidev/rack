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
 * DamperOp.h
 *
 *  Created on: Aug 7, 2011
 *      Author: mpeura
 */

#ifndef DamperOP_H_
#define DamperOP_H_

#include <drain/image/ImageFile.h>
#include <limits>

#include "drain/util/Fuzzy.h"
#include "drain/image/Intensity.h"
#include "hi5/Hi5.h"
#include "radar/Geometry.h"

#include "RemoverOp.h"

using namespace drain::image;

namespace rack {

/**  Turns reflectance (DBZH) values of anomalies down smoothly applying fuzzy deletion operator.
 *
 *   TODO: add minus-dbz range down to e.g. -25?
 */
class DamperOp: public RemoverOp {

public:

	/** Default constructor.
	 *  \param threshold - quality values below this will be considered
	 *  \param undetectThreshold - q index below which bins are marked 'undetect'.
	 *  \param minValue - measurement value towards which values are decremented.
	 */
	//DamperOp(double threshold=0.5, double undetectThreshold=0.1, double minVal = -std::numeric_limits<double>::max()) :  // double slope=0.0,
	DamperOp(double threshold=0.5, double undetectThreshold=0.1, double minValue = NAN) :  // double slope=0.0,
		RemoverOp(__FUNCTION__,"Removes anomalies with fuzzy deletion operator."){

		dataSelector.setQuantities("^DBZH$");

		parameters.link("threshold", this->threshold = threshold);
		parameters.link("undetectThreshold", this->undetectThreshold = undetectThreshold);
		parameters.link("minValue", this->minValue = minValue);

	};


	double threshold;
	double undetectThreshold;
	double minValue;

protected:


	// Main method. Like RemoverOp. remains dstQuality intact.
	/**
	 *
	 *
	 *   ? Practically, qualityRoot == dstDataRoot
	 */
	void processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
							PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstQuality) const;


};


}

#endif /* ERASEROP_H_ */

// Rack
