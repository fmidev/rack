/**


    Copyright 20014 - 2015 Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

    Created on: Nov 17, 2014
    Author: mpeura
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
