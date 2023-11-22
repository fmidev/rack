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

#include <drain/prog/CommandInstaller.h>
#include <set>
#include <ostream>

#include "drain/util/Log.h"
#include "drain/prog/Command.h"
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
#include "andre/NonMetOp.h"
#include "andre/ShipOp.h"
#include "andre/SpeckleOp.h"
#include "andre/SunOp.h"
#include "andre/TimeOp.h"

#include "andre/TestOp.h"

// ...and removal.
#include "andre/RemoverOp.h"
#include "andre/DamperOp.h"
#include "andre/GapFillOp.h"
//#include "andre/ThresholdOp.h"

// Utils
#include "andre/QualityCombinerOp.h"

#include "resources.h"
#include "andre.h"


namespace rack {



// This command is problematic in a cloned context.
/*
class ClutterMapRead : public drain::SimpleCommand<std::string> {

public:

	ClutterMapRead(ClutterOp & op) : drain::SimpleCommand<std::string>(__FUNCTION__, "Read a file containing CLUTTER quantity.", "filename",""),
			clutterOp(op) {
		drain::getRegistry().add(*this, __FUNCTION__, 0);
	};

	void exec() const {
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.info("querying " , value );
		// OLD:
		clutterOp.setClutterMap(value);
	};

	// OLD:
	ClutterOp & clutterOp;

};
*/


/// Set the default quality [0.0,1.0] of radar data, typically close to 0.9.
/**
 *   Typically set prior to anomaly detection.
 *
 */
class AnDReDefaultQuality : public drain::SimpleCommand<float>{

public:

	AnDReDefaultQuality() : drain::SimpleCommand<float>(__FUNCTION__,
			"Quality index value below which also CLASS information will be updated.", "threshold", // 0.90
			0.8, "0...1") { // getContext<RackContext>().defaultQuality
		//parameters.link("threshold", QualityCombinerOp::DEFAULT_QUALITY = 0.90, "0...1");
	};

	void exec() const {
		RackContext & ctx = this->template getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__ );
		ctx.defaultQuality = value;
		mout.unimplemented("Warning: this value is currently unused");
	}

};


class AnDReUniversal : public drain::SimpleCommand<bool> {

public:

	AnDReUniversal() : drain::SimpleCommand<bool>(__FUNCTION__, "Toggle the support for universal ie. Dataset-wide quality indices.", "unversal", true){
	};

	void exec() const {
		RackContext & ctx = this->template getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__ );
		// ctx.statusFlags.set(ANDRE_UNIVERSAL, this->value);
		DetectorOp::SUPPORT_UNIVERSAL = this->value;
	};

	/*
	const std::string & getDescription() const {
		static std::string s;
		s =  description + " Flags:" + drain::StatusFlags::getSharedDict().toStr();
		return s;
	}
	*/

	// static
	//const drain::Flagger::value_t ANDRE_UNIVERSAL;

};


//const drain::Flagger::value_t AnDReUniversal::ANDRE_UNIVERSAL = drain::StatusFlags::add("ANDRE_UNIVERSAL");



template <class OP>
class AnDReCommand : public drain::BeanCommand<OP>{

public:

	AnDReCommand(){
	};

	AnDReCommand(const AnDReCommand & cmd){
		this->bean.getParameters().copyStruct(cmd.bean.getParameters(), cmd, *this);
	};

	/// Set implicit parameters
	virtual
	void update(){

		RackContext & ctx = this->template getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, this->bean.getName() );

		mout.debug("Applying data selector and targetEncoding ");

		if (!ctx.select.empty()){
			mout.info("Storing AnDRe selector: ", ctx.select);
			ctx.andreSelect = ctx.select;
			ctx.select.clear();
		}

		if (!ctx.andreSelect.empty()){
			mout.info("AnDRe data selector is set: ", ctx.andreSelect);
			this->bean.dataSelector.setParameters(ctx.andreSelect);
			mout.debug2("-> new values: " , this->bean.getDataSelector() );
		}

		// NEW (Bug fix) 2022/07/10
		this->bean.outputDataVerbosity = ctx.outputDataVerbosity;

		//this->bean.UNIVERSAL = ctx.statusFlags.getValue(AnDReUniversal::ANDRE_UNIVERSAL);

	}

	virtual
	void exec() const {

		RackContext & ctx = this->template getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, this->bean.getName() );

		//mout.timestamp("BEGIN_ANDRE");

		mout.debug("Running:  ", this->bean);
		mout.info("AnDRe selector: ", ctx.andreSelect);

		// For AnDRe ops, src serves also as dst.  UNNEEDED NOW, with own run() ?
		Hi5Tree & dst = ctx.getHi5(
				RackContext::POLAR|RackContext::INPUT|RackContext::PRIVATE,
				RackContext::POLAR|RackContext::INPUT|RackContext::SHARED
		);
		const Hi5Tree &src = dst;
		//mout.note(src );

		//mout.warn(dst );
		this->bean.traverseVolume(src, dst);

		DataTools::updateCoordinatePolicy(dst, RackResources::polarLeft);
		DataTools::updateInternalAttributes(dst);
		ctx.currentPolarHi5 = & dst; // if cartesian, be careful with this...
		ctx.currentHi5      = & dst;

		// mout.timestamp("END_ANDRE");
	};

};

template <class OP>
class DetectorCommand : public AnDReCommand<OP>{

public:

