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
REQUIRE:    drain/util/{Log,Flags,Sprinter,String,TextDecorator,TextStyle,Type}.cpp
pikamake.sh drain/examples/TextDecorator-example.cpp

 */

//#include <stdlib.h>
//#include <iostream>
#include <sstream>

#include "drain/util/TextDecorator.h"

template <typename T>
void appendArg(std::stringstream & sstr, const T & arg){

}

void addText(std::stringstream & sstr){
}

template <typename T, typename ... TT>
void addText(std::stringstream & sstr, const T & arg, const TT &... args){
	appendArg(sstr, arg);
	addText(args...);
}




int main(int argc, char **argv){

	drain::TextStyleVT100 vt100;

	vt100.set(drain::TextStyle::BLUE, drain::TextStyle::UNDERLINE);

	std::stringstream sstr;
	vt100.write(sstr, drain::TextStyle::BLUE, "Kokeilu", " jatkuu..", drain::TextStyle::RED, "...valkoisena",
			drain::TextStyle::BOLD, " kuuna ",
			drain::TextStyle::UNDERLINE, drain::TextStyle::DIM, "haaleana", drain::TextStyle::REVERSE, " reverssinä ",
			drain::TextStyle::BOLD, " polttosena ", drain::TextStyle::GREEN);

	std::cout << sstr.str() << std::endl;

	drain::TextDecorator mika;
	mika.debug(std::cout);



	if (argc < 0){
		std::cerr << "Assign variables and literals quickly to a string(stream) \n";
		std::cerr << "Usage:\n\t" << argv[0] << " arg [ args...] <x>,<x2>,...\n";
		std::cerr << "Example:\n\t " << argv[0] << " world and the rest of the world\n";
		return 1;
	}


	mika.set(drain::TextStyle::UNDERLINE, drain::TextStyle::RED, drain::TextStyle::GREEN, drain::TextStyle::ITALIC);
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
