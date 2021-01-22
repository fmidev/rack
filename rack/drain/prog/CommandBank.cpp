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

const Flags::value_t CommandBank::GENERAL; // = 1;
const Flags::value_t CommandBank::INPUT; // = 2;
const Flags::value_t CommandBank::OUTPUT; // = 4;
const Flags::value_t CommandBank::IO; // = INPUT | OUTPUT;
const Flags::value_t CommandBank::SPECIAL;



/// Global program command registry. Optional utility.
CommandBank & getCommandBank(){
	static CommandBank commandBank;
	return commandBank;
}

std::set<std::string> & CommandBank::trimWords(){
	static std::set<std::string> s;
	if (s.empty()){
		s.insert("Cmd");
		s.insert("Command");
	}
	return s;
}

void CommandBank::deriveCmdName(std::string & name, char prefix){

	const std::set<std::string> & trims = trimWords();

	Logger mout(__FILE__, __FUNCTION__);

	std::stringstream sstr;

	if (prefix)
		sstr << prefix;

	bool lowerCase = (prefix == 0);
	const size_t nameLen = name.size();
	size_t i = 0;
	while (i < nameLen) {

		mout.debug(10) << ' ' << i << '\t' << name.at(i) << mout.endl;
		size_t len = 0;

		for (std::set<std::string>::const_iterator it=trims.begin(); it!=trims.end(); ++it){
			// std::cerr << " ..." << *it;

			const std::string & word = *it;

			len = word.length(); // = chars to skip, potentially

			if (name.compare(i, len, word) == 0){ // Match

				// general jump (always)
				// break;

				if (i == 0) // and HEAD
					break;

				if (i == (nameLen-len)) // and TAIL
					break;

			}

			len = 0; // don't skip any chars, proceed to take the current one.
		}

		if (len > 0){
			// Skip word
			i += len;
		}
		else {
			// Copy the current character
			if (lowerCase){
				char c = name.at(i);
				if ((c>='A') && (c<='Z'))
					c = ('a' + (c-'A'));
				sstr << c;
				lowerCase = false;
			}
			else {
				sstr << name.at(i);
			}
			++i;
		}
		//mout.debug()
		//std::cerr << '\n';
	}

	name = sstr.str();
};


