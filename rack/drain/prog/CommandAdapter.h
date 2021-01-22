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

#ifndef DRAINLET_WRAPPER_H_
#define DRAINLET_WRAPPER_H_

#include <map>
#include <set>
//#include "drain/util/Debug.h"
//#include "drain/util/ReferenceMap.h"

#include "drain/util/Log.h"
#include "drain/util/ReferenceMap.h"

//#include "Command.h"
//#include "CommandRegistry.h"

#include "CommandBank.h"


namespace drain {

/// Creates an instance of command class C.
//  in section S of the global command registry.
/**
 * \tparam C - command
 *
 */
/*
template <class C> //, char PREFIX=0, int SECTIONS=1>
class CommandWrapper : public C {
public:

	/// Add a command to the shared command bank.
	CommandWrapper(const std::string & name, char alias = 0){
		getCommandBank().add<C>(name,alias); //.section = S;
	}

};
*/

/// Creates an instance of command class C, deriving command name from the class name and prefixing if desired.
//  in section S of the global command registry.
/**
 * \tparam CMD - command
 * \tparam PREFIX - optional prefix, added to lower-case initialized
 * \tparam SECTIONS - command sections flags
 */
template <class CMD, char PREFIX=0, int SECTIONS=1>
class CommandWrapper : public CMD {
public:

	/// Add a command to the shared command bank.
	CommandWrapper(const std::string & name, char alias = 0){
		getCommandBank().add<CMD>(name,alias); //.section = S;
	}

	/// Add a command to the shared command bank, deriving command name automatically from the class name.
	CommandWrapper(char alias = 0){
		//std::string name = CMD::getName();
		std::string name = this->getName();
		CommandBank::deriveCmdName(name, PREFIX);
		try {
			//getCommandBank().add<CMD>(name, alias).section = SECTIONS;
			// Note cast. Otherwise infinite loop, if CommandWrapper passed and re-constructed.
			getCommandBank().addExternal((CMD &)(*this), name, alias).section = SECTIONS;
		}
		catch (const std::exception &e) {
			std::cerr << __FUNCTION__ << ':' << name << ':'  << e.what() << '\n';
			std::cerr << name << ':' <<  ((CMD *)this)->getParameters() << '\n';
		}
	}

	// Consider alternatively: set(prefix, sectionTitle, sectionFlag) ?
	static
	void setSectionTitle(const std::string & title=""){
		getCommandBank().sections.add(title, SECTIONS);
	}

	/// Return the flag(s) marking this section.
	static inline
	int getSectionFlag(){
		return SECTIONS;
	}

	/// Return the character used as prefix for the commands in this section.
	static inline
	char getPrefix(){
		return PREFIX;
	}

};

template <class C>
class BankCommandWrapper : public C {
public:

	/// Add a command to the shared command bank, deriving command name automatically from the class name.
	BankCommandWrapper(CommandBank & b, char alias = 0) : C(b){
		std::string name = C::getName();
		CommandBank::deriveCmdName(name);
		getCommandBank().add<C>(name, alias);
	}

};


template <class B>
class BeanCommandEntry : public CommandWrapper<BeanCommand<B> > {

public:
	BeanCommandEntry(char prefix = 0) : CommandWrapper<BeanCommand<B> >(prefix){
	};

};


/// Adapter registered directly as a command entry upon construction. Needed? typedef CommandEntry<BeanCommand>
/*
 *  \tparam BeanLike

template <class B>
class BeanRefEntry : public BeanCommand<B,B&> { // why not directly public BeanCommand<B, B &>

public:

	BeanRefEntry(B & bean) : BeanCommand<B,B&>(bean) {
		getRegistry().add(*this, bean.getName(), 0);
	};

	BeanRefEntry(B & bean, const std::string & cmdName, char alias = 0) : BeanCommand<B,B&>(bean) {
		getRegistry().add(*this, cmdName, alias);
	};


	BeanRefEntry(B & bean, const std::string & section, const std::string & cmdName, char alias = 0) : BeanCommand<B,B&>(bean) {
		getRegistry().add(section, *this, cmdName, alias);
	};

	virtual
	~BeanRefEntry(){};

	/// Bean may be peaceful.
	virtual
	void exec() const {};
};
*/


/// A command that is automatically added to CommandRegistry upon initialization.
/**
 *
 *  \tparam T - base class, usually Command
 *
 *  \see CommandWrapper, the newer implementation
 */
/*
template <class T>
class CommandEntry : public T {

public:

	CommandEntry(char alias = 0) {
		getRegistry().add(*this, T::getName(), alias);
	};

	CommandEntry(const std::string & name, char alias = 0) {
		getRegistry().add(*this, name, alias);
	};


	CommandEntry(const std::string & section, const std::string & name, char alias = 0) {
		getRegistry().add(section, *this, name, alias);
	};


	CommandEntry(const std::string & name, char alias, const T & init) : T(init)  { // use copy constr.
		getRegistry().add(*this, name, alias);
	};

	~CommandEntry(){};


};
*/


}

/* namespace drain */

#endif /* DRAINLET_H_ */

// Rack
