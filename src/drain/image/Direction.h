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
#ifndef DRAIN_DIRECTION_H
#define DRAIN_DIRECTION_H

#include <map>
#include <drain/PseudoTuple.h>

namespace drain
{
namespace image
{



// Faster than Point2D (with UniTuple init)
struct Position {

	typedef int value_t;
	value_t i=0;
	value_t j=0;

	inline
	Position(int i=0, int j=0) : i(i), j(j) {
	};

	inline
	Position(const Position & pos){
		*this = pos;
	}

	inline
	Position & operator=(const Position & pos){
		i = pos.i;
		j = pos.j;
		return *this;
	}
};

typedef PseudoTuple<Position> PositionTuple;

// Keep for future templated methods.
#define DIR_TURN_090(dir) (((dir << 2) & 0xff) | (dir >> 6))
#define DIR_TURN_180(dir) (((dir << 4) & 0xff) | (dir >> 4))
#define DIR_TURN_270(dir) (((dir << 6) & 0xff) | (dir >> 2))

#define DIR_TURN_DEG(dir, deg) (((dir << (deg/45)) & 0xff) | (dir >> (8 - deg/45)))


struct Direction {

	typedef unsigned char value_t;

	static const value_t NONE = 0;
	static const value_t UP=1;
	static const value_t UP_RIGHT=2;
	static const value_t RIGHT=4;
	static const value_t DOWN_RIGHT=8;
	static const value_t DOWN=16;
	static const value_t DOWN_LEFT=32;
	static const value_t LEFT=64;
	static const value_t UP_LEFT=128;



	/// Opposite direction
	/**
	 *
	 */

	/// Turn 90 degrees left
	/**
	 *
	 */
	/*
	static inline
	value_t turn90(value_t d){
		return ((d << 2)&0xff) | (d >> 6);
	}

	static inline
	value_t turn180(value_t d){
		return ((d << 4)&0xff) | (d >> 4);
	}

	static inline
	value_t turn270(value_t d){
		return ((d << 6)&0xff) | (d >> 2);
	}
	*/

	/*
	template <dir DIR>
	static inline
	dir dir90minus(){
		return (DIR >> 2) | ((DIR << 6)&0xff);
	}
	*/

	/// Turn 90 degrees right
	/**
	 *
	static inline
	dir dir90CW(dir d){
		return (d >> 2) | (d << 6);
	}
	 */


	static
	const std::map<value_t,Position> offset;

};



} // image::

} // drain::

#endif



