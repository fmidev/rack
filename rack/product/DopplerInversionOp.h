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


#ifndef DOPPLER_Inversion2_OP_H_
#define DOPPLER_Inversion2_OP_H_


#include "DopplerWindowOp.h"

#include <cmath>


namespace rack {


/// Base class for operators retrieving true wind (u,v).
/**
 *
 */
class DopplerWindOp : public DopplerOp {  // DopplerWindow unused!
public:


	virtual ~DopplerWindOp(){};


	// Outputs u and v both, so dst dataSET needed
	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const;

	//@Override
	virtual
	void processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const {
		throw std::runtime_error(name + "::" + __FUNCTION__ + ": not implemented");
	}


	// DopplerInversionWindowParameters parameters;
	int widthM;
	double heightD;
	std::string altitudeWeight;  // Functor



	// double nyquist;
	// int matchOriginal;

	bool VVP;

protected:

	/// Constructor
	/**
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	DopplerWindOp(const std::string & name, const std::string & description, int widthM = 500, double heightD = 3.0) :
		DopplerOp(name, description) { //, widthM, heightD) {
		parameters.link("width", this->widthM = widthM, "metres");
		parameters.link("height", this->heightD = heightD, "degrees");

		dataSelector.count = 1;

		odim.type = "S";
		odim.product = "AMV"; // ?

	};


};

/// Derives two-dimensional  wind (u,v) from Doppler data
/**
 *  Input data may be aliased.
 *
 *  \see DopplerReprojectOp
 */
class DopplerInversionOp : public DopplerWindOp {  // DopplerWindow unused!
public:

	/// Constructor
	/**
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	DopplerInversionOp(int widthM = 500, double heightD = 3.0, double nyquistVelocity=0.0) :
		DopplerWindOp(__FUNCTION__, "Derives 2D wind (u,v) from aliased Doppler data.", widthM, heightD) { //, widthM, heightD) {
		// parameters.link("nyquist", nyquist = nyquistVelocity, "m/s");
		//parameters.link("VVP", VVP=false, "0|1"); // SLOTS
		//parameters.link("match", matchOriginal=0, "flag(aliased=1,nodata=2)"); // ALIASED=1, NODATA=2
		parameters.link("altitudeWeight", altitudeWeight, "Functor:a:b:c..."); // ??
		//parameters.link("testSigns", testSigns = 3, "bits");

		dataSelector.count = 1;

		odim.type = "S";
		odim.product = "AMV";

	};


};






}  // ::rack


#endif /* DOPPLERInversionOP_H_ */
