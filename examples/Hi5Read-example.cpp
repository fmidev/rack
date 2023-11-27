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
   g++ -I. -Idrain -I/usr/include/hdf5/serial -L../drain examples/Hi5Read-example.cpp hi5/Hi5.cpp hi5/Hi5Read.cpp  -ldrain -lhdf5 -o Hi5Read-example
   // g++ -I.. -L../drain/Debug/ examples/Hi5Read-example.cpp hi5/Hi5.cpp hi5/Hi5Read.cpp  -ldrain -lhdf5 -o Hi5Read-example
 */
#include <iostream>
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"

using namespace std;
//using namespace drain;
//using namespace rack;

int main(int argc, char **argv){

	if (argc==1){
		cout << "Usage:\n\t" << argv[0] << "<input.h5>\n";
		return 1;
	}

	Hi5Tree h5;
	hi5::Reader::readFile(argv[1], h5);
	h5.dumpContents(cout);

	return 0;
}
