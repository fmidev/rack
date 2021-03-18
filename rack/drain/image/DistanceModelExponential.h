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
#ifndef DISTANCEMODEL_EXP_H_
#define DISTANCEMODEL_EXP_H_



#include <math.h>

#include "DistanceModel.h"


namespace drain
{

namespace image
{

/**
 *  In this model, the diagonal decay is computed as
 *  \$ d_d = \log - \sqrt { \log^2 d_x + \log^2 d_x)) } ;
 *  \$
 *  because
 *  \$ d_d = \log - \sqrt { \log^2 d_x + \log^2 d_x)) } ;
 *  \$
 */
class DistanceModelExponential : public DistanceModel {

public:

	DistanceModelExponential() : DistanceModel(__FUNCTION__){
		setRadius(horzRadius.forward, vertRadius.forward);
	};

	void setRadius(float horz, float vert = nan_f, float horzLeft = nan_f, float vertUp = nan_f);

	/// \see DistanceModel::setDecrement()
	void setDecrement(float horz, float vert = nan_f, float horzRight = nan_f, float vertUp = nan_f);

	virtual
	DistanceElement getElement(short dx, short dy, bool forward=true) const;

	// NEW
	virtual inline
	float decrease(float value, float coeff) const {
		return value * coeff;
	};

protected:

	float radius2Dec(float r, float rDefault = -1.0) const;

	float checkDec(float d, float dDefault = 0.75) const;

};



}
}
	
#endif

/*
	inline
	float decreaseHorz(float x) const {
		return static_cast<float>(horzDecay*x);
	};

	inline
	float decreaseVert(float x) const {
		return static_cast<float>(vertDecay*x);
	};

	inline
	float decreaseDiag(float x) const {
		return (diagDecay*x);
	};

	inline
	float decreaseKnightHorz(float x) const {
		return (knightDecayHorz*x);
	};

	inline
	float decreaseKnightVert(float x) const {
		return (knightDecayVert*x);
	};
*/

/*
	float horzDecay;
    float vertDecay;
    float diagDecay;
    float knightDecayHorz;
    float knightDecayVert;
*/

// Drain
