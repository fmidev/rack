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

#ifndef RACK_FILE_IO_READ
#define RACK_FILE_IO_READ

#include "drain/prog/CommandRegistry.h"
#include "drain/prog/CommandAdapter.h"

#include <hi5/Hi5Read.h>

//#include "data/Quantity.h"

#include "resources.h"

namespace rack {


/// A debugging facility. Obsolete?
/*
class CmdInputSelect : public BasicCommand {

public:

	CmdInputSelect() : BasicCommand(__FUNCTION__, "Read ATTRIBUTES (1), DATA(2) or both (3)."){
		//, "value", hi5::Reader::ATTRIBUTES|hi5::Reader::DATASETS, "flag"){};
		parameters.reference("value", getResources().inputSelect = hi5::Reader::ATTRIBUTES|hi5::Reader::DATASETS, "flag");
	}

};
*/

class CmdInputPrefix : public BasicCommand {

public:

	CmdInputPrefix() : BasicCommand(__FUNCTION__, "Path prefix for input files."){
		parameters.reference("path", getResources().inputPrefix = "");
	};
};



class CmdInputFile : public SimpleCommand<std::string>  {

public:

	CmdInputFile() : SimpleCommand<std::string>(__FUNCTION__, "Read HDF5, text or image file",
			"filename", "", "<filename>.[h5|hdf5|png|pgm|ppm|txt]"){ //, inputComplete(true) {
	};

	//mutable bool inputComplete;

	void exec() const;

protected:

	void readFileH5(const std::string & fullFilename) const;

	//void readFileAndAppendH5(const std::string & fullFilename) const;

	void appendCartesianH5(Hi5Tree & tmpSrc,  Hi5Tree & dst) const;
	void attachCartesianH5(Hi5Tree & srcRoot, Hi5Tree & dstRoot) const;

	void appendPolarH5(Hi5Tree & tmpSrc, Hi5Tree & dst) const;
	//void appendPolarH5OLD(Hi5Tree & tmpSrc, Hi5Tree & dst) const;

	void updateQuality(Hi5Tree & srcData, Hi5Tree & dstData) const;


	void readTextFile(const std::string & fullFilename) const;

	void readImageFile(const std::string & fullFilename) const;

	template <class OD>  // const drain::VariableMap & rootProperties,
	void deriveImageODIM( const drain::image::Image &srcImage,
			OD & odim) const {
		// See also EncodingODIM copyFro
		const drain::image::Geometry & g = srcImage.getGeometry();
		odim.setGeometry(g.getWidth(), g.getHeight());
		odim.type = drain::Type::getTypeChar(srcImage.getType());
		//odim.updateFromMap(rootProperties);
		odim.updateFromMap(srcImage.properties);
		if (odim.scale == 0){
			if (!odim.quantity.empty())
				getQuantityMap().setQuantityDefaults(odim, odim.quantity);
		}

		//odim.copyToData(dst);

	}

};



} // rack



/*
template <class OD>
void CmdInputFile::deriveImageODIM(){
}
*/

#endif

// Rack
