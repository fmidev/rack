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

#include <drain/Log.h>
#include <cassert>

#include <math.h>
#include <iostream>

#include <drain/prog/CommandBankUtils.h>
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
#include "test.h"
#include "images.h"
#include "image-ops.h"

#include "rack.h"

// #include <drain/imageops/FastOpticalFlowOp2.h>
// #include <drain/imageops/FloodFillOp.h>

namespace rack {


int process(int argc, const char **argv) {

	//const static PJ_INFO & pj_info =
	// proj_info();

	if (argc == 1) {
		// This could be from cmdBank ?
		std::cerr << "Usage: rack <input> [commands...] -o <outputFile>\nHelp:  rack -h\n" ;
		return 1;
	}

	RackResources & resources = getResources();
	RackResources::ctx_cloner_t & contextCloner = resources.getContextCloner();
	RackContext & ctx = contextCloner.getSourceOrig(); // baseCtx
	// NEW
	ctx.log.setVerbosity(LOG_NOTICE);
	// OLD
	drain::getLog().setVerbosity(LOG_NOTICE);
	//drain::image::getImgLog().setVerbosity(imageLevel);

	drain::Logger mout(ctx.log, "rack");




	mout.debug("Activate modules");

	MainModule commandMod;
	FileModule fileMod;
	AnDReModule andreMod;
	ScienceModule scienceMod;
	ProductModule productMod;
	AccumulationModule accumulationMod;
	CartesianModule cartesianMod;

	ImageModule   imageMod;
	ImageOpModule imageOpMod;

	HiddenModule   hiddenMod;
#ifndef RACK_DEBUG
	TestModule testMod;
	// exit(0);
#endif


	mout.debug("Add Rack-specific commands");

	drain::CommandBank & cmdBank = drain::getCommandBank();
	cmdBank.setTitle("Rack - a radar data processing program");

	/** If a plain argument <arg> is given, forward it to this command.
	 *  Equivalent to \c --inputFile \c <arg> .
	 */
	cmdBank.setDefaultCmdKey("inputFile");

	/**
	 *  If command is not found, it is redirected to this command.
	 *  \c --setODIM \c <arg>  which checks if it starts with a leading slash
	 */
	cmdBank.setNotFoundHandlerCmdKey("setODIM");

	// Also, mark the commands that trigger a script (if defined).
	const drain::Flagger::ivalue_t TRIGGER = drain::Static::get<drain::TriggerSection>().index;
	cmdBank.setScriptTriggerFlag(TRIGGER);
	cmdBank.get("inputFile").section |= TRIGGER;


	try {
		mout.info("Converting arguments to a script ");

		drain::Script script;
		cmdBank.scriptify(argc, argv, script);

		//mout.attention('\n', script);
		//mout.attention("Script ok");

		drain::Program prog(ctx);
		cmdBank.append(script, prog); // ctx is stored in each cmd
		// cmdBank.append(script, ctx, prog); // ctx is stored in each cmd

		// mout.attention("Running prog");
		mout.info("running ctx: ", ctx.getName());

		//mout.info("running, ctx.id=", ctx.getId());
		//cmdBank.run(script, contextCloner);
		cmdBank.run(prog, contextCloner);

		// mout.warn("debug-level:" , prog.begin()->second->getContext<drain::Context>().log.getVerbosity() );

		if (!ctx.statusFlags){
			mout.ok("Finished.");
		}
		else {
			// std::cerr << ctx.statusFlags.value << ' ' << ctx.statusFlags << " ERROR\n";
			mout.warn("error flags(", ctx.statusFlags.value , "): " , ctx.statusFlags.getKeys(','));
		}

	}
	catch (const std::exception & e) {
		mout.warn( e.what() );
		ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR);
	}
	//mout.note("debug-level:" , ctx.log.getVerbosity() );


	return ctx.statusFlags.value;


}

} // rack::

// Rack
