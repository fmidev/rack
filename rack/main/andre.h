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

/*
#include "andre/PrecipOp.h"
#include "andre/BioMetOp.h"
#include "andre/BirdOp.h"
#include "andre/ClutterOp.h"
#include "andre/CCorOp.h"
#include "andre/EmitterOp.h"
#include "andre/HydroClassBasedOp.h"
#include "andre/JammingOp.h"
#include "andre/LineOp.h"
#include "andre/NoiseOp.h"
#include "andre/NonMetOp.h"
#include "andre/ShipOp.h"
#include "andre/SpeckleOp.h"
#include "andre/SunOp.h"
#include "andre/TimeOp.h"

// ...and removal.
#include "andre/RemoverOp.h"
#include "andre/DamperOp.h"
#include "andre/GapFillOp.h"
//#include "andre/ThresholdOp.h"
*/

// Utils
#include "andre/QualityCombinerOp.h"

#include "andre/ClutterOp.h"

namespace rack {

template <class T>
class AnDReLetAdapter : public ProductAdapter<T>{

public:

	AnDReLetAdapter() : ProductAdapter<T>() {
	};


	virtual
	~AnDReLetAdapter(){};

	virtual inline
	Hi5Tree & getTarget() const {
		return getResources().inputHi5;
	};

	virtual
	void run(const std::string & params = "") {

		//__FUNCTION__
		drain::Logger mout(__FUNCTION__, this->adapterName.c_str());

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
		mout.debug() << "AnDRe selector: " << resources.andreSelect << mout.endl;

		const Hi5Tree &src = resources.inputHi5;
		Hi5Tree & dst = getTarget();  //For AnDRe ops, src serves also as dst.  UNNEEDED NOW, with own run() ?

		//mout.warn() << dst << mout.endl;
		op.processVolume(src, dst);

		DataTools::updateCoordinatePolicy(dst, RackResources::polarLeft);
		DataTools::updateInternalAttributes(dst);
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
};


} // rack

#endif

// Rack
