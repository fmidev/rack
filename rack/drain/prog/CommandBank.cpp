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
 
/*
 * RackLet.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: mpe
 */

//#include <fstream>


#include "drain/util/Log.h"

#include "CommandUtils.h"
#include "CommandBank.h"

namespace drain {

Flags2::value_t CommandBank::GENERAL = 1;
Flags2::value_t CommandBank::INPUT = 2;
Flags2::value_t CommandBank::OUTPUT = 4;
Flags2::value_t CommandBank::IO = INPUT | OUTPUT;
Flags2::value_t CommandBank::SPECIAL = 8;


/// Appends program with commands fo the script
void CommandBank::append(const Script2 & script, Program & prog) const {
	for (Script2::const_iterator it = script.begin(); it!=script.end(); ++it) {
		command_t & cmd = clone(it->first);
		cmd.setParameters(it->second);
		prog.add(cmd);
	}
}



// Future extension.
void CommandBank::remove(Program & prog) const {
	std::cout << "clearing prog: " << '\n';
	prog.toStream();
	for (Program::const_iterator it = prog.begin(); it!=prog.end(); ++it) {

	}
}




/// Run a single command
void CommandBank::run(const std::string & cmdKey, const std::string & params, Context & ctx){
	Logger mout(__FILE__, __FUNCTION__);

	//BasicCommand
	command_t & cmd = get(cmdKey);

	if (cmd.contextIsSet())
		mout.warn() << "replacing context " << mout.endl;
	cmd.setContext(ctx);
	cmd.run(params);
}



void CommandBank::run(Script2 & script, ClonerBase<Context> & contextSrc){

	Logger mout(__FILE__, __FUNCTION__); // warni

	bool PARALLEL_MODE = false;

	//	std::vector<Program> threads;
	ProgramVector threads;

	for (Script2::const_iterator it = script.begin(); it != script.end(); ++it) {

		mout.note() << it->first << mout.endl;

		if (it->first == "["){
			PARALLEL_MODE = true;
			continue;
		}

		const bool USE_ROUTINE = !routine.empty();

		if (!PARALLEL_MODE){

			// Check here if routine exists (the current cmd may set the routine, and the it should not be run).

			value_t & cmd = get(it->first); // or clone, to keep defaults? => consider FLAG save-params
			cmd.setParameters(it->second);
			//cmd.run(it->second);
			cmd.exec();
			if (USE_ROUTINE){
				Program prog(contextSrc.get());
				//prog.setContext(ctx);
				append(routine, prog); //, contextSrc.get());
				//prog.append(*this, routine);
				prog.run();
			}
			// TODO: when to automatically clear routine? Perhaps upon \)
		}
		else {

			// If all runs collected, run them
			if (it->first == "]"){
				PARALLEL_MODE = false;
				mout.note() << "Start threads" << mout.endl;

				#pragma omp parallel for
				for (size_t i = 0; i < threads.size(); ++i) {
					// MINIMAL CONTENT. No mout here, unless new instance!
					// std::cout << "Start thread #" << i << std::endl;
					threads[i].run();
				}
				mout.note() << "Completed..." << mout.endl;
				routine.clear(); // consider

			}
			else { // continue collecting

				// Append new thread.
				Context & ctx = contextSrc.clone();
				ctx.id = 1000 + threads.size();
				Program & prog = threads.add(ctx);

				mout.note() << "Preparing thread " << ctx.id << ':' << it->first << '(' << it->second << ')' <<  mout.endl;

				value_t & cmd = clone(it->first);
				cmd.setParameters(it->second);
				prog.add(cmd);

				if (USE_ROUTINE){ // Technically unneeded, but for mout...
					mout.note() << "appending script to thread " <<  mout.endl;
					append(routine, prog); // append
				}

			}

		}
	}

}



/// Converts linux command line to pairs of strings:  (<cmd>, [params])
/**
 *  Starts from argument 1 instead of 0.
 */
void CommandBank::scriptify(int argc, const char **argv, Script2 & script){

	Logger mout(__FILE__, __FUNCTION__); // warning, not initialized

	//bool PARALLEL = false;
	for (int i = 1; i < argc; ++i) {

		const std::string arg(argv[i]);

		if (arg.empty()){
			mout.warn() << "empty arg" <<  mout.endl;
			continue;
		}

		const std::string argNext((i+1) < argc ? argv[i+1] : "");

		bool ARG_TAKEN = scriptify(arg, argNext, script);

		if (ARG_TAKEN){
			// Swallowed argNext as well
			++i;
			if (i == argc){
				// Some commands like --help are fine with and without arg (when ending line).
				//mout.warn() << arg << ": argument missing (premature end of command line), i=" << i <<  mout.endl;
				return;
			}
		}

	}
}

void CommandBank::scriptify(const std::string & line, Script2 & script){

	Logger mout(__FILE__, __FUNCTION__); //

	typedef std::list<std::string> list_t;
	list_t l;
	drain::StringTools::split(line, l, " ");

	list_t::const_iterator it = l.begin();
	while (it != l.end()) {
		const std::string & arg = *it;
		++it;
		if (it != l.end()){
			const std::string & argNext = *it;
			if (scriptify(arg, argNext, script))
				++it;
		}
		else {
			if (scriptify(arg, "", script))
				mout.warn() << arg << ": argument missing (premature end of command line), i=" <<  mout.endl;
		}
	}
}

bool CommandBank::scriptify(const std::string & arg, const std::string & argNext, Script2 & script){

	Logger mout(__FILE__, __FUNCTION__); // warning, not initialized

	if (arg.empty()){
		mout.warn() << "empty arg" <<  mout.endl;
		return false;
	}

	if (arg.at(0) == '-'){
		const std::string & key = resolveHyphens(arg); // handle leading hyphens and aliases
		if (!key.empty()){
			//const drain::ReferenceMap & params = get(key).getParameters();
			if (get(key).getParameters().empty()){
				script.add(key);
				return false;
			}
			else {
				script.add(key, argNext);
				return true;
			}
		}
		else {
			mout.warn() << "undefined command: " << arg <<  mout.endl;
			throw std::runtime_error(arg + ": command not found");
		}

	}
	else if (arg.length() == 1){ // instructions '(', ')' (more in future)
		script.add(arg);
	}
	// Plain argument
	else if (!defaultCmd.empty()){
		script.add(defaultCmd, arg);
	}
	else {
		mout.error() << "defaultCmd undefined for plain argument (" << arg << ")" << mout.endl;
	}
	return false;
}

/**
 *
 */
void CommandBank::help(const std::string & key, std::ostream & ostr){

	Logger mout(__FILE__, __FUNCTION__); // warning, not initialized

	if (key.empty()){
		ostr << "For help on a commands, type:\n ";
		ostr << "  --help <command>\n";
		ostr << '\n';
		ostr << "For help on a command sctions, type:\n ";
		ostr << "  --help [" << sections.keysToStr('|') << "]\n";
		//help(0, ostr);
	}
	else {
		int filter = sections.getValue(key);
		if (filter > 0){
			help(filter, ostr);
			return;
		}
	}

	const std::string & cmdLong = resolveHyphens(key);

	if (!cmdLong.empty()){
		info(cmdLong, get(key), ostr);
		//  TODO: "see-also" commands as a list, which is checked.
	}
	else {
		mout.error() << "not found: " << key << mout.endl;
	}
}

void CommandBank::help(unsigned int sectionFilter, std::ostream & ostr){

	ostr << title << '\n' << std::endl;
	for (map_t::const_iterator it = this->begin(); it!=this->end(); ++it){
		if ((it->second->get().section & sectionFilter) > 0){
			info(it->first, it->second->get(), ostr);
		}
	}
	//std::flush(ostr);
}


/// Checked key and respective command
void CommandBank::info(const std::string & key, const value_t & cmd, std::ostream & ostr) const {
	ostr << "--" << key;
	char alias = getAlias(key);
	if (alias)
		ostr << ", -" << alias;
	const drain::ReferenceMap & params = cmd.getParameters();
	if (!params.empty())
		ostr << ' ' << '(' << params << ')';
	ostr << '\n';
	ostr << "  " << cmd.getDescription() << '\n';
	ostr << '\n';
}


/// Given alias or long key, possibly prefixed by hyphens, return the long key if a command exists.
/**
 *
 */
const std::string & CommandBank::resolveHyphens(const key_t & key) const {

	Logger mout(__FILE__, __FUNCTION__); // warning, not initialized

	static  const std::string empty;

	if (key.empty())
		return empty;

	const size_t hyphens = key.find_first_not_of('-');


	// Strip leading hyphens
	std::string cmd(key, hyphens);

	if (hyphens>2)
		mout.warn() << "more than 2 leading hyphens: " <<  key << mout.endl;
	else if (hyphens > 0){
		if ((hyphens==2) != (cmd.length()>1))
			mout.warn() << "mixed type argument (--x or -xx): " <<  key << mout.endl;
	}

	//mout.warn() << "searching for " << cmd << mout.endl;

	const std::string & cmdFinal = resolve(cmd);


	const_iterator it = this->find(cmdFinal);
	if (it != this->end()){
		//mout.warn() << "found " << it->first << mout.endl;
		return it->first;
	}
	else {
		return empty;
	}
}




} /* namespace drain */

// Rack
