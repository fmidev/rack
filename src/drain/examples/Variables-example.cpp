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
  REQUIRE: drain/util/{FileInfo,Output,TreeXML,TreeHTML}.cpp
  //Time,

  g+ + -I. drain/examples/Time-example.cpp drain/util/Time.cpp -o Time-example
 */

#include <stdlib.h>
#include <iostream>
#include <cassert>

#include "drain/Log.h"
#include "drain/Reference.h"
#include "drain/UniTuple.h"
#include "drain/Variable.h"
#include "drain/FlexibleVariable.h"
#include "drain/VariableAssign.h"

#include "drain/util/TreeHTML.h"
#include "drain/util/Output.h"



/// Returns the body element.
drain::TreeHTML & initHtml(drain::TreeHTML & html, const std::string & header = ""){

	drain::TreeHTML & body = drain::TreeUtilsHTML::initHtml(html, header);

	drain::TreeHTML & style = html["head"]["style"](drain::BaseHTML::STYLE);
	style->set("table, th, td", "color: blue; border: 1px solid;");
	style->set("table", "border-collapse: collapse;");
	style->set("td", "padding:3px;"); // font-family:monospace;
	// style->set("th", "border: 2px dashed; color: green; font-decoration: none;");
	style->set("th", "border: 2px solid; padding:5px"); // ; font-size:larger text-align:left;
	style->set(".code", "font-family:monospace");
	style->set(".right", "text-align: right;");
	style->set(".center", "text-align: center;");

	style->set(".warn", "color: brown;");

	return body;
}

const std::string & stripParentheses(const std::string & src, std::string & dst){

	const size_t n = src.length();

	if (n > 0){
		if ((src.at(0)=='(') && (src.at(n-1)==')')){
			dst.assign(src,1,n-2); // 2nd arg: number of
			return dst;
		}
	}

	return src;
}

/// Creates a string, showing string type with double quotes ("), and also char strings with copy sign and references with reg sign.
std::string getValue(drain::Castable &c){

	std::stringstream sstr;

	if (c.isString()){
		sstr << '"' << c << '"';
		if (c.isCharArrayString())
			sstr << " ©"; // 169	00A9	&copy;	COPYRIGHT SIGN
	}
	else
		sstr << c;

	if (c.isLinking())
		sstr << " ®"; // 174	00AE	&reg;	REGISTERED SIGN

	return sstr.str();

}

/// Drain specials: UniTuple
//  Comma ',' prevents using macros directly, so typedef applied.
typedef drain::UniTuple<int,2>    UniTuple2i;
#define INIT_2i {12, 34}

typedef drain::UniTuple<double,3> UniTuple3d;
#define INIT_3d {5.6, 7.8, 9.0}
//#define COMMA ,


// ----------------------------------------------------------------------------------------------



void addTitleRow(drain::TreeHTML & table, const std::string & title){
	drain::TreeHTML & tr = table[title](drain::NodeHTML::TR);
	drain::TreeHTML & th = tr["ctx"](drain::BaseHTML::TH);
	th -> set("colspan", 2);
	th -> addClass("right");
	th = title;
}

void addConstructorRow(drain::TreeHTML & table, const std::string & ctx = "", const std::string & expr = "", const std::string & key = "", const std::string & value = ""){

	drain::TreeHTML & tr = table[ctx+'_'+expr](drain::NodeHTML::TR);

	if (!tr.hasChildren()){

		tr->addClass("code");

		// "Fill"
		drain::TreeHTML dummy(drain::BaseHTML::TD);
		dummy = "×";

		//dummy["span"](drain::BaseHTML::SPAN) = "×";
		//dummy["span"] ->addClass("center");
		drain::TreeUtilsHTML::fillTableRow(table, tr, dummy); // rename copy/init

		// TODO: add Tree elem!
	}


	if (expr.empty()){
		//const size_t n = tr.getChildren().size();
		tr.clearChildren();
		tr->removeClass("code");

		drain::TreeHTML & th = tr["ctx"](drain::BaseHTML::TH);
		th = ctx;
		th -> set("colspan", 2);
		th -> addClass("right");
		/*
		static const drain::TreeHTML h3(drain::BaseHTML::H3);
		th["subi"] = h3;
		th["subi"] = "koe";
		*/
		return;
		// tr.erase(drain::TreeHTML::path_t("expr"));
	}

	tr["ctx"] = ctx;
	tr["ctor"] = expr;
	tr[key] = value;

}

