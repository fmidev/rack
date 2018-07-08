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


#ifndef RACK_COMMANDS2
#define RACK_COMMANDS2



#include <drain/prog/Command.h>
#include <drain/prog/CommandRegistry.h>
#include <drain/prog/CommandPack.h>
#include <drain/prog/CommandAdapter.h>

#include "data/Quantity.h"
#include "andre/DetectorOp.h"

#include "resources.h"


//using namespace drain;

namespace rack {



class CommandModule : public drain::CommandGroup {

public:
	CommandModule();

};



} /* namespace rack */


#endif /* RACK_PRODUCTS */
