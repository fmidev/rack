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

#ifndef DRAIN_PROGRAM_H_
#define DRAIN_PROGRAM_H_

// #include <iostream>


#include "CommandSequence.h"
#include "Command.h"


namespace drain {

/// Sequence consisting of Command:s retrieved from a CommandBank.
/**
 *   A program may contain a single routine.
 *
 */
//class Program :  public CommandSequence<std::pair<std::string,Command *> > {
class Program :  public CommandSequence<std::pair<std::string,Command *> >, public Contextual {

public:

	inline
	Program(){};

	inline
	Program(const Program &prog){
		std::cerr << "Program copy ctor: ctx?\n";
	};

	inline
	Program(Context & ctx){
		setExternalContext(ctx);
	};


	Command & add(const std::string & key, Command & cmd);

	// Return
	iterator add(const std::string & key, Command & cmd, iterator pos);

	/// Main
	void run() const;

};

/// Structure for implementing threads in parallelized OpenMP \c for loop.
//class ProgramVector : public std::vector<Program> {
class ProgramVector : public std::map<int,Program> {

public:

	/// Adds a new, empty program to thread vector.
	Program & add(Context & ctx);

	void debug();

};



} /* namespace drain */

#endif /* DRAINLET_H_ */

// Rack
