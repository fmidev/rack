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
#ifndef COMMAND_REGISTRY_H_
#define COMMAND_REGISTRY_H_

#include <map>
#include <set>

#include "drain/util/Log.h"
#include "drain/util/ReferenceMap.h"
#include "drain/util/VariableMap.h"
#include "drain/util/StringMapper.h"
#include "drain/util/Registry.h"

#include "Command.h"

namespace drain {



class CommandRegistry : public Registry<Command> {

public:

	inline
	CommandRegistry() : DEFAULT_HANDLER("DefaultCmd"), expandVariables(false), statusFormatter("[a-zA-Z0-9_:]+") {
		++index; };

	CommandRegistry(const CommandRegistry & r){
		std::cerr << "CommandRegistry called copy const \n";
	}

	typedef std::map<std::string, std::set<std::string> > SectionMap;

	virtual
	void add(Command & r, const std::string & name, char alias = 0){
		add(section, r, name, alias);
	}

	void add(const std::string & section, Command & r, const std::string & name, char alias);

	inline
	void setPrefix(const std::string & p)  {
		prefix.assign(p);
	};

	inline
	void setSection(const std::string & s, const std::string & p = "")  {
		section.assign(s);
		prefix.assign(p);
	};

	const SectionMap & getSections() const { return sections; };


	/// Help on all the registered drainlets (short entries).
	void help(std::ostream & ostr = std::cout) const;

	/// Help on a specific drainlet or section.
	void help(const std::string & key, std::ostream & ostr = std::cout, bool paramDescriptions = true) const;

	/// Help on a specific cmd.  // TODO: template T for Command concept instances
	void help(const Command & cmd, std::ostream & ostr = std::cout, bool paramDescriptions = false) const;

	/// Help on sections
	void helpSections(std::ostream & ostr = std::cout) const;

	/// Help on a section
	void helpSections(std::ostream & ostr, const std::string & section) const;

	/// Executes a command. Expands variables appearing in parameters if expandVariables is set.
	/**
	 *   May modify the state of the command.
	 */
	void run(Command & cmd, const std::string & parameters = "") const;

	/// Executes a command.
	void run(const std::string & name, const std::string & parameters = "") const;

	/// Executes a script
	/*
	 *  \par script - list of Commands with associated arguments.
	 */
	void run(Script & script) const;

	/// Returns the last command executed.
	const std::string & getLastCommand() const { return lastCommand;};

	/// Compiles a script from a std::string.
	/**
	 *  Splits the std::string to a script.
	 *
	 *  \par input -  the script as a std::string, one command at each line.
	 *  \par script - the resulting CommandScript .
	 */
	void scriptify(const std::string & input, Script & dst);

	/// Compiles a script from arguments like those of main(argc, argv).
	/**
	 *  Splits the std::string to a script.
	 *
	 *  \par input -  the script as a std::string, one command at each line.
	 *  \par script - the resulting CommandScript .
	 */
	void scriptify(int argc, const char **argv, Script & script);


	/// Executes a set of commands.
	/*
	 *   Converts the command arguments to a CommandScript and calls runProgram() on it.
	 *  \par argv -  the script as a std::string, one command at each line.
	 */
	inline
	void runCommands(int argc, const char **argv){
		Script script;
		scriptify(argc, argv, script);
		run(script);
	}

	/// Executes a set of commands.
	/*
	 *   Converts the script std::string to a CommandScript and calls runProgram() on it.
	 *  \par toStr -  the script as a std::string, one command at each line.
	 */
	void runCommands(const std::string & str){
		Script script;
		scriptify(str, script);
		run(script);
	};
	// void runCommands(const std::list<std::string> & input);

	/// Return the current status map, by default updated with commands.
	//drain::VariableMap & getStatusMap(bool update = true);
	drain::VariableMap & getStatusMap(bool update = true);

	std::string DEFAULT_HANDLER;

	bool expandVariables;

	mutable drain::StringMapper statusFormatter;

	/// Cf with help
	void toJSON(std::ostream & ostr) const;

protected:

	std::string prefix;

	mutable drain::VariableMap statusMap; // this should be separate? or the commands can use this internally?


	std::string section;

	SectionMap sections;

	mutable
	std::string lastCommand;

	/// Adds a command at the bottom of the script.
	bool appendCommand(const std::string & s, const std::string & parameterCandidate, Script & script);

	std::string resolveKey(const std::string & s) const;

	static int index;

};


///
/*
 *
 */
extern
CommandRegistry & getRegistry();

extern
const CommandRegistry::SectionMap & getSections();

extern
void addEntry(const std::string & section, Command & command, const std::string & name, char alias = 0);

extern
void addEntry(Command & command, const std::string & name, char alias = 0);

//void addEntry(Command & r, const std::string & name, char alias = 0);






class CommandGroup {

public:

	CommandGroup(const std::string & section, const std::string & prefix = "") {
		CommandRegistry & registry = getRegistry();
		registry.setSection(section);
		registry.setPrefix(prefix); // of: if !prefix.empty();
	}

	CommandGroup() {
	}

};


} /* namespace drain */

#endif /* DRAINREGISTRY_H_ */

// Drain
