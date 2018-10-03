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


/// Base class for computing products using Doppler speed [VRAD] data.
/**
 *
 *  \see DopplerWindowOp
 */
class DopplerOp : public PolarProductOp {

public:


	/*
	DopplerOp() : PolarProductOp(__FUNCTION__, "Projects Doppler speeds to unit circle. Window corners as (azm,range) or (ray,bin)") {
		parameters.append(w.getParameters());
		dataSelector.quantity = "^(VRAD|VRADH)$";
		dataSelector.count = 1;
	};
	*/

	virtual inline ~DopplerOp(){};

	//mutable PolarSector w;

	/// Projects wind (u,v) to beam direction (rad). Unit (typically m/s) is preserved.
	// raise
	inline
	double project(double azmR, double u, double v) const {
		// double speed = sqrt(u*u + v*v);
		return u*sin(azmR) + v*cos(azmR);
	}


	// Re-alias
	// x - 2*Vm*math.floor((Vm+x)/2.0/Vm)
	inline
	double alias(double v, double vNyq) const {
		return v - (2.0*vNyq)*floor((vNyq + v)/(2.0*vNyq));  // consider vNyq2
	}


protected:

	DopplerOp(const std::string & name, const std::string &description) : PolarProductOp(name, description){
		dataSelector.quantity = "VRAD";
		dataSelector.count = 1;
	}

	//virtual void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const ;


};

// for Testing
class DopplerReprojectOp : public DopplerOp {
public:

	DopplerReprojectOp() : DopplerOp(__FUNCTION__, "Creates virtual ") {

		parameters.reference("nyquist", odim.NI = 100.0, "max-unamb-velocity");
		parameters.reference("match", matchOriginal=0, "flag(aliased=1,nodata=2)"); // ALIASED=1, NODATA=2
		parameters.reference("quantity", odim.quantity = "VRAD", "output-quantity");

		dataSelector.count = 1;
		dataSelector.quantity = "^(AMVU|AMVV|VRAD)$";

		//odim.quantity; // VRAD_C
		//odim.NI;
		odim.type = "S";
	}

	//	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;
	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;



	int matchOriginal;
	//int order;
	//double decay;
	//double smoothNess;

};

}  // namespace rack


#endif /* RACKOP_H_ */

// Rack