//#define DEMO_C1(init, expr, html) do { init; expr; appendHTML(html, drain::NodeHTML::TD, #init, #expr, getValue(variable), (variable.isLinking()?"reference":"") ); } while (0)

//  + (variable.isLinking()?" &reg;":"")
//#define DEMO_C1(init, expr, html) do { init; expr; drain::TreeUtilsHTML::appendElem(drain::TreeUtilsHTML::addChild(html,drain::NodeHTML::TR), drain::NodeHTML::TD, #init, #expr, getValue(variable), (variable.isLinking()?"reference":"") ); } while (0)
#define DEMO_CTOR(table, clsName, ctx, expr) do { ctx; expr; addConstructorRow(table, #ctx, #expr, clsName, getValue(variable)); } while (0)

//#define DEMO_COPY_CTOR(table, clsName, ctx, v2type, ctx2, expr) do { ctx; v2type ctx2; expr; addConstructorRow(table, drain::StringBuilder<' '>(#ctx,#v2type,#ctx2), #expr, clsName, getValue(variable) + (variable.isLinking()?" REF":"") ); } while (0)
//#define DEMO_COPY_CTOR(table, clsName, ctx, expr) do { ctx; expr; addConstructorRow(table, drain::StringBuilder<' '>(#ctx,#expr), #expr, clsName, getValue(variable) + (variable.isLinking()?" REF":"") ); } while (0)

/// Constructors supported by all the classes: Variable, FlexibleVariable, Reference,
template <class V>
void demoCopyConstructors(drain::TreeHTML & table){

	const std::string & clsName  = drain::TypeName<V>::str();

	using namespace drain;

	/*
	  DEMO_CTOR(table, clsName, bool b=true; V1 variable(b);, V variable2(variable););
	  DEMO_CTOR(table, clsName, char c='Y';  V1 variable(c);, V variable2(variable););
	*/

	DEMO_CTOR(table, clsName, int i=123; Variable var(i);, V variable(var););
	DEMO_CTOR(table, clsName, int i=123; FlexibleVariable var(i);, V variable(var););
	DEMO_CTOR(table, clsName, int i=123; FlexibleVariable var; var.link(i);, V variable(var););
	DEMO_CTOR(table, clsName, int i=123; Reference var(i);, V variable(var););

	DEMO_CTOR(table, clsName, std::string str("Test"); Variable var(str);, V variable(var););
	DEMO_CTOR(table, clsName, std::string str("Test"); FlexibleVariable var(str);, V variable(var););
	DEMO_CTOR(table, clsName, std::string str("Test"); FlexibleVariable var; var.link(str);, V variable(var););
	DEMO_CTOR(table, clsName, std::string str("Test"); Reference var(str);, V variable(var););

	DEMO_CTOR(table, clsName, UniTuple3d uni3d(INIT_3d); Variable var(uni3d);, V variable(var););
	DEMO_CTOR(table, clsName, UniTuple3d uni3d(INIT_3d); FlexibleVariable var(uni3d);, V variable(var););
	DEMO_CTOR(table, clsName, UniTuple3d uni3d(INIT_3d); FlexibleVariable var; var.link(uni3d);, V variable(var););
	DEMO_CTOR(table, clsName, UniTuple3d uni3d(INIT_3d); Reference var(uni3d);, V variable(var););

}


