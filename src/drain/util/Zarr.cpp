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


// #include <iostream>
// #include <sstream>


#include <filesystem>
#include <drain/Sprinter.h>

#include "Zarr.h"
#include "Output.h"

namespace drain {

	/*
	const std::string Zarr3::ATTRIBUTE_PATH(".zattrs");
	const std::string Zarr3::ARRAY_PATH(".zarray");
	*/

	// const std::string Zarr3::ATTRIBUTE_PATH(".zattrs");

	const std::string Zarr3::ARRAY_PATH("zarr.json");

	/*
	void Zarr3::writeAttributes() const{

		Output output;

		if (!dirPath.empty()){
			output.open(FilePath(dirPath, ATTRIBUTE_PATH));
		}
		else {
		}

		output << "{}";

	};
	*/

	void Zarr3::writeMetadata(const drain::FilePath & dirPath) const {

		drain::Logger mout(__FILE__, __FUNCTION__);

		Output output;

		if (!dirPath.empty()){
			mout.info("Ensure path:", dirPath);
			std::filesystem::create_directory(dirPath.str());
			output.open(FilePath(dirPath, ARRAY_PATH));
		}
		else {
		}

		Sprinter::toStream(output, getJSON(), drain::Sprinter::jsonLayout);

		const drain::FilePath chunkPath(dirPath,"c");
		std::filesystem::create_directory(chunkPath.str());

		std::vector<size_t> dim(2);
		//drain::UniTuple<int,2> dim;
		chunk_grid_shape.toSequence(dim);
		for (int j = 0; j < dim[1]; ++j) {
			for (int i = 0; i < dim[0]; ++i) {
				std::string s = drain::StringBuilder<'.'>(i,j);
				mout.note("Creating:", chunkPath, '/', s);
				std::filesystem::create_directory(drain::FilePath(chunkPath, s).str());
			}
		}


	};

}


