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




//#include <drain/util/Fuzzy.h>
#include <drain/image/CopyOp.h>
#include <drain/image/DistanceTransformFillOp.h>
//#include <drain/image/MathOpPack.h>
#include <drain/image/RecursiveRepairerOp.h>
#include <drain/image/Intensity.h>

//#include "Geometry.h"

//#include <drain/image/MathOpPack.h>
#include <drain/image/File.h>

#include "GapFillOp.h"



namespace rack {

using namespace drain::image;
using namespace hi5;




//void GapFillOp::filterImage(const PolarODIM & odim, Image &data, Image &quality) const {
void GapFillOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {

	drain::MonitorSource mout(name, __FUNCTION__);
	mout.debug() << *this << mout.endl;

	//const drain::VariableMap &a = data.properties;
	//const double rscale = a.get("where:rscale",500.0);
	//double nrays  = a.get("where:nrays",500.0);

	//File::write(data,"GapFillOp-in.png");
	//File::write(quality,"GapFillOp-inq.png");

	DistanceTransformFillLinearOp op; // op("5,5");
	op.horz = width / srcData.odim.rscale;
	op.vert = height * srcData.data.getHeight() / 360.0;

	//mout.warn() << op << mout.endl;

	op.filter(srcData.data, srcData.getQualityData().data, dstData.data, dstData.getQualityData().data);
	//op.filter(data, quality, data, quality);
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

	op.filter(srcData.data, srcData.getQualityData().data, dstData.data, dstData.getQualityData().data);


}



}  // rack::

// Rack
