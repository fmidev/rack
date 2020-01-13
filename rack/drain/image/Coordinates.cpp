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

#include "util/Log.h"
#include "Coordinates.h"


namespace drain {

namespace image {

const int CoordinatePolicy::UNDEFINED(0);
const int CoordinatePolicy::LIMIT(1);
const int CoordinatePolicy::WRAP(2);
const int CoordinatePolicy::MIRROR(3);
const int CoordinatePolicy::POLAR(4);



CoordinateHandler2D::CoordinateHandler2D(const drain::image::AreaGeometry & area, const CoordinatePolicy &p){
	setLimits(area.getWidth(), area.getHeight());
	setPolicy(p);
}

CoordinateHandler2D::CoordinateHandler2D(int xUpperLimit, int yUpperLimit, const CoordinatePolicy &p){
	setLimits(xUpperLimit, yUpperLimit);
	setPolicy(p);
}


CoordinateHandler2D::CoordinateHandler2D(const CoordinateHandler2D &h){
	setPolicy(h.getPolicy());
	setLimits(h._xMin, h._yMin, h._xMax, h._yMax);
}


/// Sets outer upper limits for x and y.
void CoordinateHandler2D::setLimits(int xUpperLimit,int yUpperLimit){
	setLimits(0, 0, xUpperLimit, yUpperLimit);
}

/// Sets minimum values and outer upper limits for x and y.
void CoordinateHandler2D::setLimits(int xMin, int yMin, int xUpperLimit, int yUpperLimit){

	// todo EPSILON<T>

	_xMin = xMin;
	_xUpperLimit = xUpperLimit;
	_xMax = xUpperLimit - 1;

	_yMin = yMin;
	_yUpperLimit = yUpperLimit;
	_yMax = yUpperLimit - 1;

}


void CoordinateHandler2D::setPolicy(int xUnderFlowPolicy, int yUnderFlowPolicy, int xOverFlowPolicy, int yOverFlowPolicy){

	Logger mout(getImgLog(), "CoordinateHandler2D", __FUNCTION__);

	// For bookkeeping
	CoordinatePolicy::set(xUnderFlowPolicy, yUnderFlowPolicy, xOverFlowPolicy, yOverFlowPolicy);
	/*
	policy.xUnderFlowPolicy = xUnderFlowPolicy;
	policy.yUnderFlowPolicy = yUnderFlowPolicy;
	policy.xOverFlowPolicy  = xOverFlowPolicy;
	policy.yOverFlowPolicy  = yOverFlowPolicy;
	*/

	switch (xUnderFlowPolicy){
	case LIMIT:
		_handleXUnderFlow = & CoordinateHandler2D::handleXUnderFlow_Limit;
		break;
	case WRAP:
		_handleXUnderFlow = & CoordinateHandler2D::handleXUnderFlow_Wrap;
		break;
	case MIRROR:
		_handleXUnderFlow = & CoordinateHandler2D::handleXUnderFlow_Mirror;
		break;
	case POLAR:
		_handleXUnderFlow = & CoordinateHandler2D::handleXUnderFlow_Polar;
		break;
	default:
		mout.warn() << "undefined xUnderFlowPolicy" << xUnderFlowPolicy << mout.endl;
		_handleXUnderFlow = & CoordinateHandler2D::handle_Undefined;
		break;
	}

	switch (yUnderFlowPolicy){
	case LIMIT:
		_handleYUnderFlow = & CoordinateHandler2D::handleYUnderFlow_Limit;
		break;
	case WRAP:
		_handleYUnderFlow = & CoordinateHandler2D::handleYUnderFlow_Wrap;
		break;
	case MIRROR:
		_handleYUnderFlow = & CoordinateHandler2D::handleYUnderFlow_Mirror;
		break;
	case POLAR:
		_handleYUnderFlow = & CoordinateHandler2D::handleYUnderFlow_Polar;
		break;
	default:
		mout.warn() << "undefined yUnderFlowPolicy" << yUnderFlowPolicy<< mout.endl;
		_handleYUnderFlow = & CoordinateHandler2D::handle_Undefined;
		break;
	}

	switch (xOverFlowPolicy){
	case LIMIT:
		_handleXOverFlow = & CoordinateHandler2D::handleXOverFlow_Limit;
		break;
	case WRAP:
		_handleXOverFlow = & CoordinateHandler2D::handleXOverFlow_Wrap;
		break;
	case MIRROR:
		_handleXOverFlow = & CoordinateHandler2D::handleXOverFlow_Mirror;
		break;
	case POLAR:
		_handleXOverFlow = & CoordinateHandler2D::handleXOverFlow_Polar;
		break;
	default:
		mout.warn() << "undefined xOverFlowPolicy" << xOverFlowPolicy << mout.endl;
		_handleXOverFlow = & CoordinateHandler2D::handle_Undefined;
		break;
	}

	switch (yOverFlowPolicy){
	case LIMIT:
		_handleYOverFlow = & CoordinateHandler2D::handleYOverFlow_Limit;
		break;
	case WRAP:
		_handleYOverFlow = & CoordinateHandler2D::handleYOverFlow_Wrap;
		break;
	case MIRROR:
		_handleYOverFlow = & CoordinateHandler2D::handleYOverFlow_Mirror;
		break;
	case POLAR:
		_handleYOverFlow = & CoordinateHandler2D::handleYOverFlow_Polar;
		break;
	default:
		mout.warn() << "undefined yOverFlowPolicy" << yOverFlowPolicy << mout.endl;
		_handleYOverFlow = & CoordinateHandler2D::handle_Undefined;
	}

}

std::ostream & operator<<(std::ostream & ostr, const CoordinatePolicy & policy){
	ostr << "policy[" << policy.xUnderFlowPolicy << ',' << policy.yUnderFlowPolicy << ',' ;
	ostr << policy.xOverFlowPolicy << ',' << policy.yOverFlowPolicy << ']';
	return ostr;
}


std::ostream & operator<<(std::ostream & ostr, const CoordinateHandler2D & handler){
	ostr << "handler[" << handler.getXMax() << ',' << handler.getYMax()<< ']';
	ostr << ' ' << handler.getPolicy();
	return ostr;
}

} // image

} // drain

// Drain