/// Constructors supported by all the classes: Variable, FlexibleVariable, Reference,
template <class V>
void demoBasicConstructors(drain::TreeHTML & table){

	const std::string & clsName = drain::TypeName<V>::str();

	addConstructorRow(table, "Default constructor");
	DEMO_CTOR(table, clsName, , V variable;);

	addConstructorRow(table, "Copy constructor");

	//DEMO_CTOR(table, clsName, nullptr_t n=nullptr; V variable(n);, V variable2(variable););
	DEMO_CTOR(table, clsName, bool b=true; V var(b);, V variable(var););
	DEMO_CTOR(table, clsName, char c='Y';  V var(c);, V variable(var););
	DEMO_CTOR(table, clsName, int i=123;   V var(i);, V variable(var););
	DEMO_CTOR(table, clsName, double d=45.678; V var(d);, V variable(var););
	DEMO_CTOR(table, clsName, std::string str("Test"); V var(str);, V variable(var););
	DEMO_CTOR(table, clsName, UniTuple2i uni2i(INIT_2i); V var(uni2i);, V variable(var););
	DEMO_CTOR(table, clsName, UniTuple3d uni3d(INIT_3d); V var(uni3d);, V variable(var););

	addConstructorRow(table, "Copy constructor variants");

	demoCopyConstructors<V>(table);

	addConstructorRow(table, "Non-const variable arg");
	DEMO_CTOR(table, clsName, bool b=true;, V variable(b););
	DEMO_CTOR(table, clsName, char c='Y';,  V variable(c););
	DEMO_CTOR(table, clsName, int i=123;,   V variable(i););
	DEMO_CTOR(table, clsName, double d=45.678;, V variable(d););
	DEMO_CTOR(table, clsName, std::string str("Test");, V variable(str););
	DEMO_CTOR(table, clsName, UniTuple2i uni2i(INIT_2i), V variable(uni2i););
	DEMO_CTOR(table, clsName, UniTuple3d uni3d(INIT_3d), V variable(uni3d););

}

template <class V>
void demoConstructors(drain::TreeHTML & table){

	const std::string & clsName = drain::TypeName<V>::str();

	demoBasicConstructors<V>(table);

	addConstructorRow(table, "Type initialisation");

	DEMO_CTOR(table, clsName, , V variable(typeid(bool)););
	DEMO_CTOR(table, clsName, , V variable(typeid(char)););
	DEMO_CTOR(table, clsName, , V variable(typeid(int)););
	DEMO_CTOR(table, clsName, , V variable(typeid(double)););

	// addConstructorRow(table, "?", "value init");
	addConstructorRow(table, "Initial value");

	DEMO_CTOR(table, clsName, , V variable(false););
	DEMO_CTOR(table, clsName, , V variable('X'););
	DEMO_CTOR(table, clsName, , V variable(12345););
	DEMO_CTOR(table, clsName, , V variable(56.789););
	DEMO_CTOR(table, clsName, , V variable("Hello!"););
	DEMO_CTOR(table, clsName, , V variable(std::string("Test")););
	DEMO_CTOR(table, clsName, , V variable(UniTuple2i(INIT_2i)););
	DEMO_CTOR(table, clsName, , V variable(UniTuple3d(INIT_3d)););


	// addConstructorRow(table, "const var", "value init");
	addConstructorRow(table, "Const variable");

	DEMO_CTOR(table, clsName, const bool b=true;, V variable(b););
	DEMO_CTOR(table, clsName, const char c='Y';,  V variable(c););
	DEMO_CTOR(table, clsName, const int i=123;,   V variable(i););
	DEMO_CTOR(table, clsName, const double d=45.678;, V variable(d););
	DEMO_CTOR(table, clsName, const std::string str("Test");, V variable(str););
	DEMO_CTOR(table, clsName, const char *s="123abc";, V variable(s););
	DEMO_CTOR(table, clsName, UniTuple2i ut(INIT_2i);, V variable(ut););
	DEMO_CTOR(table, clsName, UniTuple3d ut(INIT_3d);, V variable(ut););


	addConstructorRow(table, "Constructor with assignment");

	DEMO_CTOR(table, clsName, , V variable = true;);
	DEMO_CTOR(table, clsName, , V variable = 'Z';);
	DEMO_CTOR(table, clsName, , V variable = 1234;);
	DEMO_CTOR(table, clsName, , V variable = 345.67;);
	DEMO_CTOR(table, clsName, , V variable = "12.34567";);
	DEMO_CTOR(table, clsName, , V variable = INIT_2i;);
	DEMO_CTOR(table, clsName, , V variable = UniTuple3d(INIT_3d););


	//DEMO_CTOR(table, clsName, V variable("Hello!"), V variable2(variable););

}


