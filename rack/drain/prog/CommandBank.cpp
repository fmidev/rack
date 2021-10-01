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

#include <iostream>
#include <sstream>
//#include <istream>


#include "drain/util/Log.h"
#include "drain/util/Input.h"
#include "drain/util/Static.h"

#include "CommandBank.h"
#include "Program.h"

namespace drain {


//const Flagger::value_t CommandBank::SCRIPT_DEFINED =  StatusFlags::add("SCRIPT_DEFINED");

/// Global program command registry. Optional utility.
CommandBank & getCommandBank(){
	return Static::get<CommandBank>();
	//static CommandBank commandBank;
	//return commandBank;
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

	// Check initial of the first valid char (after pruning, so possibly i>0)
	bool checkInitialCase = true;

	const size_t nameLen = name.size();
	size_t i = 0;
	while (i < nameLen) {

		//mout.debug(10) << ' ' << i << '\t' << name.at(i) << mout.endl;
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
			char c = name.at(i);
			if (checkInitialCase){ // initial char, ensure ...
				if (prefix != 0){ // ...Uppercase
					if ((c>='a') && (c<='z'))
						c = ('A' + (c-'a'));
				}
				else { // ...lowercase
					if ((c>='A') && (c<='Z'))
						c = ('a' + (c-'A'));
				}
				checkInitialCase = false;
			}
			sstr << c;
			++i;
		}
		//mout.debug()
		//std::cerr << '\n';
	}

	// mout.warn() << ' ' << prefix << ((int)prefix) << "\t" << name << "\t => ";
	name = sstr.str();
	// mout << name << mout.endl;

};


/// Appends program with commands fo the script
void CommandBank::append(const Script & script, Context & ctx, Program & prog) const {


	for (Script::const_iterator it = script.begin(); it!=script.end(); ++it) {

		if (it->first.size()==1){
			static BasicCommand dummy("Marker", "[marker]");
			prog.add(it->first, dummy);
		}
		else {
			command_t & cmd = clone(it->first);
			cmd.setExternalContext(ctx);
			cmd.setParameters(it->second);
			prog.add(it->first, cmd);
		}
	}

}


/// Converts linux command line to pairs of strings:  (<cmd>, [params])
/**
 *  Starts from argument 1 instead of 0.
 */
