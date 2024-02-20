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
REQUIRE:    drain/util/{Log,TextReader,TextStyle}.cpp
pikamake.sh drain/examples/TextReader-example.cpp

 */

#include <sstream>

#include "drain/util/TextReader.h"

class CppValueReader : public drain::TextReader {

public:

	/// Default. For scalars etc.
	template <class T>
	static
	char scan(std::istream & istr, T & dst){
		return scanSegmentToValue(istr, ",}", dst);
	}

	template <class T>
	static
	char scan(std::istream & istr, std::list<T> & dst);

	template <class T1,class T2>
	static
	char scan(std::istream & istr, std::pair<T1,T2> & dst);


};

template <class T1,class T2>
char CppValueReader::scan(std::istream & istr, std::pair<T1,T2> & dst){
	//return scanSegmentToValue(istr, ",}", dst);
	char c;
	c = scan(istr, dst.first);
	if (c != ','){
		throw std::runtime_error("illegal char, expected comma ");
	}
	c = scan(istr, dst.second);
	return c; // can be ',' or '}'
}

template <>
char CppValueReader::scan(std::istream & istr, std::string & dst){
	char c = scanSegmentToValue(istr, "\"", dst);
	std::cout << " end char " << c << '\n';
	return c;
}


void test(const std::string & s){
	std::stringstream sstr(s);
	std::cout << sstr.str() << '\n';
	std::string s2;
	CppValueReader::scan(sstr, s2);
	std::cout << s << " <- " << sstr.str() << " <- " << s2 << '\n';
}

int main(int argc, char **argv){

	drain::TextReader mika;
	//mika.debug(std::cout);

	test("{koe}\", mika");

	if (argc < 0){
		std::cerr << "Assign variables and literals quickly to a string(stream) \n";
		std::cerr << "Usage:\n\t" << argv[0] << " arg [ args...] <x>,<x2>,...\n";
		std::cerr << "Example:\n\t " << argv[0] << " world and the rest of the world\n";
		return 1;
	}



	//std::cout << "Moi" << '\n';

	// std::cout << mika << '\n';

	for (int i=1; i<argc; ++i){
		// mika.reset();
		// mika.set(argv[i]);
		// std::cout << "now " << mika << '\n';
	}
	//std::cerr << drain::StringBuilder("Hello '", argv[1], "', and the rest ", argc-2 , " arguments...")       << std::endl;


	return 0;
}
