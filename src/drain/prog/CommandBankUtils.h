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
#include "CommandSections.h"
#include "drain/image/Image.h"


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
		drain::image::getImgLog().setVerbosity(value-1);
	}

};

class CmdDebug : public BasicCommand {

public:

	inline
	CmdDebug() : BasicCommand(__FUNCTION__, "Set verbosity to LOG_DEBUG") { // TODO
	};

	inline
	void exec() const {
		Context & ctx = getContext<Context>();
		ctx.log.setVerbosity(LOG_DEBUG);
		drain::getLog().setVerbosity(LOG_DEBUG);
		drain::image::getImgLog().setVerbosity(LOG_DEBUG);
		//r.run("verbose","8");  // FIXME r.setVerbosity();
	};

};

//class CmdLog : public SimpleCommand<> {
class CmdLog : public BasicCommand {

public:

	CmdLog(CommandBank & cmdBank);

	CmdLog(const CmdLog & cmd);

	void exec() const;

protected:

	CommandBank & bank;

	std::string filename;

	// For numeric or string keys
	std::string level;

	bool timing;

};





class CmdStatus : public drain::BasicCommand {

public:

	inline
	CmdStatus() : drain::BasicCommand(__FUNCTION__, "Dump information on current images.") {
	};

	void exec() const;

};


class CmdExpandVariables : public BasicCommand {

public:

	inline
	CmdExpandVariables() : BasicCommand(__FUNCTION__, "Toggle variable expansions on/off") {
	};

	inline
	void exec() const {
		SmartContext & ctx = getContext<SmartContext>();
		//CommandRegistry & r = getRegistry();
		ctx.expandVariables = !ctx.expandVariables;
	};

};


/// Load script file and executes the commands immediately
/**
 *   Lightweight. Keeps commands until commandBank reads and stores them.
 *
 */
class CmdScript : public SimpleCommand<std::string> {

public:

	inline
	CmdScript(CommandBank & cmdBank) :
		SimpleCommand<std::string>(__FUNCTION__, "Define script.", "script"),
		bank(cmdBank){
		cmdBank.scriptCmd = getName(); // mark me special
	};

	inline
	CmdScript(const CmdScript & cmd) : SimpleCommand<std::string>(cmd), bank(cmd.bank){
	}

protected:

	// Copy constructor should copy this as well.
	// FUture versions may store the script in Context?
	CommandBank & bank;

};


/// Executes the defined script
/**
 *   Note: automatic triggering by selected commands is often more handy.
 *
 */
class CmdExecScript : public BasicCommand {

public:

	inline
	CmdExecScript() :
		BasicCommand(__FUNCTION__, "Execute script.") {
		// const drain::Flagger::ivalue_t
		cmd_section_type TRIGGER = drain::Static::get<drain::TriggerSection>().index;
		this->section |= TRIGGER;
		/*
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.experimental("constr: TRIGGER=" , TRIGGER );
		mout.experimental("My sections: " , this->section );
		*/
	};

	/*
	inline
	CmdExecScript(const	CmdExecScript & cmd) : BasicCommand(cmd){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.experimental("Copy constr, their sections: " , cmd.section );
		mout.experimental("Copy constr, my sections:    " , this->section );
	};
	*/
	/*
	CmdExecScript(CommandBank & cmdBank) :
		BasicCommand(__FUNCTION__, "Execute script.") {
		//cmdBank.setScriptExecCmd(getName());
		const drain::Flagger::value_t TRIGGER = drain::Static::get<drain::TriggerSection>().index;
		cmdBank.setScriptTriggerFlag(TRIGGER);
		this->section |= TRIGGER;

		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.experimental("constr: TRIGGER=" , TRIGGER );
		mout.experimental(*this );
		mout.experimental("My sections: '" , this->section , "' ." );

	};
	*/

	inline
	void exec() const {
		SmartContext & ctx = getContext<SmartContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		mout.debug("Executing script with '" , getName() , "' ." );
		//mout.warn("My sections: '" , this->section , "' ." );
		//mout.error("This command '" , getName() , "' cannot be run independently." );
	};

	// reconsider exec();

protected:

	// Copy constructor should copy this as well.
	// FUture versions may store the script in Context!
	//CommandBank & bank;

};



/// Load script file and execute the commands immediately using current Context.
/**
 *
 *   Does not store the commands into a CommandBank::script.
 */
class CmdExecFile : public SimpleCommand<std::string> {

public:

	CmdExecFile(CommandBank & cmdBank) :
		SimpleCommand<std::string>(__FUNCTION__, "Execute commands from a file.", "filename"),
		bank(cmdBank){
		cmdBank.execFileCmd = getName(); // mark me special
	};

protected:

	// Copy constructor should copy this as well.
	CommandBank & bank;

};


/// CommandBank-dependent
class HelpCmd : public SimpleCommand<std::string> {

public:


	HelpCmd(CommandBank & cmdBank) : SimpleCommand<std::string>(__FUNCTION__, "Display help.", "key", "", "command|sections"), cmdBank(cmdBank) {
	};

	inline
	void exec() const {
		if (value.empty()){
			if (cmdBank.has("general")){
				cmdBank.help("general");
				exit(0);
			}
		}
		cmdBank.help(value);
		//  TODO: "see-also" commands as a list, which is checked.
		exit(0);
	}

protected:

	// Copy constructor should copy this as well.
	CommandBank & cmdBank;

};




//template <class C=Context>
class CmdFormat : public SimpleCommand<std::string> {

public:

	CmdFormat() :  SimpleCommand<std::string>(__FUNCTION__,"Set format for data dumps (see --sample or --outputFile)", "format","") {  // SimpleCommand<std::string>(getResources().generalCommands, name, alias, "Sets a format std::string.") {
	};

	inline
	void exec() const {
		SmartContext &ctx = getContext<SmartContext>();
		ctx.formatStr = value;
	}


};




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

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

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
		mout.error(getName() , ": opening file '" , value , "' failed." );

};


/// Special command for handling undefined commands.
class CmdNotFound : public SimpleCommand<> {

public:

	CmdNotFound(CommandBank & cmdBank) :
		SimpleCommand<std::string>(__FUNCTION__, "Throw exception on unfound ", "cmdArg","")
		//cmdBank(cmdBank)
		{
		section = 0; // hidden
		cmdBank.notFoundHandlerCmdKey = "notFound"; // getName();
	};

	void exec() const {
		Context & ctx = getContext<>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		mout.error("Command '" , value , "' not found." );
		/// cmdBank.help() ?
	}

};



} /* namespace drain */

#endif
