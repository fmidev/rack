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

#include "CommandBank.h"


namespace drain {


/// Global command registry.
extern
CommandBank & getCommandBank();

// Static
class CommandBankUtils : public CommandBank {

public:

	// Consider new class (CommandBankUtils)
	static
	void simplifyName(std::string & name, const std::set<std::string> & prune, char prefix=0);

	// Consider new class (CommandBankUtils)
	static
	std::set<std::string> & prunes();

};

/// Creates an instance of command class C in section S of the global command registry.
/**
 * \tparam C - command
 *
 */
template <class C>
class CommandWrapper : public C {
public:

	/// Add a command to the shared command bank.
	CommandWrapper(const std::string & name, char alias = 0){
		getCommandBank().add<C>(name,alias); //.section = S;
	}

	/// Add a command to the shared command bank, deriving command name automatically from the class name.
	CommandWrapper(char prefix = 0){
		std::string name = C::getName();
		CommandBankUtils::simplifyName(name, CommandBankUtils::prunes(), prefix);
		getCommandBank().add<C>(name);
	}

	/// Add a command to the shared command bank, deriving command name automatically from the class name.
	/*
	CommandWrapper(C & cmd, const std::string & name="", char alias = 0){
		if (name.empty()){
		}
		//std::string name = C::getName();
		//CommandBankUtils::simplifyName(name, CommandBankUtils::prunes(), prefix);
		getCommandBank().addExternal(cmd, name, alias);
	}
	*/

};


template <class B>
class BeanCommandEntry : public CommandWrapper<BeanCommand<B> > {

public:
	BeanCommandEntry(char prefix = 0) : CommandWrapper<BeanCommand<B> >(prefix){
	};

};





class CmdVerbosity : public SimpleCommand<int> {

public:

	CmdVerbosity() : SimpleCommand<int>(__FUNCTION__, "Logging verbosity", "level", drain::getLog().getVerbosity()) {
	};

	inline
	void exec() const {
		drain::getLog().setVerbosity(value);
	}

};


class HelpCmd : public SimpleCommand<std::string> {

public:

	inline
	HelpCmd(CommandBank & bank, const std::string & key=__FUNCTION__, const std::string & description = "Display help.") :
	SimpleCommand<std::string>(key, description, "[command|sections]"), bank(bank) {};

	inline
	void exec() const {

		if (value.empty())
			bank.help();
		else {
			bank.help(value);
			//  TODO: "see-also" commands as a list, which is checked.
		}

	}

protected:

	// Copy constructor should copy this as well.
	CommandBank & bank;

};

class CmdRoutine : public SimpleCommand<std::string> {

public:

	CmdRoutine(CommandBank & bank) : SimpleCommand<std::string>(__FUNCTION__, "Define script.", "xx"), bank(bank) {};

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


} /* namespace drain */

#endif