	/*
	DetectorCommand(): AnDReCommand(){
	};

	DetectorCommand(const DetectorCommand & cmd) : AnDReCommand(){
		this->bean.getParameters().copyStruct(cmd.bean.getParameters(), cmd, *this);
	};
	*/

	virtual
	void update(){

		RackContext & ctx = this->template getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, this->bean.getName() );


		mout.unimplemented("...Applying UNIVERSAL" );

		this->AnDReCommand<OP>::update(); // ? why down here

		// TODO: redesign this...
		// DetectorOp::SUPPORT_UNIVERSAL = ctx.statusFlags.isSet(AnDReUniversal::ANDRE_UNIVERSAL);
		//this->bean.UNIVERSAL = ctx.statusFlags.isSet(AnDReUniversal::ANDRE_UNIVERSAL);

	}

};


/// Keep combined OBSOLETE?
/*
class AnDReStoreCombined : public drain::SimpleCommand<std::string> {
    public: //re 
	//std::string path;

	AnDReStoreCombined() : drain::SimpleCommand<std::string>(__FUNCTION__, "Store combined detection results in .../quality1/<path> : 'data' for overwriting, 'data~' for tmp (unsaved).",
			"path", "data~", "std::string"){
		//parameters.separators.clear();
		//parameters.link("path", path, "data~", "std::string");
	};

	void exec() const {
		AndreOp::pathCombinedQuality = value;
	};

};
static drain::CommandEntry<AnDReStoreCombined> anDReStoreCombined("andre", "aStoreCombined");
*/

/*
*/

// } // namespace ::




typedef drain::CommandInstaller<'a',AnDReSection> AnDReInstaller;

class DetectorInstaller: public AnDReInstaller {

public:

	DetectorInstaller(drain::CommandBank & bank = drain::getCommandBank()) : AnDReInstaller(bank){
	};

	template <class OP>
	//DetectorCommand<OP> & install(char alias = 0){
	drain::Command & install(char alias = 0){
		std::string name = OP().getName();
		drain::CommandBank::deriveCmdName(name, getPrefix());
		// return cmdBank.add<DetectorCommand<OP> >(name);
		drain::Command  & cmd = cmdBank.add<DetectorCommand<OP> >(name);
		cmd.section = getSection().index;
		return cmd;

	};

};


class RemoverInstaller: public AnDReInstaller {

public:

	RemoverInstaller(drain::CommandBank & bank = drain::getCommandBank()) : AnDReInstaller(bank){
	};

	/**
	 *   AnDReCommand<OP>
	 */
	template <class OP>  // AnDReCommand<OP>
	drain::Command & install(char alias = 0){
		std::string name = OP().getName();
		drain::CommandBank::deriveCmdName(name, getPrefix());
		//return cmdBank.add<AnDReCommand<OP> >(name);
		drain::Command & cmd = cmdBank.add<AnDReCommand<OP> >(name);
		cmd.section = getSection().index;
		return cmd;
	};

};

// Alternatively, could be two separate modules: AnDReDetectorModule and AnDReRemoverModule
AnDReModule::AnDReModule(drain::CommandBank & cmdBank) : module_t(cmdBank) { // : CommandSection("andre"){

	drain::CommandBank::trimWords().insert("Op");
	drain::CommandBank::trimWords().insert("AnDRe");

	//drain::CommandBank & bank = drain::getCommandBank();

	// Shared/general AnDRe commands
	//AnDReInstaller installer(cmdBank);
	install<AnDReUniversal>();
	install<AnDReDefaultQuality>();
	install<AnDReCommand<QualityCombinerOp> >(); // qualityCombiner;


	// Wrapper for detector ops
	DetectorInstaller detectorInstaller(cmdBank);
	detectorInstaller.install<AttenuationOp>(); // attn;
	detectorInstaller.install<BiometOp>(); //   biomet;
	detectorInstaller.install<BirdOp>(); //     bird;
	detectorInstaller.install<DopplerNoiseOp>(); // dopplerNoise;
	detectorInstaller.install<EmitterOp>(); // emitter;
	detectorInstaller.install<InsectOp> (); //  insect;
	detectorInstaller.install<JammingOp>(); // jamming;
	//detectorInstaller.install<NoiseOp>     noise; // on hold (bak)
	detectorInstaller.install<NonMetOp> (); // nonMet;
	detectorInstaller.install<ShipOp>(); //       ship;
	detectorInstaller.install<SpeckleOp>(); // speckle;

	// Other detector-like operators
	detectorInstaller.install<DefaultOp>(); //   defaultOp; // ?
	detectorInstaller.install<PrecipOp>(); //   precip; // ?
	detectorInstaller.install<ClutterOp>(); //  clutter;

	// ClutterMapRead clutterMapRead(clutter.bean); // RE-design
	detectorInstaller.install<CCorOp>(); //       ccor;
	detectorInstaller.install<HydroClassBasedOp>(); // hydroClass;
	detectorInstaller.install<SunOp>(); //   sun;
	detectorInstaller.install<TimeOp>(); // time;

	detectorInstaller.install<TestOp>().section |= drain::Static::get<drain::HiddenSection>().index;

	// Wrapper for removal ops
	RemoverInstaller removerInstaller(cmdBank);
	removerInstaller.install<GapFillOp>(); //         gapFill; // Dist;
	removerInstaller.install<GapFillRecOp>(); //   gapFillRec;
	removerInstaller.install<DamperOp>(); //           damper;
	removerInstaller.install<RemoverOp>(); //         remover;



}


} // namespace rack

// Rack
