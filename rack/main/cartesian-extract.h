/**


    Copyright 2006 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

//#pragma once

#ifndef RACK_CART_EXTRACT
#define RACK_CART_EXTRACT




#include <drain/prog/CommandAdapter.h>

#include "resources.h"

#include "cartesian-extract.h"



namespace rack {

class CartesianExtract : public SimpleCommand<std::string> {

public:

	CartesianExtract() : SimpleCommand<>(__FUNCTION__,"Extract data that has been composited on the accumulation array",
			"value", "dw", "Layers: data,count,weight,std.deviation") {
		//parameters.reference("channels", channels, "dw", "Accumulation layers to be extracted");
	};

	void extract(const std::string & channels) const;

	inline
	void exec() const {
		extract(value);
	}

};
/// static RackLetAdapter<CompositeExtract> cExtract("cExtract");



} // rack::



#endif
