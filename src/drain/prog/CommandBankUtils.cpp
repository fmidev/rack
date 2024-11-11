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

#include "drain/util/Input.h"
#include "CommandBankUtils.h"

namespace drain {


CmdLog::CmdLog(CommandBank & cmdBank) : BasicCommand(__FUNCTION__, "Redirect log to file. Status variables like ${ID}, ${PID} and ${CTX} supported."), bank(cmdBank){
	//, "filename", "/tmp/thread-${ID}.log"), bank(cmdBank) {
	// TODO: change order of params!
	getParameters().link("file", filename);
	getParameters().link("level", level);
	getParameters().link("timing", timing=false); // could be static, directly. (See copyStruct FLAGS?)
	getParameters().link("vt100", Log::USE_VT100); // Note: -DVT100
};

CmdLog::CmdLog(const CmdLog & cmd) : BasicCommand(cmd), bank(cmd.bank), timing(false) {
	getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK); //
}


void CmdLog::exec() const {

	Context & ctx = getContext<Context>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// TODO: change order of params!
	if (!filename.empty()){
		bank.logFileSyntax.parse(filename);
		mout.debug("parsed filename: ", bank.logFileSyntax);
	}

	if (!level.empty()){
		mout.unimplemented("level argument...");
		//mout.warn("under constr...");
	}

	drain::Logger::TIMING = timing;

}

void CmdStatus::exec() const {

	Context & ctx = getContext<Context>();

	// std::ostream & ostr = std::cout; // for now...

	const drain::VariableMap & statusMap = ctx.getStatusMap();

	/*
	static
	const SprinterLayout layout(",", "\n", "{:}", "");
	Sprinter::mapToStream(std::cout, statusMap, layout, statusMap.getKeyList());
	std::cout << '\n';
	*/
	//ostr << drain::sprinter(statusMap, layout) << '\n';


	for (const auto & entry: statusMap){
		//ostr << it->first << '=' << it->second << ' ';
		std::cout << entry.first << '=' << entry.second << ' ';
		entry.second.typeInfo(std::cout);
		std::cout << '[';
			if (entry.second.isString())
				std::cout << "string";
			else
				std::cout << drain::Type::call<drain::compactName>(entry.second.getType());
				//std::cout << drain::Type::getTypeChar(entry.second.getType());
			//if (size_t n = getElementCount() != 1)
			std::cout << entry.second.getElementCount();
			std::cout << '@' << entry.second.getElementSize() << ']';
		std::cout << '\n';
	}

	//ostr << "errorFlags: " << ctx.statusFlags << std::endl;

};



} /* namespace drain */

// Rack
