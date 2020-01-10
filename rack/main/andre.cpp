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

#include <set>
#include <ostream>

#include <drain/util/Log.h>
#include <drain/prog/Command.h>

#include "data/ODIM.h"
#include "andre/AndreOp.h"
#include "andre/DetectorOp.h"

#include "andre/PrecipOp.h"

#include "andre/AttenuationOp.h"
#include "andre/BioMetOp.h"
#include "andre/BirdOp.h"
#include "andre/ClutterOp.h"
#include "andre/CCorOp.h"
#include "andre/DopplerNoiseOp.h"
#include "andre/EmitterOp.h"
#include "andre/HydroClassBasedOp.h"
#include "andre/JammingOp.h"
#include "andre/LineOp.h"
//#include "andre/NoiseOp.h"
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

public:

	ClutterMapRead(ClutterOp & op) : SimpleCommand<std::string>(__FUNCTION__, "Read a file containing CLUTTER quantity.", "filename",""),
			clutterOp(op) {
		getRegistry().add(*this, __FUNCTION__, 0);
	};

	void exec() const {
		drain::Logger mout(__FUNCTION__, __FILE__);
		mout.info() << "querying " << value << mout.endl;
		//clutter.productOp.setClutterMap(value);
		clutterOp.setClutterMap(value);
	};

	ClutterOp & clutterOp;

};


/// Set the default quality [0.0,1.0] of radar data, typically close to 0.9.
/**
 *   Typically set prior to anomaly detection.
 *
 */
class DefaultQuality : public BasicCommand {

public:

	DefaultQuality() : BasicCommand(__FUNCTION__, "Quality index value below which also CLASS information will be updated.") {
		parameters.reference("threshold", QualityCombinerOp::DEFAULT_QUALITY = 0.90, "0...1");
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

	static AnDReLetAdapter<AttenuationOp> attn;
	static AnDReLetAdapter<BiometOp>   biomet;
	static AnDReLetAdapter<BirdOp>     bird;
	static AnDReLetAdapter<DopplerNoiseOp> dopplerNoise;
	static AnDReLetAdapter<EmitterOp> emitter;
	static AnDReLetAdapter<InsectOp>   insect;
	static AnDReLetAdapter<JammingOp> jamming;
	//static AnDReLetAdapter<NoiseOp>     noise; // on hold (bak)
	static AnDReLetAdapter<RhoHVLowOp>  rhoHV;
	static AnDReLetAdapter<ShipOp>       ship;
	static AnDReLetAdapter<SpeckleOp> speckle;


	// Other detector-like operators
	static AnDReLetAdapter<DefaultOp>   defaultOp; // ?
	static AnDReLetAdapter<PrecipOp>   precip; // ?
	static AnDReLetAdapter<ClutterOp>  clutter;
	static ClutterMapRead clutterMapRead(clutter.productOp);

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

	static DefaultQuality defaultQuality;
	static Universal universal;

}


} // namespace rack

// Rack
