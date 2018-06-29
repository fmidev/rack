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

#ifndef COORDINATES_H_
#define COORDINATES_H_

#include <ostream>
#include <stdexcept>

//#include "ImageLike.h"
//#include "Image.h"

#include "Point.h"

namespace drain {

namespace image {

/// Policies for coordinate underflows and overflows.
/**
 *
 */
class CoordinatePolicy {

public:

	static const int UNDEFINED;
	static const int LIMIT;
	static const int WRAP;
	static const int MIRROR;
	static const int POLAR;
	//static const int DROP=4;

	inline
	CoordinatePolicy(int p = LIMIT){
		set(p);
	};

	inline
	CoordinatePolicy(const CoordinatePolicy & policy){
		set(policy);
	};

	inline
	CoordinatePolicy(int xUnderFlowPolicy, int yUnderFlowPolicy, int xOverFlowPolicy, int yOverFlowPolicy) {
		set(xUnderFlowPolicy, yUnderFlowPolicy, xOverFlowPolicy, yOverFlowPolicy);
	};


	inline
	void set(const CoordinatePolicy & policy){
		set(policy.xUnderFlowPolicy, policy.yUnderFlowPolicy, policy.xOverFlowPolicy, policy.yOverFlowPolicy);
	}

	inline
	void set(int xUnderFlowPolicy, int yUnderFlowPolicy, int xOverFlowPolicy, int yOverFlowPolicy){
		this->xUnderFlowPolicy = xUnderFlowPolicy;
		this->yUnderFlowPolicy = yUnderFlowPolicy;
		this->xOverFlowPolicy  = xOverFlowPolicy;
		this->yOverFlowPolicy  = yOverFlowPolicy;
	}

	inline
	void set(int policy){
		set(policy, policy, policy, policy);
	}

	inline
	void set(const std::vector<int> & v){
	// CoordinatePolicy & operator=(const std::vector<int> & v){
		if (v.size()!=4){
			throw std::runtime_error("CoordinatePolicy::set v size not 4");
			return; //  *this;
		}
		set(v[0], v[1], v[2], v[3]);
	}

	inline
	operator const std::vector<int> & (){
		v.resize(4);
		v[0] = xUnderFlowPolicy;
		v[1] = yUnderFlowPolicy;
		v[2] = xOverFlowPolicy;
		v[3] = yOverFlowPolicy;
		return v;
	}

	int xUnderFlowPolicy;
	int xOverFlowPolicy;
	int yUnderFlowPolicy;
	int yOverFlowPolicy;


protected:

	mutable
	std::vector<int> v;


};

class CoordinateHandler2D : protected CoordinatePolicy {

public:

	CoordinateHandler2D();

	/// Constrcutor
	/**
	 *  \par xUpperLimit - value above maximum allowed value
	 *  \par xUpperLimit - value above maximum allowed value
	 */
	CoordinateHandler2D(int xUpperLimit, int yUpperLimit, const CoordinatePolicy &p = CoordinatePolicy());

	CoordinateHandler2D(const CoordinateHandler2D &h);

	virtual
	~CoordinateHandler2D(){};

	static const int UNCHANGED  = 0;
	static const int X_OVERFLOW = 1;
	static const int X_UNDERFLOW= 2;
	static const int Y_OVERFLOW = 4;
	static const int Y_UNDERFLOW= 8;

	/// Inverse move would not result original position.
	static const int IRREVERSIBLE = 128;

	/// Sets minimum values and outer upper limits for x and y.
	void setLimits(int xMin, int yMin, int xUpperLimit, int yUpperLimit);

	/// Sets outer upper limits for x and y.
	void setLimits(int xUpperLimit,int yUpperLimit);

	inline
	int getXMax() const { return _xMax; };

	inline
	int getYMax() const { return _yMax; };

	/// Assigns internal function pointers.
	inline
	void setPolicy(const CoordinatePolicy &p) { setPolicy(p.xUnderFlowPolicy, p.yUnderFlowPolicy, p.xOverFlowPolicy, p.yOverFlowPolicy); };

	/// Assigns internal function pointers.
	void setPolicy(int xUnderFlowPolicy, int yUnderFlowPolicy,  int xOverFlowPolicy, int yOverFlowPolicy);

	/// Set the same policy in all the directions.
	inline
	void setPolicy(int p){
		setPolicy(p, p, p, p);
	}


	inline
	const CoordinatePolicy & getPolicy() const { return *this; }

	/// Adopts to the coordinate policy and limits of the image.
	/*
	inline
	void use(const Image & src){
		setLimits(src.getWidth(), src.getHeight());
		setPolicy(src.getCoordinatePolicy());
	};
	*/