template<>
void demoConstructors<drain::Reference>(drain::TreeHTML & table){
	demoBasicConstructors<drain::Reference>(table);
}

/// Utility to supply/substitute template parameters as an argument: <Variable,Reference,...>
template<typename...>
struct typelist{};

template<typename T, typename ... TT>
void createConstructorTable(drain::TreeHTML & table, typelist<T,TT...>) {

	const std::string clsName = drain::TypeName<T>::str();
	// drain::Type::call<drain::simpleName>(typeid(V));
	// table -> addClass(clsName);

	//drain::FlexibleVariable & name = table -> get("name");
	std::string name = table->get("name", "");
	if (name.empty()){
		table->set("name", clsName);
	}
	else {
		name += '-';
		name += clsName;
		table->set("name", name);
	}

	// Prefix op: create (update) title row
	drain::TreeHTML & tr = table["header"](drain::BaseHTML::TR);
	if (tr.empty()){
		// Note: contracted keys
		tr["ctx"](drain::BaseHTML::TD)  = ""; // colspan 2?
		tr["ctor"](drain::BaseHTML::TD) = "";
	}
	drain::TreeHTML & clsTh = tr[clsName](drain::BaseHTML::TH);
	clsTh = clsName;
	clsTh->addClass("code");

	// Recursion
	createConstructorTable(table, typelist<TT...>());

	// Postfix op: create the actual table
	demoConstructors<T>(table);
};

void createConstructorTable(drain::TreeHTML & table, typelist<>){}

/*
template<typename...classes>
void createConstructorTable(drain::TreeHTML & table){
	createConstructorTable(table, typelist<classes...>());
}
*/


template<typename ...VV>
void testConstructors(const std::string & pathPrefix = ""){

	drain::getLog().setVerbosity(5);
	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::TreeHTML html;
	drain::TreeHTML & body = initHtml(html);

	drain::TreeHTML & table = body["table"](drain::BaseHTML::TABLE);

	mout.note("createConstructorTable");

	//createConstructorTable<VV...>(table);
	createConstructorTable(table, typelist<VV...>());

	html["head"](drain::BaseHTML::HEAD)["title"](drain::BaseHTML::TITLE) = table->get("name");

	std::string filename = drain::StringBuilder<>(pathPrefix, "VariableT-ctors-", table->get("name"), ".html");

	mout.note("Writing: ", filename);

	drain::Output outfile(filename);
	drain::NodeHTML::docToStream(outfile, html);

	mout.note("Written");

}


// ----------------------------------------------------------------------------------------------

/**
 *  \tparam T - initial internal type of variable V
 *  \tparam V - variable type: Variable, FlexibleVariable or Reference
 *  \tparam T2 - assigned type
 *
 *  \param tr - HTML row element (TR)
 *  \param value - value assigned to variable
 *
 */
