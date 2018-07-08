/**


    Copyright 2014 - 2015   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.


 */


#ifndef ANDRE_PRODUCTS
#define ANDRE_PRODUCTS

//#include "commands.h"
#include "productAdapter.h"
#include "andre/AndreOp.h"
#include "andre/DetectorOp.h"

#include "andre/PrecipOp.h"
#include "andre/BioMetOp.h"
#include "andre/BirdOp.h"
#include "andre/ClutterMapOp.h"
#include "andre/CCorOp.h"
#include "andre/EmitterOp.h"
#include "andre/HydroClassBasedOp.h"
#include "andre/JammingOp.h"
#include "andre/LineOp.h"
#include "andre/NoiseOp.h"
#include "andre/RhoHVLowOp.h"
#include "andre/ShipOp.h"
#include "andre/SpeckleOp.h"
#include "andre/SunOp.h"
#include "andre/TimeOp.h"

// ...and removal.
#include "andre/RemoverOp.h"
#include "andre/DamperOp.h"
#include "andre/GapFillOp.h"
//#include "andre/ThresholdOp.h"

// Utils
#include "andre/QualityCombinerOp.h"

#include "andre/ClutterMapOp.h"

namespace rack {

template <class T>
class AnDReLetAdapter : public ProductAdapter<T>{

public:

	AnDReLetAdapter() : ProductAdapter<T>() {
	};


	virtual
	~AnDReLetAdapter(){};

	virtual inline
	HI5TREE & getTarget() const {
		return getResources().inputHi5;
	};

	virtual
	void run(const std::string & params = "") {

		drain::Logger mout(this->adapterName, __FUNCTION__);

		mout.timestamp("BEGIN_ANDRE");

		VolumeOp<PolarODIM> & op = this->productOp;
		op.setParameters(params);

		RackResources & resources = getResources();

		if (!resources.select.empty()){
			mout.debug() << "Setting AnDre data selector: " << resources.select << mout.endl;
			resources.andreSelect = resources.select;
			mout.debug(1) << "New values: " << op.getDataSelector() << mout.endl;
			resources.select.clear();
		}

		op.dataSelector.setParameters(resources.andreSelect);

		mout.debug() << "Running:  " << op << mout.endl;
		mout.debug() << "Selector: " << resources.andreSelect << mout.endl;

		const HI5TREE &src = resources.inputHi5;
		HI5TREE & dst = getTarget();  //For AnDRe ops, src serves also as dst.  UNNEEDED NOW, with own run() ?

		//mout.warn() << dst << mout.endl;
		op.processVolume(src, dst);

		DataTools::updateCoordinatePolicy(dst, RackResources::polarLeft);
		DataTools::updateAttributes(dst);
		resources.currentPolarHi5 = & dst; // if cartesian, be careful with this...
		resources.currentHi5      = & dst;

		mout.timestamp("END_ANDRE");
	};


	// "Sticky" selector which not cleared after each operation.
	//std::string selector;

};


class AnDReModule : public drain::CommandGroup {
    public: //re 
	//AnDReModule(); //
	AnDReModule(const std::string & section = "andre", const std::string & prefix = "a"); // : drain::CommandGroup(section, prefix); // { //,
	//	clutterMapRead(clutter.productOp) {};
	//AnDReModule(CommandRegistry & registry);

	/*
	AnDReLetAdapter<PrecipOp>   precip; // ?
	AnDReLetAdapter<BiometOp>   biomet;
	AnDReLetAdapter<BirdOp>     bird;
	AnDReLetAdapter<EmitterOp> emitter;
	AnDReLetAdapter<InsectOp>   insect;
	AnDReLetAdapter<JammingOp> jamming;
	AnDReLetAdapter<NoiseOp>     noise; // ??
	AnDReLetAdapter<RhoHVLowOp>     rhoHV;
	AnDReLetAdapter<ShipOp>       ship;
	AnDReLetAdapter<SpeckleOp> speckle;


	// Other detector-like operators

	AnDReLetAdapter<CCorOp>       ccor;
	AnDReLetAdapter<HydroClassBasedOp> hydroClass;
	AnDReLetAdapter<SunOp>   sun;
	AnDReLetAdapter<TimeOp> time;

	// Removal   ops
	AnDReLetAdapter<GapFillOp> gapFillDist;
	AnDReLetAdapter<GapFillRecOp>   gapFillRec;
	AnDReLetAdapter<DamperOp>           damper;
	AnDReLetAdapter<RemoverOp>         remover;

	AnDReLetAdapter<QualityCombinerOp> qualityCombiner;


	AnDReLetAdapter<ClutterMapOp>  clutter;
	ClutterMapRead clutterMapRead;

	ClassThreshold classThreshold;
	Universal universal;
	 */
};

} /* namespace rack */

#endif /* RACK_PRODUCTS */
