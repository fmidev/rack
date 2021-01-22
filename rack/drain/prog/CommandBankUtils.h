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


// New design (2020)

#ifndef COMMAND_BANK_UTILS_H_
#define COMMAND_BANK_UTILS_H_

#include <iostream>
#include <fstream>

#include "CommandBank.h"


namespace drain {


// Static
/*
class CommandBankUtils : public CommandBank {
public:
};
*/





class CmdVerbosity : public SimpleCommand<int> {

public:

	CmdVerbosity() : SimpleCommand<int>(__FUNCTION__, "Logging verbosity", "level", drain::getLog().getVerbosity()) {
	};


	inline
	void exec() const {
		// NEW
		Context & ctx = getContext<Context>();
		ctx.log.setVerbosity(value);

		// OLD
		drain::getLog().setVerbosity(value);
	}

};

class CmdDebug : public BasicCommand {

public:

	CmdDebug() : BasicCommand(__FUNCTION__, "Set verbosity to LOG_DEBUG") { // TODO
	};

	inline
	void exec() const {
		Context & ctx = getContext<Context>();
		ctx.log.setVerbosity(LOG_DEBUG);
		drain::getLog().setVerbosity(LOG_DEBUG);
		//r.run("verbose","8");  // FIXME r.setVerbosity();
	};

};

class CmdLogFile : public SimpleCommand<> {

public:

	CmdLogFile() : SimpleCommand<>(__FUNCTION__, "Set log ", "filename", getContext<Context>().logFile) {
	};

	inline
	void exec() const {
		Context & ctx = getContext<Context>();
		ctx.logFile = value;
		// TODO: setLog ptr right away?
	}

};





class CmdStatus : public drain::BasicCommand {

public:

	CmdStatus() : drain::BasicCommand(__FUNCTION__, "Dump information on current images.") {
	};

	void exec() const {

		Context & ctx = getContext<Context>();

		std::ostream & ostr = std::cout; // for now...

		const drain::VariableMap & statusMap = ctx.getStatus();

		for (drain::VariableMap::const_iterator it = statusMap.begin(); it != statusMap.end(); ++it){
			ostr << it->first << '=' << it->second << ' ';
			it->second.typeInfo(ostr);
			ostr << '\n';
		}
		//ostr << "errorFlags: " << ctx.statusFlags << std::endl;

	};


};


class CmdExpandVariables : public BasicCommand {

public:

	CmdExpandVariables() : BasicCommand(__FUNCTION__, "Toggle variable expansions on/off") {
	};

	inline
	void exec() const {
		SmartContext & ctx = getContext<SmartContext>();
		//CommandRegistry & r = getRegistry();
		ctx.expandVariables = !ctx.expandVariables;
	};

};

class HelpCmd : public SimpleCommand<std::string> {

public:

	inline
	HelpCmd(CommandBank & bank, const std::string & key=__FUNCTION__, const std::string & description = "Display help.") :
	SimpleCommand<std::string>(key, description, "[command|sections]"), bank(bank) {};

	inline
	void exec() const {
		bank.help(value);
		//  TODO: "see-also" commands as a list, which is checked.
		exit(0);
	}

protected:

	// Copy constructor should copy this as well.
	CommandBank & bank;

};

class CmdRoutine : public SimpleCommand<std::string> {

public:

	CmdRoutine(CommandBank & bank) : SimpleCommand<std::string>(__FUNCTION__, "Define script.", "script"),
		bank(bank) {};

	inline
	void exec() const {
		//drain::Logger mout(__FILE__, getName());
		//mout.debug() << "---" << mout.endl;
		bank.scriptify(value, bank.routine);
		//bank.routine.toStream();
		//mout.debug() << "---" << mout.endl;
	}

protected:

	CommandBank & bank;

};


/// Load script file and executes the commands immediately
/**
 *   Does not store the commands into a script.
 */
class CmdExecFile : public SimpleCommand<std::string> {

public:

	CmdExecFile(CommandBank & bank, ContextClonerBase & contextCloner) : SimpleCommand<std::string>(__FUNCTION__, "Execute commands from a file.", "filename"),
		bank(bank), contextCloner(contextCloner) {

	};

	//CommandCommandLoader(std::set<std::string> & commandSet, const std::string & name, char alias = 0);

	virtual
	void exec() const;

protected:

	CommandBank & bank;
	ContextClonerBase & contextCloner;
};

template <class C=Context>
class CmdFormat : public SimpleCommand<std::string> {

public:

	CmdFormat() :  SimpleCommand<std::string>(__FUNCTION__,"Set format for data dumps (see --sample or --outputFile)", "format","") {  // SimpleCommand<std::string>(getResources().generalCommands, name, alias, "Sets a format std::string.") {
	};

	void exec() const {
		C &ctx = getContext<C>();
		ctx.formatStr = value;
	}

};


/*
void CmdFormat::exec() const {
	Context &ctx = getContext<Context>();
	ctx.formatStr = value;
}
*/



template <class C=Context>
class CmdFormatFile : public SimpleCommand<std::string> {

public:


	CmdFormatFile() : SimpleCommand<>(__FUNCTION__, "Read format for metadata dump from a file","filename","","std::string") {
	};

	void exec() const;

};


template <class C>
void CmdFormatFile<C>::exec() const {

	C &ctx = getContext<C>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	//drain::Input ifstr(value);
	std::ifstream ifstr;
	ifstr.open(value.c_str(), std::ios::in);
	if (ifstr.good()){
		std::stringstream sstr;
		sstr << ifstr.rdbuf();
		/*
		for (int c = ifstr.get(); !ifstr.eof(); c = ifstr.get()){ // why not getline?
			sstr << (char)c;
		}
		*/
		ifstr.close();
		//Context &ctx = getContext<>();
		ctx.formatStr = sstr.str(); // SmartContext ?

	}
	else
		mout.error() << name << ": opening file '" << value << "' failed." << mout.endl;

};


} /* namespace drain */

#endif
