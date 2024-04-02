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

#include <drain/image/CoordinatePolicy.h>
#include <drain/Log.h>
#include "drain/util/Static.h"



namespace drain {

namespace image {

const EdgePolicy::index_t EdgePolicy::UNDEFINED = 0; //(0);
const EdgePolicy::index_t EdgePolicy::LIMIT = 1; // EdgePolicy::dict.add("LIMIT", 1).second; // = drain::Static::get<coord_policy_dict_t>().add("LIMIT", 1).second;
const EdgePolicy::index_t EdgePolicy::WRAP  = 2; //  = EdgePolicy::dict.add("WRAP"  ,2).second;
const EdgePolicy::index_t EdgePolicy::MIRROR= 3; //= EdgePolicy::dict.add("MIRROR",3).second;
const EdgePolicy::index_t EdgePolicy::POLAR = 4; // = EdgePolicy::dict.add("POLAR", 4).second;

const EdgePolicy::dict_t EdgePolicy::dict =  {  //drain::Static::get<coord_policy_dict_t>();
		{"UNDEFINED", UNDEFINED},
		{"LIMIT",     LIMIT},
		{"WRAP",      WRAP},
		{"MIRROR",    MIRROR},
		{"POLAR",     POLAR},
};

std::ostream & operator<<(std::ostream & ostr, const CoordinatePolicy & policy){
	char sep = 0;
	for (EdgePolicy::index_t p: policy.tuple()){
		if (sep)
			ostr << sep;
		else
			sep = ',';
		ostr << EdgePolicy::dict.getKey(p);
	}
	return ostr;
}


} // image

} // drain

// Drain
