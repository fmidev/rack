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


#include <set>
#include <map>
#include <list>

#include "drain/util/TypeUtils.h"
#include "drain/util/String.h"
#include "drain/util/StringMapper.h"
#include "CommandRegistry.h"


namespace drain {



std::string CommandRegistry::resolveKey(const std::string & key) const {

	Logger mout( __FUNCTION__, __FILE__);

	const size_t l = key.length();

	if (l == 0)
		return key;

	if (key.at(0) != '-'){
		return key;
	}
	else {

		if (key.at(1) != '-'){
			if (l > 2){
				mout.warn() << " mixed-type argument (several chars after single hyphen): " << key << mout.endl;
				return key.substr(1);
			}
			else
				return getKey(key.at(1));
		}
		else {
			if (l < 4)
				mout.warn() << " mixed type argument '--x' (single char after double hyphen)" << key << mout.endl;
			return key.substr(2);
		}
	}
}

int CommandRegistry::index(0);


void CommandRegistry::add(const std::string & section, Command & r, const std::string & name, char alias){

	// Logger mout("CommandRegistry", __FUNCTION__); // warning, not initialized?
	if (name == DEFAULT_HANDLER){
		entryMap.insert(std::pair<std::string, Command &>(DEFAULT_HANDLER, r));
		return;
	}


	std::string nameFinal;

	// Cut off "Cmd"
	if (name.compare(0,3,"Cmd") == 0)
		nameFinal = name.substr(3);
	else
		nameFinal = name;

	if (!prefix.empty()){

		const char lastPrefixChar = prefix.at(prefix.length()-1); //PREFIX_LENGTH-1);

		if ((lastPrefixChar >= 'a') && (lastPrefixChar <= 'z')) // last char
			StringTools::upperCase(nameFinal,1);
		else if ((lastPrefixChar >= 'A') && (lastPrefixChar <= 'Z')) // last char
			StringTools::lowerCase(nameFinal,1);

		nameFinal = prefix + nameFinal;

	}
	else {
		StringTools::lowerCase(nameFinal, 1);
	}

	/*
	entryMap.insert(std::pair<string, Command &>(nameFinal, r));
	//std::cerr << "Inserted.\nAliasing:" << std::endl;
	if (alias){
		//aliases[alias] = r.getName();
		aliases[alias] = nameFinal;
		//aliasesInv[r.getName()] = alias;
		aliasesInv[nameFinal] = alias;
	}
	*/

	sections[section].insert(nameFinal);

	Registry<Command>::add(r, nameFinal, alias);
	// std::cerr << __FUNCTION__ << ":end." << std::endl;
}


drain::VariableMap & CommandRegistry::getStatusMap(bool update){

	if (update){

		// std::cerr << "CommandRegistry::getStatusMap, UPDATE\n";
		const CommandRegistry::map_t & m = getMap();

		for (CommandRegistry::map_t::const_iterator dit = m.begin(); dit != m.end(); ++dit) {
			//std::cerr << "*** " << dit->first << std::endl;

			// if (dit->first.at(0) != 'i'){ // iOp bug TOD FIX!!
				// if (char c = dit->first.at(1) != UPPERCASE ){}
				const drain::ReferenceMap & params = dit->second.getParameters();
				drain::Variable & v = statusMap[dit->first];
				v = params.getValues();
				//vField = params.getKeys();
			// }

			//else
			//	std::cerr << "*** skipped" << std::endl;

		}
	}

	return statusMap;
}



void CommandRegistry::help(std::ostream & ostr) const {

	for (map_t::const_iterator it = entryMap.begin(); it != entryMap.end(); ++it){
		//ostr << it->first << '\n';
		//const Command & r = it->second;
		//help(r, ostr, false);
		help(it->first, ostr, false); // This way finds aliases, too
		ostr << '\n';
	}
}


//template <class T>
void CommandRegistry::help(const std::string & key, std::ostream & ostr, bool parameterDescriptions) const {

	const map_t::const_iterator it = find(key);

	if (it != entryMap.end()){
		ostr << "--" << it->first;
		Dictionary2<char, std::string>::const_iterator ait = aliasesNew.findByValue(it->first);
		//ostr << " | " << aliasesNew << it->first;
		if (ait != aliasesNew.end()){
			ostr << ", -" << ait->first;
		}
		help(it->second, ostr, parameterDescriptions);
	}
	else {
		std::cerr << "Warning: help not found for key=" << key << std::endl;
	}

}

//template <class T>
void CommandRegistry::help(const Command & cmd, std::ostream & ostr, bool parameterDescriptions) const {

	//ostr << '\t' << cmd.getName() << '\n';
	const ReferenceMap & params = cmd.getParameters();
	const std::map<std::string,std::string> & units = params.getUnitMap();
	const std::list<std::string> & keys = params.getKeyList();  // To get keys in speficied order.

	ostr << ' ' << ' ';
	char separator = 0;
	for (std::list<std::string>::const_iterator kit = keys.begin(); kit != keys.end(); ++kit){

		if (separator)
			ostr << separator;
		else
			separator = params.separator; //',';

		ostr << '<' << *kit << '>';
	}
	ostr << '\n';

	ostr << ' ' << ' ' << cmd.getDescription() << '\n';
	//ostr << cmd.getDescription() << '\n';

	/// Iterate variable keys
	if (parameterDescriptions){
		for (std::list<std::string>::const_iterator kit = keys.begin(); kit != keys.end(); ++kit){
			const std::string & key = *kit;
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

//template <class T>
void CommandRegistry::helpSections(std::ostream & ostr) const {
	ostr << "Help available on sections:\n";
	for (SectionMap::const_iterator it = sections.begin(); it != sections.end(); ++it){
		ostr << "  --help " << it->first << '\n'; // description?
		helpSections(ostr, it->first);
	}
}

//template <class T>
void CommandRegistry::helpSections(std::ostream & ostr, const std::string & section) const {

	Logger mout(__FUNCTION__, __FILE__);

	SectionMap::const_iterator it = sections.find(section);
	if (it == sections.end()){
		mout.error() << " section '" << section << "' not found" << mout.endl;
	}

	const std::set<std::string> & commands = it->second;
	for (std::set<std::string>::const_iterator it = commands.begin(); it != commands.end(); ++it){
		// ostr << "    --" << *it << '\n'; // description?
		help(*it, ostr, false);
		ostr << '\n';
	}
}

void CommandRegistry::toJSON(std::ostream & ostr) const {

	char sep = 0;
	ostr << "{\n";

	for (map_t::const_iterator it = entryMap.begin(); it != entryMap.end(); ++it){
		// const Command & r = it->second;
		if (sep)
			ostr << sep << '\n';
		else
			sep = ',';
		const drain::ReferenceMap & p = it->second.getParameters();
		ostr << "  \"" << it->first << "\" : {\n";
		drain::JSONwriter::mapElementsToStream(p, p.getKeyList(), ostr, 4);
		ostr << "\n  }";
		//p.toJSON(ostr, 4);
		/*
		const bool SINGLE = (p.size() == 1);
		if (!SINGLE)
			ostr << '{';
		char sep2 = 0;
		for (drain::ReferenceMap::const_iterator pit = p.begin(); pit != p.end(); ++pit){

			if (sep2)
				ostr << sep2;
			else
				sep2 = ',';
			ostr << '\n';

			drain::VariableMap m;

			if (!SINGLE)
				ostr << "  \"" << pit->first << "\" : ";
			else
				m["name"] = pit->first;

			const std::type_info & t = pit->second.getType();
			if (t == typeid(std::string)){
				m["type"] = "string";
			}
			else if (t == typeid(void)){
				m["type"]  = drain::Type::getTypeChar(t);
			}
			else if (drain::Type::call<drain::typeIsInteger>(t)){
				m["type"] = "integer";
			}
			else
				m["type"] = "float";

			m["value"] = (const drain::Castable &)pit->second;
			m.toJSON(ostr, 4);
			//ostr << "\n";
			//if (MULTIPLE)
			//	ostr << ")";
		}
		//p.toJSON(ostr);
		if (!SINGLE){
			ostr << "\n}";
		}
		 */
	}

}


void CommandRegistry::run(Command & cmd, const std::string & params) const {

	Logger mout(__FUNCTION__, __FILE__);

	try {

		if (params.empty()){
			cmd.run(params);
		}
		else if (expandVariables){
			//mout.toOStr() << "expanding. map: " << statusMap << mout.endl;
			statusFormatter.parse(params);
			mout.debug() << "expanded: '" << params << "' => '" << statusFormatter.toStr(statusMap) << "'" << mout.endl;
			//statusFormatter.debug(std::cerr, statusMap);
			cmd.run(statusFormatter.toStr(statusMap));
		}
		else {
			mout.debug(2) << "no expansion: " << params << mout.endl;
			cmd.run(params);  // may reset lastCommand
		}

		if (cmd.getName() != DEFAULT_HANDLER)  // CHECK IF CAUSES CONFLICTS
			lastCommand = cmd.getName(); // generally, not the same as cmd line name !

	}
	catch (std::exception & e) {
		std::cerr << e.what() << std::endl;
		exit(-1);
	}

}


void CommandRegistry::run(const std::string & name, const std::string & params) const {

	Logger mout(__FUNCTION__, __FILE__);  //REPL "CommandRegistry",  __FUNCTION__);

	const map_t::const_iterator it = find(name);
	if (it != entryMap.end()){

		mout.debug() << name << '(' << it->second.getName() << ')' << '[' << params << ']' << mout.endl;
		run(it->second, params);

		/*
		it->second.run(params);
		//lastCommand = it->first;
		//if (it->second.getName() != DEFAULT_HANDLER)
		lastCommand = it->second.getName();
		// std::cerr << "*** CommandRegistry::run:" << index << '\t' << it->first << " ("<< name << "=" << params << ") " << lastCommand << std::endl;
		//std::cerr << " CommandRegistry::run:" << it->first << " ("<< name << "=" << params << ")" << std::endl;
		 */
	}
	else {
		mout.error() << "no such command/option: " << name << mout.endl;
		//std::cerr << "Command not found: " << name << " (" << params << ")"<< std::endl;
		//throw std::runtime_error(name + " <- DrainRegistry::run: no such command/option.");
	}
}

void CommandRegistry::run(Script & script) const {

	Logger mout(__FUNCTION__, __FILE__);

	/// Applied, if (expandVariables == true)
	//  drain::StringMapper strmap("[a-zA-Z0-9_:]+");

	//std::list<std::string> l;
	//	Script parallel;

	// Script parallelTasks;

	for (Script::iterator it = script.begin(); it != script.end(); ++it){

		Command & cmd = it->first;
		const std::string & params = it->second;

		/*
		if (!parallel.empty()){
			//parellel.p
			Script & parallelTask = parallelTasks.push_back(__x);
			std::list<std::string> l;
			std::list<char> a;
		}
		*/

		// std::cout << cmd << std::endl;
		// mout.debug() << "cmd=(" << cmd.getName() << "); params=" << params << mout.endl;
		mout.debug() << cmd.getName() << '[' << params << ']' << mout.endl;
		run(cmd, params);
		//mout.warn() << " lastCommand" << lastCommand << mout.endl;
	}

}

void CommandRegistry::scriptify(int argc, const char **argv, Script & script) {
		//Script & script = *this;

	//Logger mout(std::string(__FUNCTION__) + "(argc,argv)");
	Logger mout(__FUNCTION__, __FILE__);
	mout.info() << mout.endl;

	for (int i = 1; i < argc; ++i) {
		const std::string command   = argv[i];
		const std::string arguments = (i+1)<argc ? argv[i+1] : "";
				//argv[std::min(i+1, argc-1)];
		if (appendCommand(command, arguments, script))
		//if (appendCommand(command, arguments))
			++i;
	}
	//runCommands(argList);
}

void CommandRegistry::scriptify(const std::string & arg, Script & script) {

	Logger mout(__FUNCTION__, __FILE__);
	//Logger mout(std::string(__FUNCTION__) + "(std::string)");
	//std::cerr << "ARGS: " << arg.substr(0, arg.find_last_not_of(" \t\n\r")+1) << std::endl;

	const size_t n = arg.find_last_not_of(" \t\n\r");

	if (n == std::string::npos){
		mout.note() << "empty script std::string?" << mout.endl;
		return;
	}

	std::stringstream sstr(arg.substr(0, n+1)); // strip trailing whitespace
	//std::stringstream sstr; // strip trailing whitespace
	//sstr.setf(ios::skipws);
	//sstr.toStr(arg);
	//
	int i = 0;
	std::string str[2];
	sstr >> str[0];
	//do {
	while (true){

		std::string & command   = str[i];
		if (command.empty()){
			//mout.warn() << "end" << mout.endl;
			return;
		}

		std::string & arguments = str[i^1];
		if (!sstr.eof())
			sstr >> arguments;
		else
			arguments.clear();

		//mout.warn() << "ADD: " << command << '\t' << arguments << mout.endl;
		if (appendCommand(command, arguments, script)){ // Returns true, if 'arguments' used, so...
			if (!sstr.eof())
				sstr >> arguments; // ... reload next round's command
			else
				return;
		}


		i ^= 1;

	}

}

bool CommandRegistry::appendCommand(const std::string & command, const std::string & arguments, Script & script) {

	Logger mout(__FUNCTION__, __FILE__);

	std::string key = resolveKey(command);
	map_t::iterator dit = find(key);

	if (dit != entryMap.end()){
		Command & cmd = dit->second;
		//mout.debug() << " Found command for: '" << key << "' -> " << keyPlain << cmd << mout.endl;
		if (cmd.hasArguments()){
			//mout.warn() << cmd.getName() << " args:" << arguments << mout.endl;
			script.push_back(Script::value_type(cmd, arguments));
			return true;
		}
		else {
			// mout.warn() << cmd.getName() << " (no args)" << mout.endl;
			script.push_back(Script::value_type(cmd, ""));
			return false;
		}
	}
	else {

		map_t::iterator dit = find(DEFAULT_HANDLER);
		if (dit != entryMap.end()){
			script.push_back(Script::value_type(dit->second, command));  // key would strip off hyphens of --/sjsjjs
			return false;
		}
		else
			throw std::runtime_error(command + ": fatal: could not find (default) handler '" + DEFAULT_HANDLER + "'");

		return false;
	}
}










CommandRegistry & getRegistry(){
	static CommandRegistry registry;
	return registry;
}

void addEntry(const std::string & section, Command & command, const std::string & name, char alias){
	CommandRegistry & r = getRegistry();
	r.add(section, command, name, alias);
}

void addEntry(Command & command, const std::string & name, char alias){
	CommandRegistry & r = getRegistry();
	r.add(command, name, alias);
}


const CommandRegistry::SectionMap & getSections(){
	CommandRegistry & r = getRegistry();
	return r.getSections();
}

//int CommandRegistry::index(0);


} // namespace drain

// Drain
