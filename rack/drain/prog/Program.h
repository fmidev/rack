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

#ifndef COMMANDUTILS_H_
#define COMMANDUTILS_H_

#include <iostream>

#include "Command.h"
#include "drain/util/Sprinter.h"


namespace drain {

// Consider raise: Smart list. Or streamers.
template <class T>
class CommandSequence : public std::list<T> {

public:

	typedef T entry_t;

	typedef std::list<T> list_t;


	// For debugging
	/*
	void toStream(std::ostream & ostr = std::cout) const {
		for (typename list_t::const_iterator it = this->begin(); it != this->end(); ++it)
			entryToStream(*it);
	}
	*/

	// virtual void entryToStream(const typename list_t::value_type & entry, std::ostream & ostr = std::cout) const = 0;

	/// Tell if this routine is
	inline
	operator bool(){ return !this->empty(); };

};

// typedef Scrupt<std::string>    Script;


template <class T>
inline
std::ostream & operator<<(std::ostream & ostr, const CommandSequence<T> & commands){
	static const SprinterLayout layout("\n", "<?>", "\0=;", "''");
	SprinterBase::sequenceToStream(ostr, commands, layout);
	return ostr;
}




class Script : public CommandSequence<std::pair<std::string,std::string> > {

public:

	typename list_t::value_type & add(const std::string & key, const std::string & params="");

	//virtual	void entryToStream(const typename list_t::value_type & entry, std::ostream & ostr = std::cout) const;


};



/// Sequence consisting of Command:s retrieved from a CommandBank.
/**
 *   A program may contain a single routine.
 *
 */
//class Program : public CommandSequence<Command *> , public Contextual{
// consider: protected Contextual public Contextual,
class Program :  public CommandSequence<std::pair<std::string,Command *> > {

public:

	//inline	Program(){};

	//inline	Program(Context & ctx) : Contextual(ctx){};

	//inline	Program(Context & ctx = getCloner<Context>().get()) : Contextual(ctx){};

	Command & add(const std::string & key, Command & cmd);
	//Command & add(Command & cmd);

	//void append(const CommandBank & commandBank, const Script & script);
	Script routine;

	void run() const;

	//virtual 	void entryToStream(const list_t::value_type & entry, std::ostream & ostr = std::cout) const;


};

/// Structure for implementing threads in parallelized OpenMP \c for loop.
class ProgramVector : public std::vector<Program> {

public:

	/// Adds a new, empty program to thread vector.
	inline
	Program & add(){
		push_back(Program());
		return back();
	};

	/// Adds a new, empty program with desired context to thread vector
	/*
	inline
	Program & add(Context & ctx){
		push_back(Program(ctx));
		return back();
	};
	*/

};



} /* namespace drain */

#endif /* DRAINLET_H_ */

// Rack
