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
#ifndef DISTANCEMODEL_H_
#define DISTANCEMODEL_H_



#include <math.h>


#include "ImageOp.h"


namespace drain
{

namespace image
{


class DistanceModel {
	
public:

	/// Base class for digital distance models. Supports 4-, 8- and extended "chess knight" distance.
	/** Sets the horizontal and vertical radius for the distance function.
	 *  It is recommended to make \c vert and \c diag optional. ie. apply default values.
	 *  By default, the geometry is octagonal, applyin 8-distance, but setting diag 
	 *  a large value makes it square (4-distance, diamond). 
	*/
	DistanceModel(){
		setMax(255);
		//setRadius(10.0,10.0,10.0);
	};

	typedef float dtype;

	virtual ~DistanceModel(){};

	inline
	void setMax(dtype max){ this->max = max; };

	inline
	size_t getMax() const { return max; };

	// virtual
	//void getRadius(dtype & horz, dtype & vert) const = 0;

	/// Sets the geometry of the distance model as a vanishing distance.
	/** The values are in float, as eg. half-width radius may be sharp, under 1.0.
	 *   \param horz - horizontal distance scale
	 *   \param vert - vertical radius; if negative, will be set same as the horizontal one
	 *   \param diag - diagonal radius; if negative, will be set to sqrt((h*h+v*v)/2)
	 *   \param knight - if true, chess knight decrements will be computed
	 */
	virtual 
	void setRadius(dtype horz, dtype vert, bool diag=true, bool knight=true) = 0;
	
	/// Alternative for setRadii(). Set the decrements or decays directly.
	virtual 
	void setDecrement(dtype horz,dtype vert, bool diag=true, bool knight=true) = 0;

	/// Decreases value by horizonal decay
	virtual
	dtype decreaseHorz(const dtype &x) const = 0;

	/// Decreases value by vertical decay
	virtual 
	dtype decreaseVert(const dtype &x) const = 0;

	/// Decreases value by diagonal decay
	virtual
	dtype decreaseDiag(const dtype &x) const = 0;

	/// Decreases value by a chess knight step (2-up, 1-left) decay
	virtual 
	dtype decreaseKnightHorz(const dtype &x) const = 0;

	/// Decreases value by a chess knight step (2-up, 1-left) decay
	virtual
	dtype decreaseKnightVert(const dtype &x) const = 0;

	/// Chess knight move computation on/off.
	bool KNIGHT;

protected:

	/// Needed internally to get diag decrement larger than horz/vert decrements. (Not used for scaling).
	dtype max;


};


class DistanceModelLinear : public DistanceModel
{

public:

	void setDecrement(dtype horz, dtype vert = -1, bool diag=true, bool knight=true);

	void setRadius(dtype horz, dtype vert = -1, bool diag=true, bool knight=true);
	

	
	inline
	dtype decreaseHorz(const dtype &x) const {
		if (x > horzDecrement)
			return x-horzDecrement;
		else
			return 0;
	};

	inline
	dtype decreaseVert(const dtype &x) const {
		if (x > vertDecrement)
			return x-vertDecrement;
		else
			return 0;
	};

	inline
	dtype decreaseDiag(const dtype &x) const {
		if (x > diagDecrement)
			return x-diagDecrement;
		else
			return 0;
	};

	inline
	dtype decreaseKnightHorz(const dtype &x) const {
		if (x > knightDecrementHorz)
			return x-knightDecrementHorz;
		else
			return 0;
	};

	inline
	dtype decreaseKnightVert(const dtype &x) const {
		if (x > knightDecrementVert)
			return x-knightDecrementVert;
		else
			return 0;
	};

private:
	
	//T max;
	dtype horzDecrement;
	dtype vertDecrement;
	dtype diagDecrement;
	dtype knightDecrementHorz;
	dtype knightDecrementVert;
	
};

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

	void setRadius(dtype horz, dtype vert = -1, bool diag=true, bool knight=true);

	void setDecrement(dtype horz, dtype vert = -1, bool diag=true, bool knight=true);

	inline
	dtype decreaseHorz(const dtype &x) const {
		return static_cast<dtype>(horzDecay*x);
		//return (horzDecay10*x)>>10;
	};

	inline
	dtype decreaseVert(const dtype &x) const {
		return static_cast<dtype>(vertDecay*x);
	};

	inline
	dtype decreaseDiag(const dtype &x) const {
		return (diagDecay*x);
	};

	inline
	dtype decreaseKnightHorz(const dtype &x) const {
		return (knightDecayHorz*x);
	};

	inline
	dtype decreaseKnightVert(const dtype &x) const {
		return (knightDecayVert*x);
	};

private:
	
	dtype horzDecay;
    dtype vertDecay;
    dtype diagDecay;
    dtype knightDecayHorz;
    dtype knightDecayVert;

};



}
}
	
#endif /*DISTANCETRANSFORMOP_H_*/

// Drain
