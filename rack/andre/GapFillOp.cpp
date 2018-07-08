/**

    Copyright 2010-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#include <drain/image/Intensity.h>
#include <drain/imageops/CopyOp.h>
#include <drain/imageops/DistanceTransformFillOp.h>
#include <drain/imageops/RecursiveRepairerOp.h>



#include "GapFillOp.h"



namespace rack {

using namespace drain::image;
using namespace hi5;




//void GapFillOp::filterImage(const PolarODIM & odim, Image &data, Image &quality) const {
void GapFillOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << *this << mout.endl;

	//const drain::VariableMap &a = data.properties;
	//const double rscale = a.get("where:rscale",500.0);
	//double nrays  = a.get("where:nrays",500.0);

	//File::write(data,"GapFillOp-in.png");
	//File::write(quality,"GapFillOp-inq.png");

	DistanceTransformFillLinearOp op; // op("5,5");
	double h = width / srcData.odim.rscale;
	double v = height * srcData.data.getHeight() / 360.0;
	op.setRadius(h, v);

	//mout.warn() << op << mout.endl;

	ImageTray<const Channel> srcTray;
	srcTray.setChannels(srcData.data, srcData.getQualityData().data);

	ImageTray<Channel> dstTray;
	dstTray.setChannels(dstData.data, dstData.getQualityData().data);

	op.traverseChannels(srcTray, dstTray);
	//op.traverseChannels(srcData.data, srcData.getQualityData().data, dstData.data, dstData.getQualityData().data);
	//File::write(data,"GapFillOp-out.png");
	//File::write(quality,"GapFillOp-outq.png");

}

//void GapFillRecOp::filterImage(const PolarODIM & odim, Image &data, Image &quality) const {
void GapFillRecOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	RecursiveRepairerOp op;
	op.width =  width /  srcData.odim.rscale;
	op.height = height * srcData.data.getHeight() / 360.0;
	op.loops = loops;
	//op.decay = decay;

	ImageTray<const Channel> srcTray;
	srcTray.setChannels(srcData.data, srcData.getQualityData().data);
	ImageTray<Channel> dstTray;
	dstTray.setChannels(dstData.data, dstData.getQualityData().data);
	//op.filter(srcData.data, srcData.getQualityData().data, dstData.data, dstData.getQualityData().data);
	op.traverseChannels(srcTray, dstTray);

}



}  // rack::
