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
#ifndef DRAIN_BOUNDING_BOX_H_
#define DRAIN_BOUNDING_BOX_H_

#include <cmath>
#include "Rectangle.h"
#include "Units.h"

namespace drain
{


/// Container for geographical extent spanned by lowerLeft(x,y) and upperRight(x,y). Assumes longitude=x and latitude=y .
/**
 *
 *
 */
class BBox : public Rectangle<double> {
public:

	inline
	BBox(){
	}

	inline
	BBox(const BBox & bbox) : Rectangle<double>(bbox){
	}

	inline
	BBox(const Rectangle<double> & bbox) : Rectangle<double>(bbox){
	}

	/*
	inline
	BBox(const UniTuple<double,4> & bbox) : Rectangle<double>(bbox){
	}
	*/


	/// Checks if a coordinate (x,y) == (lon,lat) looks like metric, that is, beyond [-90,+90] or [-180,+180]
	/** This simple heuristic assumes that user does not need geographical areas of 360m × 180m or smaller.
	 *
	 */
	static inline
	bool isMetric(const Point2D<double> & p){
		return isMetric(p.x, 180.0) || isMetric(p.y, 90.0);
	}


	/// Checks if a coordinate looks like metric, that is, beyond [-90,+90] or [-180,+180]
	static inline
	bool isMetric(double x, double limit){
		return (x < -limit) || (x > limit);
	}



	/// Check if this Bounding Box has metric coordinates, instead of degrees.
	/**
	 *
	 */
	inline
	bool isMetric() const {
		return isMetric(lowerLeft) || isMetric(upperRight);
	}


	static inline
	Unit guessUnit(double x, double y){
		/*
		if (isWithin(x, -1.0,+1.0)){
			return Unit::RADIAN;
		}
		*/
		if (isWithin(x, -M_PI,+M_PI) && isWithin(y, -M_PI,+M_PI)){
			return Unit::RADIAN;
		}
		else if (isWithin(x, -180.0,+180.0) && isWithin(y, -90.0,+90.0)){
			return Unit::DEGREE;
		}
		else {
			return Unit::METRE;
		}

	}

	static inline
	bool isWithin(double x, double min, double max){
		return (x > min) && (x < max);
	}


};



}  // drain

#endif /* DRAIN_BOUNDING_BOX_H_ */
