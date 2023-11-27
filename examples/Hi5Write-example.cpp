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
  g++  -I/usr/include/hdf5/serial -I. -Idrain -L/usr/lib/x86_64-linux-gnu/hdf5/serial/  -L../drain/Debug/ examples/Hi5Write-example.cpp data/ODIMPath.cpp  hi5/{Hi5,Hi5Write}.cpp  -ldrain -lhdf5 -o Hi5Write-example
 */
#include <iostream>

#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h"

using namespace std;
using namespace drain;

int main(int argc, char **argv){

	if (argc==1){
		cout << "Usage:\n\t" << argv[0] << "<output.h5>\n";
		return 0;
	}

	Hi5Tree h5;

	h5.data.attributes["test"] = 12345;

	h5["group1"].data.attributes["testInt"] = 123456789;
	h5("group1/data").data.attributes["testString"] = "Hello, world!";
	h5("group1/data").data.dataSet.initialize(typeid(unsigned char), 640,400);
	h5["group2"]["data"].data.dataSet.initialize(typeid(unsigned short), 320,200);
	h5["group2"]["data"].data.attributes["testDouble"] = 0.1234567890;

	hi5::NodeHi5 & node = h5["group2"]["data"].data;

	node.attributes["LEGEND"] = "1:Test,2:Other,3.456:Third";

	hi5::Writer::writeText(h5);

	//h5.dumpContents();

	/// todo attributes do not get stored?
	hi5::Writer::writeFile(argv[1], h5);


	return 0;
}
