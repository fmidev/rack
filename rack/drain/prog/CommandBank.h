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

#ifndef COMMAND_BANK_H_
#define COMMAND_BANK_H_

#include <iostream>

#include "drain/util/Bank.h"
#include "drain/util/Flags.h"
#include "Command.h"
#include "Program.h"


namespace drain {

/// Container and execution tools for commands derived from Command.
/**
 *   # Storage for commands, providing list, help and cloning facilities
 *   # Utilities for creating scripts and programs
 *
 */
class CommandBank : public BankSuper<Command> { // BankSuper<BasicCommand> {

public:

	typedef data_t command_t;

	CommandBank() :  scriptTriggerFlag(0) {
	};

	CommandBank(const CommandBank & bank) :  scriptTriggerFlag(bank.scriptTriggerFlag){
		// Copy sections?
	};

	//static  // Otherwise ok, but handler as error...
	//const Flagger::value_t SCRIPT_DEFINED;


	/// Words that are moved from class name prior to composing a command name.
	static
	std::set<std::string> & trimWords();

	/// Given a command class name like MyFileReadCommand, derives a respective command line option ("myFileRead")
	static
	void deriveCmdName(std::string & name, char prefix=0);
	//const std::set<std::string> & trim = trimWords());


	template <class D, char PREFIX=0>
	D & addExternal(const D & entry, char alias = 0){
		std::string key(entry.getName());
		deriveCmdName(key, PREFIX);
		//return BankSuper<Command>::template addExternal<D>(entry, key, alias);
		return BankSuper<Command>::addExternal(entry, key, alias);
	}

	template <class D, char PREFIX=0>
	inline
	D & addExternal(const D & entry, const std::string & key, char alias = 0){
		//return BankSuper<Command>::template addExternal<D>(entry, key, alias);
		return BankSuper<Command>::addExternal(entry, key, alias);
	}



	/// A mini program executed after each cmd until ']' or ')' is encountered
	void readFile(const std::string & filename, Script & script) const;

	/// Convert program arguments a script. Like in main(), actual command arguments start from 1.
	/**
	 *   CommandBank is needed in checking if a command has arguments.
	 */
	void scriptify(int argc, const char **argv, Script & script) const;

	/// Splits a command line to a list of commands, that is, a script.
	/**
	 *   CommandBank is needed in checking if a command has arguments.
	 */
	void scriptify(const std::string & cmdLine, Script & script) const;

	/// Converts a Unix/Linux command line to pairs (cmd,params) of strings.
	/**
	 *   CommandBank is needed in checking if a command has arguments.
	 *
	 *   \return - true if command argument (argNext) was digested.
	 */
	bool scriptify(const std::string & arg, const std::string & argNext, Script & script) const;


	/// Converts command strings to executable command objects, appending them to a program.
	/**
	 *   Note: *appends* commands to the end of the program, use prog.clear() if needed.
	 */
	//  void append(const Script & script, Program & prog, Context & context) const ;
	void append(const Script & script, Context & ctx, Program & prog) const ;

	void remove(Program & prog) const;

	/// Run a single command
	void run(const std::string & cmd, const std::string & params, Context & ctx);

	/// Unlike compile, "interprets" script by running it command by command. \see compile()
	//void run(ScriptTxt & script, drain::Context & context);
	// void run(Script & script, ClonerBase<Context> & contextSrc);

	// Newish
	void run(Program & prog, ClonerBase<Context> & contextCloner); // !! May be RackContext!

	void help(const std::string & key, std::ostream & ostr = std::cout);

	void help(Flagger::value_t sectionFilter = 0xffffffff, std::ostream & ostr = std::cout);

	/// Checked key and respective command
	void info(const std::string & key, const command_t & cmd, std::ostream & ostr = std::cout, bool detailed=true) const ;

	/// Given an alias or a long key, possibly prefixed by hyphens, return the long key if a command exists.
	/**
	 *
	 */
	const std::string & resolveFull(const key_t & key) const;

	template <class T>
	void exportStatus(std::map<std::string, T> & statusMap) const;


	/// Member and methods for special commands.
	drain::Flagger::value_t scriptTriggerFlag;


	inline
	void setScriptTriggerFlag(drain::Flagger::value_t sectionFlag){
		scriptTriggerFlag = sectionFlag;
	};

	/// Command that is used, if a plain argument is given.
	/**
	 *   Technically,
	 *   plain \c <argument> is replaced by \c defaultCmd \c <argument>
	 */
	std::string defaultCmdKey;

	/// Set command - key, not getName() - that is used if a plain argument is given. \see
	inline
	void setDefaultCmdKey(const std::string & s){
		defaultCmdKey = s;
	};

	/// If defined, the command - key not getName() - to which all unresolved commands are directed.
	std::string notFoundHandlerCmdKey;

	/// Set command to be executed when a command - argument with leading hyphen(s) - is not found.
	/**
	 *   The argument will be passed to the command.
	 */
	inline
	void setNotFoundHandlerCmdKey(const std::string & s){
		notFoundHandlerCmdKey = s;
	};

	/// Command for storing a routine.
	std::string scriptCmd;

	/// Set command for storing a routine.
	/***
	 *   For example: "script"
	 */
	inline
	void setScriptCmd(const std::string & s){
		scriptCmd = s;
	}


	/// Set command for reading and executing commands from a file in the current (running) context
	/***
	 *   For example: "execFile"
	 */
	inline
	void setScriptFileCmd(const std::string & s){
		execFileCmd = s;
	}

	/// Command for reading and executing commands from a file in the current (running) context
	std::string execFileCmd;



};

template <class T>
void CommandBank::exportStatus(std::map<std::string, T> & statusMap) const {
	for (map_t::const_iterator it = this->begin(); it!=this->end(); ++it){
		statusMap[it->first] = it->second->getSource().getParameters().getValues();
	}
}


/// Global program command registry. Optional utility.
extern
CommandBank & getCommandBank();


} /* namespace drain */

#endif
