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

/**
REQUIRE: rack/{drain/util/{Log,String,Flags,TextReader},data/ODIMPath{,Matcher}}.cpp
// plus -I. -Idrain rack/examples/ODIMPath-example.cpp rack/{drain/util/{Log,String},data/ODIMPath}.cpp -o ODIMPath-example
g++ -I. -Idrain rack/examples/ODIMPath-example.cpp rack/{drain/util/{Log,String,Flags,TextReader},data/ODIMPath{,Matcher}}.cpp -o ODIMPath-example

 */

#include <ios>
#include <sstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <algorithm>
//#include "drain/util/Path.h"
//#include "drain/util/Dictionary.h"
#include "rack/data/ODIMPath.h"
#include "rack/data/ODIMPathMatcher.h"


template <class T>
void demo(const std::string & text, const T &x){
	std::cout << text << ':' << '\n';
	std::cout << "\t'" << x << "'\n\n";
}

using namespace rack;


int main(int argc, char **argv){

	typedef ODIMPathElem::dict_t dict_t;

	drain::Log & log = drain::getLog();
	log.setVerbosity(6);

	drain::Logger mout(__FILE__);

	ODIMPath path;
	ODIMPathMatcher matcher;



	std::string pathStr = "/dataset1/data2/quality21/abc123";

	switch (argc){
	case 3:
		matcher.assign(argv[2]);
	case 2:
		pathStr = argv[1];
		path.assign(argv[1]);
		break;

	default:

		std::cout << "ODIMPath assignment and matching demo\n\n";  // [separator]

		std::cout << "Usage: "  << argv[0] << " path \n";  // [separator]
		std::cout << "Usage: "  << argv[0] << " path [pathMatcher] \n";  // [separator]
		std::cout << "Example: "<< argv[0] << ' ' << pathStr << '\n'; // << separator << '\n';
		std::cout << "Example: "<< argv[0] << pathStr << " dataset1:3/data2 " << '\n'; // << separator << '\n';

		mout.info() << ODIMPathElem::getDictionary() << mout.endl;

		demo("empty path", path);
		demo("length", path.size());

		const dict_t & dict = ODIMPathElem::getDictionary();
		std::cout << "Types:\n";
		for (dict_t::const_iterator it=dict.begin(); it!=dict.end(); ++it){
			std::cout << "\t" << it->first << "\t= " << (int)it->second;
			if (ODIMPathElem::isIndexed(it->second))
				std::cout << "[]";
			std::cout << '\n';
		}

		path.assign(pathStr);
		path.debug();
		return 0;
	}

	if (!matcher.empty()){

		drain::getLog().setVerbosity(8);

		if (!path.empty()){
			if (!path.front().isRoot()){
				mout.note() = "Added root (leading empty element) to the path.";
				mout.info() = "The HDF5 paths handled in rack (in data selection) are always rooted. (Selectors are not.)";
				path.push_front(ODIMPathElem::ROOT);
			}
		}

		mout.ok() << "path    " << path << mout.endl;


		mout.ok() << "matcher " << matcher << mout.endl;

		if (matcher.match(argv[1])){
			mout.success() = "matches!";
		}
		else {
			mout.fail() = "match failed";
		}

		return 0;

	}

	if (path.empty()){
		mout.warn() << "empty path, skipping remaining operations" << mout.endl;
		return 0;
	}



	demo("input", pathStr);

	path = pathStr; //demo("input, trimmed", drain::StringTools::trim(pathStr));
	demo("path obj", path);
	path.debug();

	path.setElems(ODIMPathElem::DATASET, ODIMPathElem::DATA, ODIMPathElem::ARRAY);


	demo("write to output stream", path);


	demo("stl::front() returns the 1st element (often the root, which is an empty string)", path.front());

	demo("stl:: *(++path.begin()) returns the next element", *(++path.begin()));

	demo("stl::back() returns the last element", path.back());


	path.pop_back();
	demo("stl::pop_back (remove last)", path);

	path.push_back("extra");
	demo("stl::push_back", path);

	ODIMPathElem e;
	path >> e;
	demo("Path >> elem ; // applies back(), pop_back()", e);
	demo("path now", path);
	e = "new";
	path << e;
	demo("Path << elem ; // applies push_back(elem)", path);
	path.appendElems(ODIMPathElem::DATASET, ODIMPathElem::DATA);
	path.appendElem("what");
	path << ODIMPath("dataset/quality"); // should add indices
	//path <<
	demo("Path << \"subdir\" ; // applies push_back(elem)", path);
	path << "data";//  Ambiguous (elem or path).
	//

	//path.sort();
	//demo("stl::sort()", path);


	std::cout << "stl::splice (take one element from the middle):\n";
	if (path.size() < 3){
		std::cout << "(skipped, path too small)\n";
	}
	else {
		drain::Path<ODIMPathElem> path2;
		path2.splice(path2.begin(), path, ++(++path.begin()));
		std::cout << "path2: " << path2 << "\n";
		std::cout << "path:  " << path << "\n\n";


		std::cout << "stl::splice:\n";
		path2.splice(path2.begin(), path, ++(++path.begin()), path.end());
		std::cout << "path2: " << path2 << "\n";
		std::cout << "path:  " << path << "\n\n";

		std::cout << "stl::merge:\n";
		path.merge(path2);
		std::cout << path << "\n\n";

		/*
		std::cout << "stl::list::sort():\n";
		path.sort();
		std::cout << '\t' << path << "\n\n";
		 */
	}

	return 0;
}

