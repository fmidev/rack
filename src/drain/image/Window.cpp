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
//#include "drain/util/Point.h"
#include "Window.h"

namespace drain
{


namespace image
{

DRAIN_TYPENAME_DEF(WindowConfig);
DRAIN_TYPENAME_DEF(WindowCoreBase);
DRAIN_TYPENAME_DEF(WindowCore);
DRAIN_TYPENAME_DEF(WeightedWindowCore);

std::ostream & operator<<(std::ostream & ostr, const WindowConfig & conf){
	ostr << drain::TypeName<WindowConfig>::str() << ":: " << conf.frame << ',' << conf.getFunctorName() << ':' << conf.getFunctorParams();
	//ostr << "WindowConfig: " << conf.getFunctorName() << ':' << conf.getFunctorParams();
	//ostr << "WindowConfig: " << "conf.getFunctorName()";
	return ostr;
}


/*
std::ostream & operator<<(std::ostream & ostr, const WindowConfig & conf){
	// const drain::UnaryFunctor & ftor = conf.getFunctor();
	// ostr << "WindowConfig:: " << conf.frame << ':' << ftor.getName()<< '>' << ftor.getgetParameters();
	//ostr << "WindowConfig:: " << conf.frame << ',' << conf.getFunctorName() << ':' << conf.getFunctorParams();
	ostr << "WindowConfig:: " << conf.frame;
	// std::cerr << __FILE__ << '\n';
	return ostr;
}
*/

//IdentityFunctor WindowConfig::idFtor;


}

}

/*
std::ostream & operator<<(std::ostream & ostr, const drain::image::WindowConfig & conf){
	//const drain::UnaryFunctor & ftor = conf.getFunctor();
	//ostr << "WindowConfig:: " << conf.frame << ':' << ftor.getName()<< '>' << ftor.getgetParameters();
	ostr << "WindowConfig:: " << conf.frame << ',' << conf.getFunctorName() << ':' << conf.getFunctorParams();
	// ostr << "WindowConfig:: "; // << conf.frame;
	// std::cerr << __FILE__ << '\n';
	return ostr;
}
*/


// Drain
