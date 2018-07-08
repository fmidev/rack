/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

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
