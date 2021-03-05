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

#include "drain/util/Log.h"
//#include "drain/util/Static.h"

#include "CoordinateHandler.h"


namespace drain {

namespace image {

const coord_overflow_t  CoordinateHandler2D::UNCHANGED  = 0;
const coord_overflow_t  CoordinateHandler2D::X_OVERFLOW = coord_overflow_flagger_t::add("X_OVERFLOW");
const coord_overflow_t  CoordinateHandler2D::X_UNDERFLOW= coord_overflow_flagger_t::add("X_UNDERFLOW"); // = 2;
const coord_overflow_t  CoordinateHandler2D::Y_OVERFLOW = coord_overflow_flagger_t::add("Y_OVERFLOW");  // = 4;
const coord_overflow_t  CoordinateHandler2D::Y_UNDERFLOW= coord_overflow_flagger_t::add("Y_UNDERFLOW"); //= 8;
const coord_overflow_t  CoordinateHandler2D::IRREVERSIBLE=coord_overflow_flagger_t::add("IRREVERSIBLE"); //= 128


/*
CoordinateHandler2D::CoordinateHandler2D(const drain::image::AreaGeometry & area, const CoordinatePolicy &p){
	setLimits(area.getWidth(), area.getHeight());
	setPolicy(p);
}

CoordinateHandler2D::CoordinateHandler2D(const ImageFrame & src){
	setLimits(src.getWidth(), src.getHeight());
	setPolicy(src.getCoordimatePolicy());
}


CoordinateHandler2D::CoordinateHandler2D(int xUpperLimit, int yUpperLimit, const CoordinatePolicy &p){
	setLimits(xUpperLimit, yUpperLimit);
	setPolicy(p);
}


CoordinateHandler2D::CoordinateHandler2D(const CoordinateHandler2D &h){
	setLimits(h.xRange.min, h.yRange.min, h.xRange.max, h.yRange.max);
	setPolicy(h.getPolicy());
}
*/

/// Sets outer upper limits for x and y.
/*
void CoordinateHandler2D::setLimits(int xUpperLimit,int yUpperLimit){
	setLimits(0, 0, xUpperLimit, yUpperLimit);
}

/// Sets minimum values and outer upper limits for x and y.
void CoordinateHandler2D::setLimits(int xMin, int yMin, int xUpperLimit, int yUpperLimit){
	xRange.set(xMin, xUpperLimit - 1);
	yRange.set(yMin, yUpperLimit - 1);
	area.set(xUpperLimit, yUpperLimit);
}
*/


void CoordinateHandler2D::setPolicy(coord_pol_t xUnderFlowPolicy, coord_pol_t yUnderFlowPolicy, coord_pol_t xOverFlowPolicy, coord_pol_t yOverFlowPolicy){

	Logger mout(getImgLog(), "CoordinateHandler2D", __FUNCTION__);

	// For bookkeeping
	policy.set(xUnderFlowPolicy, yUnderFlowPolicy, xOverFlowPolicy, yOverFlowPolicy);

	//switch (xUnderFlowPolicy){
	if (xUnderFlowPolicy ==  CoordinatePolicy::LIMIT)
		handleXUnderFlow = & CoordinateHandler2D::limitXUnderFlow;
		// break;
	else if (xUnderFlowPolicy ==  CoordinatePolicy::WRAP)
		handleXUnderFlow = & CoordinateHandler2D::wrapXUnderFlow;
		// break;
	else if (xUnderFlowPolicy ==  CoordinatePolicy::MIRROR)
		handleXUnderFlow = & CoordinateHandler2D::mirrorXUnderFlow;
		// break;
	else if (xUnderFlowPolicy ==  CoordinatePolicy::POLAR)
		handleXUnderFlow = & CoordinateHandler2D::polarXUnderFlow;
		// break;
	else // if (xUnderFlowPolicy ==  CoordinatePolicy::UNDEFINED)
	{  // default:
		mout.warn() << "undefined xUnderFlowPolicy" << xUnderFlowPolicy << mout.endl;
		handleXUnderFlow = & CoordinateHandler2D::skipUndefined;
		// break;
	}

	//switch (yUnderFlowPolicy){
	if (yUnderFlowPolicy ==  CoordinatePolicy::LIMIT)
		handleYUnderFlow = & CoordinateHandler2D::limitYUnderFlow;
		// break;
	else if (yUnderFlowPolicy ==  CoordinatePolicy::WRAP)
		handleYUnderFlow = & CoordinateHandler2D::wrapYUnderFlow;
		// break;
	else if (yUnderFlowPolicy ==  CoordinatePolicy::MIRROR)
		handleYUnderFlow = & CoordinateHandler2D::mirrorYUnderFlow;
		// break;
	else if (yUnderFlowPolicy ==  CoordinatePolicy::POLAR)
		handleYUnderFlow = & CoordinateHandler2D::polarYUnderFlow;
		// break;
	else // if (yUnderFlowPolicy ==  CoordinatePolicy::UNDEFINED)
	{  // default:
		mout.warn() << "undefined yUnderFlowPolicy" << yUnderFlowPolicy<< mout.endl;
		handleYUnderFlow = & CoordinateHandler2D::skipUndefined;
		// break;
	}

	//switch (xOverFlowPolicy){
	if (xOverFlowPolicy ==  CoordinatePolicy::LIMIT)
		handleXOverFlow = & CoordinateHandler2D::limitXOverFlow;
		// break;
	else if (xOverFlowPolicy ==  CoordinatePolicy::WRAP)
		handleXOverFlow = & CoordinateHandler2D::wrapXOverFlow;
		// break;
	else if (xOverFlowPolicy ==  CoordinatePolicy::MIRROR)
		handleXOverFlow = & CoordinateHandler2D::mirrorXOverFlow;
		// break;
	else if (xOverFlowPolicy ==  CoordinatePolicy::POLAR)
		handleXOverFlow = & CoordinateHandler2D::polarXOverFlow;
		// break;
	else  // if ( ==  CoordinatePolicy::UNDEFINED)
	{  // default:
		mout.warn() << "undefined xOverFlowPolicy" << xOverFlowPolicy << mout.endl;
		handleXOverFlow = & CoordinateHandler2D::skipUndefined;
		// break;
	}

	//switch (yOverFlowPolicy){
	if (yOverFlowPolicy ==  CoordinatePolicy::LIMIT)
		handleYOverFlow = & CoordinateHandler2D::limitYOverFlow;
		// break;
	else if (yOverFlowPolicy ==  CoordinatePolicy::WRAP)
		handleYOverFlow = & CoordinateHandler2D::wrapYOverFlow;
		// break;
	else if (yOverFlowPolicy ==  CoordinatePolicy::MIRROR)
		handleYOverFlow = & CoordinateHandler2D::mirrorYOverFlow;
		// break;
	else if (yOverFlowPolicy ==  CoordinatePolicy::POLAR)
		handleYOverFlow = & CoordinateHandler2D::polarYOverFlow;
		// break;
	else // if ( ==  CoordinatePolicy::UNDEFINED)
	{  // default:
		mout.warn() << "undefined yOverFlowPolicy" << yOverFlowPolicy << mout.endl;
		handleYOverFlow = & CoordinateHandler2D::skipUndefined;
	}

}


std::ostream & operator<<(std::ostream & ostr, const CoordinateHandler2D & handler){
	ostr << "handler[" << handler.getXRange() << ',' << handler.getYRange()<< ']';
	ostr << ' ' << handler.getPolicy();
	return ostr;
}

} // image

} // drain

// Drain
