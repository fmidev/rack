/*

    Copyright 2001 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

/*
 REQUIRE: drain/util/{Log,Proj6,Sprinter,String,TextStyle}.cpp
 LIBS: -lproj
 */
#include <cmath>
#include <iostream>
#include <sstream>
//#include <proj_api.h>
#include <proj.h>

#include "drain/util/Log.h"
#include "drain/util/Point.h"
#include "drain/util/Proj6.h"
//#include "drain/util/Sprinter.h"

using namespace std;
using namespace drain;

void tests(){

	std::list<int> epsg = {3067, 3035};


}

int main(int argc, char **argv){

	drain::getLog().setVerbosity(LOG_INFO);

	/*
	drain::Dictionary<std::string,std::string> projDef;
	projDef.set("+proj", "latlon");
	projDef.set("+lat_0", "60");
	projDef.set("+proj", "durch");
	*/
	// NOTICE: Sprinter cannot hande non-arg options like +no_defs !
	// drain::Sprinter::sequenceToStream(cout, projDef, SprinterLayout(" ","","=", "",""));
	// drain::Sprinter::mapToStream(cout, tree, Sprinter::jsonLayout, {});
	// std::cerr << drain::sprinter(tree) << std::endl;


	PJ_CONTEXT * projCtx = proj_context_create();

	const char *projDefIn  = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs +type=crs";
	const char *projDefOut = "EPSG:3067";
	//+proj=longlat +datum=WGS84 +no_defs

	int wkt = -1;

	switch (argc) {
		case 1:
			cout << "C++ drain::Proj6 demo (version: "<< Proj6::getProjVersion() << ")\n";
			// cout << proj.getProjectionSrc() << '\n';
			cout << "Usage: cat coordinates.txt | " << argv[0] << " <projDefIn> <projDefOut>\n";
			cout << "Examples:\n";
			cout << "  echo 25.0 63.1  | " << argv[0] << " '" << projDefIn << "' '" << projDefOut << "'\n";
			cout << "  echo 25.0 63.1  | " << argv[0] << " '" << projDefIn << "' '+proj=aeqd +lon_0=25 +lat_0=60  +ellps=WGS84'\n";
			cout << "  echo 0 1000000  | " << argv[0] << " '+proj=aeqd +lon_0=25 +lat_0=60  +ellps=WGS84' 'EPSG:4326'\n";
			cout << "  echo 100000.0 0 | " << argv[0] << " '+proj=aeqd +lon_0=25 +lat_0=60  +ellps=WGS84 +type=crs' 'EPSG:4326' \n";
			cout << "  echo 25.0  70.0 | " << argv[0] << " 'EPSG:4326' '+proj=aeqd +lon_0=25 +lat_0=60  +ellps=WGS84 +type=crs' \n";

			return 0;
			// no break
		case 4:
			wkt = atoi(argv[3]);
			// no break;
		case 3:
			projDefOut  = argv[2];
			// no break
		case 2:
			projDefIn = argv[1];
			break;
		default:
			cerr << "Wrong number of arguments: " << argc << endl;
			return 1;
	}

	Proj6 proj;
	proj.setProjectionSrc(projDefIn); // , drain::Projector::REMOVE_CRS
	cerr << "Retrieved projStr src: " << proj.getProjectionSrc() << '\n';
	proj.setProjectionDst(projDefOut); // , drain::Projector::REMOVE_CRS
	cerr << "Retrieved projStr dst: " << proj.getProjectionDst() << '\n';
	//cerr << "Is longLat: " << std::flush;
	//cerr << (proj.isLongLat()?"YES":"NO") << std::endl;

	//cerr << "Debug:\n";
	proj.debug(cerr, wkt);
	//cerr << "End debug\n";


	// Standard
	/*
	PJ *P = proj_create_crs_to_crs(0, proj.getProjectionSrc().c_str(), proj.getProjectionDst().c_str(), 0);
	if (P == nullptr){
		cerr << "Error...\n";
		return 1;
	}
	PJ_COORD coord;
	PJ_COORD coord2;
	const double r2d = 180.0/M_PI;
	const double d2r = M_PI/180.0;
	*/

	//proj_trans(P, PJ_DIRECTION::PJ_FWD, coord);
	/**
	 *		double PJ_LP.lam - Longitude. Lambda.
	 *		double PJ_LP.phi - Latitude. Phi.
	 *
	proj_trans_generic (
	    P, PJ_INV,
	    &(survey[0].x), stride, 345,  //  We have 345 eastings  /
	    &(survey[0].y), stride, 345,  //  ...and 345 northings.
	    &height, sizeof(double), 1,   //  The height is the constant  23.45 m
	    0, 0, 0                       //  and the time is the constant 0.00 s
	);
	 */
	typedef drain::Point2D<double> pnt2;
	pnt2 point;



	while (!cin.eof()){
		cin >> point.x >> point.y;
		if (cin.eof())
			return 0;

		cout  << "Input: " << (point.x) << " E \t" << (point.y) << " N\n";
		proj.projectFwd(point);
		cout  << "Fwd:   " << (point.x) << " E \t" << (point.y) << " N\n";
		proj.projectInv(point);
		cout  << "Inv:   " << (point.x) << " E \t" << (point.y) << " N\n";
		cout << endl;
	}

	/*
	while (false){

		cin >> coord.lp.lam >> coord.lp.phi ;
		if (cin.eof())
			return 0;

		cout  << "Input: " << (coord.lp.lam) << " E \t" << (coord.lp.phi) << " N\n";

		// coord2 = proj_trans(P, PJ_DIRECTION::PJ_FWD, coord);
		// cout << (coord2.xy.x) << '\t' << (coord2.xy.y) << '\n';

		coord2.xy = {0,0};
		proj.projectFwd(coord.lp.lam, coord.lp.phi, coord2.xy.x, coord2.xy.y);
		cout << (coord2.xy.x) << '\t' << (coord2.xy.y) << " // drain fwd() \n";

		point.x = coord2.xy.x;
		point.y = coord2.xy.y;
		cout << "Point: " << point << " // orig \n";
		//proj_trans_generic(P, PJ_FWD, &pointM.x, sizeof(pnt2), 1, &pointM.y, sizeof(pnt2), 1, 0, 0, 0, 0, 0, 0);
		proj.projectInv(point);
		cout << "Point: " << point << " // fwd() \n";
		proj.projectFwd(point);
		//proj_trans_generic(P, PJ_INV, &pointM.x, sizeof(pnt2), 1, &pointM.y, sizeof(pnt2), 1, 0, 0, 0, 0, 0, 0);
		cout << "Point: " << point << " // inv() \n";

		//coord2.xy = {0,0};
		//coord.lp  = {0,0};
		coord = proj_trans(P, PJ_DIRECTION::PJ_INV, coord2);
		cout  << (coord.lp.lam) << " \t" << (coord.lp.phi) << " PJ-1 \n";

		coord.xy = coord2.xy;
		coord.lp = {0,0};
		proj_trans_array(P, PJ_DIRECTION::PJ_INV, 1, &coord);
		cout  << (coord.lp.lam) << " \t" << (coord.lp.phi) << " PJ-2 \n";

		coord.lp = {0,0};
		coord.xy = {0,0};
		proj.projectInv(coord2.xy.x, coord2.xy.y,  coord.lp.lam, coord.lp.phi);
		cout << (coord.lp.lam) << " \t" << (coord.lp.phi) << " // drain inv() \n";

		// cout << (coord.lp.phi) << '\t' << (coord.lp.lam) << '\n';
		// proj.projectFwd(x*d2r,y*d2r, x2,y2);
		// cout << x2 << '\t' << y2 << '\t';
		// proj.projectInv(x2,y2, x,y);
		// cout << x*r2d << '\t' << y*r2d << '\n';
	}
	*/

	/*
	while (false){
		cin >> coord.xy.x >> coord.xy.y;
		if (cin.eof())
			return 0;
		coord = proj_trans(P, PJ_DIRECTION::PJ_INV, coord2);
		cout << (coord.lp.phi) << '\t' << (coord.lp.lam) << '\n';
	}
	*/


	/*
	if (argc == 1){
	}


	stringstream sstr;
	for (int i = 0; i < argc; ++i) {
		sstr << argv[i] << ' ';
	}
	proj.setProjectionDst(sstr.str());

	cout << proj;

	const double r2d = 180.0/M_PI;
	const double d2r = M_PI/180.0;
	double x, y;
	double x2, y2;
	//while (!cin.eof()){
	while (true){
		cin >> x >> y;
		if (cin.eof())
			return 0;
		proj.projectFwd(x*d2r,y*d2r, x2,y2);
		cout << x2 << '\t' << y2 << '\t';
		proj.projectInv(x2,y2, x,y);
		cout << x*r2d << '\t' << y*r2d << '\n';
	}

	*/

	proj_context_destroy(projCtx);

	return 0;

}
