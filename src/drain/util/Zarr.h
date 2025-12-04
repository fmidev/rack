/*

    Copyright 2001 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */



/*
#include <iostream>
#include <sstream>
#include <string.h> // plain C strlen
#include <map>
#include <list>
#include <vector>
#include <set>
*/

#include "JSON.h"

#ifndef DRAIN_ZARR
#define DRAIN_ZARR

namespace drain {


class Zarr3 {

protected:

	// Must be defined before referencing definitions below.
	drain::JSONtree json;

public:

	drain::Variable & zarr_format = json["zarr_format"] = 3;
	drain::Variable & node_type   = json["node_type"]   = "array";
	drain::Variable & shape       = json["shape"]       = {0,0};
	drain::JSONtree & chunk_grid  = json["chunk_grid"];
	drain::Variable & chunk_grid_type  = chunk_grid["type"] = "regular";
	drain::Variable & chunk_grid_shape = chunk_grid["chunk_shape"] = {0,0};
	drain::Variable & data_type        = json["data_type"] = "float32";
	drain::JSONtree & chunk_key_encoding = json["chunk_key_encoding"];
	drain::Variable & chunk_key_encoding_separator = chunk_key_encoding["separator"] = ".";

	inline
	const drain::JSONtree & getJSON() const {return json;};

	static
	const std::string ATTRIBUTE_PATH;

	static
	const std::string ARRAY_PATH;

	/*
	template <class ...T>
	inline
	void setDir(const T & ... args){
		dirPath.set(args...);
	}

	drain::FilePath dirPath;
	*/

	// void writeAttributes() const ;
	void writeMetadata(const drain::FilePath & dirPath) const ;

	virtual inline
	void writeArray(const drain::FilePath & dirPath) const {
		Logger mout(__FILE__, __FUNCTION__);
		mout.unimplemented("Under constr.");
	};
};

}

#endif


