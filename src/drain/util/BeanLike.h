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
#include "Variable.h"

namespace drain
{


/// Something which has a name, a description and possibly some parameters of varying type.
/**
     In \b rack program, VolumeBaseOp is derived from BeanLike.
 */
class BeanLike {

public:

	virtual inline
	~BeanLike(){};

	/// Return the name of an instance.
	virtual inline
	const std::string & getName() const { return name; };


	/// Return a brief description.
	virtual inline
	const std::string & getDescription() const {
		return description;
	};

	///
	inline
	bool hasParameters() const {
		return !parameters.empty();
	};

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

	/// Grants access to (if above hidden)
	/*
	inline
	void shareParameters(ReferenceMap & rmap) {
		return rmap.append(parameters);
	};
	*/
	inline
	void setParameters(std::initializer_list<std::pair<const char *, const Variable> > l){
		for (const auto & entry: l){
			setParameter(entry.first, entry.second);
			//parameters[entry.first] = entry.second;
		}
	}

	/// Sets comma-separated parameters in a predetermined order "a,b,c" or by specifing them "b=2".
	/**
	 *  This function is virtual because derived classes may redefine it to update str members.
	 */
	virtual inline
	void setParameters(const std::string &p, char assignmentSymbol='=', char separatorSymbol=0){
		parameters.setValues(p, assignmentSymbol, separatorSymbol);
		updateBean();
	};

	/// Set parameters
	/**
	 */
	template <class T>
	inline
	void setParameters(const std::map<std::string,T> & p){
		parameters.importMap(p);
		updateBean();
	}

	/// Set parameters
	/**
	 */
	template <class T>
	inline
	void setParameters(const SmartMap<T> & p){
		parameters.importCastableMap(p);
		updateBean();
	}

	/// Sets a single parameter
	inline
	void setParameter(const std::string &p, const Castable & value){
		parameters[p].assignCastable(value);
		updateBean();
	}

	/// Sets a single parameter
	inline
	void setParameter(const std::string &p, const Variable & value){
		parameters[p].assignCastable(value);
		updateBean();
	}

	/// Sets a single parameter
	inline
	void setParameter(const std::string &p, const Referencer & value){
		parameters[p].assignCastable(value);
		updateBean();
	}

	/// Sets a single parameter
	template <class F>
	inline
	void setParameter(const std::string &p, const F &value){
		parameters[p] = value;
		updateBean();
	}

	/// Sets a single parameter
	template <class F>
	inline
	void setParameter(const std::string &p, std::initializer_list<F> value){
		parameters[p] = value;
		updateBean();
	}

	inline
	BeanLike & operator=(const BeanLike & b){ //
		//copy(b);
		parameters.importMap(b.getParameters()); // 2021
		updateBean(); // ADDED 2021/10
		return *this;
	}


	virtual
	std::ostream & toStream(std::ostream & ostr, bool compact = true) const;

	/*
	virtual inline
	std::ostream & toStream(std::ostream & ostr) const {
		//ostr << name << ':' << parameters;
		ostr << name << ": " << description << '\n';
		ostr << '\t' << parameters << '\n';
		return ostr;
	}
	*/


protected:


	BeanLike(const std::string & name, const std::string & description="") : name(name), description(description) {
	}

	BeanLike(const BeanLike & b) : name(b.name), description(b.description){
		// copy(b);
		parameters.copyStruct(b.getParameters(), b, *this, ReferenceMap::RESERVE);
	}


	const std::string name;  // todo separate (Beanlet)

	const std::string description; // todo separate (Beanlet)

	ReferenceMap parameters;  // todo separate (Beanlet)

	/// Called after setParameters()
	virtual inline
	void updateBean() const {};

	// Copy variables
	/*
	void copy(const BeanLike & b){
		parameters.importMap(b.getParameters());
	}
	*/


};


inline
std::ostream & operator<<(std::ostream &ostr, const BeanLike & bean){
	bean.toStream(ostr);
	return ostr;
}

}


#endif

// Drain
