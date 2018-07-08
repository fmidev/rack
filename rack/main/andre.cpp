/**


    Copyright 2015 -  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include <set>
#include <ostream>

#include <drain/util/Log.h>
#include <drain/prog/Command.h>

#include "data/ODIM.h"
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

#include "andre.h"


namespace rack {




class ClutterMapRead : public SimpleCommand<std::string> {
    public: //re 
	ClutterMapRead(ClutterMapOp & op) : SimpleCommand<std::string>(__FUNCTION__, "Read a file containing CLUTTERMAP quantity.", "filename",""),
			clutterOp(op) {
		getRegistry().add(*this, __FUNCTION__, 0);
	};

	void exec() const {
		drain::Logger mout(name, __FUNCTION__);
		mout.info() << "reading " << value << mout.endl;
		//clutter.productOp.setClutterMap(value);
		clutterOp.setClutterMap(value);
	};

	ClutterMapOp & clutterOp;

};


class ClassThreshold : public BasicCommand {

public:

	ClassThreshold() : BasicCommand(__FUNCTION__, "Quality index value below which also CLASS information will be updated.") {
		parameters.reference("threshold", QualityCombinerOp::CLASS_UPDATE_THRESHOLD = 0.5, "0...1");
		getRegistry().add(*this, __FUNCTION__, 0);
	};

};


class Universal : public BasicCommand {

public:

	Universal() : BasicCommand(__FUNCTION__, "Toggle the support for universal ie. Dataset-wide quality indices."){
		getRegistry().add(*this, __FUNCTION__, 0);
	};

	void exec() const {
		// toggle
		DetectorOp::SUPPORT_UNIVERSAL = !DetectorOp::SUPPORT_UNIVERSAL;
	};

};

// These are "global" (potentially)

/*
class AnDReClassUpdateThreshold : public BasicCommand {
    public: //re 
	// "Toggle the support for universal ie. Dataset-wide quality indices.") {
	AnDReClassUpdateThreshold() : BasicCommand(__FUNCTION__, "Quality index value below which also CLASS information will be updated.") {
		parameters.reference("threshold", QualityCombinerOp::CLASS_UPDATE_THRESHOLD = 0.5, "0...1");
	};

};
static CommandEntry<AnDReClassUpdateThreshold> anDReClassUpdateThreshold("andre", "aClassThreshold");
*/








/// Keep combined OBSOLETE?
/*
class AnDReStoreCombined : public SimpleCommand<std::string> {
    public: //re 
	//std::string path;

	AnDReStoreCombined() : SimpleCommand<std::string>(__FUNCTION__, "Store combined detection results in .../quality1/<path> : 'data' for overwriting, 'data~' for tmp (unsaved).",
			"path", "data~", "std::string"){
		//parameters.separators.clear();
		//parameters.reference("path", path, "data~", "std::string");
	};

	void exec() const {
		AndreOp::pathCombinedQuality = value;
	};

};
static CommandEntry<AnDReStoreCombined> anDReStoreCombined("andre", "aStoreCombined");
*/

// } // namespace ::


AnDReModule::AnDReModule(const std::string & section, const std::string & prefix) : drain::CommandGroup(section, prefix){

	static AnDReLetAdapter<PrecipOp>   precip; // ?
	static AnDReLetAdapter<BiometOp>   biomet;
	static AnDReLetAdapter<BirdOp>     bird;
	static AnDReLetAdapter<EmitterOp> emitter;
	static AnDReLetAdapter<InsectOp>   insect;
	static AnDReLetAdapter<JammingOp> jamming;
	static AnDReLetAdapter<NoiseOp>     noise; // ??
	static AnDReLetAdapter<RhoHVLowOp>     rhoHV;
	static AnDReLetAdapter<ShipOp>       ship;
	static AnDReLetAdapter<SpeckleOp> speckle;


	// Other detector-like operators

	static AnDReLetAdapter<CCorOp>       ccor;
	static AnDReLetAdapter<HydroClassBasedOp> hydroClass;
	static AnDReLetAdapter<SunOp>   sun;
	static AnDReLetAdapter<TimeOp> time;

	// Removal   ops
	static AnDReLetAdapter<GapFillOp>         gapFill; // Dist;
	static AnDReLetAdapter<GapFillRecOp>   gapFillRec;
	static AnDReLetAdapter<DamperOp>           damper;
	static AnDReLetAdapter<RemoverOp>         remover;

	static AnDReLetAdapter<QualityCombinerOp> qualityCombiner;


	static AnDReLetAdapter<ClutterMapOp>  clutter;

	static ClutterMapRead clutterMapRead(clutter.productOp);

	static ClassThreshold classThreshold;
	static Universal universal;

}


} // namespace rack
