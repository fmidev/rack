/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef DRAIN_BEANLIKE_H_
#define DRAIN_BEANLIKE_H_

//
#include <cmath>

#include <vector>
#include <iostream>
#include <stdexcept>
#include <map>

#include "Log.h"
#include "ReferenceMap.h"

namespace drain
{

// using namespace std;

/// Something which has a name, a description and possibly some parameters of varying type.
/**
     In \b rack program, VolumeBaseOp is derived from BeanLike.
 */
class BeanLike {

public:

	virtual inline
	~BeanLike(){};

	inline
	const std::string & getName() const { return name; };


	inline
	const std::string & getDescription() const { return description; };

	/// Sets comma-separated parameters in a predetermined order "a,b,c" or by specifing them "b=2".
	/**
	 *  This function is virtual because derived classes may redefine it to update other members.
	 */
	virtual
	inline
	void setParameters(const std::string &p, char assignmentSymbol='=', char separatorSymbol=0){ parameters.setValues(p, assignmentSymbol, separatorSymbol); };  // , true);

	/// Sets parameters
	/**
	 *  This function is virtual because derived classes may redefine it to update other members.
	 */
	template <class T>
	inline
	void setParameters(const std::map<std::string,T> & p){ parameters.importMap(p); }

	/// Sets a single parameter
	template <class F>
	inline
	void setParameter(const std::string &p, const F &value){ parameters[p] = value; }

	/// Gets a single parameter
	template <class F>
	F getParameter(const std::string & p) const {
		if (parameters.hasKey(p))
			return parameters[p];
		else {
			throw std::runtime_error(p + ": no such parameter (BeanLike::getParameter)");
		}
	}




	inline
	const ReferenceMap & getParameters() const { return parameters; };

	inline
	ReferenceMap & getParameters() { return parameters; };


	inline
	void toOStream(std::ostream & ostr) const {
		//ostr << name << ':' << parameters;
		ostr << name << ": " << description << '\n';
		ostr << '\t' << parameters << '\n';
	}

	inline
	std::string toStr() const {
		std::stringstream sstr;
		toOStream(sstr);
		return sstr.str();
	}



protected:


	BeanLike(const std::string & name, const std::string & description="") : name(name), description(description) {
	}

	BeanLike(const BeanLike & b) : name(b.name), description(b.description){
		copy(b);
	}


	const std::string name;  // todo separate (Beanlet)

	const std::string description; // todo separate (Beanlet)

	ReferenceMap parameters;  // todo separate (Beanlet)

	// Copy variables
	void copy(const BeanLike & b){
		parameters.importMap(b.getParameters());
	}


};


inline
std::ostream & operator<<(std::ostream &ostr, const BeanLike & bean){
	bean.toOStream(ostr);
	return ostr;
}

}


#endif
