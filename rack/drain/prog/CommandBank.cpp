/*

MIT License

Copyright (c) 2023 FMI Open Development / Markus Peura, first.last@fmi.fi

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
 

#include <iostream>
#include <sstream>

#include "drain/util/Log.h"
#include "drain/util/Input.h"
#include "drain/util/JSON.h"
#include "drain/util/Output.h"
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
	static std::set<std::string> s = {"Cmd", "Command"};
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

		//for (std::set<std::string>::const_iterator it=trims.begin(); it!=trims.end(); ++it){
		for (const std::string & word : trims){
			// std::cerr << " ..." << *it;

			// const std::string & word = *it;

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


void CommandBank::append(const Script & script, Program & prog) const {
	append(script, prog.getContext<>(), prog);
}


/// Appends program with commands of the script
void CommandBank::append(const Script & script, Context & ctx, Program & prog) const {

	Logger mout(__FILE__, __FUNCTION__);

	for (const auto & entry: script) {
		if (entry.first.size()==1){
			// Non-prefixed single-char commands have a special, dedicated handling.
			static BasicCommand dummy("Marker", "[marker]");
			prog.add(entry.first, dummy);
		}
		else if (get(entry.first).getName() == execFileCmd){
			mout.special("pre-processing execFileCmd: ", entry.first, '/', get(entry.first).getName());
			readFile(entry.second, prog);
			mout.experimental(prog);
		}
		else { // default
			command_t & cmd = clone(entry.first);
			cmd.setExternalContext(ctx);
			cmd.setParameters(entry.second);
			prog.add(entry.first, cmd);
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

	Logger mout(__FUNCTION__, __FILE__); // warning, not initialized

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
				drain::StringTools::trim(argNext, "'\"");
				//script.add(key, argNext);
				script.add(key, drain::StringTools::trim(argNext, "'\""));
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
	else if ((arg.at(0) == '[') || (arg.at(arg.size()-1) == ']')){
		mout.warn("check argument: ", arg);
		mout.error("parallel processing markers '[' and ']' must separated from other args");
		return false;
	}
	else if (!defaultCmdKey.empty()){
		// Plain argument
		script.add(defaultCmdKey, arg);
	}
	else {
		mout.error() << "defaultCmd undefined for plain argument (" << arg << ")" << mout.endl;
	}
	return false;
}


void CommandBank::readFile(const std::string & filename, Program & prog) const {

	Logger mout(__FILE__, __FUNCTION__);

	FilePath path(filename);

	mout.info("inserting commands from path: ", path);

	drain::Context & ctx = prog.getContext<>();

	if (path.extension != "json"){
		Script subScript;
		readFileTXT(filename, subScript);
		append(subScript, ctx, prog);
	}
	else {

		drain::JSONtree2 cmdTree;
		drain::Input input(filename);

		drain::JSON::readTree(cmdTree, input);
		mout.experimental("parsed JSON structure:\n", drain::sprinter(cmdTree));

		for (const auto & node: cmdTree){
			mout.debug("inserting: ", node.first); //, node.second.data);
			command_t & cmd = clone(node.first);
			cmd.setExternalContext(ctx);
			if (cmd.hasArguments()){
				if (node.second.hasChildren()){
					for (const auto & subNode: node.second){
						cmd.setParameter(subNode.first, subNode.second.data);
					}
				}
				else {
					cmd.setParameters((const std::string &)node.second.data);
				}
			}
			else {
				if (node.second.hasChildren()){
					mout.warn("cmd '", node.first, "' takes no args, but was provided named arg(s): ");
					for (const auto & subNode: node.second){
						mout.warn('\t', subNode.first, '=', subNode.second.data);
						//cmd.setParameter();
					}
				}
				if (!node.second.data.empty()){
					mout.warn("cmd '", node.first, "' takes no args, but was provided one: ", node.second.data);
				}
			}
			prog.add(node.first, cmd);
		}

	}


}


void CommandBank::readFileTXT(const std::string & filename, Script & script) const {

	Logger mout(__FILE__, __FUNCTION__);

	drain::Input input(filename);

	mout.note("opening command list: ", filename);

	// mout.note() << "open list: " << filename << mout.endl;
	if (drain::JSON::fileInfo.checkPath(filename)){
		mout.error("Reading: ", filename, ": JSON not supported for ", __FUNCTION__, ",use readFile()");
	}
	else {
		// TEXT file
		std::string line;
		while ( std::getline((std::ifstream &)input, line) ){
			if (!line.empty()){
				mout.debug2() << line << mout.endl;
				if (line.at(0) != '#')
					scriptify(line, script);
			}
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

// TEST
/*
Script & getRoutine(Program & prog, Script task){
	//return prog.routine;
	if (threads.empty())
		threads.add();
	return threads[0];
}
*/

