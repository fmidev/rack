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

#ifndef DOPPLER_OP_H_
#define DOPPLER_OP_H_

#include "PolarProductOp.h"

//#include "radar/Doppler.h"
#include "radar/PolarSector.h"


namespace rack {


/// Base class for computing products from Doppler speed (VRAD) data.
/** Input and output as HDF5 data, which has been converted to internal structure, drain::Tree<NodeH5>.
 *
 *  \see DopplerWindowOp
 */
class DopplerOp : public PolarProductOp {
public:

	DopplerOp(const std::string & name, const std::string &description) : PolarProductOp(name, description){
		dataSelector.quantity = "VRAD";
	}

	DopplerOp() : PolarProductOp("DopplerOp", "Projects Doppler speeds to unit circle. Window corners as (azm,range) or (ray,bin)") {
		parameters.append(w.getParameters());
		/*
		parameters.reference("azm1", w.azm1 = 0.0, "deg");
		parameters.reference("azm2", w.azm2 = 0.0, "deg");
		parameters.reference("range1", w.range1 = 0, "km");
		parameters.reference("range2", w.range2 = 0, "km");
		parameters.reference("ray1", w.ray1 = 0, "index");
		parameters.reference("ray2", w.ray2 = 0, "index");
		parameters.reference("bin1", w.bin1 = 0, "index");
		parameters.reference("bin2", w.bin2 = 0, "index");
		 */
		// reference("draw", this->draw, 1, "0/1");
		dataSelector.quantity = "^(VRAD|VRADH)$";
		dataSelector.count = 1;
	};

	virtual ~DopplerOp();

	mutable PolarSector w;

	//int draw;


protected:

	// Potentially a general utility. rack::Geometry / ODIM?
	//void deriveWindow(const PolarODIM srcOdim, int & ray1, int & ray2, int & bin1, int & bin2) const ;

	//void drawWindow(int ray1, int ray2, int bin1, int bin2, int marker, PlainData<PolarDst> & dstData) const;

	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const ;


};


}  // namespace rack


#endif /* RACKOP_H_ */

// Rack
