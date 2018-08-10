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

// Rack
