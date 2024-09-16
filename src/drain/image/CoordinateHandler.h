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

#ifndef COORD_HANDLER_H_
#define COORD_HANDLER_H_

#include <drain/image/CoordinatePolicy.h>
#include <drain/Log.h>
#include <ostream>
#include <stdexcept>

#include "drain/util/Flags.h"
#include "drain/util/Frame.h"
#include "Geometry.h"

#include "ImageFrame.h"
#include "Direction.h" //~ "Position.h", experimental

namespace drain {


namespace image {




typedef drain::GlobalFlags<CoordinatePolicy> coord_overflow_flagger_t;
typedef drain::Flags::ivalue_t coord_overflow_t;
//enum coord_overflow_t {UNCHANGED=0, X_OVERFLOW=1, X_UNDERFLOW=2, Y_OVERFLOW=4, Y_UNDERFLOW=8, IRREVERSIBLE=128};

//template <>
//const drain::SingleFlagger<coord_overflow_t>::dict_t drain::SingleFlagger<coord_overflow_t>::dict;

class CoordinateHandler2D { //: protected CoordinatePolicy {

public:

	static const drain::FlaggerBase<size_t>::dict_t dict;

	static const coord_overflow_t UNCHANGED   = 0; // = 0;
	static const coord_overflow_t X_OVERFLOW  = 1; //  = 1;
	static const coord_overflow_t X_UNDERFLOW = 2; // = 2;
	static const coord_overflow_t Y_OVERFLOW  = 4; //  = 4;
	static const coord_overflow_t Y_UNDERFLOW = 8; //  8;
	/// Equal move in inverse direction would not result original position.
	static const coord_overflow_t IRREVERSIBLE = 128; // = 128;



	CoordinatePolicy policy;
	drain::Frame2D<int> area;


	/// Default constructor
	inline
	CoordinateHandler2D(const drain::image::AreaGeometry & area = AreaGeometry(), const CoordinatePolicy &policy = CoordinatePolicy()){
		set(area, policy);
	}

	// Copy constructor
	inline
	CoordinateHandler2D(const CoordinateHandler2D &handler){
		// setLimits(handler.xRange.min, handler.yRange.min, handler.xRange.max+1, handler.yRange.max+1); // +1
		// setLimits(handler.xRange.min, handler.yRange.min, handler.xRange.max+1, handler.yRange.max+1); // +1
		xRange.set(handler.getXRange());
		yRange.set(handler.getYRange());
		area.set(handler.area);
		setPolicy(handler.getPolicy());
	}

	//
	inline
	CoordinateHandler2D(const ImageFrame & src){
		set(src.getGeometry().area, src.getCoordinatePolicy());
	}


	/// Constructor
	/**
	 *  \par xUpperLimit - value one greater than maximum allowed value
	 *  \par xUpperLimit - value one greater than maximum allowed value
	 */
	inline
	CoordinateHandler2D(int xUpperLimit, int yUpperLimit, const CoordinatePolicy &policy = CoordinatePolicy()){
		setLimits(xUpperLimit, yUpperLimit);
		setPolicy(policy);
	}

	inline virtual
	~CoordinateHandler2D(){};


	inline
	void set(const drain::image::ImageConf & src){
		setLimits(src.getWidth(), src.getHeight());
		setPolicy(src.getCoordinatePolicy());
	}

	inline
	void set(const drain::image::AreaGeometry & area = AreaGeometry(), const CoordinatePolicy &p = CoordinatePolicy()){
		setLimits(area.getWidth(), area.getHeight());
		setPolicy(p);
	}

	/// Sets minimum values and outer upper limits for x and y.
	inline
	void setLimits(int xMin, int yMin, int xUpperLimit, int yUpperLimit){
		xRange.set(xMin, xUpperLimit - 1);
		yRange.set(yMin, yUpperLimit - 1);
		area.set(xUpperLimit, yUpperLimit);
	}

	/// Sets outer upper limits for x and y.
	void setLimits(int xUpperLimit,int yUpperLimit){
		setLimits(0, 0, xUpperLimit, yUpperLimit);
	}


	inline
	const Range<int> & getXRange() const {
		return xRange;
	}

	inline
	const Range<int> & getYRange() const {
		return yRange;
	}


	/// Assigns internal function pointers.
	inline
	void setPolicy(const CoordinatePolicy &p) {
		setPolicy(p.xUnderFlowPolicy, p.yUnderFlowPolicy, p.xOverFlowPolicy, p.yOverFlowPolicy);
	};

	/// Assigns internal function pointers.
	void setPolicy(EdgePolicy::index_t xUnderFlowPolicy, EdgePolicy::index_t yUnderFlowPolicy,  EdgePolicy::index_t xOverFlowPolicy, EdgePolicy::index_t yOverFlowPolicy);

	/// Set the same policy in all the directions.
	inline
	void setPolicy(EdgePolicy::index_t p){
		setPolicy(p, p, p, p);
	}


	inline
	const CoordinatePolicy & getPolicy() const {
		return policy; //*this;
	}


	/// Ensures the validity of the coordinates. If inside limits, arguments (x,y) remain intact and 0 is returned.
	/**
	 *
	 *   \return - value that describes overflow; zero if no limits crossed and hence, x and y intact.
	 *
	 *   \see CoordinateHandler2D::IRREVERSIBLE  and str constants.
	 *   \see validate()
	 *
	 */
	virtual	inline
	coord_overflow_t handle(int &x,int &y) const {

		result = 0;

		if (x < xRange.min)
			result |= (this->*handleXUnderFlow)(x,y);

		if (x >= area.width)
			result |= (this->*handleXOverFlow)(x,y);

		if (y < yRange.min)
			result |= (this->*handleYUnderFlow)(x,y);

		if (y >= area.height)
			result |= (this->*handleYOverFlow)(x,y);

		return result;

	}