template <class T, class V, class T2>
void addAssignment(drain::TreeHTML & tr, const T2 & value){

	using namespace drain;

	// drain::getLog().setVerbosity(7);
	drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.note("start");

	// Create a VariableT of storage type T.
	// Note: (only) this initialization is supported by all the variable classes.
	// The resulting integral storage type is T.
	T x = Variable(); // init somehow...

	mout.info("Creating: ", drain::TypeName<VariableT<V> >::str(), "var(", x, ") // ", TypeName<T>::str());

	drain::VariableT<V> var(x);

	// "Main"
	mout.info("Assigning: ", "var = ", value, "; // ", TypeName<T2>::str());
	var = value;

	std::string key(var.getTypeName());

	drain::TreeHTML & td = tr[key](drain::BaseHTML::TD); // , getValue(var)
	td -> set("type", key);


	if (var == value){
		td = getValue(var);
	}
	else {
		// Variable could not store the value (correctly)
		/*
		mout.reject<LOG_WARNING>() << "Comparison failed: " << TypeName<T>::str() << " = " << value << " != ";
		var.info(mout);
		//T2 xx;
		//xx = (T2)var;
		//mout <<  " test2: " << (xx==value);
		mout.end();
		*/
		drain::TreeHTML & span = td["span"](drain::BaseHTML::SPAN);
		span -> addClass("warn");
		span = getValue(var);
	}

}




template <class V, class T>
void createAssignmentRow(drain::TreeHTML & table, const std::string &t, const std::string &v, const T & value){

	// Ensure header is the fiAssignment
	drain::TreeHTML & header0 = table["header0"](drain::BaseHTML::TR);
	drain::TreeHTML & header  = table["header"](drain::BaseHTML::TR);
	drain::TreeHTML & tr = table.addChild()(drain::BaseHTML::TR);
	tr->addClass("code");

	// Start the row with variable declaration, like \c int and \c 123.
	std::string tmp;
	tr["type"](drain::BaseHTML::TD)  = stripParentheses(t, tmp); // t;

	//tr["value"](drain::BaseHTML::TD)  = v;
	tr["value"](drain::BaseHTML::TD) = stripParentheses(v, tmp);

	// Fill the test with assigments to Variable initialized to different types.

	addAssignment<bool,V>(tr, value);
	addAssignment<unsigned char,V>(tr, value);
	addAssignment<char,V>(tr, value);
	addAssignment<short int,V>(tr, value);
	addAssignment<int,V>(tr, value);
	addAssignment<unsigned long,V>(tr, value);
	addAssignment<float,V>(tr, value);
	addAssignment<double,V>(tr, value);
	addAssignment<std::string,V>(tr, value);

	//UniTuple2d uni2d = {1.1, 2.2};
	// DEMO_ASSIGN(table, typename V::varbase_t, UniTuple<2 double>, uni2d);
	//addAssignment<UniTuple2d,V>(tr, value);
	//addAssignment<UniTuple3i,V>(tr, value);

	if (header.empty()){

		const std::string clsName = drain::TypeName<drain::VariableT<V> >::str(); // drain::Type::call<drain::simpleName>(typeid(V));

		drain::TreeHTML &  input   = drain::TreeUtilsHTML::addChild(header0, drain::BaseHTML::TH, "input")    = "input" ;
		//input->addClass("");
		drain::TreeHTML & variable = drain::TreeUtilsHTML::addChild(header0, drain::BaseHTML::TH, "variable") = clsName; // "variable" ;
		variable->addClass("code");

		// Copy keys of the (first) data row to header as well.
		for (const auto & entry: tr.getChildren()){
			drain::TreeHTML & th = drain::TreeUtilsHTML::addChild(header, drain::BaseHTML::TH, entry.first);
			th = entry.first;
			th->addClass("code");
		}
		header["type"]->removeClass("code");
		header["value"]->removeClass("code");

		input->set("colspan", 2);
		variable->set("colspan", header.getChildren().size() -2);


	}

}



