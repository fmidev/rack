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

#ifndef DRAINLET_H_
#define DRAINLET_H_

//#include <map>
#include <set>
//#include "drain/util/Debug.h"
//#include "drain/util/ReferenceMap.h"

#include "drain/util/Log.h"
#include "drain/util/BeanLike.h"
#include "drain/util/ReferenceMap.h"
//#include "CommandRegistry.hsepa"


namespace drain {

class Context {

public:

	Context() : id(++counter){
	}

	long int getId(){
		return id;
	}
	// std::string name;

	//protected:

	long int id;

private:

	static long int counter;



};

/*

template <class C>
class Contextual {
public:

	Contextual() : contextPtr(NULL){
	}

	void setContext(C & context){
		contextPtr = & context;
	}

	Context & getBaseContext() const {
		if (contextPtr != NULL){
			return *contextPtr;
		}
		else {
			static Context defaultContext;
			return defaultContext;
		}
	}

	//template <class C>
	C & getContext() const {
		if (contextPtr != NULL){
			return (C &)*contextPtr;
		}
		else {
			static C defaultContext;
			return defaultContext;
		}
	}


protected:

	// Note: common base class. The actual object may be a derived class.
	Context *contextPtr;

};
*/


/// Base class for commands: typically actions taking parameters but also plain variable assignments and parameterless actions.
/**
 *
 */
class Command { //: public Contextual<Context> {

public:

	inline
	Command() : contextPtr(NULL) {};

	//inline	Command(Command & cmd){};

	virtual inline
	~Command(){};

	// TODO: getFullName(), the "C++ name" of this (derived) class.
	// Typically not same as the command line command.

	virtual
	const std::string & getName() const = 0; //{ return name; };


	virtual
	const std::string & getDescription() const = 0;

	// TODO: setParameters?

	virtual
	const ReferenceMap & getParameters() const = 0;

	inline
	bool hasArguments() const { // tODO rename
		Logger mout("Command", __FUNCTION__);

		const ReferenceMap & params = this->getParameters();
		//mout.warn() << params << mout.endl;

		const ReferenceMap::const_iterator it = params.begin();
		if (it == params.end()) // empty
			return false;
		else
			return it->second.getType() != typeid(void);  // ???
	};

	/// Description of result, comparable to a return type of a function. ?
	virtual inline
	const std::string & getType() const {
		static const std::string empty;
		return empty;
	};


	// Currently, non-const, because may run
	/**
	 *  See exec const below.
	 */
	virtual
	void run(const std::string & params) = 0;



	void setContext(Context & ctx){
		contextPtr = & ctx;
	};

	Context & getBaseContext() const {
		if (contextPtr != NULL){
			return *contextPtr;
		}
		else {
			static Context defaultContext;
			return defaultContext;
		}
	}

	/// Returns the linked context.
	/**
	 *  Risky: The actual object may be a derived class.
	 */
	template <class C>
	C & getContext() const {
		if (contextPtr != NULL){
			return (C &)*contextPtr;
		}
		else {
			static C defaultContext;
			return defaultContext;
		}
	}


	protected:

		// Note: common base class. The actual object may be a derived class.
		Context *contextPtr;

};

inline
std::ostream & operator<<(std::ostream & ostr, const Command &cmd){
	ostr << cmd.getName();
	if (cmd.hasArguments()){
		ostr << '(' << cmd.getParameters() << ')';
	}
	return ostr;
}

// See new implementations in CommandBank, CommandUtils.
typedef std::list<std::pair<Command &, std::string> > Script; // TODO: move




/// Simple implementation of Command. Adds description and parameters to members.
/** Wrapper for simple template classes; no member functions are required.
 *  BasicDrainLet implements getDescription(), getParameters(), setParameters().
 *  The default implementation of run() calls setParameters() and exec().
 *  The default implementation exec() does nothing, it should be redefined in derived classes.
 */
class BasicCommand : public Command {  // Todo consider BeanLike

public:


	BasicCommand(const std::string & name, const std::string & description);

	inline
	BasicCommand(const BasicCommand & cmd): Command(), section(cmd.section), name(cmd.name), description(cmd.description) {
		// remember to call importParameters()
	}


	virtual
	inline
	const std::string & getName() const { return name; };

	virtual
	inline
	const std::string & getDescription() const { return description; };

	virtual
	inline
	const ReferenceMap & getParameters() const { return parameters; };


	virtual
	void setParameters(const std::string & args, char assignmentSymbol='=');



	template <class T>
	void setParameters(const SmartMap<T> & p){
		parameters.setValues(p);
	}

	template <class T>
	void setParameter(const std::string & key, const T & value) {
		parameters[key] = value;
	}


	/// Sets new parameters and runs.
	//  Note: semantics are a bit weird ( const would be more intuitive)
	virtual
	inline
	void run(const std::string & params = ""){
		// std::cerr << " Command::run: " << params << std::endl;
		setParameters(params);
		exec();
	}

	virtual
	inline
	void exec() const {};

	// Experimental
	//bool parallel;

	// Optional bit flag(s) marking the command type (compare with manual page sections)
	/**
	 *  Typically, zero section for "hidden" commands, which do not appear in section helps.
	 */
	int section;

protected:

	const std::string name;

	const std::string description;

	ReferenceMap parameters;

	void importParams(const BasicCommand & cmd){
		parameters.copyStruct(cmd.parameters, cmd, *this); // FIX: may be wrong (cmd has linked members, this has none, yet.
	}

};




/// A single-parameter command.
/**
 *  \tparam T - storage type of the command parameter
 */
template <class T = std::string>
class SimpleCommand : public BasicCommand {

public:

	T value;

	SimpleCommand(const std::string & name, const std::string & description,
			const std::string & key="value", const T & initValue = T(), const std::string & unit = "") : BasicCommand(name, description) {

		parameters.separator = '\0';
		if (key.empty())
			std::cerr << "warning: param key empty for: " << name << std::endl;

		parameters.link(key, value = initValue, unit);
	};

	SimpleCommand(const SimpleCommand & cmd) :  BasicCommand(cmd.name, cmd.description) {
		parameters.separator = '\0';
		parameters.link(cmd.parameters.getKeys(), value = cmd.value); // kludge
	};

	inline
	operator const T &() const { return value; }; // needed?

	inline
	operator       T &(){ return value; }; // needed?

	inline
	SimpleCommand<T> & operator =(const T &s){ value = s; return *this; };

protected:



};



} /* namespace drain */

#endif /* DRAINLET_H_ */

// Rack
