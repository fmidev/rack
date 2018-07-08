/*

    Copyright 2012-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#ifndef DATA_OUTPUT_H_
#define DATA_OUTPUT_H_


//#include <drain/util/BeanLike.h>
#include <ostream>

//#include <drain/util/RegExp.h>

#include "hi5/Hi5.h"
#include "BaseODIM.h"


namespace rack {

/// Tool for writing
/**
 *
 */
class DataOutput {

public:

	static inline
	void writeGroupToDot(std::ostream & ostr, const HI5TREE & tree){

		int index = 0;
		ostr << "digraph G { \n";
		writeGroupToDot(ostr, tree, index);
		ostr << "}\n";

	}


protected:

	static
	void writeGroupToDot(std::ostream & ostr, const HI5TREE & group, int & id,
			BaseODIM::group_t selector = (BaseODIM::ROOT | BaseODIM::IS_INDEXED), const ODIMPath & path = ODIMPath());

};



} // rack::

#endif /* DATASELECTOR_H_ */
