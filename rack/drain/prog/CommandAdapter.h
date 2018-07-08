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

#ifndef DRAINLET_WRAPPER_H_
#define DRAINLET_WRAPPER_H_

#include <map>
#include <set>
//#include <drain/util/Debug.h>
//#include <drain/util/ReferenceMap.h>

#include "util/Log.h"
#include "util/ReferenceMap.h"

#include "Command.h"
#include "CommandRegistry.h"

using namespace drain;

namespace drain {

/// Base for derived classes using member BeanLikes or referenced BeanLikes.
/**
 *   \tparam B  - bean class
 *   \tparam BS - same as B, or reference of B
 */
template <class B, class BS>
class BeanAdapter : public Command {

public:

	BeanAdapter(){};

	BeanAdapter(B & bean) : bean(bean) {};

	typedef B bean_t;

	BS bean;

	inline
	const std::string & getName() const { return bean.getName(); };

	inline
	const std::string & getDescription() const { return bean.getDescription(); };

	virtual
	inline
	const ReferenceMap & getParameters() const { return bean.getParameters(); };  // or getParameters

	virtual
	inline
	void run(const std::string & params = ""){
		bean.setParameters(params);
		exec();
	}

	virtual
	void exec() const = 0;


};

/// Wraps operators into commands
template <class B>
class BeanWrapper : public BeanAdapter<B, B> {
};


/// Applies a referenced bean.
template <class B>
class BeanRefAdapter : public BeanAdapter<B, B &> {

public:

	BeanRefAdapter(B & bean) : BeanAdapter<B,B&>(bean) { //
	};

	BeanRefAdapter(BeanRefAdapter<B> & a) : BeanAdapter<B,B&>(a.bean) { //
	};
};

/// Adapter registered directly as a command entry upon construction.
/*
 *  \tparam BeanLike
 */
template <class B>
class BeanRefEntry : public BeanRefAdapter<B> {

public:

	BeanRefEntry(B & bean) : BeanRefAdapter<B>(bean) {
		getRegistry().add(*this, bean.getName(), 0);
	};

	BeanRefEntry(B & bean, const std::string & cmdName, char alias = 0) : BeanRefAdapter<B>(bean) {
		getRegistry().add(*this, cmdName, alias);
	};


	BeanRefEntry(B & bean, const std::string & section, const std::string & cmdName, char alias = 0) : BeanRefAdapter<B>(bean) {
		getRegistry().add(section, *this, cmdName, alias);
	};

	virtual
	~BeanRefEntry(){};

	/// Bean may be peaceful.
	/*
	virtual
	void run(const std::string & params){
		this->bean.setParameters(params);
	};
	*/

	virtual
	void exec() const {};
};



/// A command that is automatically added to CommandRegistry upon initialization.
/**
 *
 *  \tparam T - base class, usually Command
 */
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


} /* namespace drain */

#endif /* DRAINLET_H_ */
