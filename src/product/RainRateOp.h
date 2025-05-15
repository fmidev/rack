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
#ifndef RAINRATEOP_H_
#define RAINRATEOP_H_

//#include <algorithm>

#include "radar/Precipitation.h"

#include "PolarProductOp.h"


namespace rack
{

using namespace drain::image;

class FreezingLevel : public drain::BeanLike {

public:

	FreezingLevel() : BeanLike(__FUNCTION__, "Freezing level modelled simply as its height and thickness."){
		parameters.link("height", height = NAN, "km");
		parameters.link("thickness", thickness = 0.1, "km");
	}

	virtual
	~FreezingLevel(){};

	double height;
	double thickness;

};

/// Computes rain rate from dBZ.
/*!
 *
 *
 *  The lowest data array matching "quantity=DBZ" is applied. Typically it is the lowest sweep in a volume, but the data may also result form a CAPPI or maximum echo product.
 *  The geometry of the result is the same as the input array. Quality field is copied from the input.
 */
class RainRateOp : public PolarProductOp {

public:

	static PrecipZrain precipZrain;
	static PrecipZsnow precipZsnow;
	static PrecipKDP   precipKDP;
	static PrecipKDPZDR precipKDPZDR;
	static PrecipZZDR   precipZZDR;

	static FreezingLevel freezingLevel;

	/// Currently type and gain are not handled due to char<=> int mapping problem.
	/**
	 *  \param freezingLevel - height of undetectValue temperature threshold above sea level.
	 *  \param freezingLevelThickness - width of undetectValue temperature region
	 *  \param type - storage type ('d' = double, 'C'=unsigned char, 'S'=unsigned short)
	 *  \param gain - scaling; 0.01 and 0.0001 may be good for 'C' and 'S', respectively.
	 *
	 * - Marshall & Palmer equation for rain (liquid precipitation):
	 *  \f[
     *      Z = 200 R^{1.6}
     *  \f]
     *  or\f$R=Z/200^{0.625}\f$
     *
	 * - Gunn & Marshall equation for snow:
	 *  \f[
     *      Z = 2000 R^{2.0}
     *  \f]
     *  or\f$R=Z/2000^{0.5}\f$
	 */
	// TODO: freezinglevel as a polar pre-prod.
	RainRateOp(double freezingLevel = 10.0, double freezingLevelThickness = 0.2, double z_ra=200.0, double z_rb=1.6, double z_sa=2000.0, double z_sb=2.0) :
		PolarProductOp(__FUNCTION__, "Estimates precipitation rate [mm/h] from reflectance [dBZ].") {

		dataSelector.setQuantities("^DBZH$");
		dataSelector.setMaxCount(1);

		odim.product = "SURF";
		odim.quantity = "RATE";

		odim.type = "S";
	}


protected:

	virtual
	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;

};





} // ::rack

#endif /*RainRate_H_*/

// Rack
