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

#include <string>

#include <drain/RegExp.h>

#include "drain/prog/CommandBank.h"
#include "drain/prog/CommandInstaller.h"

#include "drain/util/FileInfo.h"
//#include "drain/image/FileGeoTIFF.h"

#include "hi5/Hi5.h"

#include "resources.h"

namespace rack {

/// Syntax for recognising hdf5 files.
//  Edited 2017/07 such that also files without extension are considered h5 files.
extern
const drain::RegExp h5FileExtension;

/// Syntax for recognising GeoTIFF files.
//extern
//const drain::RegExp tiffFileExtension;
//const drain::FileInfo fileInfoTIFF;

/// Syntax for recognising Portable Networks Image image file.
//extern
//const drain::RegExp pngFileExtension;

/// Syntax for recognising PNM (PGM,PPM) image file.
//extern
//const drain::RegExp pnmFileExtension;

/// Syntax for recognising text files.
extern
const drain::RegExp textFileExtension;

/// Syntax for recognising numeric array files (in plain text format anyway).
extern
const drain::RegExp arrayFileExtension;

/// Syntax for sparsely resampled data.
extern
const drain::RegExp sampleFileExtension;

class CmdOutputFile : public drain::SimpleCommand<std::string> {

public:

	inline
	CmdOutputFile() : drain::SimpleCommand<>(__FUNCTION__, "Output data to HDF5, text, image or GraphViz file. See also: --image, --outputRawImages.",
			"filename", "", "<filename>.[h5|hdf5|png|pgm|txt|dat|mat|dot]|-") {
	};

	//void writeGeoTIFF(const drain::image::Image & src, const std::string & filename) const;

	void writeProfile(const Hi5Tree & src, const std::string & filename) const;

	void writeSamples(const Hi5Tree & src, const std::string & filename) const;

	void writeDotGraph(const Hi5Tree & src, const std::string & filename, ODIMPathElem::group_t selector = (ODIMPathElem::ROOT | ODIMPathElem::IS_INDEXED)) const;

	void exec() const;



};

class CmdOutputTree : public drain::SimpleCommand<std::string> {

public:

	inline
	CmdOutputTree() : drain::SimpleCommand<>(__FUNCTION__, "Output data as simple tree structure.",
			"filename", "", "<filename>|-") {
	};

	void exec() const;


};


/*
class CmdOutputPanel : public drain::BasicCommand {

public:

	inline
	CmdOutputPanel() : drain::BasicCommand(__FUNCTION__, "Save SVG panel of latest images. See also: --image, --outputRawImages.") {
		parameters.link("filename", filename="");
		parameters.link("layout", layout, "basic");
	};

	CmdOutputPanel(const CmdOutputPanel & cmd) : drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.parameters, cmd, *this);
	}


	void exec() const;


	std::string layout;
	std::string filename;

	void appendImage(TreeSVG & group, const std::string & prefix, drain::VariableMap & vmap,
			const drain::Point2D<double> & location, const drain::image::Image & image, drain::BBox & bbox) const;
};
*/


class FileModule : public drain::CommandModule<> { // : public drain::CommandGroup {

public:

	FileModule(drain::CommandBank & bank = drain::getCommandBank());
	//virtual void initialize();

};

}

#endif

// Rack
