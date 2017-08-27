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
#include "andre/ThresholdOp.h"

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

	virtual
	//inline
	HI5TREE & getTarget() const {
		return getResources().inputHi5;
	};

	//std::string resources.andreSelect;

	virtual
	void run(const std::string & params = "") {

		drain::MonitorSource mout(this->adapterName, __FUNCTION__);

		mout.timestamp("BEGIN_ANDRE");

		VolumeOp<PolarODIM> & op = this->productOp;
		op.setParameters(params);

		RackResources & resources = getResources();

		if (!resources.select.empty()){
			mout.debug() << "Setting data selector: " << resources.select << mout.endl;
			resources.andreSelect = resources.select;
			// op.dataSelector.setParameters(resources.select);
			mout.debug(1) << "New values: " << op.getDataSelector() << mout.endl;
			//mout.debug(1) << "New values: " << op.getDataSelector() << mout.endl;
			// cmdSelect.rackLet.value.clear();  DONT CLEAR!
			resources.select.clear();
		}

		op.dataSelector.setParameters(resources.andreSelect);

		mout.debug() << "Running:  " << op << mout.endl;
		mout.debug() << "Selector: " << resources.andreSelect << mout.endl;


		const HI5TREE &src = resources.inputHi5;
		HI5TREE & dst = getTarget();  //For AnDRe ops, src serves also as dst.  UNNEEDED NOW, with own run() ?

		//mout.warn() << dst << mout.endl;
		op.processVolume(src, dst);

		RackResources::updateCoordinatePolicy(dst, RackResources::polarLeft);
		DataSelector::updateAttributes(dst);
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

// Rack