#define DEMO_ASSIGN(table, vartype, type, value) do {createAssignmentRow<vartype>(table, #type, #value, value); } while (0);
//#define DEMO_ASSIGN(table, vartype, type, value) do {createRow<vartype>(table, #type, #value, type(value) ); } while (0);

template <class V>
void testAssignments(const std::string & pathPrefix = ""){

	//drain::getLog().setVerbosity(5);
	drain::Logger mout(__FILE__, __FUNCTION__);

	const std::string clsName = drain::TypeName<V>::str(); // drain::Type::call<drain::simpleName>(typeid(V));

	drain::TreeHTML html;

	drain::TreeHTML & body = initHtml(html);
	html["head"](drain::BaseHTML::HEAD)["title"](drain::BaseHTML::TITLE) = clsName + " - assignments";
	// drain::TreeHTML & body = initHtml(html, clsName + " - assignments");
	// body["title"](drain::BaseHTML::H3);


	drain::TreeHTML & table = body["table"](drain::NodeHTML::TABLE);

	DEMO_ASSIGN(table, typename V::varbase_t, bool, false);
	DEMO_ASSIGN(table, typename V::varbase_t, bool, true);
	DEMO_ASSIGN(table, typename V::varbase_t, int, 123);
	// DEMO_ASSIGN(table, typename V::varbase_t, short, 16384);
	DEMO_ASSIGN(table, typename V::varbase_t, float, 12.345f); // NOTE trailing f
	DEMO_ASSIGN(table, typename V::varbase_t, double, 3.14159265358979323846); // 3.14159265358979323846	/* pi */
	DEMO_ASSIGN(table, typename V::varbase_t, double, 314.15926E-02); // 3.14159265358979323846	/* pi */
	DEMO_ASSIGN(table, typename V::varbase_t, std::string, "Hello!");
	DEMO_ASSIGN(table, typename V::varbase_t, std::string, "true");
	DEMO_ASSIGN(table, typename V::varbase_t, std::string, "false");
	DEMO_ASSIGN(table, typename V::varbase_t, std::string, "3141592E-6");  /* ~ pi */
	DEMO_ASSIGN(table, typename V::varbase_t, const char *, "Test");
	DEMO_ASSIGN(table, typename V::varbase_t, const char *, "true");
	DEMO_ASSIGN(table, typename V::varbase_t, const char *, "false");



	// Commas must be inside parentheses
	UniTuple2i uni2i = INIT_2i;
	DEMO_ASSIGN(table, typename V::varbase_t, (UniTuple<2,int>), uni2i);

	UniTuple3d uni3d = INIT_3d;
	DEMO_ASSIGN(table, typename V::varbase_t, (UniTuple<3,double>) , uni3d);

	DEMO_ASSIGN(table, typename V::varbase_t, void *, nullptr); // FIX! Becomes a string...


	using namespace drain;

	addTitleRow(table, "Variables...");

	int i=123;
	DEMO_ASSIGN(table, typename V::varbase_t, int i=123, Variable(i));
	DEMO_ASSIGN(table, typename V::varbase_t, int i=123, FlexibleVariable(i));
	DEMO_ASSIGN(table, typename V::varbase_t, int i=123, Reference(i));

	std::string s("test");
	DEMO_ASSIGN(table, typename V::varbase_t, std::string s("test"), Variable(s));
	DEMO_ASSIGN(table, typename V::varbase_t, std::string s("test"), FlexibleVariable(s));
	DEMO_ASSIGN(table, typename V::varbase_t, std::string s("test"), Reference(s));

	DEMO_ASSIGN(table, typename V::varbase_t, UniTuple, Variable(uni3d));
	DEMO_ASSIGN(table, typename V::varbase_t, UniTuple, FlexibleVariable(uni3d));
	DEMO_ASSIGN(table, typename V::varbase_t, UniTuple, Reference(uni3d));


	const std::string filename = drain::StringBuilder<>(pathPrefix, "VariableT-assign-",clsName, ".html");
	mout.note("Writing: ", filename);

	drain::Output outfile(filename);
	drain::NodeHTML::docToStream(outfile, html);


}

