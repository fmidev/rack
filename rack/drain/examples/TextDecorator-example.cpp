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
REQUIRE:    drain/util/{Log,Flags,Sprinter,String,TextDecorator}.cpp
pikamake.sh drain/examples/TextDecorator-example.cpp

 */

//#include <stdlib.h>
//#include <iostream>
#include <sstream>

#include "drain/util/TextDecorator.h"



int main(int argc, char **argv){

	drain::TextDecorator mika;
	mika.debug(std::cout);


	if (argc < 0){
		std::cerr << "Assign variables and literals quickly to a string(stream) \n";
		std::cerr << "Usage:\n\t" << argv[0] << " arg [ args...] <x>,<x2>,...\n";
		std::cerr << "Example:\n\t " << argv[0] << " world and the rest of the world\n";
		return 1;
	}


	mika.set(drain::TextDecorator::UNDERLINE, drain::TextDecorator::RED, drain::TextDecorator::GREEN, drain::TextDecorator::ITALIC);
	//std::cout << "Moi" << '\n';

	std::cout << mika << '\n';

	for (int i=1; i<argc; ++i){
		mika.reset();
		mika.set(argv[i]);
		std::cout << "now " << mika << '\n';
	}
	//std::cerr << drain::StringBuilder("Hello '", argv[1], "', and the rest ", argc-2 , " arguments...")       << std::endl;


	return 0;
}
