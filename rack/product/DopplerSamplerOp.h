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

#ifndef DOPPLERSAMPLER_OP_H_
#define DOPPLERSAMPLER_OP_H_

#include "DopplerOp.h"

//#include "radar/Doppler.h"
#include "radar/PolarSector.h"


namespace rack {


/// Base class for computing products using Doppler speed [VRAD] data.
/**
 *
 *  \see DopplerWindowOp
 */
class DopplerSamplerOp : public DopplerOp {
public:


	DopplerSamplerOp() : DopplerOp(__FUNCTION__, "Projects Doppler speeds to unit circle. Window corners as (azm,range) or (ray,bin)") {
		parameters.append(w.getParameters());
		dataSelector.quantity = "^(VRAD|VRADH)$";
		dataSelector.count = 1;
	};

	virtual inline ~DopplerSamplerOp(){};

	mutable PolarSector w;


protected:

	DopplerSamplerOp(const std::string & name, const std::string &description) : DopplerOp(name, description){
		dataSelector.quantity = "VRAD";
		dataSelector.count = 1;
	}

	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const ;


};

class DopplerDiffPlotterOp : public DopplerSamplerOp {  // DopplerWindow unused!

public:


	DopplerDiffPlotterOp() : DopplerSamplerOp(__FUNCTION__, "Plots differences in VRAD data as fucntion of azimuth") {
		parameters.append(w.getParameters());
		dataSelector.count = 1;
	}

	// Outputs u and v both, so dst dataSET needed
	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const ;

};



}  // namespace rack


#endif

