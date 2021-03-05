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
#ifndef DISTANCEMODEL_LIN_H_
#define DISTANCEMODEL_LIN_H_



#include <math.h>

#include "DistanceModel.h"


namespace drain
{

namespace image
{


class DistanceModelLinear : public DistanceModel
{

public:

	DistanceModelLinear() : DistanceModel(__FUNCTION__){
		//setRadius(widths[0], heights[0]);
	};

	/// \see DistanceModel::setRadius()
	void setRadius(float horz, float vert = nan_f, float horzLeft = nan_f, float vertUp = nan_f);

	/// \see DistanceModel::setDecrement()
	void setDecrement(float horz, float vert = nan_f, float horzRight = nan_f, float vertUp = nan_f);

	virtual
	DistanceElement getElement(short dx, short dy, bool forward=true) const;

	// NEW
	virtual inline
	float decrease(float value, float coeff) const {
		return value - coeff;
	};
	
protected:

	float radius2Dec(float r, float rDefault = -1.0) const;

	float checkDec(float d, float dDefault = 1.0) const;


};




}
}
	
#endif /*DISTANCETRANSFORMOP_H_*/





// Drain