void CommandBank::scriptify(int argc, const char **argv, Script & script) const {

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

void CommandBank::scriptify(const std::string & line, Script & script) const{

	Logger mout(__FILE__, __FUNCTION__); //

	typedef std::list<std::string> list_t;
	list_t l;
	//drain::StringTools::split(line, l, " ");
	std::stringstream sstr(line);
	while (sstr) {
		l.push_back("");
		sstr >> l.back();
	}

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

bool CommandBank::scriptify(const std::string & arg, const std::string & argNext, Script & script) const {

	Logger mout(__FILE__, __FUNCTION__); // warning, not initialized

	if (arg.empty()){
		mout.debug() << "empty arg" <<  mout.endl;
		return false;
	}

	if (arg.at(0) == '-'){
		const std::string & key = resolveFull(arg); // handle leading hyphens and aliases
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
		else if (!notFoundHandlerCmdKey.empty()){
			const Command & cmd = get(notFoundHandlerCmdKey);
			if (cmd.hasArguments()){
				size_t i = arg.find_first_not_of('-');
				if (i != std::string::npos){
					script.add(notFoundHandlerCmdKey, arg.substr(i));
				}
				else {
					mout.warn() << "plain dash(es): " << arg <<  mout.endl;
				}
			}
			else {
				script.add(notFoundHandlerCmdKey);
			}
		}
		else {
			mout.warn() << "undefined command: " << arg << " (and no 'notFoundHandlerCmd' set in main program)" <<  mout.endl;
			throw std::runtime_error(arg + ": command not found");
		}


	}
	else if (arg.length() == 1){ // instructions '(', ')' (more in future)
		script.add(arg);
	}
	// Plain argument
	else if (!defaultCmdKey.empty()){
		script.add(defaultCmdKey, arg);
	}
	else {
		mout.error() << "defaultCmd undefined for plain argument (" << arg << ")" << mout.endl;
	}
	return false;
}


void CommandBank::readFile(const std::string & filename, Script & script) const {

	Logger mout(__FILE__, __FUNCTION__);

	std::string line;
	drain::Input ifstr(filename);

	// mout.note() << "open list: " << filename << mout.endl;
	// ifstr.open(params.c_str());

	while ( std::getline((std::ifstream &)ifstr, line) ){
		if (!line.empty()){
			mout.debug2() << line << mout.endl;
			if (line.at(0) != '#')
				scriptify(line, script);
		}
	}

}



// Future extension.
void CommandBank::remove(Program & prog) const {
	std::cout << "clearing prog: " << '\n';
	//prog.toStream();
	std::cout << prog << '\n';
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



void CommandBank::run(Program & prog, ClonerBase<Context> & contextCloner){

	// Which log? Perhaps prog first cmd ctx log?
	//const drain::Flagger::value_t execScript = drain::Static::get<drain::TriggerSection>().index;

	bool CREATE_THREADS = false;

	//	std::vector<Program> threads;
	ProgramVector threads;

	// Iterate commandsconst auto& kv : myMap
	/*
	for (auto & entry: prog) {
		const key_t & key = entry.first;
		value_t & cmd     = entry.second;
	 */
	 for (Program::iterator it = prog.begin(); it != prog.end(); ++it) {
		// mout.note() << it->first << mout.endl;
		const key_t & key =  it->first;
		value_t & cmd     = *it->second;
		// const std::string & cmdName = cmd.getName();
		Context & ctx = cmd.getContext<>(); //.log;
		Log & log = ctx.log;
		const bool TRIGGER = (cmd.section & this->scriptTriggerFlag); // AND

		Logger mout(log, __FUNCTION__, __FILE__); // warni

		//if (TRIGGER)
		mout.debug() << "TRIGGER? " << cmd.section << '+' << this->scriptTriggerFlag << '=' << sprinter(TRIGGER) << mout.endl;

		if (cmd.getName().empty()){
			mout.warn() << "Command name empty for key='" << key << "', " << cmd << mout.endl;
		}
		else if (cmd.getName() == scriptCmd){ // "script"
			ReferenceMap::const_iterator pit = cmd.getParameters().begin();
			mout.warn() << "'" <<  scriptCmd << "' -> storing routine: '" << pit->second << "'" << mout.endl;
			scriptify(pit->second.toStr(), prog.routine);
			//ctx.statusFlags.set(SCRIPT_DEFINED);
			ctx.setStatus("script", true);
			//ctx.setStatus(key, value)
			continue;
		}
		// Explicit launch NOT needed, but a passive trigger cmd
		/*
		else if (cmd.getName() == execScriptCmd){
			// Explicit launch
			mout.experimental() << "executing script" << mout;
			Program progSub;
			append(prog.routine, cmd.getContext<Context>(), progSub); // append() has access to command registry (Prog does not)
			run(progSub, contextCloner);
		}
		*/
		else if (cmd.getName() == execFileCmd){ // "execFile"
			ReferenceMap::const_iterator pit = cmd.getParameters().begin();
			// TODO catch
			mout.info() << "embedding (inserting commands on-the-fly) from '" << pit->second << "'" << mout;
			Script script;
			readFile(pit->second, script);
			mout.debug() << script << mout.endl;
			Program::iterator itNext = it;
			++itNext;
			for (Script::value_type & subCmd: script){
				mout.debug() << sprinter(subCmd) << mout.endl;
				command_t & cmd = clone(subCmd.first);
				cmd.setExternalContext(ctx);
				cmd.setParameters(subCmd.second);
				//prog.add(it->first, cmd);
				prog.insert(itNext, Program::value_type(subCmd.first, &cmd));
				//run(subCmd.first, subCmd.second, ctx);
			}
			// Debug: print resulting program that contains embedded commands
			// mout.warn() << sprinter(prog) << mout;
			continue;
		}
		else if (key == "["){
			mout.note() << "Switching to parallel mode." << mout.endl;
			mout.warn() << "Step 1: compose threads." << mout.endl;
			mout.warn() << "Hey, " << log.getVerbosity() << mout.endl;
			CREATE_THREADS = true;
			continue;
		}
		else if (key == "]"){ 	// All the triggering commands collected, run them

			if (!CREATE_THREADS){
				mout.warn() << "Leading brace '[' missing?" << mout.endl;
			}

			CREATE_THREADS = false;

			if (threads.empty()){
				mout.warn() << "No threads?" << mout.endl;
				mout.note() << "No thread-triggering commands inside braces [ <cmds> ...].  See --help trigger " << mout.endl;
				//mout.warn() << "No threads defined? Syntax: --script '<cmd> <cmd2>...' [ <cmds> ...] " << mout.endl;
				continue;
			}
			else {

				mout.special() << "Start threads" << log.getVerbosity() << mout.endl;

				#pragma omp parallel for
				for (size_t i = 0; i < threads.size(); ++i) {
					// Keep this minimal. (No variable writes here, unless local.)
					run(threads[i], contextCloner);
				}

				mout.special() << "Threads completed." << mout.endl;
				prog.routine.clear(); // reconsider
			}

		}
		else if (CREATE_THREADS && TRIGGER) {

			//mout.warn() << "Prepare a new thread: " << key << '(' << cmd << ')' <<  mout.endl;

			Context & ctxCloned = contextCloner.getCloned();
			ctxCloned.log.setVerbosity(log.getVerbosity());

			Program & thread = threads.add();

			// Add at least the current command
			value_t & cmdCloned = clone(key);
			cmdCloned.setExternalContext(ctxCloned);
			cmdCloned.setParameters(cmd.getParameters());
			thread.add(it->first, cmdCloned);

			mout.ok() << "Thread #" << threads.size() << "-" << ctxCloned.id << ':' << key << '(' << cmd << ')' <<  mout.endl;

			// And also append a script, if defined.
			if (!prog.routine.empty()){ // Technically unneeded, but for mout...
				mout.debug() << "routine exists, appending it to thread " <<  mout.endl;
				append(prog.routine, ctxCloned, thread); // append
			}

		}
		else if (TRIGGER && !prog.routine.empty()){

			// Check here if routine exists (the current cmd may set the routine, and the it should not be run).
			// First command (cmd) could be run above, but here in prog for debugging (and clarity?)
			mout.info() << "script triggering command: '" << cmd << mout.endl;

			Program progSub;
			progSub.add(key, cmd).section = 0;  // execRoutine = false; // COPIES! .setParameters(it->second);
			append(prog.routine, cmd.getContext<Context>(), progSub); // append() has access to command registry (Prog does not)
			//mout.note() << "Exec routine: " << prog.getContext<Context>().id << mout.endl;
			mout.info() << "running routine: '" << key << ' ' << cmd.getParameters().getValues() << " --" << prog.routine.begin()->first<< " ...'" << mout.endl;
			//mout.note() << "running routine: "  << prog.routine.begin()->first << "..." << mout.endl;
			run(progSub, contextCloner);
			// ? prog.routine.clear();
		}
		else {
			mout.debug() << "Executing: " << key << " = " << cmd << " "  << mout.endl;
			//try {
			cmd.update(); //  --select etc
			cmd.exec();
			ctx.setStatus(key, cmd.getParameters().getValues());
			/*
			}
			catch (const std::exception & e){
				mout.fail() << e.what() << mout.endl;
				mout.warn() << "stopping" << mout.endl;
				return;
			}
			*/
		}
		// TODO: when to explicitly clear routine?

	}

}



/**
 *
 */
void CommandBank::help(const std::string & key, std::ostream & ostr){

	Logger mout(__FILE__, __FUNCTION__); // warning, not initialized

	if (key.empty()){
		//help(0xffffffff, ostr);
		//help(CommandBank::GENERAL, ostr);
		help(0, ostr);
		return;
	}
	else if (key == "all"){
		help(FlagResolver::ALL, ostr);
		//help(0xffffffff, ostr);
	}
	else {

		// Try to find the command directly
		const std::string & fullKey = resolveFull(key);

		if (!fullKey.empty()){
			//mout.warn() << "fullKey: " << key << " -> " << fullKey << mout.endl;
			try {
				info(fullKey, get(fullKey), ostr, true);
			}
			catch (const std::exception &e) {
				ostr << "error: " << e.what() << '\n';
			}
			//  TODO: "see-also" commands as a list, which is checked.
		}
		else {

			// Or is it a section, or several?
			Flagger::value_t filter = FlagResolver::getValue(sections, key); // Accepts also numeric strings.

			//mout.deprecating() << "Flagger: " << sections << " -> " << filter << mout.endl;
			if (filter > 0){
				help(filter, ostr);
				return;
			}

			mout.error() << "not found: " << key << mout.endl;
		}



	}


}

void CommandBank::help(Flagger::value_t sectionFilter, std::ostream & ostr){

	ostr << title << '\n' << std::endl;

	const bool TEST = false; //true;

	if (sectionFilter > 0){
		//Flagger::value_t filter = FlagResolver::getValue(sections, key);
		//Flagger::value_t filter = 0;
		//Flagger flagger(filter, sections, ',');
		//flagger.set(sectionFilter);
		ostr << "Section: " << FlagResolver::getKeys(sections, sectionFilter) << '\n' << '\n';
		for (map_t::const_iterator it = this->begin(); it!=this->end(); ++it){
			if ((it->second->getSource().section & sectionFilter) > 0){
				try {
					if (!TEST){
						info(it->first, it->second->getSource(), ostr, false);
					}
					else {
						ostr << it->first << '\n';
						std::stringstream sstr, sstr2;
						Command & cmdOrig = it->second->getSource();
						Command & cmdCopy = it->second->getCloned();
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
	if (sections.size() > 2)
		ostr << '|' << "all";
	ostr << "]\n";

	/* ostr << "Bonus:\n";
	ostr << drain::sprinter(sections);
	ostr << "Done.\n";
	*/


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

	if (detailed)
		ostr << "  (Section: " << FlagResolver::getKeys(sections,cmd.section, ',') << ')';

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
const std::string & CommandBank::resolveFull(const key_t & key) const {

	Logger mout(__FILE__, __FUNCTION__); // warning, not initialized

	static
	const std::string empty;

	if (key.empty())
		return empty;

	const size_t hyphens = key.find_first_not_of('-');

	// Substring starting from first letter (strip leading hyphens)
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
