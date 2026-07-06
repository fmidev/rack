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
#ifndef ODIM_BASE
#define ODIM_BASE


#include <drain/Log.h>
#include <ostream>
//#include <cmath>
#include <string>
//#include <map>
//#include <algorithm>

#include <drain/util/EnumFlagger.h>
#include <drain/Convert.h>



namespace odim {

enum GroupId {
	WHERE = 1,
	WHAT  = 2,
	HOW   = 4,
	// todo...
};

DRAIN_ENUM_DICT(GroupId);

}



namespace odim {

class ODIMgroup : public drain::EnumFlagger<drain::SingleFlagger<GroupId> > {

public:

	typedef drain::EnumFlagger<drain::MultiFlagger<GroupId> > GroupTypeFlagger;


	static inline
	bool isAttributeGroup(GroupId group){

		static
		const GroupTypeFlagger attributeGroupFlagger(WHERE, WHAT, HOW);

		return attributeGroupFlagger.isSet(group);
	}

	inline
	bool isAttributeGroup() const {
		return isAttributeGroup(getValue());
	}

};


}

namespace drain {

DRAIN_ENUM_DICT(odim::GroupId) = {
		{"where", odim::GroupId::WHERE},
		{"what",  odim::GroupId::WHAT},
		{"how",   odim::GroupId::HOW},
};

DRAIN_ENUM_CONV(odim::GroupId);

}



#endif

// Rack
