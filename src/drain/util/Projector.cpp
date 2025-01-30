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


void Projector::clear(){
	// projDefs = {{ORIG,""}, {MODIFIED,""}, {PROJ4,""}, {PROJ5,""}, {SIMPLE,""}};
	for (auto & entry: projDefs){
		entry.second.clear();
	}
	// TODO/ RE-DESIGN:
	// proj_destroy_context(pjContext) (because its cloned, right?)
	pjContext = nullptr; // TODO: flag for own CTX => destroy at end
	proj_destroy(pj);
	pj = nullptr;
	projDef.clear();
	epsg = 0;
}


void Projector::setProjection(int epsg, CRS_mode crs){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.info("Setting epsg: ", epsg, ", expanding projDef");
	clear();

	// drain::StringBuilder<':'> projDefStr("EPSG", epsg);
	this->epsg = epsg;
	createProjection(drain::StringBuilder<':'>("EPSG", epsg), crs);

	/*
	projDefs[ORIG] = epsgStr; //sstr.str();

	PJ *pjTmp = proj_create(0, epsgStr.c_str()); // pjContext: better with or without?

	const char *projStr = proj_as_proj_string(pjContext, pjTmp, PJ_PROJ_4, nullptr);
	mout.info("Converted 'EPSG:", epsg, "' to non-(x,y)-swapping '", projStr,"'");

	projDefDict.clear();
	getProjDefDict(projStr, projDefDict);
	proj_destroy(pjTmp);

	updateProjectionDefs(crs);
	*/
}





void Projector::setProjection(const std::string &projDefStr, CRS_mode crs){

	drain::Logger mout(__FILE__, __FUNCTION__);

	int epsg = extractEPSG(projDefStr);
	if (epsg > 0){
		mout.accept<LOG_DEBUG>("EPSG=", epsg, " <- '", projDefStr, "'");
		setProjection(epsg);
	}
	else {
		clear(); // new 2025
		createProjection(projDefStr, crs);
	}

	mout.accept<LOG_DEBUG>("final PROJ5 format: ", getProjDef(PROJ5));

}



void Projector::createProjection(const std::string & projDefStr, CRS_mode crs){

	drain::Logger mout(__FILE__, __FUNCTION__);

	/*
	projDefs[ORIG].clear();
	projDefs[PROJ4].clear();
	projDefs[PROJ5].clear();
	projDefs[SIMPLE].clear();
	projDefs[MODIFIED].clear();
	*/

	/* https://proj.org/development/reference/functions.html#c.proj_create
	 *
	 * If a proj-string contains a +type=crs option, then it is interpreted as a CRS definition.
	 * In particular geographic CRS are assumed to have axis in the longitude, latitude order and with degree angular unit.
	 * The use of proj-string to describe a CRS is discouraged.
	 * It is a legacy means of conveying CRS descriptions: use of object codes (EPSG:XXXX typically) or
	 * WKT description is recommended for better expressivity.
	 */
	// mout.attention("CREATING: ", projDefStr);// sstr.str());
	pj = proj_create(pjContext, projDefStr.c_str()); // sstr.str().c_str());

	if (pj == nullptr){
		mout.warn("orig:    ", projDefs[ORIG]);
		mout.warn("checked: ", projDefs[MODIFIED]);
		// mout.warn("final:   ", projDefs[FINAL]);
		mout.error(getErrorString(), " - could not derive projection from: ", projDefStr); // sstr.str()
		//throw std::runtime_error(getErrorString() + "(" + str + ") /" + __FILE__ + ':' + __FUNCTION__);
		// if warn only -> clear() ?
		return;
	}

	projDefs[ORIG] = projDefStr;

	projDefs[PROJ4] = proj_as_proj_string(pjContext, pj, PJ_PROJ_4, nullptr);
	mout.success<LOG_DEBUG>("created (proj4): ", projDefs[PROJ4]);

	projDefs[PROJ5] = proj_as_proj_string(pjContext, pj, PJ_PROJ_5, nullptr);
	mout.success<LOG_DEBUG>("created (proj5): ", projDefs[PROJ5]);

	//getProjDefDict(projDefs[PROJ4], projDefDict);
	getProjDefDict(projDefs[PROJ4]);

	std::set<std::string> excludeKeys = {"+init"};
	switch (crs) {
		case REMOVE_CRS:
			excludeKeys.insert("+type");    // REMOVE!
			break;
		case FORCE_CRS:
			projDef.set("+type","crs");  // ADD
			break;
		default:
			break;
	}

	std::stringstream sstr;
	getProjDefStr(sstr, excludeKeys);
	projDefs[MODIFIED] = sstr.str();

	//std::stringstream sstrSimple;
	sstr.str("");
	getProjDefStr(sstr, {"+init", "+type", "+units", "EPSG"});
	projDefs[SIMPLE] = sstr.str();

	mout.accept<LOG_DEBUG>("CREATED proj5: ", projDefs[SIMPLE]);

	// test: redo
	// proj_destroy(pj);
	// pj = proj_create(pjContext, s);
	// The returned string is valid while the input obj parameter is valid, and until a next call to proj_as_proj_string() with the same input object.

}


void Projector::getProjDefDict(const std::string & src){ //, ProjDef & projDef){ // , const std::set<string> & exclude){

	drain::Logger mout(__FILE__, __FUNCTION__);

	std::list<std::string> projDefList;
	drain::StringTools::split(src, projDefList, ' ');

	mout.debug("Split: ", projDefList.size(), " items");

	std::string key,value;
	for (const std::string & entry: projDefList) {

		if (entry.empty()){
			// Late trimming of input string...  (leading and/or trailing part)
			continue;
		}

		value.clear(); // split2 does not clear

		drain::StringTools::split2(entry, key, value, "=:"); // Using '=' for standard +params, ':' for EPSG
		//mout.pending<LOG_NOTICE>(key, " - ", value);

		if (key == "+init"){
			mout.discouraged("Use of +init (", entry, ')');
		}
		else if (key == "+type"){
			if (value != "crs"){
				mout.warn("+type detected, with value='", value, "' (not 'crs' )");
			}
		}
		if (value.empty() && (key != "+no_defs")){
			mout.warn("value empty for key '", key, "', entry:", entry);
		}
		projDef.set(key, value);

	}

	return;
}


//void Projector::getProjDefStr(const ProjDef & projDef, std::stringstream & sstr, const std::set<std::string> & exclude){
void Projector::getProjDefStr(std::stringstream & sstr, const std::set<std::string> & exclude) const {

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
	}

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

int Projector::extractEPSG(const std::string & s){

	for (std::string key: {"EPSG:", "+init=epsg:"}){
		int epsg = 0;
		size_t i=0;
		 if ((i = s.find(key)) != std::string::npos){
			 epsg = ::atoi(s.substr(i+key.length()).c_str());
			 if (i > 0){
				 drain::Logger(__FILE__, __FUNCTION__).suspicious("extra arguments in EPSG(", epsg, ") definition ", s);
			 }
			 if (epsg == 0){
				 drain::Logger(__FILE__, __FUNCTION__).suspicious("Failed in extracting non-zero EPSG code from '", s.substr(i+key.length()), "', from: ", s, "'");
			 }
			 return epsg;
		 }
	}

	return ::atoi(s.c_str());

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



}

// Drain
