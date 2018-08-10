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

#include "Proj4.h"

namespace drain
{

Proj4::Proj4() : projSrc(NULL), projDst(NULL) {
}

Proj4::Proj4(const Proj4 &p) : projSrc(NULL), projDst(NULL) {
	//cerr << "Proj4(const Proj4 &p) - someone needs me" << endl;
	setProjectionSrc(p.getProjectionSrc());
	setProjectionDst(p.getProjectionDst());
}

Proj4::~Proj4(){
	pj_free( projSrc );
	pj_free( projDst  );
}


void Proj4::_setProjection(const std::string & str, projPJ & p){

	pj_free(p);
	p = pj_init_plus(str.c_str());

	if (p == NULL){
		throw std::runtime_error(std::string("proj4 error: ") + pj_strerrno(*pj_get_errno_ref())+ " (" + str + ")");
	}
	
}

std::ostream & operator<<(std::ostream & ostr, const Proj4 &p){
	ostr << "Proj4 src: " << p.getProjectionSrc() << '\n';
	ostr << "Proj4 dst: " << p.getProjectionDst() << '\n';
	return ostr;
}


}

// Drain
