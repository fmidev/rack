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

#ifndef COORD_POLICY_H_
#define COORD_POLICY_H_

#include <drain/Log.h>
#include <drain/UniTuple.h>
#include <ostream>
#include <stdexcept>

#include "drain/util/Frame.h"
#include "drain/util/Point.h"
#include "drain/util/Range.h"
#include "drain/util/StatusFlags.h"
#include "Geometry.h"

namespace drain {


namespace image {



//typedef unsigned short coord_pol_t;

struct EdgePolicy {

	typedef unsigned short index_t;

	static const EdgePolicy::index_t UNDEFINED;// = 0;
	static const EdgePolicy::index_t LIMIT;    // = 1;
	static const EdgePolicy::index_t WRAP; 	   // = 2;
	static const EdgePolicy::index_t MIRROR;   // = 3;
	static const EdgePolicy::index_t POLAR;    // = 4;

	typedef drain::Dictionary<std::string,index_t> dict_t;

	static
	const dict_t dict; // Unused, future option

};



//typedef drain::Dictionary<std::string,EdgePolicy::index_t> coord_policy_dict_t;


//typedef drain::GlobalFlags<EdgePolicy::index_t> coord_policy_flags;

// Only 2D, or templated?
/*
struct CoordinatePolicyFlags {
	static const EdgePolicy::index_t UNDEFINED = 0;
	static const EdgePolicy::index_t LIMIT;
	static const EdgePolicy::index_t WRAP;
	static const EdgePolicy::index_t MIRROR;
	static const EdgePolicy::index_t POLAR;
};
*/

/*
typedef enum { // consider GlobalFlagger
	UNDEFINED = 0,
	LIMIT = 1, // = constexpr drain::GlobalFlags<long>::add("LIMIT"),
	WRAP = 2,
	MIRROR = 3,
	POLAR = 4
} EdgePolicy::index_t;
*/

//typedef drain::GlobalFlags<Coordinates> coord_policy_flags;

/// Policies for coordinate underflows and overflows.
/**
 *
 */
class CoordinatePolicy : public UniTuple<EdgePolicy::index_t,4> {

public:

	EdgePolicy::index_t & xUnderFlowPolicy;
	EdgePolicy::index_t & yUnderFlowPolicy;
	EdgePolicy::index_t & xOverFlowPolicy;
	EdgePolicy::index_t & yOverFlowPolicy;


	// Don't use enum (difficult to export/import).

	//static const drain::FlaggerBase<EdgePolicy::index_t>::dict_t dict;


	inline
	CoordinatePolicy(EdgePolicy::index_t p = EdgePolicy::LIMIT) :
		xUnderFlowPolicy(next()), yUnderFlowPolicy(next()), xOverFlowPolicy(next()), yOverFlowPolicy(next()) {
		fill(p); // or set(p, p, p, p); // No auto fill.
	};

	inline
	CoordinatePolicy(const CoordinatePolicy & policy) : //v(4, LIMIT), xUnderFlowPolicy(v[0]), yUnderFlowPolicy(v[1]), xOverFlowPolicy(v[2]), yOverFlowPolicy(v[3]) {
		xUnderFlowPolicy(next()), yUnderFlowPolicy(next()), xOverFlowPolicy(next()), yOverFlowPolicy(next()) {
		assign(policy);
	};

	inline
	CoordinatePolicy(EdgePolicy::index_t xUnderFlowPolicy, EdgePolicy::index_t yUnderFlowPolicy, EdgePolicy::index_t xOverFlowPolicy, EdgePolicy::index_t yOverFlowPolicy) : //v(4, LIMIT), xUnderFlowPolicy(v[0]), yUnderFlowPolicy(v[1]), xOverFlowPolicy(v[2]), yOverFlowPolicy(v[3]) {
		xUnderFlowPolicy(next()), yUnderFlowPolicy(next()), xOverFlowPolicy(next()), yOverFlowPolicy(next()) {
		set(xUnderFlowPolicy, yUnderFlowPolicy, xOverFlowPolicy, yOverFlowPolicy);
	};

	inline
	CoordinatePolicy & operator=(const CoordinatePolicy & policy){
		assign(policy);
		return *this;
	}

	inline
	bool isSet(){
		for (EdgePolicy::index_t p: tuple()){
			if (p == EdgePolicy::UNDEFINED)
				return false;
		}
		return true;
	}


};

std::ostream & operator<<(std::ostream & ostr, const CoordinatePolicy & policy);

} // image

} // drain
#endif

// Drain