	/// Calls handle(int &x,int &y)
	inline
	coord_overflow_t handle(Point2D<int> &p) const {
		return handle(p.x, p.y);
	}

	inline // NEW
	coord_overflow_t handle(Position &p) const {
		return handle(p.i, p.j);
	}

	/// Handles the coordinate, returning \c true if the position is reversible.
	inline
	bool validate(Point2D<int> &p) const {
		return (handle(p.x, p.y) & CoordinateHandler2D::IRREVERSIBLE) == 0;
	}

	/// Handles the coordinate, returning \c true if the position is reversible.
	inline
	bool validate(int &x, int &y) const {
		return (handle(x, y) & CoordinateHandler2D::IRREVERSIBLE) == 0;
	}

private:

	mutable coord_overflow_t result;

protected:

	//CoordinatePolicy policy;
	drain::Range<int> xRange;
	drain::Range<int> yRange;

	coord_overflow_t (CoordinateHandler2D::*handleXUnderFlow)(int &x, int &y) const;
	coord_overflow_t (CoordinateHandler2D::*handleXOverFlow)(int &x, int &y) const;
	coord_overflow_t (CoordinateHandler2D::*handleYUnderFlow)(int &x, int &y) const;
	coord_overflow_t (CoordinateHandler2D::*handleYOverFlow)(int &x, int &y) const;


	/// Does nothing to the coordinates
	coord_overflow_t skipUndefined(int &x, int &y) const {
		return 0;
	};

	// LIMIT
	///
	coord_overflow_t limitXUnderFlow( int &x, int &y) const {
		x = xRange.min;
		return X_UNDERFLOW | IRREVERSIBLE;
	};

	///
	coord_overflow_t limitYUnderFlow( int &x, int &y) const {
		y = yRange.min;
		return Y_UNDERFLOW | IRREVERSIBLE;
	};

	coord_overflow_t limitXOverFlow( int &x, int &y) const {
		x = xRange.max;
		return X_OVERFLOW | IRREVERSIBLE;
	};

	coord_overflow_t limitYOverFlow( int &x, int &y) const {
		y = yRange.max;
		return Y_OVERFLOW | IRREVERSIBLE;
	};

	// WRAP
	///
	coord_overflow_t wrapXUnderFlow( int &x, int &y) const {
		// Assuming minValue = 0
		x = x % area.width + area.width;
		return X_UNDERFLOW;
	};

	///
	coord_overflow_t wrapYUnderFlow( int &x, int &y) const {
		// Assuming minValue = 0
		y = y % area.height + area.height;
		return Y_UNDERFLOW;
	};

	///
	coord_overflow_t wrapXOverFlow( int &x, int &y) const {
		// Assuming minValue = 0
		x = x % area.width;
		return X_OVERFLOW;
	};

	///
	coord_overflow_t wrapYOverFlow( int &x, int &y) const {
		// Assuming minValue = 0
		y = y % area.height;
		return Y_OVERFLOW;
	};

	// MIRROR
	///
	coord_overflow_t mirrorXUnderFlow( int &x, int &y) const {
		x = xRange.min-x;
		return X_UNDERFLOW;
	};

	///
	coord_overflow_t mirrorYUnderFlow( int &x, int &y) const {
		y = yRange.min-y;
		return Y_UNDERFLOW;
	};

	///
	coord_overflow_t mirrorXOverFlow( int &x, int &y) const {
		x = 2*xRange.max - x;
		return X_OVERFLOW;
	};

	///
	coord_overflow_t mirrorYOverFlow( int &x, int &y) const {
		y = 2*yRange.max - y;
		return Y_OVERFLOW;
	};

	// POLAR
	///
	/**
	 *  Especially, maps x=-1 to x=0.
	 *
	 */
	coord_overflow_t polarXUnderFlow( int &x, int &y) const {
		x = xRange.min - x - 1;
		y = (y + area.height/2) % area.height;
		//std::cout << "handleXUnderFlow_Polar ";
		return X_UNDERFLOW; // ??
	};

	///
	/**
	 *  Especially, maps y=-1 to y=0.
	 *
	 */
	coord_overflow_t polarYUnderFlow( int &x, int &y) const {
		y = yRange.min - y - 1;
		x = (x + area.width/2) % area.width;
		return Y_UNDERFLOW; // ??
	};

	///
	coord_overflow_t polarXOverFlow( int &x, int &y) const {
		x = 2*xRange.max - x + 1;
		y = (y + area.height/2) % area.height;
		return X_OVERFLOW; // ??
	};

	///
	coord_overflow_t polarYOverFlow( int &x, int &y) const {
		y = 2*yRange.max - y + 1;
		x = (x + area.width/2) % area.width;
		return X_OVERFLOW; // ??
	};

	/*
    Alternative approach would be to coord in dependent have:
    coord_overflow_t handleAUnderFlow_Limit(const int &a,const int &b,
          const int &aMin, const int &aMax, 
          const int &bMin, const int &bMax )
	 */


	// etc.

};

// using namespace std;

// std::ostream & operator<<(std::ostream & ostr, const CoordinatePolicy & policy);


std::ostream & operator<<(std::ostream & ostr, const CoordinateHandler2D & handler);

} // image

} // drain
#endif

// Drain
