/**


    Copyright 2010 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

   Created on: Sep 30, 2010
*/

#ifndef RACK_ACCUMULATION
#define RACK_ACCUMULATION

#include <drain/prog/CommandRegistry.h>
#include <drain/prog/CommandAdapter.h>

#include "resources.h"

namespace rack {


class AccumulationModule : public drain::CommandGroup {
    public: //re 
	AccumulationModule(const std::string & section = "prod", const std::string & prefix = "p");


};


}


#endif
