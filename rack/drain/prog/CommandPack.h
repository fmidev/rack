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

#ifndef COMMANDPACK_H_
#define COMMANDPACK_H_

#include "CommandRegistry.h"
#include "CommandAdapter.h"



namespace drain {




class CmdHelp : public SimpleCommand<std::string> { // public BasicCommand {//
    public: //re 

	inline
	CmdHelp(const std::string & title, const std::string & usage = "") : SimpleCommand<std::string>(__FUNCTION__, "Dump help and exit.", "keyword"), title(title), usage(usage) { //BasicCommand("Dump help and exit") {
	};

	void exec() const;


	const std::string title;

	const std::string usage;

protected:

	void helpOnModules(std::ostream & ostr, const std::string & excludeKey = "*") const ;

};

/// A base class
/**
 *  Typically, this wrapper is used for displaying version info etc. through defining
 *
 *  Now member  std::string value is applied as info container.
 */
class CommandInfo : public BasicCommand { //public SimpleCommand<std::string> {
    public: //re 

	const std::string info;

	//CommandInfo(const std::string & description, const std::string & info) : SimpleCommand<std::string>(description) { value.assign(info); };
	CommandInfo(const std::string & description, const std::string & info) : BasicCommand(__FUNCTION__, description), info(info) {
		//value.assign(info);
	};


	virtual
	inline
	void run(const std::string & params = ""){
		std::cout << info << std::endl;
	};

};


class ScriptParser : public SimpleCommand<std::string> {
    public: //re 

	Script script;

	int autoExec;

	inline
	ScriptParser() : SimpleCommand<std::string>(__FUNCTION__, "Define a script.", "script"), autoExec(-1) {
	};

	virtual
	inline
	void run(const std::string & s){
		//std::cerr << name << ", value=" << value << std::endl;
		//std::cerr << name << ", calling with param=" << s << std::endl;
		if (autoExec < 0)
			autoExec = 1;
		getRegistry().scriptify(s, script);
	};

};



/// Command loader that executes the commands immediately - it does not store the commands into a script.
class CommandLoader : public SimpleCommand<std::string> {
    public: //re 


	CommandLoader() : SimpleCommand<std::string>(__FUNCTION__, "Execute commands from a file.", "filename") {}; // const std::string & name, char alias = 0

	//CommandCommandLoader(std::set<std::string> & commandSet, const std::string & name, char alias = 0);

	virtual
	void run(const std::string & params);

};



//CommandScriptExec::CommandScriptExec(CommandScript & script) : BasicCommand(name, alias, "Execute a script."), script(script) {}
class ScriptExec : public BasicCommand {
    public: //re 

	inline
	ScriptExec(Script & script) : BasicCommand(__FUNCTION__, "Execute a script."), script(script) {};


	virtual
	inline
	void exec() const {
		getRegistry().run(script);
	};

	Script & script;

};

class CmdFormat : public SimpleCommand<std::string> {
    public: //re 

	CmdFormat() :  SimpleCommand<std::string>(__FUNCTION__,"Set format for data dumps (see --sample or --outputFile)", "format","") {  // SimpleCommand<std::string>(getResources().generalCommands, name, alias, "Sets a format std::string.") {
	};

};
extern CommandEntry<CmdFormat> cmdFormat;

/// Handler for requests without a handler - tries to read a file.
/**  Currently unused
*/
class DefaultHandler : public BasicCommand {
    public: //re 

	//std::string value;
	DefaultHandler() : BasicCommand(__FUNCTION__, "Handles arguments for which no specific Command was found."){};

	virtual  //?
	inline
	void run(const std::string & params){

		MonitorSource mout(getName());

		mout.debug(1) << "params: " << params << mout.endl;

		/// Syntax for recognising text files.
		//static const drain::RegExp odimSyntax("^--?(/.+)$");

		if (params.empty()){
			mout.error() << "Empty parameters" << mout.endl;
		}
		else {
			try {
				mout.debug(1) << "Assuming filename, trying to read." << mout.endl;
				getRegistry().run("inputFile", params);  // Note: assumes only that a command 'inputFile' has been defined.
			} catch (std::exception & e) {
				mout.error() << "could not handle params='" << params << "'" << mout.endl;
			}
		}

	};

};
//DefaultHandler defaultHandler;

// GENERAL
class CmdVerbose : public SimpleCommand<int> {
    public: //re 

	CmdVerbose() : SimpleCommand<int>(__FUNCTION__, "Set verbosity level", "level", 0) {
	};

	inline
	void exec() const {
		drain::monitor.setVerbosity(value);
		// drain::image::iMonitor.setVerbosity(value-2);
	};

};


class CmdDebug : public BasicCommand {
    public: //re 

	CmdDebug() : BasicCommand(__FUNCTION__, "Set debugging mode") { // TODO
	};

	inline
	void exec() const {
		CommandRegistry & r = getRegistry();
		r.run("verbose","8");  // FIXME r.setVerbosity();
	};

};


class CmdExpandVariables : public BasicCommand {
    public: //re 

	CmdExpandVariables() : BasicCommand(__FUNCTION__, "Toggle variable expansion on/off") {
	};

	inline
	void exec() const {
		CommandRegistry & r = getRegistry();
		r.expandVariables = ~r.expandVariables;
	};

};


} /* namespace drain */

#endif /* DRAINLET_H_ */

// Rack
