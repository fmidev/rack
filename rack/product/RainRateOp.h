/*

    Copyright 2013 -    Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack software for C++.

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
		parameters.reference("height", height = NAN, "km");
		parameters.reference("thickness", thickness = 0.1, "km");
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

	static PrecipitationZ precipZrain;
	static PrecipitationZ precipZsnow;
	static PrecipitationKDP precipKDP;
	static PrecipitationKDPZDR precipKDPZDR;
	static PrecipitationZZDR precipZZDR;

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
		//RackOp("RainRate",	"Computes rain rate [mm/h] from reflectance [dBZ].") {
		PolarProductOp(__FUNCTION__,	"Estimates precipitation rate [mm/h] from reflectance [dBZ].") {

		//parameters.reference("freezingLevel", this->freezingLevel = freezingLevel, "km");
		//parameters.reference("freezingLevelThickness", this->freezingLevelThickness = freezingLevelThickness, "km");

		/*
			reference("z_ra", this->z_ra, z_ra);
		reference("z_rb", this->z_rb, z_rb);
		reference("z_sa", this->z_sa, z_sa);
		reference("z_sb", this->z_sb, z_sb);
		 */
		//reference("type", odim.type, type);
		//reference("gain", odim.gain, gain);

		dataSelector.quantity = "^DBZH$";
		dataSelector.path = ".*/data[0-9]+/?$";  // groups
		dataSelector.count = 1;

		odim.product = "SURF";
		odim.quantity = "RATE";

		// defaultEncoding="S,0.0001,220"
		// target="S,0.0001,220"

		odim.type = "S";
		//odim.setTypeDefaults();
		//odim.gain = 0.001;
		//odim.offset = 0.0;

		// Not supported.
		/*
		allowedEncoding.dereference("nbins");
		allowedEncoding.dereference("nrays");
		allowedEncoding.dereference("rscale");
		*/
		/*
		allowedEncoding.reference("type", odim.type);
		allowedEncoding.reference("gain", odim.gain);
		allowedEncoding.reference("offset", odim.gain);
		*/

		//parameters.append(precip.parameters);
		/*
		parameters.reference("ar", rainFromZ.a);
		parameters.reference("br", rainFromZ.b);
		parameters.reference("as", snowFromZ.a);
		parameters.reference("bs", snowFromZ.b);
		*/
	}

	//PrecipitationZ precip;
	// PrecipitationZ rainFromZ;

	//double freezingLevel;
	//double freezingLevelThickness;
	/*
	double z_ra;
	double z_rb;
	double z_sa;
	double z_sb;
	*/

protected:

	virtual
	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;

};





} // ::rack

#endif /*RainRate_H_*/
