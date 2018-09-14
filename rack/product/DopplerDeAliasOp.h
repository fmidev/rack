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
*//**

    Copyright 2001 - 2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)

    This file is part of AnoRack, a module of Rack for C++.

    AnoRack is not free software.

*/

#ifndef DOPPLER_DEALIAS2_OP_H_
#define DOPPLER_DEALIAS2_OP_H_


#include "DopplerWindowOp.h"

#include <cmath>


namespace rack {


// Consider rename DopplerWind derived classes DopplerVVP, DopplerDeAlias

class DopplerWindOp : public DopplerOp {  // DopplerWindow unused!
public:


	virtual ~DopplerWindOp(){};


	// Outputs u and v both, so dst dataSET needed
	virtual
	void processDataSet(const DataSet<PolarSrc> & srcSweep, DataSet<PolarDst> & dstProduct) const ;

	//@Override
	virtual
	void processData(const Data<PolarSrc> & vradSrc, Data<PolarDst> & dstData) const {
		throw std::runtime_error(name + "::" + __FUNCTION__ + ": not implemented");
	}


	// DopplerDeAliasWindowParameters parameters;
	int widthM;
	double heightD;
	std::string altitudeWeight;  // Functor

	/// Projects wind (u,v) to beam. Unit (typically m/s) is preserved.
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
		return v - (2.0*vNyq)*floor((vNyq + v)/(2.0*vNyq));
		/*
		double n=0;
		if (v >= 0.0){
			v += vNyquist;
			n = floor(v/(2.0*vNyquist));
			return v - n * (2.0*vNyquist) - vNyquist;
		}
		else {
			v -= vNyquist;
			n = floor(-v/(2.0*vNyquist));
			return v + n * (2.0*vNyquist) + vNyquist;
		}
		*/
	}


	double nyquist;

	bool VVP;
	//bool matchAliased;
	int matchOriginal;

protected:

	/// Constructor
	/**
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	DopplerWindOp(const std::string & name, const std::string & description, int widthM = 500, double heightD = 3.0) :
		DopplerOp(name, description) { //, widthM, heightD) {
		parameters.reference("width", this->widthM = widthM, "metres");
		parameters.reference("height", this->heightD = heightD, "degrees");

		dataSelector.count = 1;

		odim.type = "S";
		odim.product = "AMV"; // ?

	};


};


class DopplerDeAliasOp : public DopplerWindOp {  // DopplerWindow unused!
public:

	/// Constructor
	/**
	 *  \param widthM - width of the window, in metres.
	 *  \param heightD - azimuthal width of the window, in degrees.
	 */
	DopplerDeAliasOp(int widthM = 500, double heightD = 3.0, double nyquistVelocity=0.0) :
		DopplerWindOp(__FUNCTION__, "Inverts Doppler speed to (u,v), optionally de-aliases.", widthM, heightD) { //, widthM, heightD) {
		parameters.reference("nyquist", nyquist = nyquistVelocity, "m/s");
		//parameters.reference("VVP", VVP=false, "0|1"); // SLOTS
		parameters.reference("match", matchOriginal=0, "flag(aliased=1,nodata=2)"); // ALIASED=1, NODATA=2
		parameters.reference("altitudeWeight", altitudeWeight, "Functor:a:b:c..."); // ??
		//parameters.reference("testSigns", testSigns = 3, "bits");

		dataSelector.count = 1;

		odim.type = "S";
		odim.product = "AMV";

	};


};

}


#endif /* DOPPLERDEALIASOP_H_ */
