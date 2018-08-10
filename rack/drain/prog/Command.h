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
//#include <drain/util/Debug.h>
//#include <drain/util/ReferenceMap.h>

#include "util/Log.h"
#include "util/BeanLike.h"
#include "util/ReferenceMap.h"
//#include "CommandRegistry.hsepa"


namespace drain {



/// Base class for commands: typically actions taking parameters but also plain variable assignments and parameterless actions.
/**
 *
 */
struct Command {

	inline
	Command(){};

	inline
	virtual
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
		if (it == params.end())
			return false;
		else
			return it->second.getType() != typeid(void);  // ???
	};

	// Currently, non-const, because may run
	virtual
	void run(const std::string & params) = 0;



};

typedef std::list<std::pair<Command &, std::string> > Script; // TODO: move




/// Simple implementation of Command. Adds description and parameters to members.
/** Wrapper for simple template classes; no member functions are required.
 *  BasicDrainLet implements getDescription(), getParameters(), setParameters().
 *  The default implementation of run() calls setParameters() and exec().
 *  The default implementation exec() does nothing, it should be redefined in derived classes.
 */
class BasicCommand : public Command {  // Todo consider BeanLike

public:

	inline
	BasicCommand(const std::string & name, const std::string & description) : Command(), name(name), description(description) {
		if (name.find(' ') != std::string::npos){
			std::cerr << "BasicCommand(): name contains space(s): " << name << " descr=" << description << std::endl;
			exit(1);
		}
	};

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
	inline
	void setParameters(const std::string & params, char assignmentSymbol='=') {
		//const bool ALLOW_SPECIFIC = (parameters.separator != '\0'); //!parameters.separators.empty();  // consider automatic
		if (parameters.separator)
			parameters.setValues(params, assignmentSymbol); //
		else
			parameters.setValues(params, '\0', false);
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


protected:

	const std::string name;

	const std::string description;

	ReferenceMap parameters;

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
			const std::string & key, const T & initValue = T(), const std::string & unit = "") : BasicCommand(name, description) {

		parameters.separator = '\0';
		if (key.empty())
			std::cerr << "warning: param key empty for: " << name << std::endl;

		parameters.reference(key, value = initValue, unit);
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
