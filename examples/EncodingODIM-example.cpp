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
 *

RE: drain/util/{Log,Sprinter,String}.cpp data/{EncodingODIM,ODIMPath}.cpp

REQUIRE: data/{EncodingODIM,ODIMPath,Quantity}.cpp
FLAGS: -I../ -I/usr/include/hdf5/serial
LIBS: -L../../drain/Debug/  -ldrain

g + + -I.. -L../drain/Debug/ examples/Hi5Read-example.cpp hi5/Hi5.cpp hi5/Hi5Read.cpp  -ldrain -lhdf5 -o Hi5Read-example
 */
#include <iostream>
#include "data/Quantity.h"


using namespace std;
//using namespace drain;
using namespace rack;

int main(int argc, char **argv){

	drain::Log log(std::cerr, LOG_DEBUG);

	drain::Logger mout(log, __FUNCTION__, __FILE__);


	if (argc==-1){
		cout << "Usage:\n\t" << argv[0] << "<input.h5>\n";
		return 0;
	}

	//const string parameters(argv[1]);
	//drain::Range<double> range = {-32.0, 64.0};
	//mout.note(range);

	drain::ValueScaling scaling1({0.5, -32, {-32.0, +80.0}});
	mout.note(scaling1);

	drain::ValueScaling scaling2 = {0.5, -32, {-32.0, +80.0}};
	mout.note(scaling2);
	mout.note("scale2/tuple:", (const drain::UniTuple<double,4> &) scaling2);
	mout.note("scale2:", scaling2.scale, " offset:",  scaling2.offset, " range:", scaling2.getPhysicalRange());
	mout.note("scale2:", scaling2.scale, " offset:",  scaling2.offset, " range:", scaling2.physRange);

	drain::ReferenceMap rmap;
	rmap.link("sc2", scaling2.tuple(), "m/s");
	rmap.link("sc2phys", scaling2.physRange.tuple(), "...");
	mout.special("rmap", rmap);

	//drain::ValueScaling sc1
	drain::ValueScaling scalingCopy;
	scalingCopy = scaling2;
	mout.note("scaleCopied:", scalingCopy);

	EncodingODIM odimScaled = {'C', +0.5, -32.0,   0.0, 255.0, {-32.0, 80.0}};
	mout.note("odimScaled:", odimScaled);
	//odimScaled.setValues("how:physRange=-5:50");
	mout.special("odimScaled, scaling: ", odimScaled.scaling, " physRange", odimScaled.scaling.physRange);
	mout.note("odimScaled:", odimScaled);


	EncodingODIM odimRanged = {'C', {-32.0, 80.0}, 0.0, 255.0,  +0.5, -32.0,};
	mout.note("odimScaled:", odimScaled);
	mout.note("odimRanged:", odimRanged);
	EncodingODIM odimCopy;
	odimCopy = odimRanged;
	mout.note("odimCopied:", odimCopy);
	mout.note("odimCopied, scaling: ", odimCopy.scaling, " physRange", odimCopy.scaling.physRange);


	// Quantity quantity = {{}, 'C', -33.0};
	Quantity quantity = {"dBZ", {-32.0, 64.0}, 'C',
			{
					{'C', +0.5, -32.0,   0.0, 255.0, {-32.0, 80.0}},
					{'S', {-32.0, 80.0}}
			},
			-33.0
	};


	mout.note("quantity:", quantity);



	return 0;
}
