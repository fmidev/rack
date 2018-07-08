/*

    Copyright 2013-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack library for C++.

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
#include <drain/image/File.h>

#include "PolarNeuralVprOp.h"
#include "radar/Constants.h"

namespace rack
{

using namespace drain::image;


void PolarNeuralVPROp::filter(const HI5TREE &src, const std::map<double,std::string> & srcPaths, HI5TREE &dst) const {

	drain::Logger mout(drain::getLog(),"PolarNeuralVPROp::filter");
	mout.debug(1) << "start" << mout.endl;


	// TARGET DATAEARTH_RADIUS_43
	/*
	HI5TREE & dstDataGroup = dst["data1"];
	Image & dstData = dstDataGroup["data"].data.dataSet;

	HI5TREE & dstQualityGroup = dst["quality1"];
	Image & dstQuality = dstQualityGroup["data"].data.dataSet;


	ODIM odimOut;
    */
	mout.debug(1) << "Finished" << mout.endl;

}



}
