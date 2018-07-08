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

#ifndef RACK_FILE_IO
#define RACK_FILE_IO

#include <drain/util/RegExp.h>
#include <drain/prog/CommandRegistry.h>
#include <drain/prog/CommandAdapter.h>

#include "data/Quantity.h"

#include "resources.h"

namespace rack {

/// Syntax for recognising hdf5 files.
//  Edited 2017/07 such that also files without extension are considered h5 files.
extern
const drain::RegExp h5FileExtension;

/// Syntax for recognising GeoTIFF files.
extern
const drain::RegExp tiffFileExtension;

/// Syntax for recognising image files (currently, png supported).
extern
const drain::RegExp pngFileExtension;

/// Syntax for recognising text files.
extern
const drain::RegExp textFileExtension;

/// Syntax for recognising numeric array files (in plain text format anyway).
extern
const drain::RegExp arrayFileExtension;

/// Syntax for sparsely resampled data.
extern
const drain::RegExp sampleFileExtension;


class FileModule : public CommandGroup {

public:

	FileModule(const std::string & section = "", const std::string & prefix = "");

};

}

#endif
