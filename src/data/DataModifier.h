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

#ifndef RACK_DATA_MODIFIER
#define RACK_DATA_MODIFIER

/*
#include <map>
#include <drain/image/CoordinatePolicy.h>
#include <drain/util/ReferenceMap.h>
#include "ODIM.h"
#include "PolarODIM.h"
*/
#include "DataSelector.h"
// #include "DataTools.h"

namespace rack {

/// Tool for selecting datasets based on paths, quantities and min/max elevations.
/**
 *
 *    Applies drain::RegExp in matching.
 */
class DataModifier {

public:

	static
	void remove(Hi5Tree & dst, const DataSelector & dataSelector);

	/// Complement of remove(): keep the selected, remove else.
	static
	void keep(Hi5Tree & dst, const DataSelector & selector);



	static inline
	int removeEmptyGroups(Hi5Tree & dst){
		return handleEmptyGroups(dst, true);
	}

	/// Mark everything but selected "excluded".
	static
	void markIncluded(Hi5Tree & dst, const DataSelector & dataSelector);

	/// Mark everything along the path included
	static inline
	void markPathIncluded(Hi5Tree & dst, const ODIMPath & path){
		DataTools::markExcluded(dst, path, false);
	}




protected:

	static
	int handleEmptyGroups(Hi5Tree & dst, bool REMOVE, const ODIMPath & path = ODIMPath());



};


} // rack::

#endif // DATA_MODIFIER

