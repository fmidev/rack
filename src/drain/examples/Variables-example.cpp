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
/*  // ,VariableLike
  REQUIRE: drain/{Caster,Castable,FlexibleVariable,Log,Reference,Sprinter,String,RegExp,TextStyle,Type,TypeUtils,Variable,VariableBase}.cpp
  REQUIRE: drain/util/Time.cpp
  g+ + -I. drain/examples/Time-example.cpp drain/util/Time.cpp -o Time-example
 */

#include <stdlib.h>
#include <iostream>

#include "drain/Log.h"
//#include "drain/VariableLike.h"
#include "drain/Reference.h"
#include "drain/Variable.h"
#include "drain/FlexibleVariable.h"
#include "drain/VariableAssign.h"

// \tparam T - Reference, Variable, FlexibleVariable
/*
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
*/

/// Checks if the value assigned to a drain::Castable stays same when reassigned.
/**
 *  \tparam T - base type
 *  \param  t - destination Type object
 */
template <class C = drain::Castable>
struct CastableTester {

	/**
	 *  \tparam T - base type
	 *  \param  t - destination Type object
	 */
	template <class T>
	static inline
	void callback(C & c){

		drain::Logger mout(__FILE__, __FUNCTION__);

		// Well, potentially becomes/arrives restricted already here...
		mout.note() << drain::TypeName<C>::get() << " <- " << drain::TypeName<T>::get();


		if ((c.isCharArrayString() && (c.getElementCount()<=1))  && (typeid(T)==typeid(char)) ){
			mout << "storage type char, empty(ish) variable object, skipping test (uninitialized local char)" << mout.endl;
			return;
		}


		const T x = c;
		mout << '(' << x << ')';
		c = x;

		// mout.note() << drain::TypeName<T>::get()
		// mout << '(' << x << ')' << " -> " << drain::TypeName<C>::get() << '=';
		mout << " => '" << c << "' : ";
		c.info(mout);
		mout << mout.endl;

		// TEST 1: Re-assign: back to basetype
		const T y = c;
		if (x != y){
			// drain::Logger mout(__FILE__, __FUNCTION__);
			mout.warn("Re-assign test failed: '", x, "' != '", y, "'");
		}
		//assert(("Re-assign test failed", x == y));
		// std::cerr << '\n';

	}

};


template <>
template <>
void CastableTester<drain::Castable>::callback<void>(drain::Castable & x){
}


template <>
template <>
void CastableTester<drain::Variable>::callback<void>(drain::Variable & x){
}

template <>
template <>
void CastableTester<drain::Reference>::callback<void>(drain::Reference & x){
}

template <>
template <>
void CastableTester<drain::FlexibleVariable>::callback<void>(drain::FlexibleVariable & x){
}


// Own type (Variable::getType means a problem...)
template <class C>
void testAssignment(C & c){
	drain::Logger mout(__FILE__, __FUNCTION__);
	//std::cerr << __FILE__ << ':' << __FUNCTION__ << '\n';
	mout.warn(__FUNCTION__);
	drain::Type::call<CastableTester<C> >(c, c.getType());
}


template <class T, class C>
void compareValues(drain::Logger & mout, const C & c, const T & x){

	if (c.getType() != typeid(T)){
		if (!(c.isString() && (typeid(T)==typeid(std::string)))){
			mout.reject() << "type not adapted [" << drain::TypeName<T>::get() << "] -> ";
			c.info(mout);
			mout.end();
		}
		else {
			mout.ok();
			c.info(mout);
			mout.end();
		}
	}

}


template <class T, class C>
void testAssignment(C & c, const T & x){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.warn(__FUNCTION__, ':', x, " [", drain::TypeName<T>::get(), "]");

	c = x;

	compareValues(mout, c, x);

	const T y = c;

	compareValues(mout, c, y);

	/*
	if (y != x){
		mout.reject() << "value [" << drain::TypeName<T>::get() << "] changed:" <<  x <<  " != " <<  y << " passed through ";
		c.info(mout);
		mout << mout.endl;
	}
	*/

	/*
	try {
			if (expectError){
				mout.error("no exception thrown: ");
				return;
			}
		}
		catch (const std::exception & e) {
		}
	*/

}


