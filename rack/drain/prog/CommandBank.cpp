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


/// Appends program with commands fo the script
void CommandBank::compile(const ScriptTxt & script, Program & prog) const {
	for (ScriptTxt::const_iterator it = script.begin(); it!=script.end(); ++it) {
		prog.add(clone(it->first)).setParameters(it->second);
	}
}

// Future extension.
void CommandBank::remove(Program & prog) const {
	std::cout << "clearing prog: " << '\n';
	prog.toStream();
	for (Program::const_iterator it = prog.begin(); it!=prog.end(); ++it) {

	}
}


void CommandBank::run(ScriptTxt & script){

	Logger mout(__FILE__, __FUNCTION__); // warni

	bool PARALLEL = false;
	std::vector<Program> parallelRuns;

	for (ScriptTxt::const_iterator it = script.begin(); it != script.end(); ++it) {

		if (it->first == "("){
			PARALLEL = true;
			continue;
		}

		if (!PARALLEL){
			BasicCommand & cmd = get(it->first); // or clone, to keep defaults? => consider FLAG save-params
			cmd.setParameters(it->second);
			//cmd.run(it->second);
			cmd.exec();
			if (!routine.empty()){
				Program prog;
				compile(routine, prog);
				prog.run();
			}
			// TODO: when to automatically clear routine?
		}
		else {

			// If all runs collected, run them
			if (it->first == ")"){
				mout.warn() << "Invoke parallel runs" << mout.endl;
				#pragma omp parallel for
				for (size_t i = 0; i < parallelRuns.size(); ++i) {
					mout.warn() << "Start parallel run #" << i << mout.endl;
					parallelRuns[i].run();
					mout.warn() << "Ended parallel run #" << i << mout.endl;
				}
				PARALLEL =false;
				continue;
			}
			else { // continue collecting

				mout.warn() << "Preparing parallel run for " << it->first << '(' << it->second << ')' <<  mout.endl;
				const size_t i = parallelRuns.size();
				// Append new (sub)script
				parallelRuns.push_back(Program());
				Program & prog = parallelRuns[i];
				BasicCommand & cmd = clone(it->first);

				//prog.add(&cmd, it->second);
				prog.add(cmd).setParameters(it->second);
				if (!routine.empty()){ // Technically unneeded, but for mout.debg()
					mout.warn() << "constructing script " <<  mout.endl;
					compile(routine, prog);
				}
				//
			}

		}
	}

}



/// Converts linux command line to pairs of strings:  (<cmd>, [params])
/**
 *  Starts from argument 1 instead of 0.
 */
void CommandBank::scriptify(int argc, const char **argv, ScriptTxt & script){

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
				mout.warn() << arg << ": argument missing (premature end of command line), i=" << i <<  mout.endl;
				return;
			}
		}

	}
}

void CommandBank::scriptify(const std::string & line, ScriptTxt & script){

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

bool CommandBank::scriptify(const std::string & arg, const std::string & argNext, ScriptTxt & script){

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
				/*
					++i;
					if (i<argc)
						script.add(key, argv[i]);
					//else // help?
					//	mout.warn() << arg << ": argument missing (premature end of command line), i=" << i <<  mout.endl;
				 *
				 */
				script.add(key, argNext);
				return true;
			}
		}
		else {
			mout.warn() << arg << ": ???"  <<  mout.endl;
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

	const std::string & cmdLong = resolveHyphens(key);

	if (!cmdLong.empty()){
		info(cmdLong, get(key), ostr);
	}
	else {
		mout.error() << "not found: " << key << mout.endl;
	}
}

void CommandBank::help(std::ostream & ostr){

	for (map_t::const_iterator it = this->begin(); it!=this->end(); ++it){
		info(it->first, it->second->get(), ostr);
	}
	//std::flush(ostr);
}


/// Checked key and respective command
void CommandBank::info(const std::string & key, const BasicCommand & cmd, std::ostream & ostr) const {
	ostr << "--" << key;
	char alias = getAlias(key);
	if (alias)
		ostr << ", -" << alias;
	const drain::ReferenceMap & params = cmd.getParameters();
	if (!params.empty())
		ostr << ' ' << '(' << params << ')';
	ostr << '\n';
	ostr << "  " << cmd.getDescription() << '\n';
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
