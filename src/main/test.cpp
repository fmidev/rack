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
//#include <stddef.h>


#include <string>

#include <drain/prog/Command.h>
#include <drain/prog/CommandInstaller.h>
#include <drain/prog/CommandBank.h>

#include "resources.h"
#include "test.h"

namespace rack {




/*
template <class C=drain::Command>
class ScienceCmdWrapper : public drain::CommandWrapper<C,0, ScienceSection> {
public:

	ScienceCmdWrapper(char alias = 0): drain::CommandWrapper<C,0,ScienceSection>(alias) {};

	//ImageCmdWrapper(const std::string & name, char alias = 0): drain::CommandWrapper<C,0,RACK_SEC_IMG>(name, alias) {};

};
*/

struct TestSection : public drain::CommandSection {

	inline
	TestSection(): CommandSection("tests"){
	};

};


class CmdTestTree : public drain::SimpleCommand<std::string> {

public:

	CmdTestTree() : drain::SimpleCommand<std::string>(__FUNCTION__, "Dump XML track") {
		//getParameters().link("level", level = 5);
	}

	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		Hi5Tree test;
		test["what"].data.attributes["object"] = "PVOL";
		test["what"].data.attributes["source"] = "FMI";
		test["how"].data.attributes["point"] = {25.0, 60.0};
		Hi5Tree & t = test["dataset1"]["data2"];
		t["data"].data.image.initialize(typeid(short), {500,360});
		t["what"].data.attributes["quantity"] = "DBZH";
		t["what"].data.attributes["gain"]     =  0.5;
		t["what"].data.attributes["offset"]   = -32.0;
		mout.experimental("Testing...");
		drain::TreeUtils::dataDumper(test, std::cout);
		drain::TreeUtils::dump(test, std::cout,  DataTools::treeToStream);

		mout.experimental("The following outputs should be equivalent");

		mout.note("[str][str]");
		drain::TreeUtils::dump(test["dataset1"]["data2"], std::cout,  DataTools::treeToStream);
		mout.note("[elem][elem])");
		drain::TreeUtils::dump(test[ODIMPathElem(ODIMPathElem::DATASET,1)][ODIMPathElem(ODIMPathElem::DATA,2)], std::cout,  DataTools::treeToStream);
		mout.note("[elem(str)][elem(str)])");
		drain::TreeUtils::dump(test[ODIMPathElem("dataset1")][ODIMPathElem("data2")], std::cout,  DataTools::treeToStream);

		mout.note("(str/str)");
		drain::TreeUtils::dump(test("dataset1/data2"), std::cout,  DataTools::treeToStream);
		mout.note("(path(str/str))");
		drain::TreeUtils::dump(test(ODIMPath("dataset1/data2")), std::cout,  DataTools::treeToStream);
		mout.note("(path(elem, elem))");
		drain::TreeUtils::dump(test(ODIMPath(ODIMPathElem(ODIMPathElem::DATASET,1), ODIMPathElem(ODIMPathElem::DATA,2))), std::cout,  DataTools::treeToStream);
		mout.note("(path(elem(str), elem(str)))");
		drain::TreeUtils::dump(test(ODIMPath(ODIMPathElem("dataset1"),ODIMPathElem("data2"))), std::cout,  DataTools::treeToStream);

	}
};

TestModule::TestModule(){ // : CommandSection("science"){

	//	#ifndef RACK_DEBUG...

	//ScienceCmdWrapper<>::setSectionTitle("science");
	// drain::Flagger::ivalue_t section = drain::Static::get<TestSection>().index;
	// drain::CommandBank & cmdBank = drain::getCommandBank();

	install<CmdTestTree>().section; //  = section;

	//drain::BeanRefCommand<FreezingLevel> freezingLevel(RainRateOp::freezingLevel);
	// cmdBank.addExternal(freezingLevel).section = section;

};


} // namespace rack

