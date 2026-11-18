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

	const drain::VariableMap & statusMap = ctx.getUpdatedStatusMap();

	for (const auto & entry: statusMap){

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



void CmdForEach::exec() const  {

	Context & ctx = getContext<Context>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = resources.mout;

	std::string key;
	std::string values;
	drain::StringTools::split2(value, key, values, '=');

	if (values.empty()){
		mout.warn("empty argument: '<key>=<values>'");
		return;
	}

	StringMapper stringMapper;
	stringMapper.parse(values, true);
	values = stringMapper.toStr(ctx.getUpdatedStatusMap(), StringMapper::KEEP_MISSING_VARIABLE);

	if (values.empty()){
		mout.warn("empty argument after expansion of value=", value);
		return;
	}
	else {
		ctx.loops.push_back(drain::Loop());
		drain::Loop & loop = ctx.loops.back();
		loop.set(key, values);
		mout.experimental("Added loop: ", key, ':', sprinter(loop.getValueList()));
		/*
			for (const auto & value: loop.getValueList()){
				mout.experimental(key, "=", value);
			}
		 */
	}

}



void CmdJSON::commandToJSON_old(const drain::Command & cmd, drain::JSONtree & json){

	const drain::ReferenceMap & m = cmd.getParameters();

	const drain::ReferenceMap::unitmap_t & u = m.getUnitMap();

	// drain::JSONtree jsonRoot;
	//drain::JSONtree & json = jsonRoot[value];
	json["title"] = cmd.getDescription();
	drain::JSONtree & variables = json["variables"];

	const drain::ReferenceMap::keylist_t & keys = m.getKeyList();
	//std::ostream & ostr = std::cout;

	for (const drain::ReferenceMap::key_t & key: keys){

		// const drain::Referencer & entry = m[key];

		drain::JSONtree & variable = variables[key];

		variable["value"] = m[key]; //entry;

		drain::ReferenceMap::unitmap_t::const_iterator uit = u.find(key);
		if (uit != u.end()){
			variable["unit"] = uit->second;
		}

	}

	//drain::Sprinter::toStream(ostr, jsonRoot, drain::Sprinter::jsonLayout);
	// drain::TreeUtils::dump(jsonRoot, ostr, nullptr);

};

/**
 *  Stores the following "official" attributes:
 *  - \name :
 *  - \desc :
 *  - \desc :
 *
 */
void CmdJSON::commandToJSON(const drain::Command & cmd, drain::JSONtree & dst){

	drain::Logger mout(__FILE__, __FUNCTION__);

	dst["cls"]  = cmd.getName();
	dst["desc"] = cmd.getDescription();

	const drain::ReferenceMap & params = cmd.getParameters();
	const drain::ReferenceMap::unitmap_t & unitMap = params.getUnitMap();

	drain::JSONtree & dstParams = dst["params"];

	for (const auto & key: params.getKeyList()){
		const drain::Reference & param = params[key];
		drain::JSONtree & dstParam = dstParams[key];
		// dstParam["param"] = key; // repeated, for easier row-by-row exports
		/*
		if (param.isString() && param.empty()){
			dstParam["value"] = "";
		}
		else {
		}
		*/
		dstParam["value"].data = param;
		dstParam["type"]  = param.getTypeName();
		// param.inputSeparator
		drain::ReferenceMap::unitmap_t::const_iterator uit = unitMap.find(key);
		if (uit != unitMap.end()){
			if (!uit->second.empty()){
				dstParam["unit"] = uit->second;
			}
		}
	}


}

/**
 *  \param cmds - comma-separated list of commands
 */

void CmdJSON::commandsToJSON(const drain::CommandBank & cmdBank, const std::string & cmds, drain::JSONtree & dst, char separator){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// const drain::CommandBank & cmdBank = drain::getCommandBank(); // this
	const drain::CommandBank::map_t & cmdMap = cmdBank.getMap();

	std::list<std::string> values;
	drain::StringTools::split(cmds, values, separator);

	if (values.empty()){
		mout.error("Empty argument");
		return;
	}

	// const bool MULTI_CMD_HELP = (values.size() > 1);

	// Light strip (could use

	for (const std::string & v: values){

		if (v.empty()){
			mout.debug("(empty key, ok)");
			continue;
		}

		// Light strip (could use
		const std::string key = cmdBank.resolveFull(v);

		if (key.empty()){
			mout.error("command not found: '", v, "'");
		}
		else if (key != v){
			mout.info("resolved: " , v, " -> ", key );
		}

		drain::CommandBank::map_t::const_iterator it = cmdMap.find(key);

		if (it != cmdMap.end()){
			const drain::Command & cmd = it->second->getSource();
			commandToJSON(cmd, dst[key]);
		}
		else {
			// Should be impossible. Resolved key should
			mout.error("command not found: '", key, "' (", v,")");
		}


	}
}


/**
 *   Implicit names do not appear in the JSON structure but are recommendations for the keys therein.
 */
const std::map<std::string,std::string> & CmdJSON::getVariables() {

	static
	const std::map<std::string,std::string> m = {
			// Command properties
			{"cls",    "Class"},     // Implicit CmdCappi
			{"key",    "Option"},     // As appears on command line, without leading hyphens, for example: "pCappi"
			{"desc",   "Description"},
			{"params", "Parameters"},
			// Parameter properties
			{"param",  "Parameter"}, // Implicit
			{"value",  "Default"},
			{"type",   "Type"},      // Automatic drain::TypeName<T>
			{"unit",   "Unit"},      // User-defined
	};

	return m;
};


void CmdScriptFile::exec() const {

	Context & ctx = getContext<Context>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = resources.mout;

	const CommandBank & bank = getCommandBank();
	// bank.scriptify();
	// ctx.routine;
	Program prog(getContext<Context>());
	bank.readFile(value, prog); // under construction

	mout.unimplemented<LOG_ERR>(prog);

}

/* No need for separate ForEach, because for "i=${var}" can be expanded.
void CmdForEach::exec() const  {

	Context & ctx = getContext<Context>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = resources.mout;

	// StringMapper statusFormatter;
	VariableMap & statusMap = ctx.getUpdatedStatusMap();

	std::string key;
	std::string varKey;
	drain::StringTools::split2(value, key, varKey, ':');

	if (varKey.empty()){
		mout.warn("Empty variable name. Syntax: '<key>:<varName>'");
		return;
	}

	ctx.loops.push_back(drain::Loop());
	drain::Loop & loop = ctx.loops.back();
	loop.set(key, statusMap.get(varKey,""));

	if (loop.getValueList().empty()){
		mout.warn("Value of ", varKey, " is empty (arguments of: forEach ", key, ": ...)");
	}
	else {
		mout.accept<LOG_NOTICE>(DRAIN_LOG(key));
		mout.accept<LOG_NOTICE>(DRAIN_LOG(varKey));
		mout.experimental("Added loop: ", key, ':', sprinter(loop.getValueList()));
	}

}
*/



} // drain
