/*
 * JavaScript.h
 *
 *  Created on: Jan 26, 2026
 *      Author: mpeura
 */

#ifndef DRAIN_UTIL_OUTLET
#define DRAIN_UTIL_OUTLET

#include <ostream>

//#include <drain/Sprinter.h>


namespace drain {


class Outlet {

public:

	inline
	Outlet(std::ostream & ostr) : dst_ostr(ostr), dst_str(dummy_str){
	}

	inline
	Outlet(std::string & s) : dst_ostr(sstr), dst_str(s){
	}

	template <class T>
	inline
	std::ostream & operator<<(const T & x) const {
		dst_ostr << x;
		return dst_ostr;
	}

	inline
	~Outlet(){
		//if (&dst_ostr == &sstr){
		if (&dst_str != &dummy_str){
			dst_str.assign(sstr.str());
		}
	}

	inline
	std::ostream & getOStream() const {
		return dst_ostr;
	}

protected:

	std::ostream & dst_ostr;
	std::string  & dst_str;

	mutable
	std::string dummy_str;

	mutable
	std::stringstream sstr;

};




} // drain::

#endif // DRAIN_OUTLET