/*
template <class T, class C>
void testResetedAssignment(C & c){

	drain::Logger mout(__FILE__, __FUNCTION__);

	const T x = c;

	c.reset();
	c = x;

	if (c.getType() != typeid(T)){
		mout.reject() << "type not adapted:" <<  x <<  " -> " <<  c;
		c.info(std::cerr);
		mout << mout.endl;
	}


}
*/


template <class C, class T>
void testAssignmentToReseted(C & c, const T & x, bool expectError = true){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.warn("...");

	c.reset();

	if (c.typeIsSet()){
		mout.warn();
		c.info(mout);
		mout.error("type still set after reset");
	}

	if (!c.empty()){
		mout.warn();
		c.info(mout);
		mout.error("non-empty after reset");
	}

	//mout.note(drain::TypeName<T>::get(), '(', x, ')', " -> ", drain::TypeName<C>::get());
	mout.note() << drain::TypeName<T>::get() <<  '(' <<  x <<  ')' << " -> ";//, drain::TypeName<C>::get());
	c.info(mout);
	mout.end();

	try {
		c = x;

		if (expectError){
			mout.error("no exception thrown: ");
			return;
		}

		compareValues(mout, c, x);

	}
	catch (const std::exception & e) {
		if (expectError){
			mout.accept("exception thrown: ", e.what());
		}
		else {
			mout.reject("exception thrown: ", e.what());
		}
	}

	std::cerr << '\n';
}

template <class C, class T>
void testAssignmentRelinked(C & c, const T & x){

	drain::Logger mout(__FILE__, __FUNCTION__);

	T y;
	c.link(y);

	mout.note(drain::TypeName<T>::get(), '(', x, ')', " -> ", drain::TypeName<C>::get());

	try {
		c = x;
		//mout.fail("no exception thrown: ");
	}
	catch (const std::exception & e) {
		mout.fail("exception thrown: ", e.what());
	}

	if (y != x){
		mout.warn("value changed: ", x, " -> ", y);
	}


}


template <class S, class T>
void demo(const T & value, char outputSeparator = ','){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.special("Input type: ", drain::TypeName<T>::get(), ", local storage type: ", drain::TypeName<S>::get() );

	S data;

	if ((typeid(S)==typeid(char)) && (outputSeparator == 0)){
		mout.attention("Storage type is a single char, following errors for are expected for Castable, Reference and linked FlexibleVariable.");
	}

	mout.note("Castable:");
	drain::Castable c(data);
	c.setSeparator(outputSeparator);
	// std::cout << "Castable: c = value \n";
	// c = value;
	// std::cout << "Castable: \n";
	testAssignment(c, value);
	//testAssignment(c);
	//testAssignment(c = value);
	testAssignmentToReseted(c, value, true);
	std::cerr << '\n';

	//std::cout << "Reference: \n";
	mout.note("Reference");
	drain::Reference r(data);
	r.setSeparator(outputSeparator);
	//r.link(data);
	// testAssignment(r = value);
	testAssignment(r, value);
	testAssignmentToReseted(r, value, true);
	std::cerr << '\n';
	// testAssignmentRelinked(r, value);

	//std::cout << "Variable: \n";
	mout.note("Variable");
	drain::Variable v;
	//v.getType();
	v.setSeparator(outputSeparator);
	//testAssignment(v = value);
	testAssignment(v, value);
	testAssignmentToReseted(v, value, false);
	std::cerr << '\n';

	/*  should test all these: undef, initialized, assigned, initialized-type, set-type
	drain::Variable v(data);
	v.setSeparator(outputSeparator);
	v.setType(typeid(S));
	testAssignment(v = value);
	*/

	mout.note("FlexibleVariable:");
	drain::FlexibleVariable fv;
	fv.setSeparator(outputSeparator);
	testAssignment(fv, value);
	testAssignmentToReseted(fv, value, false);
	// testAssignmentRelinked(fv, value);
	std::cerr << '\n';

	//std::cout << "FlexVar, linked:   ";
	//fv.link(data);
	//testAssignment(fv);

	// std::cout << "Var=Cas:   ";
	mout.note("Variable -> FlexibleVariable");
	// testAssignment(fv, v);
	testAssignmentToReseted(fv, v, false);
	std::cerr << '\n';

	mout.note("FlexibleVariable -> Reference");
	r.link(data);
	//testAssignment(r, fv);

	/*
	v = r;
	v = fv;
	r = v;
	r = fv;
	fv = r;
	fv = v;
	*/
	std::cerr << '\n';


	std::cerr << '\n';

}


