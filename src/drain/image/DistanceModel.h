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

#include "drain/util/BeanLike.h"
#include "drain/imageops/ImageOp.h"

#include "drain/UniTuple.h"


namespace drain
{

template <typename T>
class Bidirectional : public drain::UniTuple<T,2> {
public:

	T & forward;
	T & backward;

	/// Default constructor.
	Bidirectional(T forward=T(), T backward=T()) : forward(this->at(0)), backward(this->at(1)) {
		this->set(forward, backward);
	};

	/// Copy constructor.
	Bidirectional(const Bidirectional & r) : forward(this->at(0)),  backward(this->at(1)) {
		this->set(r.forward, r.backward);
	};

	virtual inline
	void updateTuple(){
		if (isnan(backward))
			backward = forward;
	};


};


namespace image
{


class DistanceElement {
public:

	// Todo: sync with DistanceModel::float
	// typedef float float;

	/// A small digital offset, for example {+1,-1} or {-2,+1}
	const drain::Point2D<short int> diff;

	// Saves connection even with changing weights
	const float coeff;

	inline
	DistanceElement(short int di, short int dj, const float coeff) : diff(di,dj), coeff(coeff) {
	};

	inline
	DistanceElement(const DistanceElement & elem) : diff(elem.diff), coeff(elem.coeff) {
	};

	virtual
	~DistanceElement(){
	};


};

inline
std::ostream & operator<<(std::ostream &ostr, const DistanceElement & d){
	//ostr.setf(std::ostream::showpos);
	ostr << std::showpos << d.diff << std::noshowpos << ": " << d.coeff;
	return ostr;
}


typedef std::vector<DistanceElement> DistanceNeighbourhood; // faster...

inline
std::ostream & operator<<(std::ostream &ostr, const DistanceNeighbourhood & chain){
	for (const DistanceElement & element: chain){
		ostr << element << '\n';
	}
	return ostr;
}


/// Base class for linear and exponential distances in rectangular pixel images.
class DistanceModel : public BeanLike {


public:

	enum PIXEL_ADJACENCY {CONN_UNSET=0, CONN4=4, CONN8=8, KNIGHT=16};

	typedef PIXEL_ADJACENCY topol_t; // Needed? Consider topol_enum_t


	typedef drain::EnumFlagger<drain::SingleFlagger<PIXEL_ADJACENCY> > PixelAdjacencyFlagger;
	PixelAdjacencyFlagger pixelAdjacency;

	static
	const float nan_f; // = std::numeric_limits<float>::quiet_NaN();

	virtual ~DistanceModel(){};

	/// Set maximum (expected) code value. Radii, if given, will set pixel-to-pixel decrements scaled to this value.
	inline
	void setMax(float maxCodeValue){
		this->maxCodeValue = maxCodeValue;
	};

	/// Returns the maximum (expected) code value.
	inline
	float getMax() const {
		return maxCodeValue;
	};

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
	void setRadius(float horz, float vert=nan_f, float horzLeft=nan_f, float vertUp=nan_f) = 0;

	/// Simply copies values, does not try to set defaults.
	inline
	void setRadiusVerbatim(float horz, float vert=nan_f, float horzLeft=nan_f, float vertUp=nan_f){
		horzRadius.set(horz, horzLeft);
		vertRadius.set(vert, vertUp);
	};

	inline
	const Bidirectional<float> & getRadiusHorz() const {
		return horzRadius;
	}

	inline
	const Bidirectional<float> & getRadiusVert() const {
		return vertRadius;
	}

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

	void update();

	virtual
	void updateBean() const override;

	/// Sets the topology of the computation grid: 0=diamond, 1=diagonal, 2=extended (chess knight steps)
	inline
	void setTopology(PIXEL_ADJACENCY topology){
		pixelAdjacency.set(topology);
		updateBean();
	};

	inline
	void setTopology(const std::string & topology){
		pixelAdjacency.set(topology);
		updateBean();
	};


	/// Creates a list of DistanceElements
	/**
	 *   Diagonal (dx+dy) and "chess knight" (2dx+dy or dx+2dy) values will be adjusted as well.
	 *
	 */
	// static
	void createChain(DistanceNeighbourhood & chain, PIXEL_ADJACENCY topology = PIXEL_ADJACENCY::KNIGHT, bool forward=true) const;

	inline
	void createChain(DistanceNeighbourhood & chain, bool forward=true) const {
		createChain(chain, pixelAdjacency.getValue(), forward);
	}

	virtual
	float decrease(float value, float coeff) const = 0;


protected:

	/// Base class for digital distance models. Supports 4-, 8- and extended "chess knight" distance.
	/**
	 *  Sets the horizontal and vertical radius for the distance function.
	 *  The parameters are called "width" and "height"
	 *
	 *  By default, the geometry is octagonal, applying 8-distance.
	*/
	DistanceModel(const std::string & name, const std::string & description = "") : BeanLike(name, description), horzRadius(11.0, nan_f), vertRadius(nan_f, nan_f) {
	// horzRadius(11.0, 12.0), vertRadius(-1.0, -1.0) {
	parameters.link("width",  horzRadius.tuple(), "pix").fillArray = true;
		parameters.link("height", vertRadius.tuple(), "pix").fillArray = true;
		parameters.link("topology", pixelAdjacencyStr = "16-CONNECTED", sprinter(EnumDict<DistanceModel::PIXEL_ADJACENCY>::dict.getKeys()).str());
		//drain::sprinter(drain::EnumDict<DataOrder::Oper>::dict.getKeys()).str()
		// ? update();
		setMax(255); // warning
		// drain::Logger mout(getImgLog(), __FUNCTION__, getName());
		// mout.warn(*this );
	};

	DistanceModel(const DistanceModel & dm) : BeanLike(dm), horzRadius(dm.horzRadius), vertRadius(dm.vertRadius){
		parameters.copyStruct(dm.getParameters(), dm, *this);
		// setTopology(dm.topology);
		setMax(dm.getMax()); // warning
	}

	mutable
	std::string pixelAdjacencyStr;  // number to string


	//  Horizontal distance(s); right and left steepness radii. Internal parameter applied upon initParams?
	drain::Bidirectional<float> horzRadius;

	//  Vertical distance(s); right and left steepness radii. Internal parameter applied upon initParams?
	drain::Bidirectional<float> vertRadius;


	/// Final decrement or decay per pixel in horizontal direction. Derived from horzRadius; definition varies in subclasses.
	drain::Bidirectional<float> horzDec;

	/// Final decrement or decay	per pixel in vertical direction. Derived from vertRadius; definition varies in subclasses.
	drain::Bidirectional<float> vertDec;

	/// Needed internally to get diag decrement larger than horz/vert decrements. (Not used for scaling).
	float maxCodeValue;



};


}
}
	
#endif
// Drain
