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

#ifndef RACK_FILE_IO_GTIF
#define RACK_FILE_IO_GTIF

//nclude <string>

//nclude <drain/util/RegExp.h>
//nclude <drain/prog/CommandBank.h>
//nclude <drain/prog/CommandInstaller.h>

// #include <drain/util/FileInfo.h>
#include <drain/image/FileGeoTIFF.h>

//nclude "hi5/Hi5.h"

#include "resources.h"

namespace rack {


class CmdGeoTiff : public drain::BasicCommand {

public:

	inline
	CmdGeoTiff() : drain::BasicCommand(__FUNCTION__, "GeoTIFF tile size. Deprecating?, use --outputConf tif:tile=<width>:<height>,compression=LZW") {
		getParameters().link("tile", drain::image::FileTIFF::defaultTile.tuple());
		getParameters().link("compression", compression="", drain::sprinter(drain::image::FileGeoTIFF::compressionDict, "|").str());
		// getParameters().link("tilewidth",  drain::image::FileTIFF::defaultTile.width=256);

		//getParameters().link("tileheight", drain::image::FileTIFF::defaultTile.height=0);
	};

	inline
	CmdGeoTiff(const CmdGeoTiff & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK); // static targets
	}

	/// Checks arguments (currently compression only)
	void exec() const;

	static
	void write(RackContext & ctx, const drain::image::Image & src, const std::string & filename);

	// RackContext & ctx = getContext<RackContext>();
	// void write(const drain::image::Image & src, const std::string & filename) const;


protected:
	//drain::Frame2D<int> tile;

	std::string compression;

	//static
	// const drain::image::FileTIFF::dict_t & compressionDict;

};

}


#endif

// Rack
