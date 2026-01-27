/*
 * JavaScript.h
 *
 *  Created on: Jan 26, 2026
 *      Author: mpeura
 */

#ifndef DRAIN_UTIL_JAVASCRIPT_H_
#define DRAIN_UTIL_JAVASCRIPT_H_

#include <drain/Sprinter.h>
#include <drain/util/EnumUtils.h>

namespace drain {

void super_test();

class Outlet {

public:

	Outlet(std::ostream & ostr) : dst_ostr(ostr), dst_str(dummy_str){
	}

	Outlet(std::string & s) : dst_ostr(sstr), dst_str(s){
	}

	template <class T>
	inline
	std::ostream & operator<<(const T & x) const {
		dst_ostr << x;
		return dst_ostr;
	}

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


class JavaScript {

public:

	enum NumType {
		Int8,
		Int16,
		Int32,
		Float16,
		Float32,
		Float64,
	};

	template <typename T>
	static
	void createArray(const Outlet & code, const std::string & variableName, NumType type, const T & sequence);

protected:

	static
	void assign();

};

DRAIN_ENUM_DICT(JavaScript::NumType);
DRAIN_ENUM_OSTREAM(JavaScript::NumType);


template <typename T>  // std::string & code  // std::ostream & ostr
void JavaScript::createArray(const Outlet & code, const std::string & variableName, NumType type, const T & sequence){
	code << "const " << variableName << " = new " << type << "Array(";
	Sprinter::toStream(code.getOStream(), sequence, Sprinter::jsLayout);
	code << ");";
}



} // drain::

#endif // SRC_DRAIN_UTIL_JAVASCRIPT_H_ 
