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

#include "Variable.h"

// // // using namespace std;


namespace drain
{

//const Variable VariableMap::_empty;

template <>
void Variable::toVector(std::vector<Variable> & v) const {

	//std::cerr << "toVector" << typeid(v).name() << std::endl;
		if (getType() == typeid(std::string)){
			std::list<std::string> l;
			String::split(toStr(), l, std::string(1,this->separator)," \t\n");
			v.resize(l.size());
			//typename
			std::list<std::string>::const_iterator lit = l.begin();
			std::vector<Variable>::iterator vit = v.begin();
			while (lit != l.end()) {
				*vit = *lit;
				++vit;
				++lit;
			}
		}
		else {
			v.resize(getElementCount());
			CastableIterator it = begin();
			std::vector<Variable>::iterator vit = v.begin();
			while (it != end()) {
				//std::cerr << *vit << "<=" << *it << std::endl;
				*vit = *it;
				++it;
				++vit;
			}
		}
}



} // drain

// Drain
