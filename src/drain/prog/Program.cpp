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

#include <drain/Log.h>
#include "Program.h"

namespace drain {

/*
typename Script::entry_t & Script::add(const std::string & cmd, const std::string & params){
	this->push_back(typename list_t::value_type(cmd, params));
	return back();
}
*/




Command & Program::add(const list_t::value_type::first_type & key, Command & cmd){ // const std::string & params = ""){

	this->push_back(typename list_t::value_type(key, & cmd));

	if (contextIsSet()){
		cmd.setExternalContext(getContext<>());
	}

	return cmd;
}

Program::iterator Program::add(const list_t::value_type::first_type & key, Command & cmd, Program::iterator pos){ // const std::string & params = ""){

	//this->push_back(typename list_t::value_type(key, & cmd));
	this->insert(pos, typename list_t::value_type(key, & cmd));

	if (contextIsSet()){
		cmd.setExternalContext(getContext<>());
	}

	return ++pos;

}



void Program::run() const {

	Logger mout(__FILE__, __FUNCTION__);

	for (const auto & entry: *this) {

		const Command & cmd = *entry.second;

		if (entry.first == "script"){
			mout.warn("skipping script " , '(' , cmd.getParameters() , ')' );
			continue;
		}

		mout.warn("executing ", entry.first, "-> ",  cmd.getName(), '(', cmd.getParameters(), ')');
		//mout.note("  context: "  , cmd.getContext<>().getId() );
		cmd.exec();
	}

}

/// Adds a new, empty program to thread vector.
Program & ProgramVector::add(Context & ctx){

	if (ctx.statusFlags.value != 0){
		Logger mout(__FILE__, __FUNCTION__);
		mout.warn("clearing status flags: ", ctx.statusFlags);
		ctx.statusFlags.reset();
	}

	//push_back(Program());
	//back().setExternalContext(ctx);
	Program & program = (*this)[size()];
	//(*this)[size()] = Program();
	program.setExternalContext(ctx);
	return program;
	//return back();
};

void ProgramVector::debug(){

	Logger mout(__FILE__, __FUNCTION__);

	for (const auto & entry: *this){

		const Program & prog = entry.second;
		mout.attention(entry.first, '\t', prog.getContext<>().getName(), " vs ", prog.getContext<>().getId());

		for (const auto & cmdEntry: prog){
			Context & cmdCtx = cmdEntry.second->getContext<>();
			mout.attention('\t', cmdEntry.first, '\t', *cmdEntry.second, ':', cmdCtx.getName());
		}

		//for (const auto & line: prog){
		//	std::cerr << line.first << '\t' << line.second << '\n';
		//}
	}
}

}


