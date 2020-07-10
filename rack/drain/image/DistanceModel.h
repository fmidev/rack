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

#include "util/BeanLike.h"

#include "imageops/ImageOp.h"


namespace drain
{

namespace image
{


class DistanceElement {
public:

	// Todo: sync with DistanceModel::float
	//typedef float float;

	const drain::Point2D<short int> diff;

	inline
	DistanceElement(short int di, short int dj, const float coeff) : diff(di,dj), coeff(coeff) {
	};

	inline
	DistanceElement(const DistanceElement & elem) : diff(elem.diff), coeff(elem.coeff) {
	};

	virtual
	~DistanceElement(){
	};

	// Saves connection even with changing weights
	const float coeff;

};

inline
std::ostream & operator<<(std::ostream &ostr, const DistanceElement & d){
	ostr << d.diff << ':' << d.coeff;
	return ostr;
}


typedef std::list<DistanceElement> DistanceNeighbourhood;


inline
std::ostream & operator<<(std::ostream &ostr, const DistanceNeighbourhood & chain){
	for (DistanceNeighbourhood::const_iterator it=chain.begin(); it!=chain.end(); ++it){
		ostr << *it << '\n';
	}
	return ostr;
}


/// Base class for linear and exponential distances in rectangular pixel images.
class DistanceModel : public BeanLike {
	
public:

	static
	const float nan_f; // = std::numeric_limits<float>::quiet_NaN();
	//typedef float float;

	virtual ~DistanceModel(){};

	/// Set maximum (expected) code value. Radii, if given, will set pixel-to-pixel decrements scaled to this value.
	inline
	void setMax(float maxCodeValue){ this->maxCodeValue = maxCodeValue; };

	/// Returns the maximum (expected) code value.
	inline
	float getMax() const { return maxCodeValue; };

	// virtual
	//void getRadius(float & horz, float & vert) const = 0;

	/// Sets the geometry of the distance model.
	/**
	 * 	The values are in float, as eg. half-width radius may be sharp, under 1.0.
	 *   \param horz - horizontal distance, "width".
	 *   \param vert - vertical distance, "height"; if negative, set equal to horizontal
	 *
	 *   Special values:
	 *   - NaN: use default value (\c horz for \c vert and \c horzRight; \c vert for \vertUp )
	 *   - negative value: no decay; continue to infinity with constant value
	 *   - zero: sharp decay, value is zero in neighboring pixels
	 *
	 */
	virtual 
	void setRadius(float horz, float vert=nan_f, float horzLeft=nan_f, float vertUp=nan_f) = 0; //, bool diag=true, bool knight=true) = 0;
	
	/// Set the distance geometry directly by modifying decrement/decay coefficients. Alternative to setRadus().
	///
	/**
	 *  Set final decrement or decay per pixel. The exact definition varies in subclasses; the decay may be linear
	 *  decrement as with DistanceModelLinear or multiplicative as with DistanceModelExponential .
	 *
	 *   \param horz     - horizontal distance decrement/decay
	 *   \param vert     - vertical   distance decrement/decay; if NaN, set equal to \c horz
	 *   \param horzLeft - horizontal distance decrement/decay; if NaN, set equal to \c horz
	 *   \param vertUp   - vertical   distance decrement/decay; if NaN, set equal to \c vert
	 *
	 *   Special values:
	 *   - NaN: use default value
	 *   - negative value: no decay; continue to infinity with constant value
	 *   - zero: sharp decay, value is zero in neighboring pixels
	 *
	 *
	 */
	virtual 
	void setDecrement(float horz, float vert = NAN, float horzRight = NAN, float vertUp = NAN) = 0; // , bool diag=true, bool knight=true) = 0;

	virtual
	DistanceElement getElement(short dx, short dy, bool forward=true) const = 0;

	inline
	void update(){
		//setTopology(topology);
		setRadius(widths[0], heights[0], widths[1], heights[1]);
	}

	/// Sets the topology of the computation grid: 0=diamond, 1=diagonal, 2=extended (chess knight steps)
	inline
	void setTopology(unsigned short topology){
		this->topology = topology;
	};

