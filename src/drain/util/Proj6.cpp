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
#include <drain/String.h>

#include "MapReader.h"
#include "Proj6.h"

namespace drain
{

const SprinterLayout Projector::projDefLayout(" ","","=", "",""); // space-separated, =, no hypens

const std::string Projector::proj4version = drain::StringBuilder<'.'>(PROJ_VERSION_MAJOR, PROJ_VERSION_MINOR, PROJ_VERSION_PATCH);



Projector::Projector(const Projector & pr) :
#if PROJ_VERSION_MAJOR == 6
		pjContext(nullptr),
		#else
		pjContext(proj_context_clone(pr.pjContext)), // NEW 2024
#endif
	// TODO: flag for own CTX => destroy at end
	pj(proj_clone(pjContext, pr.pj)),
	epsg(pr.epsg)
{
	// TODO
	//projDefs = {{ORIG,"*"}, {CHECKED,"**"}, {FINAL,"***"}, {INTERNAL,"****"}};
	setProjection(pr.getProjDef(ORIG)); // imitate ?
}


void Projector::setProjection(int epsg, CRS_mode crs){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.info("Setting epsg: ", epsg, ", expanding projDef");

	std::stringstream sstr;
	sstr << "EPSG:" << epsg;

	this->epsg = epsg;
	projDefs[ORIG] = sstr.str();

	PJ *pjTmp = proj_create(0, sstr.str().c_str()); // pjContext: better with or without?

	const char *projStr = proj_as_proj_string(pjContext, pjTmp, PJ_PROJ_4, nullptr);
	mout.info("Converted 'EPSG:", epsg, "' to non-(x,y)-swapping '", projStr,"'");

	projDefDict.clear();
	getProjDefDict(projStr, projDefDict);
	proj_destroy(pjTmp);

	updateProjectionDefs(crs);
}


void Projector::setProjection(const std::string &projDefStr, CRS_mode crs){

	drain::Logger mout(__FILE__, __FUNCTION__);

	getProjDefDict(projDefStr, projDefDict);

	mout.debug("projDefDict: ", projDefDict);

	epsg = extractEPSG(projDefDict);

	if (epsg > 0){
		setProjection(epsg);
		/*
		mout.info("Read epsg: ", epsg, ", expanding projDef");
		projDefDict.clear();
		std::stringstream sstr;
		sstr << "EPSG:" << epsg;
		PJ *pjTmp = proj_create(0, sstr.str().c_str()); // pjContext: better with or without?
		const char *projDefStrNew = proj_as_proj_string(pjContext, pjTmp, PJ_PROJ_4, nullptr);
		mout.attention("Converted 'EPSG:", epsg, "' to non-(x,y)-swapping '", projDefStrNew,"'");
		getProjDefDict(projDefStrNew, projDefDict);
		proj_destroy(pjTmp);
		*/
	}
	else {
		updateProjectionDefs(crs);
	}

	projDefs[ORIG] = projDefStr;

}

void Projector::updateProjectionDefs(CRS_mode crs){

	drain::Logger mout(__FILE__, __FUNCTION__);

	std::set<std::string> excludeKeys = {"+init"};
	switch (crs) {
		case REMOVE_CRS:
			excludeKeys.insert("+type");  // +type=crs
			break;
		case FORCE_CRS:
			projDefDict.set("+type","crs");
			break;
		default:
			break;
	}

	std::stringstream sstr;
	getProjDefStr(projDefDict, sstr, excludeKeys);
	projDefs[MODIFIED] = sstr.str();

	projDefs[PROJ4].clear();
	projDefs[PROJ5].clear();
	projDefs[SIMPLE].clear();

	/* https://proj.org/development/reference/functions.html#c.proj_create
	 *
	 * If a proj-string contains a +type=crs option, then it is interpreted as a CRS definition.
	 * In particular geographic CRS are assumed to have axis in the longitude, latitude order and with degree angular unit.
	 * The use of proj-string to describe a CRS is discouraged.
	 * It is a legacy means of conveying CRS descriptions: use of object codes (EPSG:XXXX typically) or
	 * WKT description is recommended for better expressivity.
	 */
	pj = proj_create(pjContext, sstr.str().c_str());

	if (pj == nullptr){
		mout.warn("orig:    ", projDefs[ORIG]);
		mout.warn("checked: ", projDefs[MODIFIED]);
		// mout.warn("final:   ", projDefs[FINAL]);
		mout.error(getErrorString(), " - could not derive projection from: ", sstr.str());
		//throw std::runtime_error(getErrorString() + "(" + str + ") /" + __FILE__ + ':' + __FUNCTION__);
		// if warn only -> clear() ?
		return;
	}

	projDefs[PROJ4] = proj_as_proj_string(pjContext, pj, PJ_PROJ_4, nullptr);
	projDefs[PROJ5] = proj_as_proj_string(pjContext, pj, PJ_PROJ_5, nullptr);

	std::stringstream sstrSimple;
	getProjDefStr(projDefDict, sstrSimple, {"+init", "+type", "+units", "EPSG"});
	projDefs[SIMPLE] = sstrSimple.str();


	// test: redo
	//proj_destroy(pj);
	//pj = proj_create(pjContext, s);

	// The returned string is valid while the input obj parameter is valid, and until a next call to proj_as_proj_string() with the same input object.

}


void Projector::getProjDefDict(const std::string & src, ProjDef & projDef){ // , const std::set<string> & exclude){

	drain::Logger mout(__FILE__, __FUNCTION__);

	std::list<std::string> projDefList;
	drain::StringTools::split(src, projDefList, ' ');

	mout.debug("Split: ", projDefList.size(), " items");

	std::string key,value;
	for (const auto & entry: projDefList) {

		value.clear(); // split2 does not clear

		drain::StringTools::split2(entry, key, value, "=:"); // Using '=' for standard +params, ':' for EPSG
		mout.debug(key, " - ", value);

		if (key == "+init"){
			mout.discouraged("Use of +init (", entry, ')');
		}
		else if (key == "+type"){
			if (value != "crs"){
				mout.warn("+type detected, with value='", value, "' (not 'crs' )");
			}
		}
		projDef.set(key, value);

	}

	return;
}


void Projector::getProjDefStr(const ProjDef & projDef, std::stringstream & sstr, const std::set<std::string> & exclude){

	drain::Logger mout(__FILE__, __FUNCTION__);
	//drain::Sprinter::sequenceToStream(cout, projDef, SprinterLayout(" ","","=", "",""));

	char sep = 0;
	for (const ProjDef::entry_t & entry: projDef){

		if (exclude.find(entry.first) != exclude.end()){
			mout.debug("Skipping ", entry.first, "=", entry.second);
			continue;
		}

		if (sep)
			sstr << sep;
		else
			sep = ' ';

		sstr << entry.first;
		if (entry.first == "EPSG"){ // special handling
			sstr << ':' << entry.second;
		}
		else if (!entry.second.empty()){
			sstr << '=' << entry.second;
		}

		/*
		if (entry.first == "EPSG"){
			//epsg = atoi(entry.second);
			epsg = drain::StringTools::convert<int>(entry.second);
			sstr.str("");
			sstr << entry.first << ':' << entry.second;
			if (projDef.size() > 1){
				mout.warn("Proj def 'EPSG:", entry.second, "', discarding (", projDef.size(),") extra arguments");
			}
			return epsg;
		}
		else if (entry.first == "+init"){
			std::string k;
			StringTools::split2(entry.second, k, epsg, ':');
			if (k == "epsg"){
				mout.discouraged("+init detected, EPSG: ", epsg);
				mout.advice("Consider plain format: 'EPSG: ", epsg, "'");
			}
			else {
				mout.warn("+init detected,  but without EPSG setting, arg=", entry.second);
			}
		}
		else {

		}
		*/
	}

	//src = sstr.str();
	//return epsg;

}


/*
PJ *Projector::getProjection(const std::string & projDef, CRS_mode CRS) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	std::stringstream sstr;

	// If +type=crs in string, remove it initially, then add if required.
	const int EPSG = filterProjStr(projDef, sstr, CRS); // (CRS==FORCE_CRS ? REMOVE_CRS:CRS));

	if (EPSG > 0){
		//sstr.str("");
		// sstr << "EPSG:" << EPSG;
		if (EPSG==4326){
			mout.note("Using EPSG:4326 implies CRS mode with swapped axis order");
			if (CRS != FORCE_CRS){
				mout.advice("Consider also proj str: '+proj=longlat +datum=WGS84 +no_defs +type=crs'");
			}
		}
		if (CRS == ACCEPT_CRS){
			mout.note("using EPSG:<code> implies CRS mode");
		}
		else if (CRS == REMOVE_CRS){
			mout.error("using EPSG:<code> implies CRS mode");
		}
	}
	else {
		if (CRS == FORCE_CRS){
			sstr << " +type=crs";
		}
	}

	const std::string projDefFinal(sstr.str());

	mout.note("Final proj str: ", projDefFinal);

	PJ *pj = proj_create(pjContext, projDefFinal.c_str());

	if (pj == nullptr){
		mout.warn("orig:  ", projDef);
		mout.warn("final: ", projDefFinal);
		mout.error(getErrorString(), " - could not derive projection from: ", projDefFinal);
		//throw std::runtime_error(getErrorString() + "(" + str + ") /" + __FILE__ + ':' + __FUNCTION__);
	}

	return pj;
}
*/



bool Projector::isLongLat(const PJ *pj) {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (pj == nullptr){
		mout.error("PJ object not set");
		return false;
	}

	bool result = false;
	/*
	const PJ_COORDINATE_SYSTEM_TYPE type = proj_cs_get_type(pjContext, proj);
	switch (type) {
	case PJ_CS_TYPE_ELLIPSOIDAL:
	case PJ_CS_TYPE_SPHERICAL:
		result = true;
		break;
	default:
		break;
	}
	*/

	const PJ_TYPE type = proj_get_type(pj);
	switch (type) {
	case PJ_TYPE_GEOGRAPHIC_CRS:
	case PJ_TYPE_GEOGRAPHIC_2D_CRS:
	case PJ_TYPE_GEOGRAPHIC_3D_CRS:
		result = true;
		break;
	default:
		break;
	}


	if (mout.isLevel(LOG_DEBUG)){
		mout.debug("PJ_TYPE=", type, " '", proj_as_proj_string(0, pj, PJ_PROJ_5, nullptr), "' longLat=", result);
	}

	return result;
	// pj_is_latlong(projDst);
	//return pj_is_geocent(projSrc);

	/*
	PJ_COORDINATE_SYSTEM_TYPE type = proj_cs_get_type(pjContext, projDst);
	mout.attention("isLongLat:", getProjectionDst(), " type=", type);
	return (type != PJ_CS_TYPE_CARTESIAN);
	if (info.id != nullptr){
		mout.debug("ID:", info.id);
		return (strcmp(info.id, "latlong")==0);
	}
	*/

}

/*
const std::string & Projector::getProjection(const PJ *pj, std::string & projDef) const {


	if (pj == nullptr){
		projDef.clear();
    }
	else {
		// The returned string is valid while the input obj parameter is valid, and until a next call to proj_as_proj_string() with the same input object.
		const char *s = proj_as_proj_string(pjContext, pj, PJ_PROJ_5, nullptr);
		projDef.assign(s);
		// free(s);
	}
	return projDef;
}
*/

void Projector::info(PJ *pj, std::ostream & ostr, int wkt){

	if (pj == nullptr){
		ostr << "null Projector::info";
		return;
	}

	/*
	std::string str;
	getProjection(pj, str);
	ostr << "'" << str << "'";
	*/

	PJ_TYPE type = proj_get_type(pj);
	ostr << "enumtype:" << type;

	PJ_PROJ_INFO info = proj_pj_info(pj);

	if (info.id != nullptr){
		ostr << ", ID:" << info.id;
	}

	if (info.definition != nullptr){
		ostr << ", '" << info.definition << "'";
	}

	if (info.description){
		ostr << ", (" << info.description << ")";
	}

	ostr << ", accuracy:";
	if (info.accuracy != -1)
		ostr << info.accuracy;
	else
		ostr << "unknown";

	ostr << ", has_inverse:" << (info.has_inverse?"YES":"NO") << ' ';

	ostr << ", longLat:" << (isLongLat(pj)?"YES":"NO") << ' ';

	if (wkt >= 0){ // PJ_WKT2_2019_SIMPLIFIED
		ostr << " WKT:\n " << proj_as_wkt(pjContext, pj, (PJ_WKT_TYPE)wkt, nullptr) << '\n';
	}


	ostr << std::flush;

}



int Projector::extractEPSG(const ProjDef & dict){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//std::ostream dummy(nullptr);
	//return filterProjStr(projDef, dummy, ACCEPT_CRS);

	int epsg = 0;

	for (const ProjDef::entry_t & entry: dict){

		if (entry.first == "EPSG"){
			//epsg = atoi(entry.second);
			epsg = drain::StringTools::convert<int>(entry.second);
			if (dict.size() > 1){
				mout.warn("Proj def 'EPSG:", entry.second, "', with (", (dict.size()-1) ,") extra arguments");
			}
			// return epsg;
		}
		else if (entry.first == "+init"){
			std::string k;
			StringTools::split2(entry.second, k, epsg, ':');
			if (k == "epsg"){
				mout.discouraged("+init detected, epsg:", epsg);
				mout.advice("Preferred format: 'EPSG:", epsg, "'");
			}
			else {
				mout.warn("+init detected,  but without EPSG setting, arg=", entry.second);
			}
		}
		else if (entry.second.empty() && (dict.size()==1)) { // plain number
			//epsg = atoi(entry.first);
			epsg = drain::StringTools::convert<int>(entry.first);
			if (epsg > 0){
				mout.ok("Plain number (", epsg, ") handled as EPSG code.");
			}
		}
	}

	//src = sstr.str();
	return epsg;

}



#if PROJ_VERSION_MAJOR == 6

Proj6::Proj6() : pjContext(nullptr), proj(nullptr) {

}

Proj6::Proj6(const Proj6 &p) : pjContext(nullptr), proj(nullptr) {
	//cerr << "Proj4(const Proj4 &p) - someone needs me" << endl;
	setProjectionSrc(p.getProjectionSrc());
	setProjectionDst(p.getProjectionDst());
}

#else

Proj6::Proj6() : pjContext(proj_context_create()), proj(nullptr) {

}

Proj6::Proj6(const Proj6 &p) : pjContext(proj_context_clone(p.pjContext)), proj(nullptr) {
	//cerr << "Proj4(const Proj4 &p) - someone needs me" << endl;
	setProjectionSrc(p.getProjectionSrc());
	setProjectionDst(p.getProjectionDst());
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


void Proj6::setMapping(bool lenient){
	drain::Logger mout(__FILE__, __FUNCTION__);
	if (isSet()){ // (projSrc != nullptr) && (projDst != nullptr)
		//std::cout << proj_get_type(projSrc) << '#' << proj_get_type(projDst) << '\n';
		proj = proj_create_crs_to_crs_from_pj(pjContext, src.pj, dst.pj, 0, nullptr);
		mout.debug("set CRS-to-CSR mapping EPSG's ", src.getEPSG(), " <-> ", dst.getEPSG());
	}
	else if (!lenient) { // std::string()
		mout.fail(getErrorString(), " ...either proj unset");
	}
}


std::ostream & operator<<(std::ostream & ostr, const Proj6 &p){
	ostr << "Proj src: " << p.getProjectionSrc() << '\n';
	ostr << "Proj dst: " << p.getProjectionDst() << '\n';
	return ostr;
}

DRAIN_TYPENAME_DEF(Proj6);

}

// Drain
