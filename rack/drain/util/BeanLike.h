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

// Drain
