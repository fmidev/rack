/**


    Copyright 2006 -   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include <math.h>

#include <iostream>
#include <map>
#include <exception>

#include <drain/util/Log.h>


#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"
#include "hi5/Hi5Write.h"

#include "radar/Precipitation.h"
#include "product/RainRateOp.h"

//#include "commands.h"
//#include "accumulation.h"
#include "commands.h"
#include "fileio.h"
#include "andre.h"
#include "accumulation.h"
#include "products.h"
#include "cartesian.h"
#include "images.h"
#include "rack.h"

// using namespace std;


namespace rack {


int process(int argc, const char **argv) {


	drain::Logger mout("rack");
	mout.timestamp("BEGIN_RACK"); // appears never, because verbosity initially low

	if (argc == 1) {
		std::cerr << "Usage: rack <input> [commands...] -o <outputFile>\nHelp:  rack -h\n" ;
		return 1;
	}

	CommandRegistry & registry = drain::getRegistry();

	//registry.setSection("", "");
	CommandModule commands;

	registry.run("verbose", "5"); // LOG_NOTICE



	FileModule fileio;


	AnDReModule andre("andre", "a");


	AccumulationModule accumulation("prod", "p");
	ProductModule products("prod", "p");

	//getResources().composite.setMethod("MAX");

	registry.setSection("formulae", "");
	BeanRefEntry<PrecipitationZ> rainZ(RainRateOp::precipZrain, "precipZrain");
	BeanRefEntry<PrecipitationZ> snowZ(RainRateOp::precipZsnow, "precipZsnow");
	BeanRefEntry<PrecipitationKDP> pKDP(RainRateOp::precipKDP, "precipKDP");
	BeanRefEntry<PrecipitationZZDR> pZZDR(RainRateOp::precipZZDR, "precipZZDR");
	BeanRefEntry<PrecipitationKDPZDR> pKDPZDR(RainRateOp::precipKDPZDR, "precipKDPZDR");
	BeanRefEntry<FreezingLevel> pFreezingLevel(RainRateOp::freezingLevel);
	//pFreezingLevel.height =
	/*

	*/

	CompositingModule compositing("cart", "c");

	ImageRackletModule imageOps("imageOps", "i");  // Causes bug in StatusMap


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
	registry.runCommands(argc, argv);

	//mout.timestamp("END_RACK");

	/* TODO: more status flags
	 * INPUT_OK
	 * METADATA_OK
	 * WRITE_OK
	 * INCOMPLETE_PRODUCT
	 */
	if (getResources().inputOk)
		return 0; //result; future option
	else
		return 1;

}

} // rack::
