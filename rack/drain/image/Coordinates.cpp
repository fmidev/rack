/**

    Copyright 2011-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

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

/*
CoordinatePolicy::CoordinatePolicy(int policy) : xUnderFlowPolicy(policy), xOverFlowPolicy(policy),
		yUnderFlowPolicy(policy), yOverFlowPolicy(policy) {
}

CoordinatePolicy::CoordinatePolicy(int xUnderFlow, int yUnderFlow, int xOverFlow, int yOverFlow) :
		xUnderFlowPolicy(xUnderFlow), xOverFlowPolicy(xOverFlow),
		yUnderFlowPolicy(yUnderFlow), yOverFlowPolicy(yOverFlow) {
}
*/

/*
void CoordinatePolicy::setPolicy(const int & xUnderFlowPolicy, const int & yUnderFlowPolicy,  const int & xOverFlowPolicy, const int & yOverFlowPolicy){
	this->xUnderFlowPolicy = xUnderFlowPolicy;
	this->yUnderFlowPolicy = yUnderFlowPolicy;
	this->xOverFlowPolicy  = xOverFlowPolicy;
	this->yOverFlowPolicy  = yOverFlowPolicy;
}
*/


CoordinateHandler2D::CoordinateHandler2D(){
	setPolicy(CoordinatePolicy()); // turha
	setLimits(1,1);
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

	Logger mout("CoordinateHandler2D", __FUNCTION__);

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