namespace drain {

/*
template <>
template <class T, size_t N>
UniTuple<T,N> & UniTuple<T,N>::set(const Variable & x){
	return *this;
}

template <>
template <class T, size_t N>
UniTuple<T,N> & UniTuple<T,N>::set(const FlexibleVariable & x){
	return *this;
}

template <>
template <class T, size_t N>
UniTuple<T,N> & UniTuple<T,N>::set(const Reference & x){
	return *this;
}
*/

}



int main(int argc, char **argv){

	drain::Log::USE_VT100 = true;
	drain::getLog().setVerbosity(6);

	drain::Logger mout(__FILE__, __FUNCTION__);


	std::string empty;

	/// Fundamental tests - on which other tests rely as well.
	drain::Variable var;
	assert( var.isVariable());
	assert(!var.isLinkable());
	assert(!var.isLinking());

	const float f = 12.345;
	var = f;
	assert(var == f);
	const float f2 = var;
	assert(f2 == f);
	float f3;
	f3 = var;
	assert(f3 == f);


	drain::Variable var2(var);

	drain::Reference ref;
	assert(!ref.isVariable());
	assert( ref.isLinkable());
	assert(!ref.isLinking());
	ref.link(empty);
	assert( ref.isLinking());

	drain::FlexibleVariable flex;
	assert( flex.isVariable());
	assert( flex.isLinkable());
	assert(!flex.isLinking());
	flex.link(empty);
	assert( flex.isLinking());
	flex.setType(typeid(float));

	flex = nullptr;

	var.reset();
	ref.reset();
	flex.reset();

	drain::UniTuple<double,2> uni2d = {1.1, 2.2};

	drain::Variable var3(uni2d);
	// drain::Reference ref3(uni2d);


	var = "1.1,2.2";
	assert(var == uni2d);

	flex = uni2d;
	assert(flex == uni2d);
	std::cerr << __FILE__ << ':' << __FUNCTION__ << " '" << flex.toStr() << "' == '..." << "'" << std::endl;
	assert(flex == "1.1,2.2");

	flex.link(uni2d);
	flex = "3.30,4.400";
	assert(flex == "3.3,4.4");

	flex.setFill(false);
	flex = 5;
	// std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << " '" << flex.toStr() << "'" << std::endl;
	assert(flex == "5,4.4");

	flex.setFill(true);
	flex = {1.100};
	assert(flex == uni2d); // self-ref
	assert(flex == "1.1,1.1");


	/*

	int x = 5;
	flex.link(x);
	//flex = 6;
	flex.info(std::cerr);
	std::cerr << '/' << x << std::endl;
	flex.setType(typeid(double));
	flex = 7;
	flex.info(std::cerr);
	std::cerr << '/' << x << std::endl;
	*/


	if (argc <= 1){
		std::cerr << "Demonstration of assigning values to and from Castables, Variables and Referencers.\n";
		std::cerr << "Usage:\n   " << argv[0] << " <dir> \n";
		std::cerr << "Example:\n " << argv[0] << " doxygen/\n\n";
		return 1;
	}

	std::string pathPrefix(argv[1]);

	// CTORS
	testConstructors<drain::Variable>(pathPrefix);
	testConstructors<drain::FlexibleVariable>(pathPrefix);
	testConstructors<drain::Reference>(pathPrefix);
	/// Shared table for comparison
	testConstructors<drain::Variable,drain::FlexibleVariable,drain::Reference>(pathPrefix);

	// ASSIGNS
	testAssignments<drain::Variable>(pathPrefix);
	testAssignments<drain::FlexibleVariable>(pathPrefix);
	testAssignments<drain::Reference>(pathPrefix);

	return 0;
}
