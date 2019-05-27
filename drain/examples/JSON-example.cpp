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

g++ -Idrain drain/examples/JSON-example.cpp drain/util/{JSONtree,Caster,Castable,Log,String,RegExp,Type}.cpp -o JSON-example

 */
/*
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../util/Log.h"
#include "../util/RegExp.h"
#include "../util/Tree.h"
#include "../util/VariableMap.h"
*/
#include <fstream>
#include "../util/JSONtree.h"

int main(int argc, char **argv){


	if (argc == 1){

		std::cerr << "Usage:\n\t"   << argv[0] << " (<json-file> | <path>:<attr>=<value> <path2>:<attr>=<value2> ... )\n";
		std::cerr << "Example:\n\t" << argv[0] << " test.json\n";
		std::cerr << "Example:\n\t" << argv[0] << " /metadata:comment=Hello /product/parameters:altitude=2500\n";
		//return 1;

	}

	// #include "JSON-example.inc"
	/*
	 *   Applications:
	 *
	 *   Palette
	 *
	 */

	drain::JSON::tree_t json('.');
	drain::JSON::indentStep = 4;



	/*
	json.data["comment"] = "example";
	json["product"].data["name"] = "pCAPPI";
	json["product"]["parameters"].data["altitude"] = 1500;
	json["geo"].data["width"]  = 600;
	json["geo"].data["height"] = 800;
	json["geo"].data["bbox"] << 18.6 << 57.9 << 34.9 << 69.0;
	json["geo"].data["proj"] = "+proj=stere +a=6371288 +lon_0=25E +lat_0=90N +lat_ts=60 +no_defs";
	*/

	json["met"].data["index"]         = 64;
	json["met"]["rain"].data["index"] = 68;
	json["met"]["snow"].data["index"] = 72;
	json["met"]["graupel"] = 76;
	json["met"]["hail"] = 82;

	drain::JSON::path_t path;

	/*
	//path = "/a/b/c/";
	//std::cout << path << std::endl;
	//t(path); // = "2l";
	*/

	for (int i = 1; i < argc; ++i) {

		std::string arg(argv[i]);

		/// Recognize assignment /path/abc/def:key=value
		size_t j = arg.find(':');
		if (j != std::string::npos){
			path = arg.substr(0,j);
			if (json.hasPath(path))
				std::cout << "Note: has already " << path << '\n';
			std::string attr = arg.substr(j+1);
			size_t k = attr.find('=');
			if (k != std::string::npos){
				json(path).data[attr.substr(0, k)] = attr.substr(k+1);
			}
			else {
				json(path).data[attr] = "";
			};
		}
		else {
			std::ifstream infile;
			infile.open(argv[i], std::ios::in);
			drain::JSON::read(json, infile);
		}

	}

	/*
	typedef std::list<tree_t::path_t> pathlist_t;
	pathlist_t paths;
	t.getPaths(paths);
	std::cout << "Paths: " << '\n';
	for (typename pathlist_t::const_iterator it = paths.begin(); it != paths.end(); ++it){
		std::cout << '"' << *it << '"' << '\n';
	}*/

	/*
	std::cout << "Dump: " << '\n';
	t.dump();

	std::cout << "Dump: " << '\n';
	t.dumpContents();
	*/

	std::cerr << "JSON: " << '\n';
	drain::JSON::write(std::cout, json);

	return 0;
}


