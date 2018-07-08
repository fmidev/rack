/*


    Copyright 2011-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include <drain/util/Log.h>

#include "Quantity.h"

namespace rack {



EncodingODIM & Quantity::set(char typecode) {

	if (!typecode)
		typecode = 'C';  // ???

	if (!defaultType)
		defaultType = typecode;

	EncodingODIM & odim = (*this)[typecode];
	odim.type = typecode;
	if (!odim.isSet())
		odim.setTypeDefaults();

	return odim;
}


}  // namespace rack


