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

#include <drain/image/CoordinatePolicy.h>
#include <sstream>
#include <ostream>
#include "drain/image/FilePng.h"

#include "ImageOp.h"
#include "FloodFillOp.h"

namespace drain
{
namespace image
{




/*
class PickySegmentProber : public SegmentProber<double, double> {

public:

	inline
	PickySegmentProber(const Channel &s, Channel &d) : SegmentProber<double, double>(s, d) {
	}

	inline
	PickySegmentProber(Channel &d) : SegmentProber<double, double>(d, d) {
	}

protected:

	/// Experimental
	virtual inline
	bool isValidMove(int i0, int j0, int i, int j){

		src_t x = src.get<src_t>(i,j);

		if ((x >= anchorMin) && (x <= anchorMax)){

			x = x-src.get<src_t>(i0,j0);
			if ((x>-8) && (x<8))
				return true;

		}

		return false;

	}

};
*/


FloodFillOp::FloodFillOp(int i, int j, double min, double max, double value) : ImageOp(__FUNCTION__,
		"Fills an area starting at (i,j) having intensity in [min,max], with a value.") {
	parameters.link("location",  this->startPoint.tuple(),  "i:j");
	parameters.link("intensity", this->conf.anchor.tuple(), "min:max").fillArray = false;
	parameters.link("value",    this->conf.markerValue = value, "intensity");

	this->conf.anchor.set(min,max);


	//parameters.link("i", this->i0 = i, "coord");
	//parameters.link("j", this->j0 = j, "coord");
	// parameters.append(conf);

	//parameters.link("min", this->conf.anchorMin = min, "intensity");
	//parameters.link("max", this->conf.anchorMax = max, "intensity");
	//parameters.link("value", this->conf.markerValue = value, "intensity");
}

FloodFillOp::FloodFillOp(const FloodFillOp & op) : ImageOp(op){
	Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	//mout.warn("parameters" );
	parameters.copyStruct(op.getParameters(), op, *this);
	//parameters.copyStruct(op.getParameters(), op.conf, this->conf); // CONTAINED in obj
	// mout.warn(parameters );
}


void FloodFillOp::traverseChannel(const Channel & src, Channel & dst) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug(*this );

	CoordinateHandler2D preHandler(src.getGeometry());
	preHandler.setPolicy(EdgePolicy::WRAP);
	mout.debug(preHandler );
	Point2D<int> point(startPoint);
	if (preHandler.handle(point)){
		mout.info("tuned coordinates => (" , point , ')' );
	}


	const CoordinatePolicy & cp = src.getCoordinatePolicy();
	const bool HORZ = ((cp.xUnderFlowPolicy != EdgePolicy::POLAR) && (cp.xOverFlowPolicy != EdgePolicy::POLAR));


	if (Type::call<typeIsFloat>(src.getType()) || Type::call<typeIsFloat>(dst.getType()) ) {
		mout.debug2("type: double" );
		//SegmentProber<double,double,SegmentProberConf<double,double> > fill(src, dst);
		FillProber fill(src, dst);
		//PickySegmentProber fill(src, dst);
		fill.conf.updateFromMap(conf);
		fill.conf.markerValue = dst.getScaling().inv(conf.markerValue);
		fill.init();
		fill.probe(point.x, point.y, HORZ);
	}
	else {
		mout.debug2("type: integral" );
		FillProber fill(src, dst);
		//SegmentProber<int,int,SegmentProberConf<int,int> > fill(src, dst);
		//PickySegmentProber fill(src, dst);
		fill.conf.updateFromMap(conf);
		fill.conf.markerValue = dst.getScaling().inv(conf.markerValue);
		fill.init();
		mout.debug("dst: " , dst );
		//mout.warn(conf , '>' , conf.markerValue );
		mout.debug(fill , '>' , fill.conf.markerValue );
		//fill.count = 0;
		fill.probe(point.x, point.y, HORZ);

		//fill.scan();
		//mout.warn("fill.count: " , fill.count );
	}

}

} // image::
} // drain::



