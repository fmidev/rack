/*

MIT License

Copyright (c) 2026 FMI Open Development / Markus Peura, first.last@fmi.fi

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

#ifndef DRAIN_VALUE_HANDLER
#define DRAIN_VALUE_HANDLER

//#include <drain/image/CoordinatePolicy.h>
#include <ostream>

#include "drain/TypeUtils.h"

//#include "drain/util/Frame.h"
//#include "Geometry.h"

//#include "ImageFrame.h"
//#include "Direction.h" //~ "Position.h", experimental

namespace drain {


namespace image {


/**
 *  Assuming that data has some special codes, encoder should detect them and
 *  suggest different handling for processing of values.
 *
 *  Main method handle(value) should return 0, if the processing can continue the default way.
 *
 */
class ValueHandler {

public:

	typedef unsigned int flag_t;

	static const flag_t ACCEPT  = 0;
	static const flag_t REJECT  = 1;
	static const flag_t CHANGED = 2;

	/// Constructor
	inline
	ValueHandler(){};

	inline virtual
	~ValueHandler(){};

	/// Checks and appropriately, change the value.
	/*
	 *
	 */
	template <typename T>
	flag_t handle(T & value) const ;

	template <typename T>
	inline
	bool validate(T & value) const {
		return (handle(value) & REJECT) == 0;
	}



	virtual inline
	flag_t handleUChar(unsigned char & value) const {
		return ACCEPT;
	};

	virtual inline
	flag_t handleInt(int & value) const {
		return ACCEPT;
	};

	virtual inline
	flag_t handleDouble(double & value) const {
		return ACCEPT;
	};



};

template <>
inline
ValueHandler::flag_t ValueHandler::handle(unsigned char & value) const {
	return handleUChar(value);
}

template <>
inline
ValueHandler::flag_t ValueHandler::handle(int & value) const {
	return handleInt(value);
}

template <>
inline
ValueHandler::flag_t ValueHandler::handle(double & d) const {
	return handleDouble(d);
}

// using namespace std;
// std::ostream & operator<<(std::ostream & ostr, const CoordinatePolicy & policy);


} // image

//DRAIN_TYPENAME(image::ValueHandler);


} // drain
#endif

// Drain
