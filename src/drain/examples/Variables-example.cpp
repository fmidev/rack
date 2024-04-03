/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
*/

/*  // ,VariableLike
  REQUIRE: drain/{Caster,Castable,FlexibleVariable,Log,Reference,Sprinter,String,RegExp,TextStyle,Type,TypeUtils,Variable,VariableBase}.cpp
  REQUIRE: drain/util/Time.cpp
  g+ + -I. drain/examples/Time-example.cpp drain/util/Time.cpp -o Time-example
 */

#include <stdlib.h>
#include <iostream>
#include <cassert>

#include "drain/Log.h"
//#include "drain/VariableAssign.h"
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
void checkType(const C & c){

}
*/

template <class T, class C>
void checkType(drain::Logger & mout, const C & c){

	static const std::type_info & t = typeid(T);
	static const std::string & tName = drain::Type::call<drain::simpleName>(t);

	const std::type_info & ct = c.getType();
	const std::string & ctName = drain::Type::call<drain::simpleName>(ct);

	if (ct == t){
		mout.accept(tName, " => ", ctName);
	}
	else if (c.isString()){
		mout.pending(tName, " => ", ctName);
	}
	else {
		mout.reject(tName, " => ", ctName);
	}
	/*
	if (t == typeid(std::string)){
		return c.isString();
	}
	*/


}

#define mika(expr) ( { printf(#expr); printf(#expr) } )

template <class T, class C>
void testVariableConstructors(C & c, const T & value = T()){

	drain::Logger mout(__FUNCTION__, drain::TypeName<T>::str());

	mout.note(drain::TypeName<C>::str(), " <= ", value, " [",  drain::TypeName<T>::str(), "]");

	try {
		mout.attention("C c1");
		mika( C c1 );

		C c1;
		// assert(!c1.typeIsSet());
		checkType<void>(mout, c1);

		mout.attention("C c2(typeid(", drain::TypeName<T>::str(),")");
		C c2(typeid(T));
		checkType<T>(mout, c2);
		// mout.note(drain::Type::call<drain::simpleName>(c2.getType()), " [",  drain::TypeName<T>::str(), "]");
		// assert(c2.getType() == typeid(T));

		mout.attention("Constructor with initialisation");
		C c4(value);
		checkType<T>(mout, c4);

		mout.attention("Constructor with assignment");
		C c3 = value;
		// mout.note(drain::Type::call<drain::simpleName>(c3.getType()), " [",  drain::TypeName<T>::str(), "]");
		// assert(c3.getType() == typeid(T));
		checkType<T>(mout, c3);

		// assert(c4.getType() == typeid(T));
	}
	catch (const std::exception & e) {
		mout.note(e.what());
		mout.error("quitting");

	}

}



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
	testAssignment(c, value);
	testAssignmentToReseted(c, value, true);
	std::cerr << '\n';

	mout.note("Reference");
	drain::Reference r(data);
	r.setSeparator(outputSeparator);
	testAssignment(r, value);
	testAssignmentToReseted(r, value, true);
	std::cerr << '\n';

	mout.note(drain::TypeName<drain::Variable>::str());
	drain::Variable v;
	v.setSeparator(outputSeparator);
	testVariableConstructors(v, value);
	testAssignment(v, value);
	testAssignmentToReseted(v, value, false);
	std::cerr << '\n';

	/*  should test all these: undef, initialized, assigned, initialized-type, set-type
	drain::Variable v(data);
	v.setSeparator(outputSeparator);
	v.setType(typeid(S));
	testAssignment(v = value);
	*/

	mout.note(drain::TypeName<drain::FlexibleVariable>::str());
	drain::FlexibleVariable fv;
	fv.setSeparator(outputSeparator);
	testVariableConstructors(fv, value);
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

//#define mika(expr) (static_cast<bool>(expr) ? void(0) : printf(#expr) )

int main(int argc, char **argv){

	drain::getLog().setVerbosity(5);
	drain::Logger mout(__FILE__, __FUNCTION__);


	std::string empty;

	/// Fundamental tests - on which other tests rely as well.
	drain::Variable var;
	assert( var.isVariable());
	assert(!var.isLinkable());
	assert(!var.isReference());


	drain::Reference ref;
	assert(!ref.isVariable());
	assert( ref.isLinkable());
	assert( ref.isReference());
	// var.isAssignable(); // True for
	//ref.link(empty);
	//assert( ref.isReference());

	drain::FlexibleVariable flex;
	assert( flex.isVariable());
	assert( flex.isLinkable());
	assert(!flex.isReference());
	flex.link(empty);
	assert( flex.isReference());
	flex.setType(typeid(float));

	var.reset();
	ref.reset();

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
