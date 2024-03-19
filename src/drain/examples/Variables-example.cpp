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
  REQUIRE: drain/util/{Caster,Castable,Log,Sprinter,String,RegExp,TextStyle,Time,Type,VariableLike}.cpp
  g+ + -I. drain/examples/Time-example.cpp drain/util/Time.cpp -o Time-example
 */

#include <stdlib.h>
#include <iostream>

#include "drain/util/Log.h"
#include "drain/util/VariableLike.h"



namespace drain {


// typedef VariableT<VariableBase<ReferenceBase<> > > FlexVar2;




template <>
const std::string TypeName<Variable>::name;

template <>
const std::string TypeName<Reference>::name;

template <>
const std::string TypeName<FlexibleVariable>::name;



template <>
const std::string TypeName<Variable>::name("Variable");

template <>
const std::string TypeName<Reference>::name("Reference");

template <>
const std::string TypeName<FlexibleVariable>::name("FlexibleVariable");


}

// \tparam T - Reference, Variable, FlexibleVariable
template <class T>
void testCastable(const char *arg, T & object){

	drain::Logger mout(__FILE__, __FUNCTION__);

	int i = 0;

	//drain::Castable & castable = object;

	mout.note(drain::TypeName<T>::str());

	mout.info();
	object.info(mout);
	mout << mout.endl;

	object = arg;
	//object.assign(arg);
	mout.info();
	object.info(mout);
	mout << mout.endl;

	std::cerr << "\n";
}

/// Demonstrates usage of drain::Time class.
/**

 Oddities with time zone. Looks like it is GMT initially, but changes to local(e) (e.g. EET)
 if unix seconds are given (%s):

 \code
   ./Time-example '0 2018/11/16 22:58' '%s %Y/%m/%d %H:%M' '%d.%m.%Y %H:%M,%S %Z'
   ./Time-example '2018/11/16 22:58'   '%Y/%m/%d %H:%M'    '%d.%m.%Y %H:%M,%S %Z'
   ./Time-example '2018/11/16 22:58 0' '%Y/%m/%d %H:%M %s' '%d.%m.%Y %H:%M,%S %Z'
   \endcode
*/
int main(int argc, char **argv){

	drain::getLog().setVerbosity(6);
	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::Variable var;
	drain::Reference ref;
	drain::FlexibleVariable flex;


	for (int a = 1; a < argc; ++a) {

		const char *arg = argv[a];

		int i = 0;
		double d = 0.0;
		std::string s;
		bool b = true;
		drain::UniTuple<double,3> triple = {1.1, 2.2, 3.3};


		mout.special("Variable");

		drain::Variable vi(typeid(int));
		testCastable(arg, vi);
		drain::Variable vi2(1.23456);


		drain::Variable vd(typeid(double));
		testCastable(arg, vd);

		drain::Variable vs(typeid(std::string));
		testCastable(arg, vs);


		mout.special("Referencer");

		drain::Reference ri(i); //(i);
		// ri.link(i);
		testCastable(arg, ri);

		drain::Reference rd(d); //(d);
		// rd.link(d);
		testCastable(arg, rd);

		drain::Reference rs(s);
		// rs.link(s);
		testCastable(arg, rs);

		drain::Reference rU3(triple); // (triple)
		// rU3.link(triple);
		testCastable(arg, rU3);

		ri.reset();


		mout.special("FlexibleVariable");

		drain::FlexibleVariable fi(123);
		testCastable(arg, fi);

		drain::FlexibleVariable fd(d); // (d) (45.678);
		testCastable(arg, fd);

		drain::FlexibleVariable fs; // ("Hello!");
		testCastable(arg, fs);


		mout.special("FlexibleVariable, linked");

		fi.link(i=0);
		// fi.assign(1234);
		testCastable(arg, fi);

		fd.link(d=0.0);
		// fd = 12345;
		testCastable(arg, fd);

		fs.link(s = "");
		testCastable(arg, fs);


	}

	/*
	int i = 123;
	ref.link(i);
	ref = 345.56;
	std::cerr << ref << ' ' << i << '\n';
	ref.info(std::cerr);
	std::cerr << '\n';

	std::cerr << var << '\n';
	var = 12.56;
	std::cerr << var << '\n';
	var.info(std::cerr);
	std::cerr << '\n';
	var = {1, 3, 4};
	var.info(std::cerr);
	std::cerr << '\n';

	drain::UniTuple<double,3> triple = {1.1, 2.2, 3.3};
	flex = triple;
	flex.info(std::cerr);
	std::cerr << '\n';
	flex = i;
	flex.link(triple);
	flex.info(std::cerr);
	triple.at(1) = 3.12314;
	flex.info(std::cerr);
	*/


	/*
	switch (argc){
	case 4:
		formatOut = argv[3];
		// no break
	case 3:
		formatIn = argv[2];
		// no break
	case 2:
		time.setTime(argv[1], formatIn);
		break;
	default:
		std::cerr << "Usage:\n  "   << argv[0] << " <time-string> [<format-in>]  [<format-out>]\n";
		std::cerr << "Example:\n  " << argv[0] << " '" << time.str(formatIn) << "' '" << formatIn << "' '" << formatOut << "'" << std::endl;
	}
	*/

	// std::cerr << time.getTime() << std::endl;
	//  time.str() valid as well.

	return 0;
}
