/**

    Copyright 2014-   Markus Peura & Joonas Karjalainen  Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef BIRD_H_
#define BIRD_H_

//#include "PolarProductOp.h"
#include <drain/imageops/ImageOp.h>
#include "DetectorOp.h"

using namespace drain::image;

namespace rack {

///
/**

 *
 */
class BirdOp: public DetectorOp {

public:

	/**
	 *
	 *  \param dbzPeak
	 *  \param vradDevMin
	 *  \param rhoHVmax
	 *  \param zdrDevMin
	 *  \param windowWidth
	 *  \param windowHeight
	 *
	 */
	BirdOp(double dbzPeak = -5.0, double vradDevMin = 5.0, double rhoHVmax = 0.7, double zdrDevMin = 2.0, double windowWidth = 2500, double windowHeight = 5.0) :

		DetectorOp(__FUNCTION__, "Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.", ECHO_CLASS_BIRD){ // Optional postprocessing: morphological closing.

		init(dbzPeak, vradDevMin, rhoHVmax, zdrDevMin, windowWidth, windowHeight);

	};

	virtual
	inline
	~BirdOp(){};


	double dbzPeak;
	double vradDevMin;
	//double wradMin;
	double rhoHVmax;
	double zdrDevMin;


	double windowWidth;
	double windowHeight;


	virtual
	void processDataSet(const DataSet<PolarSrc> & src, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & dstAux) const;


protected:

	BirdOp(const std::string & name, const std::string & description, unsigned int code) : DetectorOp(name, description, code) {
	};

	/// Inits common for BIRD and INSECT
	// kludge
	void init(double dbzPeak = -5.0, double vradDevMin = 5.0, double rhoHVmax = 0.7, double zdrDevMin = 2.0, double windowWidth = 2500, double windowHeight = 5.0);

	/// Convenience function for "accumulating" detection results.
	/**
	 *   \param tmp - image for latest result, in a sequence of operations
	 *   \param dstData - actual result
	 *   \param dstProductAux -
	 */
	void applyOperator(const ImageOp & op, Image & tmp, const std::string & feature, const Data<PolarSrc> & src, PlainData<PolarDst> & dstData, DataSet<PolarDst> & dstProductAux) const;


};

class InsectOp: public BirdOp {

public:

	// BIRD: double dbzPeak = -5.0, double vradDevMin = 5.0, double rhoHVmax = 0.7, double zdrDevMin = 2.0, double windowWidth = 2500, double windowHeight = 5.0

	InsectOp(double dbzPeak = -10.0, double vradDevMin = -5.0, double rhoHVmax = 0.7, double zdrDevMin = 3.0, double windowWidth = 2500, double windowHeight = 5.0) :
			BirdOp("Insect", "Estimates probability from DBZH, VRAD, RhoHV and ZDR.", ECHO_CLASS_INSECT){
		init(dbzPeak, vradDevMin, rhoHVmax, zdrDevMin, windowWidth, windowHeight);
	};

};


}

#endif
