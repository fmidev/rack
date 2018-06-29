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
*//**

    Copyright 2018 -  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.


 *      Author: mpeura
 */

#ifndef Drain_TypeUtils_H
#define Drain_TypeUtils_H

#include <typeinfo>
#include <limits>

#include "Type.h"
#include "Log.h"

namespace drain {

/*
 *  TODO: move and lowerCase the native-like classes inside Type, rename call back
 *
 *  This is good enough:  Type::call<drain::typeIsInteger>(t)
 */

class sizeGetter {

public:

	typedef std::size_t value_t;

	/**
	 *  \tparam S - type to be analyzed
	 *  \tparam T - destination type  (practically value_t)
	 */
	template <class S, class T>
	static
	inline
	T callback(){
		return static_cast<T>(getSize<S>()); // static cast unneeded in these ?
	}

protected:

	// Must be here to get template<void> implemented
	template <class S>
	static inline
	size_t getSize(){
		return sizeof(S);
	}

};

template <>
inline
size_t sizeGetter::getSize<void>(){
	return 0;
}
// todo:: std::string?

/// Returns the compiler specific ie. non-standard name of the type.
class nameGetter {

public:

	typedef const char * value_t;

	template <class S, class T>
	static inline
	T callback(){
		return typeid(S).name();
	}

};

/**
 *  \tparam T - return type
 */
class isSigned {  // F2

public:

	typedef bool value_t;

	/**
	 *  \tparam S - type to be analyzed
	 *  \tparam T - destination type  (practically value_t)
	 */
	template <class S, class T>
	static inline
	T callback(){
		return static_cast<T>( std::numeric_limits<S>::is_signed);
	}

};


class typeIsInteger { // F2

public:

	typedef bool value_t;

	/**
	 *  \tparam S - selector type
	 *  \tparam T - destination type (practically value_t)
	 */
	template <class S, class T>
	static inline
	T callback(){ return std::numeric_limits<S>::is_integer; }

};

/// The maximum value of a type given as type_info, char or std::string.
/**
 *  Usage:
 *  \code
 *   Type::call<drain::typeIsSmallInt, bool>(t)
 *  \endcode
 */
class typeIsSmallInt {

public:

	typedef bool value_t;

	/**
	 *  \tparam S - type selector
	 *  \tparam D - destination type (practically value_t)
	 */
	template <class S, class D>
	static inline
	D callback(){
		return (typeid(S) == typeid(char)) || (typeid(S) == typeid(unsigned char)) || (typeid(S) == typeid(short)) || (typeid(S) == typeid(unsigned short));
	}
};


///
/**
 *
 */
template <class S>
struct typeLimits {

	static inline
	long int getMinI(){
		return static_cast<long int>(std::numeric_limits<S>::min());
	}

	static inline
	unsigned long int getMaxI(){
		return static_cast<unsigned long int>(std::numeric_limits<S>::max());
	}

	static inline
	double getMaxF(){
		return static_cast<double>(std::numeric_limits<S>::max());
	}

};


template <> inline long int typeLimits<void>::getMinI(){ return 0l; }
template <> inline unsigned long int typeLimits<void>::getMaxI(){ return 0l; }
template <> inline   double typeLimits<void>::getMaxF(){ return 0.0; }

template <> inline long int typeLimits<std::string>::getMinI(){ return 0l; }
template <> inline unsigned long int typeLimits<std::string>::getMaxI(){ return 0l; }
template <> inline   double typeLimits<std::string>::getMaxF(){ return 0.0; }


/// The minimum value of a type given as type_info, char or std::string.
/**
 *  Usage:
 *  \code
 *    double d = Type::call<typeMin, double>(t);
 *  \endcode
 */
class typeMin {

public:

	//typedef T value_t;

	/**
	 *  \tparam S - type selector
	 *  \tparam D - destination type
	 */
	template <class S, class D>
	static inline
	D callback(){
		if (std::numeric_limits<S>::is_integer)
			return static_cast<D>(+typeLimits<S>::getMinI());
		else
			return static_cast<D>(-typeLimits<S>::getMaxF());
	}
};


/// The maximum value of a type given as type_info, char or std::string.
/**
 *  Usage:
 *  \code
 *   Type::call<Type Max, double>(t)
 *  \endcode
 */
class typeMax {

public:

	//typedef T value_t;

	/**
	 *  \tparam S - type selector
	 *  \tparam D - destination type
	 */
	template <class S, class D>
	static inline
	D callback(){
		if (std::numeric_limits<S>::is_integer)
			return static_cast<D>(+typeLimits<S>::getMaxI());
		else
			return static_cast<D>(+typeLimits<S>::getMaxF());
	}
};


/// Class for ensuring that variable of type D remains within limits of type S.
/**
 *  Returns a function pointer (of type typeLimiter::value_t) .
 *  \code
 	 typedef drain::typeLimiter<double> Limiter;
	 typeLimiter::value_t fptr = Type::call<typeLimiter>(t);  // t is type_info, char or std::string.
    \endcode
 *
 */
// THIS WORKS WELL!
template <class D>
class typeLimiter  {

public:

	/// Definition that simplifies
	typedef D (*value_t)(D);

	/**
	 *  \tparam S - type selector
	 *  \tparam T - destination type
	 */
	template <class S, class T>
	static inline
	T callback(){
		if (std::numeric_limits<S>::is_integer)
			return &typeLimiter<D>::limitInteger<S,D>;
		else
			return &typeLimiter<D>::limitFloat<S,D>;
	}


protected:

	///
	/**
	 *  \tparam S - type selector
	 *  \tparam T - target value type (integer type)
	 *
	 *  Note: expecting T > S, ie. S is a narrower basetype.
	 */
	template <class S, class T>
	static
	T limitInteger(T x){

		static const T minValue = static_cast<T>(typeLimits<S>::getMinI());
		static const T maxValue = static_cast<T>(typeLimits<S>::getMaxI());
		if (x < minValue)
			return minValue;
		else if (x > maxValue)
			return maxValue;

		return x;
	}

	///
	/**
	 *  \tparam S - type selector
	 *  \tparam T - target value type (float or double)
	 */
	template <class S, class T>
	static
	T limitFloat(T x){

		static const T maxValue = static_cast<T>(typeLimits<S>::getMaxF());
		// notice minus sign
		if (x < -maxValue)
			return -maxValue;
		else if (x > maxValue)
			return maxValue;

		return x;
	}



};



}

#endif /* TypeUTILS_H_ */
