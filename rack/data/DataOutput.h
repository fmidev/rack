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

#ifndef DATA_OUTPUT_H_
#define DATA_OUTPUT_H_


//#include <drain/util/BeanLike.h>
#include <ostream>

//#include <drain/util/RegExp.h>

#include "hi5/Hi5.h"
#include "ODIMPath.h"


namespace rack {

/// Tool for writing
/**
 *
 */
class DataOutput {

public:

	/// Writes H5 structure to GraphViz (dot) format.
	static
	void writeToDot(std::ostream & ostr, const HI5TREE & tree, ODIMPathElem::group_t selector = (ODIMPathElem::ROOT | ODIMPathElem::IS_INDEXED));


protected:

	/// Writes H5 structure to GraphViz (dot) format.
	static
	void writeGroupToDot(std::ostream & ostr, const HI5TREE & group, int & id,
			ODIMPathElem::group_t selector = (ODIMPathElem::ROOT | ODIMPathElem::IS_INDEXED), const ODIMPath & path = ODIMPath());

private:

	static
	std::string quoted(const ODIMPath & path);



};



} // rack::

#endif /* DATASELECTOR_H_ */

// Rack
 // REP // REP
