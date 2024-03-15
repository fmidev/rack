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


//#include "Convert.h"
// This file is included by: Variable.h Reference.h FlexibleVariable.h


/// SmartVariable - concept for storing and assigning scalars, vectors and std::string using automatic type conversions.
/**
 *   Based on drain::Castable .
 *
 *   Implementations: Variable, Reference, FlexibleVariable
 *
 *   Variable designed for storing values with automatic forward and backward conversion.
 *   Internally, Variable uses a byte vector which is dynamically resized.
 *   The data can be a single scalar value, an array, or a string.
 *
 *   Internally, a string is implemented as a vector of chars. This is different from
 *   drain::Castable with type std::string which assumes that the object behind (void *)
 *   is actually an instance of std::string.
 *
 *   The following examples illustrates its usage:
 *   \code
 *   Variable vField;
 *   vField = 123.456;  // First assignment sets the type implicitly (here double).
 *   int i = vField;    // Automatic cast
 *   \endcode
 *
 *  The type of a Variable can be set explicitly:
 *   \code
 *   Variable vField;
 *   vField.setType<std::string>();
 *   vField = 123.456; // Stored internally as char array "123.456"
 *   int i = vField;   // Automatic cast
 *   \endcode
 *
 *  Any number of scalars can be appended to a Variable, which hence becomes a vector:
 *   \code
 *   Variable vField;
 *   vField.setType<double>();
 *   vField << 1.0 << 123.456 << 9.999;
 *   std::cout << vField << std::endl;  // dumps 1.0,123.456,9.999
 *   \endcode
 *
 *  If the type is unset, Variable converts to undetectValue or an empty std::string.
 *   \code
 *   Variable vField;
 *   double d = vField; // becomes 0.0
 *   std::string s = vField; // becomes ""
 *   \endcode
 *
 *  Unlike with drain::Castable, the pointer contained by Variable is not visible or changeable by the user.
 *
 *  Note: tried skipping Castable, inheriting directly from Caster but needs Castable for cooperation with ReferenceMap.
 *
 *
 *   \example Variable-example.cpp
 *
 */

/// Default assignment operator - aimed for basic types and std::string.
/**
 *  This is the idea of the whole thing.
 *
 */
template <class T>
inline
SmartVariable & operator=(const T &x){
	assign(x);
	return *this;
}

/// Assignment of the same class is implemented (in some of the ones) below.

SmartVariable & operator=(const Variable &x);

SmartVariable & operator=(const Referencer &x);

SmartVariable & operator=(const FlexibleVariable &x);

/*
inline
SmartVariable & operator=(const SmartVariable &x){
	assignCastable(x);
	return *this;
}
*/

/*
inline
SmartVariable & operator=(const Variable &x){
	assignCastable(x);
	return *this;
}

inline
SmartVariable & operator=(const Referencer &x){
	assignCastable(x);
	return *this;
}

inline
SmartVariable & operator=(const FlexibleVariable &x){
	assignCastable(x);
	return *this;
}
*/

/// Assignment of a Castable (exact class - not derived).
inline
SmartVariable & operator=(const Castable &x){
	assignCastable(x);
	return *this;
}


/// [Obligatory]
/**
 *  Notice:
 */
template <class T>
inline
SmartVariable & operator=(std::initializer_list<T> l){
	//std::cerr << __FILE__ << ": Check " << __FUNCTION__ << '\n';
	assignContainer(l, false);
	return *this;
}

/// Assignment of C strings. [Obligatory]
/**
 *   Assignment from char * and std::string splits the input by separator.
 *
 *   STL strings will be handled by the default operator operator=(const T &x) .
 */
inline
SmartVariable & operator=(const char *x){
	assign(x);
	return *this;
}


// Could be removed ? Handled by assign<T>() ?
template <class T, size_t N>
inline
SmartVariable &operator=(const UniTuple<T,N> & unituple){
	assignContainer(unituple);
	return *this;
}

/*
template <>
template <class D>
void Convert2<SmartVariable>::convert(const SmartVariable &src, D & dst){
	dst = (const D &)src;
	std::cout << "CONV..." << src << " -> " << dst << '\n';
}


template <>
void Convert2<SmartVariable>::convert(const char *src, SmartVariable & dst){
	dst = src;
	std::cout << "CONV..." << src << " -> " << dst << '\n';
}



template <>
template <class S>
void Convert2<SmartVariable>::convert(const S &src, SmartVariable & dst){
	dst = src;
	std::cout << "CONV..." << src << " -> " << dst << '\n';
}
*/
