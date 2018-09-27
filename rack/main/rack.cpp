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

// Rack
