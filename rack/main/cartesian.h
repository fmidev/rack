/**


    Copyright 2006 - 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#pragma once

// SINCE 2017/06/30,
/*  cCreate = cAdd + cExtract dw
 *  cAdd adds current h5 (polar or cartesian).
 */

#ifndef RACK_COMPOSITING
#define RACK_COMPOSITING



#include <drain/prog/CommandRegistry.h>


#include <drain/prog/CommandAdapter.h>
#include "data/Quantity.h"
#include "data/CartesianODIM.h"
#include "resources.h"



namespace rack {


/// Commands for conversions to Cartesian coordinate system, including generation of radar image composites.
/*!
 *  Add an instance of CompositingModule to the main program.
 *
 */
class CompositingModule : public drain::CommandGroup {

public:

	CompositingModule(const std::string & section = "cart", const std::string & prefix = "c");

};

}



#endif
