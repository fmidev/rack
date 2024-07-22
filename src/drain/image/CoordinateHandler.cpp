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

#include <drain/Log.h>
#include "CoordinateHandler.h"


namespace drain {

namespace image {


//template <>
const drain::FlagResolver::dict_t CoordinateHandler2D::dict = {
		{"X_OVERFLOW",  X_OVERFLOW},
		{"X_UNDERFLOW", X_UNDERFLOW},
		{"Y_OVERFLOW",  Y_OVERFLOW},
		{"Y_UNDERFLOW", Y_UNDERFLOW},
		{"IRREVERSIBLE", IRREVERSIBLE}
};


const coord_overflow_t  CoordinateHandler2D::UNCHANGED; // = 0;
const coord_overflow_t  CoordinateHandler2D::X_OVERFLOW; // = coord_overflow_flagger_t::addEntry("X_OVERFLOW");
const coord_overflow_t  CoordinateHandler2D::X_UNDERFLOW; //= coord_overflow_flagger_t::addEntry("X_UNDERFLOW"); // = 2;
const coord_overflow_t  CoordinateHandler2D::Y_OVERFLOW; // = coord_overflow_flagger_t::addEntry("Y_OVERFLOW");  // = 4;
const coord_overflow_t  CoordinateHandler2D::Y_UNDERFLOW; //= coord_overflow_flagger_t::addEntry("Y_UNDERFLOW"); //= 8;
const coord_overflow_t  CoordinateHandler2D::IRREVERSIBLE; //=coord_overflow_flagger_t::addEntry("IRREVERSIBLE"); //= 128



void CoordinateHandler2D::setPolicy(EdgePolicy::index_t xUnderFlowPolicy, EdgePolicy::index_t yUnderFlowPolicy, EdgePolicy::index_t xOverFlowPolicy, EdgePolicy::index_t yOverFlowPolicy){

	Logger mout(getImgLog(), "CoordinateHandler2D", __FUNCTION__);

	// For bookkeeping
	policy.set(xUnderFlowPolicy, yUnderFlowPolicy, xOverFlowPolicy, yOverFlowPolicy);

	//switch (xUnderFlowPolicy){
	if (xUnderFlowPolicy ==  EdgePolicy::LIMIT)
		handleXUnderFlow = & CoordinateHandler2D::limitXUnderFlow;
		// break;
	else if (xUnderFlowPolicy ==  EdgePolicy::WRAP)
		handleXUnderFlow = & CoordinateHandler2D::wrapXUnderFlow;
		// break;
	else if (xUnderFlowPolicy ==  EdgePolicy::MIRROR)
		handleXUnderFlow = & CoordinateHandler2D::mirrorXUnderFlow;
		// break;
	else if (xUnderFlowPolicy ==  EdgePolicy::POLAR)
		handleXUnderFlow = & CoordinateHandler2D::polarXUnderFlow;
		// break;
	else // if (xUnderFlowPolicy ==  EdgePolicy::UNDEFINED)
	{  // default:
		mout.warn("undefined xUnderFlowPolicy" , xUnderFlowPolicy );
		handleXUnderFlow = & CoordinateHandler2D::skipUndefined;
		// break;
	}

	//switch (yUnderFlowPolicy){
	if (yUnderFlowPolicy ==  EdgePolicy::LIMIT)
		handleYUnderFlow = & CoordinateHandler2D::limitYUnderFlow;
		// break;
	else if (yUnderFlowPolicy ==  EdgePolicy::WRAP)
		handleYUnderFlow = & CoordinateHandler2D::wrapYUnderFlow;
		// break;
	else if (yUnderFlowPolicy ==  EdgePolicy::MIRROR)
		handleYUnderFlow = & CoordinateHandler2D::mirrorYUnderFlow;
		// break;
	else if (yUnderFlowPolicy ==  EdgePolicy::POLAR)
		handleYUnderFlow = & CoordinateHandler2D::polarYUnderFlow;
		// break;
	else // if (yUnderFlowPolicy ==  EdgePolicy::UNDEFINED)
	{  // default:
		mout.warn("undefined yUnderFlowPolicy" , yUnderFlowPolicy);
		handleYUnderFlow = & CoordinateHandler2D::skipUndefined;
		// break;
	}

	//switch (xOverFlowPolicy){
	if (xOverFlowPolicy ==  EdgePolicy::LIMIT)
		handleXOverFlow = & CoordinateHandler2D::limitXOverFlow;
		// break;
	else if (xOverFlowPolicy ==  EdgePolicy::WRAP)
		handleXOverFlow = & CoordinateHandler2D::wrapXOverFlow;
		// break;
	else if (xOverFlowPolicy ==  EdgePolicy::MIRROR)
		handleXOverFlow = & CoordinateHandler2D::mirrorXOverFlow;
		// break;
	else if (xOverFlowPolicy ==  EdgePolicy::POLAR)
		handleXOverFlow = & CoordinateHandler2D::polarXOverFlow;
		// break;
	else  // if ( ==  EdgePolicy::UNDEFINED)
	{  // default:
		mout.warn("undefined xOverFlowPolicy" , xOverFlowPolicy );
		handleXOverFlow = & CoordinateHandler2D::skipUndefined;
		// break;
	}

	//switch (yOverFlowPolicy){
	if (yOverFlowPolicy ==  EdgePolicy::LIMIT)
		handleYOverFlow = & CoordinateHandler2D::limitYOverFlow;
		// break;
	else if (yOverFlowPolicy ==  EdgePolicy::WRAP)
		handleYOverFlow = & CoordinateHandler2D::wrapYOverFlow;
		// break;
	else if (yOverFlowPolicy ==  EdgePolicy::MIRROR)
		handleYOverFlow = & CoordinateHandler2D::mirrorYOverFlow;
		// break;
	else if (yOverFlowPolicy ==  EdgePolicy::POLAR)
		handleYOverFlow = & CoordinateHandler2D::polarYOverFlow;
		// break;
	else // if ( ==  EdgePolicy::UNDEFINED)
	{  // default:
		mout.warn("undefined yOverFlowPolicy" , yOverFlowPolicy );
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