// MAIN
void CommandBank::run(Program & prog, ClonerBase<Context> & contextCloner){

	// Which log? Perhaps prog first cmd ctx log?
	//const drain::Flagger::value_t execScript = drain::Static::get<drain::TriggerSection>().index;

	bool PARALLEL_ENABLED = false;

	bool INLINE_SCRIPT = false;


	ProgramVector threads;

	Script routine;

	Context & ctx = prog.getContext<>(); // 2023/01
	//Log & log = ctx.log; // 2022/10
	Logger mout(ctx.log, __FUNCTION__, __FILE__); // Could be thread prefix?

	mout.debug(ctx.getName());

	//ctx.SCRIPT_DEFINED = false;

	// Iterate commands.
	// foreach-auto not possible, because --execScript commands may be inserted during iteration.
	for (Program::iterator it = prog.begin(); it != prog.end(); ++it) {

		if (Logger::TIMING && !mout.timing){
			mout.startTiming("Program"); // . this->title);
		}

		const key_t & key =  it->first;
		value_t & cmd     = *it->second;

		// Context & ctx = cmd.getContext<>(); //.log;
		//Log & log = ctx.log;
		//log.setVerbosity(baseLog.getVerbosity());

		const bool TRIGGER_CMD = (cmd.section & this->scriptTriggerFlag);
		// ctx.setStatus("script", !routine.empty());

		mout.debug() << '"' << key << '"' << " ctx=" << ctx.getName() << " cmd.section=" << cmd.section << '/' << this->scriptTriggerFlag;

		if (TRIGGER_CMD)
			mout << " TRIGGER_CMD,";
		if (PARALLEL_ENABLED)
			mout << " THREADS_ENABLED,";
		if (INLINE_SCRIPT)
			mout << " INLINE_SCRIPT";
		mout << mout.endl;

		if (cmd.getName().empty()){
			mout.warn() << "Command name empty for key='" << key << "', " << cmd << mout.endl;
		}
		else if (cmd.getName() == scriptCmd){ // "--script" by default
			ReferenceMap::const_iterator pit = cmd.getParameters().begin();
			mout.debug("'", scriptCmd, "' -> storing routine: '", pit->second, "'");
			if (INLINE_SCRIPT){
				mout.warn("Script should be added prior to enabling parallel (thread triggering) mode. Problems ahead...");
			}
			scriptify(pit->second.toStr(), routine);
			ctx.SCRIPT_DEFINED = true; // For polar read! (To avoid appending sweeps)
			// ctx.setStatus("script", true);
			//ctx.statusFlags.set(SCRIPT_DEFINED);
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

			// TODO: redesign to resemble --script <cmd...>
			ReferenceMap::const_iterator pit = cmd.getParameters().begin();
			// TODO catch
			mout.attention("embedding (inserting commands on-the-fly) from '", pit->second, "' (should have been preprocessed?)");
			mout.error("mislocated: inserting commands on-the-fly, ", pit->second);
			/*
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
			*/
			// Debug: print resulting program that contains embedded commands
			// mout.warn() << sprinter(prog) << mout;
		}
		else if (key == "["){

			mout.special("Enabling parallel computation.");

			if (PARALLEL_ENABLED){
				mout.error("Parallel section already started with '[' ?");
				continue;
			}

			PARALLEL_ENABLED = true;

			if (routine.empty())
				INLINE_SCRIPT = true;

			ctx.SCRIPT_DEFINED = true; // IMPORTANT. To prevent appending sequental input sweeps
			//ctx.setStatus("script", true); // IMPORTANT. To prevent appending sequental input sweeps
		}
		else if (TRIGGER_CMD && (!routine.empty()) && !PARALLEL_ENABLED){ // Here, actually !THREADS_ENABLED implies -> TRIGGER_SCRIPT_NOW
			mout.debug("Running SCRIPT in main thread: ", ctx.getName());
			Program prog(cmd.getContext<Context>());
			//Program prog;
			prog.add(key, cmd).section = 0; // To not re-trigger?
			//append(routine, cmd.getContext<Context>(), prog);
			append(routine, prog);
			//Logger mout2(ctx.log, __FUNCTION__, __FILE__);
			//mout2.startTiming("SCRIPT");
			run(prog, contextCloner); // Run in this context
		}
		else if (PARALLEL_ENABLED && (TRIGGER_CMD || (key == "/"))){ // || (key == "]")) { // Now threads are enabled


			if ((key == "/") && !INLINE_SCRIPT){
				mout.error("Parallel section not enabled with '[' but separator '/' encountered?");
				continue;
			}

			// NOTE: cloned also for --execScript ?

			Context & ctxCloned = contextCloner.getCloned();
			//mout.attention("cloned: ", ctxCloned.getId());
			ctxCloned.log.setVerbosity(mout.getVerbosity());
			Program & thread = threads.add(ctxCloned);
			// mout.attention("cloned: ", ctxCloned.getId(), " <-> ", thread.getContext<>().getId());

			if (TRIGGER_CMD){ // ... in a thread.
				// Include (prepend) the triggering command (typically --inputFile (implicit) )
				value_t & cmdCloned = clone(key);
				cmdCloned.section = 0; // Avoid re-triggering...
				cmdCloned.setParameters(cmd.getParameters());
				thread.add(key, cmdCloned); // XXX
			}

			append(routine, thread);

			//if (!TRIGGER_CMD){
			if (INLINE_SCRIPT){// flush
				routine.clear();
			}
		}
		else if (key == "]"){ // Run the threads.

			if (INLINE_SCRIPT){
				Context & ctxCloned = contextCloner.getCloned();
				//mout.attention("cloned: ", ctxCloned.getId());
				ctxCloned.log.setVerbosity(mout.getVerbosity());
				Program & thread = threads.add(ctxCloned);
				append(routine, thread);
			}

			if (threads.empty()){
				mout.note("Parallel section ended by ']' but no threads defined?");
				continue;
			}

			mout.special("Running ", threads.size(), " thread(s)");
			//mout.special("Running log=", baseLog.id);
			if (!logFileSyntax.empty()){
				mout.note("Log file (syntax): ", logFileSyntax);
			}

			for (size_t j = 0; j < threads.size(); ++j) {

				// Display commands briefly
				const Program & p = threads[j];
				Context & cmdCtx = p.getContext<>();
				mout.debug("thread #", j, " CTX=", cmdCtx.getName(), " ID=", cmdCtx.getId());

				if (!logFileSyntax.empty()){
					// mout.attention(cmdCtx.getStatusMap());
					std::string filename = logFileSyntax.toStr(cmdCtx.getStatusMap());
					mout.info("Log file: #", j, filename);
					cmdCtx.log.setOstr(filename);
				}

			}

			if (mout.isDebug(2))
				threads.debug();

			// mout.attention("Start threads");
			#pragma omp parallel for
			for (size_t i = 0; i < threads.size(); ++i) {
				// Keep this minimal! (No variable writes here, unless local.)
				run(threads[i], contextCloner);
			}

			mout.special("...Threads completed.");
			threads.clear();

			PARALLEL_ENABLED = false;
			INLINE_SCRIPT = false; //

			//}

		}
		else if (INLINE_SCRIPT){ // In this mode, catch all the commands to current routine.
			mout.debug("Adding: ", key, " = ", cmd, " ");
			routine.add(key, cmd.getParameters().getValues()); // kludge... converting ReferenceMap back to string...
		}
		else if (key.size() == 1){
			mout.warn("Use these characters for parallel computing: [ / ]");
			mout.error("Unrecognized single-character instruction: ", key);
		}
		else {

			Logger mout2(ctx.log, __FUNCTION__, __FILE__);
			mout2.startTiming(cmd.getName(), " <tt>", cmd.getParameters().getValues(), "</tt>");

			// This is the default action!
			mout.debug() << "Executing: " << key << " = " << cmd << " "  << mout.endl;

			cmd.update(); //  --select etc
			cmd.exec();
			ctx.setStatus(key, cmd.getParameters().getValues());

			ctx.setStatus("cmd", cmd.getName()); //
			ctx.setStatus("cmdKey", key); // not getName()
			ctx.setStatus("cmdParams", cmd.getParameters().getValues());

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
		ostr << title << '\n' << std::endl;
		help(ostr);
		return;
	}
	else if (key == "all"){
		help(FlagResolver::ALL, ostr);
	}
	else if (key == "hidden"){
		help(0, ostr);
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

void CommandBank::help(std::ostream & ostr){

	ostr << "For help on a commands, type:\n";
	ostr << "  --help <command>\n";
	ostr << '\n';

	//const static drain::SprinterLayout layout("M", "ABC", "S", "<>");
	ostr << "For help on command sections, type:\n";
	ostr << "  --help [";
	//ostr << drain::sprinter((const FlagResolver::dict_t::container_t &)sections, "|");
	ostr << drain::sprinter(sections.getKeys(), "|");
	if (sections.size() > 2)
		ostr << '|' << "all";
	ostr << "]\n";

}

void CommandBank::help(Flagger::value_t sectionFilter, std::ostream & ostr){

	ostr << title << '\n' << std::endl;

	const bool TEST = false; //true;

	// if (sectionFilter > 0){
	// Flagger::value_t filter = FlagResolver::getValue(sections, key);
	// Flagger::value_t filter = 0;
	// Flagger flagger(filter, sections, ',');
	// flagger.set(sectionFilter);
	ostr << "Section: " << FlagResolver::getKeys(sections, sectionFilter) << '\n' << '\n';
	//for (map_t::const_iterator it = this->begin(); it!=this->end(); ++it){
	for (const auto & entry: *this){ // map_t::value_t
		Flagger::value_t sec = entry.second->getSource().section;
		if ((sec == sectionFilter) || (sec & sectionFilter) > 0){ // 1st test for HIDDEN
			try {
				if (!TEST){
					info(entry.first, entry.second->getSource(), ostr, false);
				}
				else {
					ostr << entry.first << '\n';
					std::stringstream sstr, sstr2;
					Command & cmdOrig = entry.second->getSource();
					Command & cmdCopy = entry.second->getCloned();
					info(entry.first, cmdOrig, sstr, false);
					info(entry.first, cmdCopy, sstr2, false);
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
	// }

	ostr << '\n';

	help(ostr);
	// else { ?

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
	//for (std::list<std::string>::const_iterator kit = keys.begin(); kit != keys.end(); ++kit){
	for (const std::string & key: keys){

		if (separator)
			ostr << separator;
		else
			separator = params.separator; //',';

		if (key.empty()){
			ostr << '<' << params.getKeys() << '>';
			if (params.size() != 1){
				mout.warn() << "the first key empty, but not unique" << mout.endl;
				//mout.warn() << "the first key empty, but not unique" << mout.endl;
			}
		}
		else {
			ostr << '<' << key << '>'; // Like a pseudo parameter, '<value>'
		}
	}

	if (detailed)
		ostr << "  (Section: " << FlagResolver::getKeys(sections,cmd.section, ',') << ')';

	ostr << '\n';

	ostr << ' ' << ' ' << cmd.getDescription() << '\n';
	//ostr << cmd.getDescription() << '\n';

	/// Iterate variable keys
	if (detailed){

		//for (std::list<std::string>::const_iterator kit = keys.begin(); kit != keys.end(); ++kit){
		//const std::string & key = *kit;
		for (const std::string & key: keys){

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
