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

#ifndef DRAIN_CMD_INSTALLER_H_
#define DRAIN_CMD_INSTALLER_H_

#include <map>
#include <set>
//#include "drain/util/Debug.h"
//#include "drain/util/ReferenceMap.h"

#include "drain/util/Log.h"
#include "drain/util/ReferenceMap.h"

//#include "Command.h"
//#include "CommandRegistry.h"

#include "CommandBank.h"
#include "CommandSections.h"


namespace drain {

/// Creates an instance of command class C.
//  in section S of the global command registry.
/**
 * \tparam C - command
 *
 */
/*
template <class C> //, char PREFIX=0, int SECTIONS=1>
class CommandWrapper : public C {
public:

	/// Add a command to the shared command bank.
	CommandWrapper(const std::string & name, char alias = 0){
		getCommandBank().add<C>(name,alias); //.section = S;
	}

};
*/



/// Creates an instance of command class C, deriving command name from the class name and prefixing if desired.
//  in section S of the global command registry.
/**
 * \tparam PREFIX - optional prefix, added to lower-case initialized
 * \tparam SECTIONS - command sections flags
 */
template <char PREFIX=0, class SECTION=GeneralSection>
class CommandInstaller { //: public CMD {
public:

	CommandBank & cmdBank;

	CommandInstaller(CommandBank & bank = getCommandBank()) : cmdBank(bank){
		 getSection();
	};

	inline static
	const CommandSection & getSection(){
		return drain::Static::get<SECTION>();
	};

	/// Return the character used as prefix for the commands in this section.
	static inline
	char getPrefix(){
		return PREFIX;
	}


	template <class CMD>
	Command & install(const std::string & name, char alias = 0){
		Command & cmd = cmdBank.add<CMD>(name,alias);
		cmd.section |= getSection().index; // keep TRIGGER
		return cmd;
	}

	template <class CMD>
	Command & install(char alias = 0){
		std::string name = CMD().getName();
		CommandBank::deriveCmdName(name, PREFIX);
		return install<CMD>(name, alias);
	}

	template <class CMD>
	Command & installExternal(CMD & cmdExt, const std::string & name, char alias = 0){
		Command & cmd = cmdBank.addExternal(cmdExt, name,alias);// must give cmdExt, for copying
		cmd.section |= getSection().index; // keep TRIGGER
		return cmd;
	}

	template <class CMD>
	Command & installExternal(CMD & cmdExt, char alias = 0){
		std::string name = cmdExt.getName();
		CommandBank::deriveCmdName(name, PREFIX);
		//Command & cmd = bank.addExternal(cmdExt, name,alias); // must give cmdExt, for copying
		//cmd.section = getSection().index;
		return installExternal(cmdExt, name, alias);
	}

	/// Install to shared CommandBank.
	template <class CMD>
	static
	Command & installShared(const std::string & name, char alias = 0){
		Command & cmd = getCommandBank().add<CMD>(name,alias);
		cmd.section |= getSection().index; // keep TRIGGER
		return cmd;
	}

	/// Install to shared CommandBank.
	template <class CMD>
	static
	Command & installShared(char alias = 0){
		std::string name = CMD().getName();
		CommandBank::deriveCmdName(name, PREFIX);
		return installShared<CMD>(name, alias);
	}

};


/// Combines command installation and sectioning.
/**
 *  \param prefix - 'c'
 *  \param name   - "cart"
 *
 */
template <char PREFIX=0, class SECTION=GeneralSection>
class CommandModule : protected CommandInstaller<PREFIX, SECTION>{

public:

	typedef CommandModule<PREFIX, SECTION> module_t;
	//typedef CommandInstaller<PREFIX, SECTION> base_t;

	/**
	 *  \param prefix - 'c'
	 *  \param name   - "cart"
	 *
	 */
	CommandModule(CommandBank & bank = getCommandBank()) : CommandInstaller<PREFIX, SECTION>(bank)
		{

	};

	// CommandModule(const std::string & name, CommandBank & bank = getCommandBank()) :
	// CommandInstaller<PREFIX, SECTION>(bank), name(name)

	// virtual	void initialize() = 0;

	/// Default installer for this module.
	//  CommandInstaller<> installer;
	//  const std::string name;


};

}



/* namespace drain */

#endif /* DRAINLET_H_ */

// Rack
