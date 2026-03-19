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
#ifndef DRAIN_TYPE_NAME
#define DRAIN_TYPE_NAME

#include <iostream>
#include <stdexcept>
#include <typeinfo>
// #include <bits/types.h> // uint8, uint16 etc
#include <cstdint>

#include <list>
#include <set>
#include <vector>



namespace drain {




/// Default implementation
/*

*/
template <typename T>
struct TypeName
{

	static const std::string & str();

	/*
    static const std::string & str(){
        return name;
    }

protected:

    static const std::string name;
	*/
	static const std::string nameOLD;

};

template <typename T>
const std::string & TypeName<T>::str(){
	static
	const std::string name(typeid(T).name());
	return name;
}

/// Default implementation: name returned by std::type_info::name()
/*
template <typename T>
const std::string TypeName<T>::name(typeid(T).name());
*/



/// Default implementation
/*

*/
template <typename T>
inline
std::ostream & operator<<(std::ostream & ostr, const TypeName<T> &t){
	ostr << t.str();
	return ostr;
}


/// Name declaration, for header files.
//#define DRAIN_TYPENAME(tname) template <>        const std::string TypeName<tname>::name
/// Name definition, for object files.
#define DRAIN_TYPENAME_DEF(tname) template <>    const std::string TypeName<tname>::nameOLD(#tname)

//#define DRAIN_TYPENAME_DEF(tname) bool x;


//#define DRAIN_TYPENAME_T(cname,T) template <class T> struct TypeName<cname<T> > {static const std::string & str(){static const std::string n=drain::StringBuilder<>(#cname, '<', drain::TypeName<T>::str(),'>'); return name;}
#define DRAIN_TYPENAME_T(cname,T)  template <class T> struct TypeName<cname<T> > {static const std::string & str(){static const std::string n=drain::StringBuilder<>(#cname, '<', drain::TypeName<T>::str(),'>'); return n;}}
#define DRAIN_TYPENAME_T0(cname,T) template <class T> struct TypeName<cname<T> > {static const std::string & str(){static const std::string n=std::string(#cname)+'<'+drain::TypeName<T>::str()+'>'; return n;}}

// NEW
/// Simple functor redefinition, only
// #define DRAIN_TYPENAME(tname)   template <>  inline const std::string & drain::TypeName<tname>::str(){static const std::string n(#tname); return n;};

// TEMPORARY FIX (fake)
#define DRAIN_TYPENAME(tname)       template <>  inline const std::string & drain::TypeName<tname>::str(){static const std::string n(#tname); return n;};
#define DRAIN_TYPENAME_t(tname) template <>  inline const std::string & drain::TypeName<tname##_t>::str(){static const std::string n(#tname); return n;};


/// Add a specialization for each type of those you want to support.
//  (Unless the string returned by typeid is sufficient.)


// DRAIN_TYPENAME(uint16_t);




// NEW
DRAIN_TYPENAME(char);
DRAIN_TYPENAME_t(int8);
DRAIN_TYPENAME_t(uint8);
DRAIN_TYPENAME_t(int16);
DRAIN_TYPENAME_t(uint16);
DRAIN_TYPENAME_t(int32);
DRAIN_TYPENAME_t(uint32);
DRAIN_TYPENAME_t(int64);
DRAIN_TYPENAME_t(uint64);

DRAIN_TYPENAME(void);
DRAIN_TYPENAME(bool);
DRAIN_TYPENAME(float);
DRAIN_TYPENAME(double);
// NEWOLD
DRAIN_TYPENAME(char *);
DRAIN_TYPENAME(const char *);  // why const separately...?
DRAIN_TYPENAME(std::string);

// OLD
/*
DRAIN_TYPENAME(char);
DRAIN_TYPENAME(unsigned char);
DRAIN_TYPENAME(short);
DRAIN_TYPENAME(unsigned short);
*/
/*
DRAIN_TYPENAME(void);
DRAIN_TYPENAME(bool);

DRAIN_TYPENAME(int);  // see what happend
DRAIN_TYPENAME(unsigned int);
DRAIN_TYPENAME(long);
DRAIN_TYPENAME(unsigned long);
DRAIN_TYPENAME(float);
DRAIN_TYPENAME(double);
DRAIN_TYPENAME(char *);
DRAIN_TYPENAME(const char *);  // why const separately...?
DRAIN_TYPENAME(std::string);
*/

DRAIN_TYPENAME_T0(std::initializer_list, T);
DRAIN_TYPENAME_T0(std::vector, T);

/*
template <typename T>
struct TypeName<std::initializer_list<T> > {

	static const std::string & str(){
		static std::string name;
		if (name.empty()){
			name = "std::initializer_list<";
			name += drain::TypeName<T>::str();
			name += ">";
			//name = drain::StringBuilder("std::initializer_list<", drain::TypeName<T>::get(), ">");
		}
		return name;
	}

};


template <typename T>
struct TypeName<std::vector<T> > {

	static const std::string & str(){
		static std::string name;
		if (name.empty()){
			name = "std::vector<"; // + drain::TypeName<T>::get() + ">";
			name += drain::TypeName<T>::str();
			name += ">";
		}
		return name;
	}

};
*/

/*
namespace std {
	template <typename K, typename V>
	class map<K,V>;
}
*/



/// Returns the basic type (integer, float, bool, string, void) as a string.
/**
 *  Usage:
 *  \code
 *  #include "Type.h"
 *  Type::call<drain::simpleName>(t);
 *  \endcode
 *
 *  \see drain::Type
 */
class simpleName {

public:

	typedef std::string value_t;
	// typedef const std::string & value_t;

	/**
	 *  \tparam S - type to be analyzed (argument)
	 *  \tparam T - return type  (practically value_t)
	 */
	template <class S, class T>
	static
	T callback(){
		//return TypeName<S>::get();
		return TypeName<S>::str();
	}


};



} // drain::

#endif