	/// Ensures the validity of the coordinates. If inside limits, arguments (x,y) remain intact and 0 is returned.
	/**
	 *
	 *   \return - value that describes overflow; zero if no limits crossed and hence, x and y intact.
	 *
	 *   \see CoordinateHandler2D::IRREVERSIBLE  and other constants.
	 *   \see validate()
	 *
	 */
	virtual
	inline
	int handle(int &x,int &y) const {

		result = 0;

		if (x < _xMin)
			result |= (this->*_handleXUnderFlow)(x,y);

		if (x >= _xUpperLimit)
			result |= (this->*_handleXOverFlow)(x,y);

		if (y < _yMin)
			result |= (this->*_handleYUnderFlow)(x,y);

		if (y >= _yUpperLimit)
			result |= (this->*_handleYOverFlow)(x,y);

		return result;

	}

	/// Calls handle(int &x,int &y)
	inline
	int handle(Point2D<int> &p) const {
		return handle(p.x, p.y);
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

	mutable int result;

protected:

	//CoordinatePolicy policy;



	int _xMin;
	int _xMax;
	int _xUpperLimit;
	int _yMin;
	int _yMax;
	int _yUpperLimit;

	int (CoordinateHandler2D::*_handleXUnderFlow)(int &x, int &y) const;
	int (CoordinateHandler2D::*_handleXOverFlow)(int &x, int &y) const;
	int (CoordinateHandler2D::*_handleYUnderFlow)(int &x, int &y) const;
	int (CoordinateHandler2D::*_handleYOverFlow)(int &x, int &y) const;


	/// Does nothing to the coordinates
	int handle_Undefined(int &x, int &y) const {
		return 0;
	};

	// LIMIT
	///
	int handleXUnderFlow_Limit( int &x, int &y) const {
		x = _xMin;
		return X_UNDERFLOW | IRREVERSIBLE;
	};

	///
	int handleYUnderFlow_Limit( int &x, int &y) const {
		y = _yMin;
		return Y_UNDERFLOW | IRREVERSIBLE;
	};

	int handleXOverFlow_Limit( int &x, int &y) const {
		x = _xMax;
		return X_OVERFLOW | IRREVERSIBLE;
	};

	int handleYOverFlow_Limit( int &x, int &y) const {
		y = _yMax;
		return Y_OVERFLOW | IRREVERSIBLE;
	};

	// WRAP
	///
	int handleXUnderFlow_Wrap( int &x, int &y) const {
		// Assuming minValue = 0
		x = x % _xUpperLimit + _xUpperLimit;
		return X_UNDERFLOW;
	};

	///
	int handleYUnderFlow_Wrap( int &x, int &y) const {
		// Assuming minValue = 0
		y = y % _yUpperLimit + _yUpperLimit;
		return Y_UNDERFLOW;
	};

	///
	int handleXOverFlow_Wrap( int &x, int &y) const {
		// Assuming minValue = 0
		x = x % _xUpperLimit;
		return X_OVERFLOW;
	};

	///
	int handleYOverFlow_Wrap( int &x, int &y) const {
		// Assuming minValue = 0
		y = y % _yUpperLimit;
		return Y_OVERFLOW;
	};

	// MIRROR
	///
	int handleXUnderFlow_Mirror( int &x, int &y) const {
		x = _xMin-x;
		return X_UNDERFLOW;
	};

	///
	int handleYUnderFlow_Mirror( int &x, int &y) const {
		y = _yMin-y;
		return Y_UNDERFLOW;
	};

	///
	int handleXOverFlow_Mirror( int &x, int &y) const {
		x = 2*_xMax - x;
		return X_OVERFLOW;
	};

	///
	int handleYOverFlow_Mirror( int &x, int &y) const {
		y = 2*_yMax - y;
		return Y_OVERFLOW;
	};

	// POLAR
	///
	/**
	 *  Especially, maps x=-1 to x=0.
	 *
	 */
	int handleXUnderFlow_Polar( int &x, int &y) const {
		x = _xMin - x - 1;
		y = (y + _yUpperLimit/2) % _yUpperLimit;
		//std::cout << "handleXUnderFlow_Polar ";
		return X_UNDERFLOW; // ??
	};

	///
	/**
	 *  Especially, maps y=-1 to y=0.
	 *
	 */
	int handleYUnderFlow_Polar( int &x, int &y) const {
		y = _yMin - y - 1;
		x = (x + _xUpperLimit/2) % _xUpperLimit;
		return Y_UNDERFLOW; // ??
	};

	///
	int handleXOverFlow_Polar( int &x, int &y) const {
		x = 2*_xMax - x + 1;
		y = (y + _yUpperLimit/2) % _yUpperLimit;
		return X_OVERFLOW; // ??
	};

	///
	int handleYOverFlow_Polar( int &x, int &y) const {
		y = 2*_yMax - y + 1;
		x = (x + _xUpperLimit/2) % _xUpperLimit;
		return X_OVERFLOW; // ??
	};

	/*
    Alternative approach would be to coord in dependent have:
    int handleAUnderFlow_Limit(const int &a,const int &b,
          const int &aMin, const int &aMax, 
          const int &bMin, const int &bMax )
	 */


	// etc.

};

// using namespace std;

std::ostream & operator<<(std::ostream & ostr, const CoordinatePolicy & policy);


std::ostream & operator<<(std::ostream & ostr, const CoordinateHandler2D & handler);

} // image

} // drain
#endif

// Drain