	/// Creates a list of DistanceElements
	/**
	 *   Diagonal (dx+dy) and "chess knight" (2dx+dy or dx+2dy) values will be adjusted as well.
	 *
	 */
	virtual
	void createChain(DistanceNeighbourhood & chain, unsigned short topology, bool forward=true) const;

	virtual
	float decrease(float value, float coeff) const = 0;

	unsigned short int topology; // NEEDED, separately?


protected:

	/// Base class for digital distance models. Supports 4-, 8- and extended "chess knight" distance.
	/**
	 *  Sets the horizontal and vertical radius for the distance function.
	 *  The parameters are called "width" and "height"
	 *
	 *  By default, the geometry is octagonal, applying 8-distance.
	*/
	DistanceModel(const std::string & name, const std::string & description = "") : BeanLike(name, description), widths(2, 10.0), heights(2, -1.0) {
		parameters.reference("width",  widths,  "pix").fillArray = true;
		parameters.reference("height", heights, "pix").fillArray = true;
		parameters.reference("topology", topology=2, "0|1|2");
		setMax(255); // warning
		// drain::Logger mout(getImgLog(), __FUNCTION__, getName());
		// mout.warn() << *this << mout.endl;
	};

	DistanceModel(const DistanceModel & dm) : BeanLike(dm.name, dm.description), widths(dm.widths), heights(dm.heights){
		parameters.reference("width",  widths,  "pix").fillArray = true;
		parameters.reference("height", heights, "pix").fillArray = true;
		parameters.reference("topology", topology=2, "0|1|2");
		setMax(255); // warning
	}


	/// parameters["width"], two elements
	//  Horizontal distance(s); right (and left) "radius". Internal parameter applied upon initParams?
	std::vector<float> widths;

	/// parameters["height"], two elements
	//  Vertical distance(s); downward (and upward) "radius". Internal parameter applied upon initParams
	std::vector<float> heights;

	/// Final decrement or decay per pixel in horizontal direction. Derived from widths; definition varies in subclasses.
	float horzDec;
	float horzDec2;
	/// Final decrement or decay	per pixel in vertical direction. Derived from heights; definition varies in subclasses.
    float vertDec;
    float vertDec2;

	/// Needed internally to get diag decrement larger than horz/vert decrements. (Not used for scaling).
	float maxCodeValue;

};


}
}
	
#endif /*DISTANCETRANSFORMOP_H_*/

/*
		KNIGHT = false;
		DIAG   = false;

		switch (topology) {
			case 2:
				KNIGHT = true;
				// no break
			case 1:
				DIAG   = true;
				// no break
			case 0:
				break;
			default:
				break;
		}
		*/

/*
/// Decreases value by horizonal decay
virtual
float decreaseHorz(float x) const = 0;

/// Decreases value by vertical decay
virtual
float decreaseVert(float x) const = 0;

/// Decreases value by diagonal decay
virtual
float decreaseDiag(float x) const = 0;

/// Decreases value by a chess knight step (2-up, 1-left) decay
virtual
float decreaseKnightHorz(float x) const = 0;

/// Decreases value by a chess knight step (2-up, 1-left) decay
virtual
float decreaseKnightVert(float x) const = 0;
*/
/*
	inline
	float decreaseHorz(float x) const {
		if (x > horzDecrement)
			return x-horzDecrement;
		else
			return 0;
	};

	inline
	float decreaseVert(float x) const {
		if (x > vertDecrement)
			return x-vertDecrement;
		else
			return 0;
	};

	inline
	float decreaseDiag(float x) const {
		if (x > diagDecrement)
			return x-diagDecrement;
		else
			return 0;
	};

	inline
	float decreaseKnightHorz(float x) const {
		if (x > knightDecrementHorz)
			return x-knightDecrementHorz;
		else
			return 0;
	};

	inline
	float decreaseKnightVert(float x) const {
		if (x > knightDecrementVert)
			return x-knightDecrementVert;
		else
			return 0;
	};
	*/

//T max;
/*
float horzDecrement;
float vertDecrement;
float diagDecrement;
float knightDecrementHorz;
float knightDecrementVert;
*/



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
/*
/// Diagnonal +1/-1 steps on/off.
bool DIAG;

/// Chess knight move computation on/off.
bool KNIGHT;
*/

// Drain