/// Appends program with commands fo the script
void CommandBank::append(const Script & script, Program & prog) const {
	for (Script::const_iterator it = script.begin(); it!=script.end(); ++it) {
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

	Logger mout(ctx.log, __FILE__, __FUNCTION__);

	command_t & cmd = get(cmdKey);
	//cmd.setKey(cmdKey);

	if (cmd.contextIsSet())
		mout.note() << "replacing original context " << mout.endl;

	cmd.setExternalContext(ctx);
	cmd.run(params);
}



void CommandBank::run(Script & script, ClonerBase<Context> & contextSrc){

	Logger mout(contextSrc.get().log, __FUNCTION__, __FILE__); // warni

	bool PARALLEL_MODE = false;

	//	std::vector<Program> threads;
	ProgramVector threads;

	for (Script::const_iterator it = script.begin(); it != script.end(); ++it) {

		// mout.note() << it->first << mout.endl;

		if (it->first == "["){
			PARALLEL_MODE = true;
			continue;
		}

		//const bool ROUTINE_DEFINED = !routine.empty();

		if (!PARALLEL_MODE){

			// Check here if routine exists (the current cmd may set the routine, and the it should not be run).
			value_t & cmd = get(it->first); // or clone, to keep defaults? => consider FLAG save-params
			//cmd.setKey(it->first);

			mout.note() << it->first << '(' << cmd << "), routine=" << cmd.execRoutine << mout.endl;

			if (cmd.execRoutine && !routine.empty()){
				// First command (cmd) could be run above, but here in prog for debugging (and clarity?)
				Program prog(contextSrc.get());
				cmd.setExternalContext(contextSrc.get());
				prog.add(cmd).setParameters(it->second);
				append(routine, prog); // append() has access to command registry (Prog does not)
				//mout.note() << "Exec routine: " << prog.getContext<Context>().id << mout.endl;
				prog.run();
			}
			else {
				cmd.setParameters(it->second);
				// mout.warn() << "Executing: " << it->first << mout.endl;
				cmd.exec();
				// mout.warn() << "Executed:  " << it->first << mout.endl;
			}
			// TODO: when to explicitly clear routine?
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
				mout.note() << "Threads completed." << mout.endl;
				routine.clear(); // consider

			}
			else { // continue collecting

				// Append new thread.
				Context & ctx = contextSrc.clone();
				ctx.id = 100 + threads.size();
				Program & prog = threads.add(ctx);

				mout.note() << "Preparing thread " << ctx.id << ':' << it->first << '(' << it->second << ')' <<  mout.endl;

				value_t & cmd = clone(it->first);
				// cmd.setKey(it->first);
				cmd.setParameters(it->second);
				prog.add(cmd);

				if (!routine.empty()){ // Technically unneeded, but for mout...
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
void CommandBank::scriptify(int argc, const char **argv, Script & script){

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

void CommandBank::scriptify(const std::string & line, Script & script){

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

bool CommandBank::scriptify(const std::string & arg, const std::string & argNext, Script & script){

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
		//help(0xffffffff, ostr);
		help(CommandBank::GENERAL, ostr);
		/*
		ostr << "For help on a commands, type:\n ";
		ostr << "  --help <command>\n";
		ostr << '\n';
		ostr << "For help on a command sctions, type:\n ";
		ostr << "  --help [" << sections.keysToStr('|') << "]\n";
		*/
		//help(0, ostr);
		return;
	}
	else {
		// int filter = sections.getValue(key);
		Flagger::value_t filter = 0;
		Flagger flagger(filter, sections, ',');
		try {
			flagger.set(key);
			// ostr << "Flagger: " << flagger << '(' << sections << ')' << '*' << filter << mout.endl;
			if (filter > 0){
				help(filter, ostr);
				return;
			}
		}
		catch (const std::exception & e) {
		}
	}

	const std::string & cmdLong = resolveHyphens(key);

	if (!cmdLong.empty()){
		//mout.warn() << "cmdLong: " << key << " -> " << cmdLong << mout.endl;
		try {
			info(cmdLong, get(cmdLong), ostr, true);
		}
		catch (const std::exception &e) {
			ostr << "error: " << e.what() << '\n';
		}
		//  TODO: "see-also" commands as a list, which is checked.
	}
	else {
		mout.error() << "not found: " << key << mout.endl;
	}
}

void CommandBank::help(Flagger::value_t sectionFilter, std::ostream & ostr){

	ostr << title << '\n' << std::endl;

	const bool TEST = false; //true;

	if (sectionFilter > 0){
		Flagger::value_t filter = 0;
		Flagger flagger(filter, sections, ',');
		flagger.set(sectionFilter);
		ostr << "Section: " << flagger << '\n' << '\n';
		for (map_t::const_iterator it = this->begin(); it!=this->end(); ++it){
			if ((it->second->get().section & sectionFilter) > 0){
				try {
					if (!TEST){
						info(it->first, it->second->get(), ostr, false);
					}
					else {
						ostr << it->first << '\n';
						std::stringstream sstr, sstr2;
						Command & cmdOrig = it->second->get();
						Command & cmdCopy = it->second->clone();
						info(it->first, cmdOrig, sstr, false);
						info(it->first, cmdCopy, sstr2, false);
						if (sstr.str() != sstr2.str()){
							ostr << sstr.str()  << '\n';
							ostr << sstr2.str() << '\n';
							ostr << "---- ERROR " << '\n';
						}
						sstr.str("  ");
						sstr2.str("  ");
						sstr <<  cmdOrig.getParameters();
						sstr2 << cmdCopy.getParameters();
						if (sstr.str() != sstr2.str()){
							ostr << sstr.str()  << '\n';
							ostr << sstr2.str() << '\n';
							ostr << "---- ERROR " << '\n';
						}
						//cmdCopy.setParameters(cmdOrig.getParameters());
					}

				}
				catch (const std::exception &e) {
					ostr << "error: " << e.what() << '\n';
				}
			}
		}
	}
	// else { ?
	ostr << '\n';
	ostr << "For help on a commands, type:\n";
	ostr << "  --help <command>\n";
	ostr << '\n';

	ostr << "For help on command sections, type:\n";
	ostr << "  --help [";
	sections.keysToStream(ostr, '|');
	ostr << "]\n";

	//ostr << "  --help {" << sections << "}\n";

	//std::flush(ostr);
}


/// Checked key and respective command
void CommandBank::info(const std::string & key, const value_t & cmd, std::ostream & ostr, bool detailed) const {

	Logger mout(__FILE__, __FUNCTION__);

	/* Dynamic
	try {
		cmd.setKey(key);
	}
	catch (const std::exception &e) {
		ostr << "error: " << e.what() << '\n';
		return;
	}
	*/

	ostr << "--" << key;
	char alias = getAlias(key);
	if (alias)
		ostr << ", -" << alias;

	const ReferenceMap & params = cmd.getParameters();
	const std::map<std::string,std::string> & units = params.getUnitMap();
	const std::list<std::string> & keys = params.getKeyList();  // To get keys in specified order.

	ostr << ' ' << ' ';
	char separator = 0;
	for (std::list<std::string>::const_iterator kit = keys.begin(); kit != keys.end(); ++kit){

		if (separator)
			ostr << separator;
		else
			separator = params.separator; //',';

		if (kit->empty()){
			ostr << '<' << params.getKeys() << '>';
			if (params.size() != 1){
				mout.warn() << "the first key empty, but not unique" << mout.endl;
				//mout.warn() << "the first key empty, but not unique" << mout.endl;
			}
		}
		else {
			ostr << '<' << *kit << '>';
		}
	}
	ostr << '\n';

	ostr << ' ' << ' ' << cmd.getDescription() << '\n';
	//ostr << cmd.getDescription() << '\n';

	/// Iterate variable keys
	if (detailed){

		for (std::list<std::string>::const_iterator kit = keys.begin(); kit != keys.end(); ++kit){
			const std::string & key = *kit;

			// special:
			if (keys.size()==1){
				if (key.find(',') != std::string::npos){
					// Latent-multikey case
					ostr << '\t' << params.begin()->second << '\n';
					return;
				}
			}

			ostr << '\t' << key;
			// ostr << ' ' << ' ' << key;
			// ostr << ' ' << ' ' << ' ' << ' ' << key;
			ReferenceMap::const_iterator rit = params.find(key); // er.. ok..
			if (rit == params.end())
				throw std::runtime_error(key + ": key listed, but no command found");
			else {
				ostr << '=' << rit->second << ' ';
				//ostr << 'Q';
				std::map<std::string,std::string>::const_iterator uit = units.find(key);
				if (uit != units.end())
					if (!uit->second.empty())
						ostr << '[' << uit->second << ']';
			}
			//ostr << 'X';
			ostr << '\n';
		}
	}
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
