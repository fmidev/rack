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

#ifndef ODIMPATH_TOOLS_H_
#define ODIMPATH_TOOLS_H_

#include <set>
#include <list>
#include <map>
#include <stdexcept>

#include "drain/util/BeanLike.h"
#include "drain/util/Range.h"
//#include "drain/util/ReferenceMap.h"
//#include "drain/util/RegExp.h"
//#include "drain/util/Sprinter.h"

//#include "drain/util/Variable.h"
#include "ODIM.h"
#include "PolarODIM.h" // elangle

#include "ODIMPathMatcher.h"


namespace rack {




/// Tool for selecting datasets based on paths, quantities and min/max elevations.
/**
 *  \see rack::CmdSelect
 */
class ODIMPathTools {

public:

	/// Searches the children of child.getType(), or g if given, and stores the one with largest index.
	static
	bool getLastChild(const Hi5Tree & tree, ODIMPathElem & child); //, (ODIMPathElem::group_t g =  ODIMPathElem::NONE);

	/// Searches children of given type, returns a non-existing child with index greater than child.index.
	/**
	 *   Unused indices may be returned.
	 */
	static
	bool getNewChild(const Hi5Tree & tree, ODIMPathElem & child, ODIMPathElem::index_t iMax=0xff);

	/// Derive a child with index one greater than the largest index encountered.
	/**
	 *  \param tree - parent structure
	 *  \param child - child to be set index = last+1, hence 1 if none found.
	 *  \return - true if child's index was incremented (ie.)
	 */
	static
	bool getNextChild(const Hi5Tree & tree, ODIMPathElem & child);


	/// Searches children of given type, returns a non-existing child with index greater than child.index.
	/**
	 *   Unused indices may be returned.
	 */
	static
	bool getChildren(const Hi5Tree & tree, std::map<std::string,ODIMPathElem> & children, ODIMPathElem::group_t groups);

};


} // rack::

#endif /* DATASELECTOR_H_ */
