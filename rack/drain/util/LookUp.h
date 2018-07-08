/*

    Copyright 2001 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */
/*
 * LookUp.h
 *
 *  Created on: Oct 23, 2012
 *      Author: mpeura
 */

#ifndef LOOKUP_H_
#define LOOKUP_H_

#include <cmath>
#include <vector>
#include <exception>

#include "util/Type.h"
#include "util/TypeUtils.h"

// // using namespace std;

namespace drain {

/// Look-up tables.
template <class T>
class LookUp : public std::vector<T> {
public:

	LookUp(size_t size, double scale=1.0) : std::vector<T>(size), max(getScale()), scale(scale*static_cast<double>(max)), scaleInv(1.0/this->scale) {
		if (scale == 0.0)
			throw std::runtime_error("LookUp: scale=0.0 not allowed.");
		//init();
	} ; // if (size == 0) throw std::runtime_error("LookUp: undetectValue sized lookup not allowed.");

	virtual ~LookUp(){};

	const T max;

	const double scale;
	const double scaleInv;

	double getScaled(size_t i) const {
		return scaleInv * (*this)[i];
	}

	void putScaled(size_t i, double value){
		(*this)[i] = scale * value;
	}


protected:

	static
	inline
	double getScale(){
		return Type::call<drain::typeIsInteger>(typeid(T)) ? Type::call<drain::typeMax,double>(typeid(T)) : 1.0;
	}

	/*
	virtual
	void init(){
		std::cerr << "Dammit, called base class" << std::endl;
	}
	*/

};

/// Values are available from [0,2PI].
/**
 *
 */
template <class T, class F>  //=1.0, typename OFFSET=0.0>
class FunctorLookUp : public LookUp<T> {
public:

	FunctorLookUp(size_t size, double scale, double scaleIndex = 1.0, double offsetIndex=0.0) : LookUp<T>(size, scale), scaleIndex(scaleIndex), offsetIndex(offsetIndex) {
		init();
	};

	const double scaleIndex;

	const double offsetIndex;

protected:

	//virtual
	inline
	void init(){
		F functor;
		const size_t n = std::vector<T>::size();
		const double coeff = scaleIndex/static_cast<double>(n);
		for (size_t i=0; i<n; ++i)
			this->putScaled(i, functor(offsetIndex + coeff*static_cast<double>(i)));
	}


};


/*
template <class T>
class LookUpSin : public LookUp<T> {
public:

	LookUpSin(size_t size, double scale) : LookUp<T>(size, scale) {};

protected:


	virtual inline
	void init(){
		const size_t n = std::vector<T>::size();
		const double coeff = 2.0*M_PI / static_cast<double>(n);
		for (size_t i=0; i<n; ++i)
			this->putScaled(i, sin(coeff * i));
	}

};

template <class T>
class LookUpCos : public LookUp<T> {
public:

	LookUpCos(size_t size) : LookUp<T>(size) {};

protected:

	virtual inline
	void init(){
		const size_t n = std::vector<T>::size();
		const double coeff = 2.0 * M_PI / n;
		for (size_t i=0; i<n; ++i)
			(*this)[i] = cos(coeff * i);
	}


};
*/

/*
template <class T>
class LookUpATan : public LookUp<T> {
public:
	LookUpATan(size_t size) : LookUp<T>(size), _max(size-1) {
		const double coeff = 1.0 / _max;
		for (size_t i=0; i<=_max; ++i)
			(*this)[i] = std::atan2(coeff * i, 1.0);  // tan enough?
	};

	//template <class T2>
	double atan2(const double & y, const double & x) const {

		if (y < 0){
			return - this->atan2(-y, x);
		}
		// now y>=0
		else if (x < 0){
			return M_PI - this->atan2(y, -x);
		}
		/// now y>=0, x>=0
		else if (x == 0){
			//std::cout << "zerox\n";
			if (y == 0)
				return 0;  // TODO nan<T>;
			else
				return M_PI_2;
		}
		/// now x>0, y>=0
		else if (y > x){
			//std::cout << "y>x\n";
			return M_PI_2 - this->atan2(x,y);
		}
		/// now x>0, 0<=y<=x
		else {
			//std::cout << " final\n";
			return (*this)[ (_max*y)/x ];
		}

	}

protected:
	const size_t _max;
};
*/


}

#endif /* LOOKUP_H_ */
