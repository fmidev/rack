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
// ,TreeUtils
 REQUIRE: drain/{Caster,Castable,Log,FlexibleVariable,RegExp,Sprinter,String,TextStyle,TextStyleVT100,Type,TypeUtils,Variable,VariableBase}.cpp
 REQUIRE: drain/util/{FileInfo,TreeXML}.cpp
 REQUIRE: drain/image/TreeXML-GDAL.cpp

 g+ + -I. drain/examples/TreeHTML-example.cpp drain/util/{Log,Caster,Castable,JSONwriter,String,TreeXML,Type,RegExp}.cpp    -o TreeHTML-example
 */
#include <iostream>

#include "drain/Log.h"
#include "drain/Type.h"
#include "drain/util/TreeUtils.h"
#include "drain/image/TreeXML-GDAL.h"


/*
 <GDALMetadata >
  <Item name="SCALE" sample="0" role="scale" >0.4</Item>
  <Item name="OFFSET" sample="0" role="offset" >-36</Item>
  <Item name="IMAGETYPE" >Weather Radar,fiuta</Item>
  <Item name="TITLE" >PPI:</Item>
  <Item name="UNITS" >TH</Item>
</GDALMetadata>
 */

#include "drain/util/Flags.h"

template <>
const drain::EnumDict<drain::image::GDAL::tag_t>::dict_t drain::EnumDict<drain::image::GDAL::tag_t>::dict = {
		{"ROOT", drain::image::GDAL::ROOT},
		{"USER", drain::image::GDAL::USER}
};

class MyClassList : public drain::ClassListXML {

public:

	template <typename E>
	inline  // drain::EnumFlagger<
	void add2(const E & arg) {
		insert(drain::EnumDict<E>::dict.getKey(arg));
	};

};


enum Luokko {EKA=1, TOKA=2};

template <>
const drain::EnumDict<Luokko>::dict_t  drain::EnumDict<Luokko>::dict = {
		{"EKA",  Luokko::EKA},
		{"TOKA", Luokko::TOKA}
};

template <typename E>
struct EnumWraponen  {

	EnumWraponen(const E & value)  : value(value){};

	const E value;

	const std::string & str() const {
		return drain::EnumDict<E>::dict.getKey(value);
	}
};


int main(int argc, char **argv){

	// std::cout << drain::TypeName<T>::str() << '/' << drain::TypeName<T>::str() << " demo \n";
	std::cout << drain::TypeName<drain::image::TreeGDAL>::str() << " demo \n";
	std::cout << "Tag enum type: " << drain::TypeName<drain::image::NodeGDAL>::str() << '\n';


	/*
	if (argc==1){
		cerr << "Usage:   " << argv[0] << " <keychars> <sample-string>  <key>=<value> <key2>=<value2> ..." << endl;
		cerr << "Example: " << argv[0] << " '[a-zA-Z0-9]+'  'Hello, ${e} and ${pi}! My name is ${x}.'  e=world  x=test" << endl;
		return 1;
	}
	*/

	//drain::TreeXML xml;
	MyClassList list;
	list.add("A", "B", "C");
	list.add("Mika");
	list.add2(drain::image::GDAL::ROOT);
	list.add2(drain::image::GDAL::USER);

	std::cerr << list << '\n';


	drain::image::TreeGDAL gdal(drain::image::GDAL::ROOT);
	gdal["first"](drain::image::GDAL::ITEM) = "setting";
	gdal(std::string("second"))(drain::image::GDAL::ITEM) = 12.345;

	drain::image::TreeGDAL & sub = gdal["first"];
	sub("John") = 123.455;

	drain::image::TreeGDAL & sub2 = gdal("second");
	sub2("Jane") = true;

	drain::TreeUtils::dump(gdal);

	std::cout << gdal;

	/*
	html["header"](NodeHTML::HEAD);
	TreeHTML & style = html["header"]["style"](BaseHTML::STYLE);
	style->setStyle("table", "block-size:1; entry-pentry:rgb(0,1,1)");
	style->setStyle("div",  "color: blue; line-width: 20pix");
	style->set("span",  "color:  blue; line-width:  20pix");

	html["body"](NodeHTML::BODY);
	html["body"]->set("name", "document");
	html["body"]->set("owner", "state");
	html["body"]->set("style", "fill:red");
	html["body"]->set("style", "stroke:blue");
	html["body"]->setStyle("opacity", 0.5);
	html["body"]->setStyle("transform:mika; keijo:mäki");

	html["body"]["p"](NodeHTML::P) = "Some text.";
	html["body"]["p"]->set("style", "fill:red");

	html["body"]["logo"](NodeHTML::IMG)->set("src", "image_url");

	html("body/rauno/käki/mäkinen");
	//html["body"].append("span");

	TreeXMLvisitor handler;
	TreeUtils::traverse(handler, html);
	std::cout << html;

	const TreeHTML & htm = html;
	TreeUtils::traverse(handler, htm);
	*/

	return 0;
}
