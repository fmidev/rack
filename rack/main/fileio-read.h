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

#ifndef RACK_FILE_IO_READ
#define RACK_FILE_IO_READ

#include <drain/prog/CommandRegistry.h>
#include <drain/prog/CommandAdapter.h>

//#include "data/Quantity.h"

#include "resources.h"

namespace rack {


class CmdInputPrefix : public BasicCommand {

public:

	CmdInputPrefix() : BasicCommand(__FUNCTION__, "Path prefix for input files."){
		parameters.reference("path", getResources().inputPrefix = "");
	};
};



class CmdInputFile : public SimpleCommand<std::string>  {

public:

	CmdInputFile() : SimpleCommand<std::string>(__FUNCTION__, "Read h5, txt or png file",
			"filename", "", "<filename>.[h5|hdf5|png|txt]"){ //, inputComplete(true) {
	};

	//mutable bool inputComplete;

	void exec() const;

protected:

	void readFileH5(const std::string & fullFilename) const;

	//void readFileAndAppendH5(const std::string & fullFilename) const;

	void appendCartesianH5(HI5TREE & tmpSrc,  HI5TREE & dst) const;
	void attachCartesianH5(HI5TREE & srcRoot, HI5TREE & dstRoot) const;

	void appendPolarH5(HI5TREE & tmpSrc, HI5TREE & dst) const;
	//void appendPolarH5OLD(HI5TREE & tmpSrc, HI5TREE & dst) const;


	void readTextFile(const std::string & fullFilename) const;

	void readImageFile(const std::string & fullFilename) const;

	template <class OD>  // const drain::VariableMap & rootProperties,
	void deriveImageODIM( const drain::image::Image &srcImage,
			OD & odim) const {
		// See also EncodingODIM copyFro
		const drain::image::Geometry & g = srcImage.getGeometry();
		odim.setGeometry(g.getWidth(), g.getHeight());
		odim.type = (char)srcImage.getType2();
		//odim.updateFromMap(rootProperties);
		odim.updateFromMap(srcImage.properties);
		if (odim.gain == 0){
			if (!odim.quantity.empty())
				getQuantityMap().setQuantityDefaults(odim, odim.quantity);
		}

		//odim.copyToData(dst);

	}

	};


}

/*
template <class OD>
void CmdInputFile::deriveImageODIM(){
}
*/

#endif
