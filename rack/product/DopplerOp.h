/*

    Copyright 2001 - 2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
