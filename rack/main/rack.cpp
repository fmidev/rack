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

#include <math.h>

#include <iostream>


#include "drain/util/Log.h"
#include "drain/prog/CommandBankUtils.h"
#include <drain/prog/CommandInstaller.h>

#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"
#include "hi5/Hi5Write.h"

//#include "commands.h"
//#include "accumulation.h"
#include "commands.h"
#include "fileio.h"
#include "andre.h"
#include "accumulation.h"
#include "products.h"
#include "cartesian.h"
#include "science.h"
#include "images.h"
#include "image-ops.h"

#include "rack.h"


#include "drain/imageops/FastOpticalFlowOp2.h"
#include "drain/imageops/FloodFillOp.h"

namespace rack {


int process(int argc, const char **argv) {


	if (argc == 1) {
		std::cerr << "Usage: rack <input> [commands...] -o <outputFile>\nHelp:  rack -h\n" ;
		return 1;
	}

	// drain::CommandRegistry & registry = drain::getRegistry();

	//registry.setSection("", "");
	RackResources & resources = getResources();
	RackResources::ctx_cloner_t & contextCloner = resources.getContextCloner();
	RackContext & ctx = contextCloner.getSourceOrig(); // baseCtx

	// NEW
	ctx.log.setVerbosity(LOG_NOTICE);
	// OLD
	drain::getLog().setVerbosity(LOG_NOTICE);
	//drain::image::getImgLog().setVerbosity(imageLevel);

	drain::Logger mout(ctx.log, "rack");
	mout.timestamp("BEGIN_RACK"); // appears never, because verbosity initially low

	mout.debug() = "Activate modules";

	MainModule commandMod; // ("general");
	FileModule fileMod; // ("general");
	AnDReModule andreMod; // ("andre");
	ScienceModule scienceMod; // ("science");
	ProductModule productMod; // ("prod");
	AccumulationModule accumulationMod; // ("acc");
	CartesianModule cartesianMod; // ("cart");
	ImageOpModule imageOpMod; // ("imageOps");
	ImageModule   imageMod; // ("images");


	mout.debug() = "Add Rack-specific commands";

	drain::CommandBank & cmdBank = drain::getCommandBank();
	cmdBank.setTitle("Rack - a radar data processing program");

	// If command is not found, it is redirected to \c --setODIM \c <arg>  which checks if it starts with a leading slash
	cmdBank.setNotFoundHandlerCmdKey("setODIM");

	// If a plain argument <arg> is given, execute it like \c --inputFile \c <arg> .
	cmdBank.setDefaultCmdKey("inputFile");
	// Also, add it to the commands taht trigger the script (if defined).
	//drain::Flags::value_t trigger = cmdBank.sections.getValue("trigger");
	const drain::Flagger::value_t TRIGGER = drain::Static::get<drain::TriggerSection>().index;
	cmdBank.setScriptTriggerFlag(TRIGGER);
	cmdBank.get("inputFile").section |= TRIGGER;

	// cmdBank.setScriptCmd("");


	/*
	mout.note() << "-- experimental commandBank --"  << mout.endl;
	drain::CommandWrapper<drain::CmdVerbosity> verbosityCmd;
	verbosityCmd.setExternalContext(ctx);
	drain::Context & ctx0 = verbosityCmd.getContext<drain::Context>();
	mout.note() << ctx0.getStatus() << mout.endl;
	RackContext & ctx1 = verbosityCmd.getContext<RackContext>();
	mout.note() << ctx1.getStatus() << mout.endl;
	mout.note() << "-- exp --"  << mout.endl;
	drain::Command & status0 = cmdBank.get("status");
	status0.exec();
	drain::Command & status1 = cmdBank.clone("status");
	status1.exec();
	mout.note() << "-- EXP --"  << mout.endl;
	status0.setExternalContext(ctx);
	status0.exec();
	drain::Command & status2 = cmdBank.clone("status");
	status2.exec();
	mout.note() << "-- EXP --"  << mout.endl;
	*/

	try {
		mout.info() << "converting arguments to a script " << mout.endl;

		drain::Script script;
		cmdBank.scriptify(argc, argv, script);

		drain::Program prog;
		cmdBank.append(script, ctx, prog); // ctx is stored in each cmd



		mout.info() << "running, ctx.id=" << ctx.getId() << mout.endl;
		//cmdBank.run(script, contextCloner);
		cmdBank.run(prog, contextCloner);

		mout.warn() << "debug-level:" << prog.begin()->second->getContext<drain::Context>().log.getVerbosity() << mout.endl;

		if (ctx.statusFlags == 0){
			mout.success() = "Finished.";
		}
		else {
			// std::cerr << ctx.statusFlags.value << ' ' << ctx.statusFlags << " ERROR\n";
			mout.warn() << "error flags("<< ctx.statusFlags.value << "): " << ctx.statusFlags.getKeys(',') << mout.endl;
		}

	}
	catch (const std::exception & e) {
		mout.warn() << e.what() << mout.endl;
		ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR);
	}
	mout.note() << "debug-level:" << ctx.log.getVerbosity() << mout.endl;

	//mout.note() << "-- end commandBank --"  << mout.endl;


	return ctx.statusFlags.value;



	//EncodingODIM().toOStr(std::cout)

	//std::cout << "EncodingODIM" << EncodingODIM() << '\n';
	/*
	std::cout << "ODIM " << ODIM() << '\n';
	PolarODIM odim;
	std::cout << "PolarODIM0 " << odim << '\n';
	odim.setTypeDefaults(typeid(unsigned short int));
	std::cout << "PolarODIM2 " << odim << '\n';
	getQuantityMap().setQuantityDefaults(odim, "ZDR", "C");
	PolarODIM odimCopy(odim);
	std::cout << "PolarODIMC " << odimCopy << '\n';
	*/

	//std::cout << "CartesianODIM" << CartesianODIM() << '\n';


	/// Main "loop".
	//registry.runCommands(argc, argv);

	// mout.timestamp("END_RACK");
	// std::cout << registry.getAliases() << '\n';
	// mout << "end" << mout.endl;


	//if (getResources().inputOk)
	/*
	if (getResources().errorFlags.isSet(255))
		return getResources().errorFlag; //result; future option
	else
		return 0;
	*/

}

} // rack::

// Rack
