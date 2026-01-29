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

#include <drain/Log.h>
#include <drain/StringTools.h>

#include "MapReader.h"
#include "Proj6.h"

namespace drain
{


#if PROJ_VERSION_MAJOR == 6

Proj6::Proj6() : pjContext(nullptr), proj(nullptr) {

}

Proj6::Proj6(const Proj6 &p) : pjContext(nullptr), proj(nullptr) {
	//cerr << "Proj4(const Proj4 &p) - someone needs me" << endl;
	setProjectionSrc(p.getProjStrSrc());
	setProjectionDst(p.getProjStrDst());
}

#else

Proj6::Proj6() : pjContext(proj_context_create()), proj(nullptr) {

}

Proj6::Proj6(const Proj6 &p) : pjContext(proj_context_clone(p.pjContext)), proj(nullptr) {
	//cerr << "Proj4(const Proj4 &p) - someone needs me" << endl;
	setProjectionSrc(p.getProjStrSrc());
	setProjectionDst(p.getProjStrDst());
}

#endif


Proj6::~Proj6(){
	proj_destroy(proj);
	proj_context_destroy(pjContext);
}

/*
const std::string & Proj6::getProjVersion(){
	const static PJ_INFO & pj_info = proj_info();// valgrind error
	const static std::string version(pj_info.version);
	return version;
}
*/


void Proj6::setDirectMapping(bool lenient){
	drain::Logger mout(__FILE__, __FUNCTION__);
	if (isSet()){ // (projSrc != nullptr) && (projDst != nullptr)
		/*
		mout.warn("srcProj ", src.getProjDef(Projector::ORIG));
		mout.warn("srcProj ", src.getProjDef(Projector::PROJ4));
		mout.warn("srcProj ", src.getProjDef(Projector::PROJ5));
		mout.warn("srcProj ", src.getProjDef(Projector::SIMPLE));

		mout.warn("dstProj ", dst.getProjDef(Projector::ORIG));
		mout.warn("dstProj ", dst.getProjDef(Projector::PROJ4));
		mout.warn("dstProj ", dst.getProjDef(Projector::PROJ5));
		mout.warn("dstProj ", dst.getProjDef(Projector::SIMPLE));
		*/
		// std::cout << proj_get_type(projSrc) << '#' << proj_get_type(projDst) << '\n';
		proj = proj_create_crs_to_crs_from_pj(pjContext, src.pj, dst.pj, 0, nullptr);
		mout.debug("set CRS-to-CSR mapping EPSG's ", src.getEPSG(), " <-> ", dst.getEPSG());
	}
	else if (!lenient) { // std::string()
		mout.fail(getErrorString(), " ...either proj unset");
	}
}


std::ostream & operator<<(std::ostream & ostr, const Proj6 &p){
	ostr << "Proj src: " << p.getProjStrSrc() << '\n';
	ostr << "Proj dst: " << p.getProjStrDst() << '\n';
	return ostr;
}

DRAIN_TYPENAME_DEF(Proj6);

}

// Drain
