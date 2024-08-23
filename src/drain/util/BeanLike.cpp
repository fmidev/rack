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

#include "BeanLike.h"

namespace drain
{

	std::ostream & BeanLike::toStream(std::ostream & ostr, bool compact) const {
		ostr << name << ": " << description;
		if (compact){
			ostr << ' ' << parameters; //  << '\n'; // ?
		}
		else {
			ostr << '\n';
			//ostr << '\t' << parameters << '\n';
			// Was that ^ supposed to be: ostr << "\t parameters\n";
			for (const std::string & key: getParameters().getKeyList()){
				const Reference & param = parameters.at(key);
				std::cout << '\t' << key << ':' << param << '\n';
			}
			/*
			for (const auto & entry: getParameters()){
				std::cout << '\t' << entry.first << ':' << entry.second << '\n';
			}
			*/
		}
		return ostr;
	}


} // drain::
