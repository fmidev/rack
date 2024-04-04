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
  REQUIRE: drain/util/{FileInfo,Output,Time,TreeXML,TreeHTML}.cpp

  g+ + -I. drain/examples/Time-example.cpp drain/util/Time.cpp -o Time-example
 */

#include <stdlib.h>
#include <iostream>
#include <cassert>

#include "drain/Log.h"
#include "drain/Reference.h"
#include "drain/Variable.h"
#include "drain/FlexibleVariable.h"
#include "drain/VariableAssign.h"

#include "drain/util/TreeHTML.h"
#include "drain/util/Output.h"


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

}



/// Demonstrates usage of drain::.
/**

 \code
 \endcode

*/


inline
drain::TreeHTML & addChild(drain::TreeHTML & elem, drain::BaseHTML::tag_t tagType){
	std::stringstream sstr("elem");
	sstr.width(3);
	sstr.fill('0');
	sstr << elem.getChildren().size(); // todo width
	return elem[sstr.str()](tagType);

}

template <class T>
inline
drain::TreeHTML & appendHTML(drain::TreeHTML & elem, drain::BaseHTML::tag_t tagType, const T & arg){
	drain::TreeHTML & child = addChild(elem,tagType);
	child = arg;
	return child;
};



// TODO: redesign logic, perhaps addChild();
inline
drain::TreeHTML & appendHTML(drain::TreeHTML & elem, drain::BaseHTML::tag_t tagType){
	if (elem.hasChildren()){
		return elem.getChildren().rbegin()->second; // last
	}
	else {
		return elem;
	}
}

// Consider to utils
template <class T, class ...TT>
inline
drain::TreeHTML & appendHTML(drain::TreeHTML & elem, drain::BaseHTML::tag_t tagType, const T & arg, const TT & ...args) {
	appendHTML(elem, tagType, arg);
	return appendHTML(elem, tagType, args...);
}

std::string getValue(drain::Castable &c){
	if (c.isCharArrayString()){
		return drain::StringBuilder<>("'", c, "'");
	}
	else if (c.isString()){
		return drain::StringBuilder<>('"', c, '"');
	}
	else
		return c;
}


#define DEMO_C1(init, expr, html) do { init; expr; appendHTML(html, drain::NodeHTML::TD, #init, #expr, getValue(variable), (variable.isReference()?"reference":"") ); } while (0)



template <class V>
void demoVariableConstructors(drain::TreeHTML & body){

	const std::string & clsName = drain::TypeName<V>::str();

	drain::TreeHTML & header = body[clsName+"-var-ctr"](drain::NodeHTML::H2);

	header = clsName+" – basic constructors";

	drain::TreeHTML & table = body[clsName+"-var-ctr-table"](drain::NodeHTML::TABLE);
	drain::TreeHTML & tr    = table["data"](drain::NodeHTML::TR);

	appendHTML(tr, drain::NodeHTML::TH, "Context", "Constructor", "Final value");

	DEMO_C1( , V variable;,                 addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable(typeid(bool));,   addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable(typeid(char));,   addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable(typeid(int));,    addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable(typeid(double));, addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable('X');,            addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable(false);,          addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable(12345);,          addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable(56.789);,         addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable("Hello!");,       addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable(std::string("Test"));,      addChild(table, drain::NodeHTML::TR));
	DEMO_C1(bool b=true;, V variable(b);,   addChild(table, drain::NodeHTML::TR));
	DEMO_C1(char c='Y';,  V variable(c);,   addChild(table, drain::NodeHTML::TR));
	DEMO_C1(int i=123;,   V variable(i);,   addChild(table, drain::NodeHTML::TR));
	DEMO_C1(double d=45.678;, V variable(d);,           addChild(table, drain::NodeHTML::TR));
	DEMO_C1(const char *s="123abc";, V variable(s);,    addChild(table, drain::NodeHTML::TR));
	DEMO_C1(std::string str("Test");, V variable(str);, addChild(table, drain::NodeHTML::TR));

	DEMO_C1( , V variable = true;,                 addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable = 'Z';,                  addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable = 1234;,                 addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable = 345.67;,               addChild(table, drain::NodeHTML::TR));
	DEMO_C1( , V variable; variable = "12.34567";, addChild(table, drain::NodeHTML::TR));


}

template <class V>
void demoReferenceConstructors(drain::TreeHTML & body){

	const std::string & clsName = drain::TypeName<V>::str();

	drain::TreeHTML & header = body[clsName+"-ref-ctr"](drain::NodeHTML::H2);

	header = clsName+" – basic constructors";

	drain::TreeHTML & table = body[clsName+"-ref-ctr-table"](drain::NodeHTML::TABLE);
	drain::TreeHTML & tr    = table["data"](drain::NodeHTML::TR);

	appendHTML(tr, drain::NodeHTML::TH, "Context", "Constructor", "Final value");

	DEMO_C1( , V variable;,                 addChild(table, drain::NodeHTML::TR));
	DEMO_C1(bool b=true;, V variable(b);,   addChild(table, drain::NodeHTML::TR));
	DEMO_C1(char c='Y';,  V variable(c);,   addChild(table, drain::NodeHTML::TR));
	DEMO_C1(int i=123;,   V variable(i);,   addChild(table, drain::NodeHTML::TR));
	DEMO_C1(double d=45.678;, V variable(d);,           addChild(table, drain::NodeHTML::TR));
	// DEMO_C1(const char *s="123abc";, V variable(s);,    addChild(table, drain::NodeHTML::TR));
	DEMO_C1(std::string str("Test");, V variable(str);, addChild(table, drain::NodeHTML::TR));

	//DEMO_C1(V v(typeid(int)), td1, td2);

}



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



	if (argc <= 1){
		std::cout << "Demonstration of assigning values to and from Castables, Variables and Referencers.\n";
		std::cout << "Usage:\n   " << argv[0] << " <input1> <input2> ... ]\n";
		std::cout << "Example:\n " << argv[0] << " 123.45  456.789E+6 Åland ... ]\n\n";
		return 1;
	}

	drain::TreeHTML html;
	html(drain::NodeHTML::HTML);

	html["head"](drain::NodeHTML::HEAD);
	drain::TreeHTML & style = html["header"]["style"](drain::BaseHTML::STYLE);
	style->set("table, th, td", "color: blue; border: 1px solid;");
	style->set("table", "border-collapse: collapse;");
	style->set("td", "font-family: monospace;");

	//style->setStyle("table", "block-size:1; entry-pentry:rgb(0,1,1)");
	//style->setStyle("div",  "color: blue; line-width: 20pix");

	drain::TreeHTML & body = html["body"](drain::NodeHTML::BODY);

	drain::TreeHTML & h1 = body["main-header"](drain::NodeHTML::H1);
	h1 = "Variable Demo";

	demoVariableConstructors<drain::Variable>(body);
	demoVariableConstructors<drain::FlexibleVariable>(body);

	demoReferenceConstructors<drain::Reference>(body);
	demoReferenceConstructors<drain::FlexibleVariable>(body);


	for (int i=1; i<argc; ++i){

		const char *s = argv[i];
		std::cerr << "Arg " << i << ":\t" << s << '\n';

		std::string str = s;
		int k = atoi(s);
		double d = atof(s);

		/*
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
		*/
	}

	drain::Output outfile("variables.html");

	drain::NodeHTML::toStream(outfile, html);

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
