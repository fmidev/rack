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
/*  // {Type,RegExp,
  g++ -Idrain drain/examples/Dictionary-example drain/util/Log.cpp -o Dictionary-example
 */
#include <iostream>

//#include "../util/Dictionary.h"

//#include "../util/Type.h"
//
#include "../util/Dictionary.h"


using namespace std;
//using namespace drain;

/*
 *
 *
 */
#include <limits>
#include <typeinfo>
#include <stdlib.h>

//#include <pair>
#include <list>


int main(int argc, char **argv){

	typedef drain::Dictionary2Ptr<int, const type_info> dict_t;
	dict_t dict;

	dict.add(123, typeid(int));
	dict.add(3,  typeid(float));
	dict.add(4,  typeid(double));
	dict.add(12, typeid(bool));

	for (dict_t::const_iterator it = dict.begin(); it!=dict.end(); ++it){
		cout << "entry: " << it->first << ':' << it->second->name() << endl;
		cout << "\tsearch: " << dict.getValue(it->first).name() << endl;
	}
	dict.toOstr();

	dict.findByKey(123);
	dict.findByValue(typeid(double));

	/*
	const std::type_info & t0 = dict.getValue(123);
	cout << "t=" << t0.name() << endl;
	*/

	if (argc <= 1){
		cout << "Sets type according to given character codes.\n";
		cout << "Usage:\n  " << argv[0] << " <type-char> [<type-char> <type-char> ... ]\n";
		cout << "Example:\n  " << argv[0] << " s S c C i I l L  f d  X=guess \n";
		return 1;
	}

	return 0;
}
