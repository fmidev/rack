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


namespace drain {

/// Container and execution tools for commands derived from Command.
/**
 *   # Storage for commands, providing list, help and cloning facilities
 *   # Utilities for creating scripts and programs
 *
 */
class CommandBank : public BankSuper<BasicCommand> {

public:

	/*
	template <class T>
	void add2(){
		const T & src = Bank2<T>::add(key)
	}
	*/
	Flags2 sections;

	///
	std::string defaultCmd;

	/// A mini program executed after each cmd until ']' or ')' is encountered
	Script2 routine;

	/// Converts command and parameter strings to executable command objects.
	//  Note: *Appends* commands to the end of the program
	void append(const Script2 & script, Program & prog, Context & context) const ;

	void remove(Program & prog) const ;

	/// Unlike compile, "interprets" script by running it command by command. \see compile()
	//void run(ScriptTxt & script, drain::Context & context);
	void run(Script2 & script, ClonerBase<Context> & contextSrc);

	/// Convert program arguments a script. Like in main(), actual command arguments start from 1.
	void scriptify(int argc, const char **argv, Script2 & script);

	/// Splits a command line to a list of commands, that is, a script.
	void scriptify(const std::string & cmdLine, Script2 & script);

	/// Converts a Unix/Linux command line to pairs (cmd,params) of strings.
	/**
	 *  \return - true if command argument (argNext) was digested.
	 */
	bool scriptify(const std::string & arg, const std::string & argNext, Script2 & script);


	void help(const std::string & key, std::ostream & ostr = std::cout);

	void help(unsigned int sectionFilter = 0xffffffff, std::ostream & ostr = std::cout);

	/// Checked key and respective command
	void info(const std::string & key, const BasicCommand & cmd, std::ostream & ostr = std::cout) const ;

protected:

	/// Given alias or long key, possibly prefixed by hyphens, return the long key if a command exists.
	/**
	 *
	 */
	const std::string & resolveHyphens(const key_t & key) const;

};



} /* namespace drain */

#endif /* DRAINLET_H_ */

// Rack