///
/**
 *  Usage:
    \code
    CastableComparer cc(var, flex);
	drain::Type::call<CastableComparer>(cc, typeid(double));
   \endcode
 */
class CastableComparer {
public:

	CastableComparer(const drain::Castable & castable1, const  drain::Castable & castable2) : c1(castable1), c2(castable2) {

	}

	// param S - target type
	// param T - type to be analyzed
	template <class S, class C>
	static
	void callback(C & target){
		target.result = target.template compareUsingType<S>();
	}

	template <class S>
	bool compareUsingType(){
		return (const S &)c1 == (const S &)c2;
	}

	bool compare(){
		drain::Type::call<CastableComparer>(*this, c1.getType());
		return result;
	}


	bool result = false;

	const drain::Castable & c1;
	const drain::Castable & c2;


};

template <>
bool CastableComparer::compareUsingType<void>(){
	if (c1.typeIsSet() || c2.typeIsSet())
		return false; // or equal, or raise error?
	else {
		// Both are void
		return true; // c1.getType() == c2.getType();
	}
}

template <>
bool CastableComparer::compareUsingType<char>(){
	//const char * s1 = nullptr;
	//const char * s2 = nullptr;
	if (c1.isCharArrayString() && c2.isCharArrayString()){
		return strcmp(c1.getCharArray(), c2.getCharArray()) == 0;
	}
	else {
		return c1.toStr() == c2.toStr();
	}
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
	std::string empty;


	// Type::call<drain::typesetter>(*this, t);



	int x = 5;
	flex.link(x);
	//flex = 6;
	flex.info(std::cerr);
	std::cerr << '/' << x << std::endl;

	flex.setType(typeid(double));
	flex = 7;
	flex.info(std::cerr);
	std::cerr << '/' << x << std::endl;
	// flex.link(empty);
	// flex = "";


	CastableComparer cc(var, flex);
	// drain::Type::call<CastableComparer>(cc, typeid(double));
	cc.compare();
	std::cerr << "result=" << cc.result << '\n';
	var = "7";
	//drain::Type::call<CastableComparer>(cc, typeid(double));
	cc.compare();
	std::cerr << "result=" << cc.result << '\n';


	var = "ABCDE";
	var = flex;
	std::cerr << "ptr:" << var.getPtr() << std::endl; // <-- BUG FOUND! Prints "ABCDE"
	std::cerr << "ptr:"<< var.getCharArray() << std::endl;
	var.info(std::cerr);
	std::cerr << '#' << var.getElementCount() << std::endl;


	if (argc <= 1){
		std::cout << "Demonstration of assigning values to and from Castables, Variables and Referencers.\n";
		std::cout << "Usage:\n   " << argv[0] << " <input1> <input2> ... ]\n";
		std::cout << "Example:\n " << argv[0] << " 123.45  456.789E+6 Åland ... ]\n\n";
		return 1;
	}


	for (int i=1; i<argc; ++i){

			const char *s = argv[i];
			std::cerr << "Arg " << i << ":\t" << s << '\n';

			std::string str = s;
			int k = atoi(s);
			double d = atof(s);

			if (d != 0){

				if (::rint(d) == k){
					std::cerr << "---INT---\n";
					demo<int>(str);
					demo<int>(k);
					demo<int>(d);
				}
				else {
					std::cerr << "---DBL---\n";
					demo<double>(str);
					demo<double>(k);
					demo<double>(d);
				}

			}

			// else {

			//std::cout << "Storage type: string\n";
			std::cout << "---STR---\n";
			demo<std::string>(str);

			//demo<std::string>(k);
			//demo<std::string>(d);

			if (false){
				// test len==1 ?
				std::cout << "---CHR---\n";
				demo<char>(str,0);
				demo<char>(k,0);
				demo<char>(d,0);

			}




		}

	/*
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
	*/

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
