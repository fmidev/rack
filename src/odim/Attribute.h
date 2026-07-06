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
#ifndef ODIM_ATTRIBUTE
#define ODIM_ATTRIBUTE


#include <drain/Log.h>
#include <ostream>
//#include <cmath>
#include <string>

#include <drain/util/EnumFlagger.h>
#include <drain/Convert.h>

#include <odim/Group.h>



namespace odim {

/*
 *
 \code

 for (const odim::ODIMvariable variable: {"where:BBOX", "where:EPSG", "where:xsize", "where:ysize"}){
	mout.pending<LOG_WARNING>(variable, " = ", variable.getGroupId(), '|', variable.getGroupName(), ':', variable.getKey());
	if (src.properties.hasKey(variable)){
		//mout.accept<LOG_WARNING>(variable, " = ", src.properties[variable]);
		mouseGroup->set(std::string("data-")+variable.getKey(), src.properties[variable]);
	}
}
 \endcode

*/

class ODIMvariable {

public:

	inline
	ODIMvariable(){
		group.set(GroupId::HOW);
	}

	/// Initialise with an argument of type  \c <group>:<key>, for example \c what:source .
	/**
	 *  Split by ':'
	 */
	template <typename T>
	inline
	ODIMvariable(const T & value){
		assign(value);
	}

	/// Initialise with group and key for example \c WHAT or "what", and \c "source" .
	template <typename T>
	inline
	ODIMvariable(const T & group, const std::string & key){
		assign(group, key);
	}


	inline
	void assign(GroupId g, const std::string & k){
		group.set(g);
		key.assign(k);
		fullName = drain::StringBuilder<':'>(g, k);
	}

	inline
	void assign(std::string &g, const std::string & k){
		group.set(g);
		key.assign(k);
		fullName = drain::StringBuilder<':'>(g, k);
	}

	inline
	void assign(const std::string & value){
		std::string g;
		drain::StringTools::split2(value, g, key, ':');
		group.set(g);
		fullName = drain::StringBuilder<':'>(g, key);
		// fullName =  value; // No double-check?
	}


	inline
	GroupId getGroupId() const {
		return group.getValue();
	}

	inline
	const std::string & getGroupName() const {
		return group.str();
	}

	inline
	const std::string & getKey() const {
		return key;
	}

	inline
	const std::string & str() const {
		return fullName;
	}

	operator const std::string & () const {
		return fullName;
	}

protected:

	ODIMgroup group;
	std::string key;
	std::string fullName;

};

inline
std::ostream & operator<<(std::ostream & ostr, const odim::ODIMvariable & variable){
	ostr << variable.getGroupName() << ':' << variable.getKey();
	return ostr;
}

}




#endif

// Rack
