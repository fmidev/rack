/**

    Copyright 2011-2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef CASTER
#define CASTER

#include <typeinfo>
#include <stdexcept>
#include <iostream>
//#include <limits>
#include <vector>
//#include <string>

#include "Type.h"



// // using namespace std;

namespace drain {



/// Reads and writes values in memory using stored type - base types (char, int, double...) or std::string.
/**
 *  Caster uses a pointer provided by the user. The pointer points to a memory segment
 *  that stores base type objects (char, int, ...). The type may change dynamically.
 *
 *  The implementation is based on function pointers to predefined template functions performing casts between
 *  base types.
 *
 *  As such, does not support arrays. See derived classes:
 *  - Castable
 *  - CastableIterator
 *
 *  \example Caster-example.cpp
 */
class Caster {


	//protected:
public:

	/// Default constructor. Leaves the type undefined.
	/** Initially, type is void, which can be indentified
	 *  with an unset type or a type requiring storage of zero length.
	 */
	inline
	Caster (){
		unsetType();
	};

	/// Copy constructor. Copies the type.
	/**
	 */
	inline
	Caster (const Caster &c){
		std::cerr << "Caster::  warning: copy const;" << std::endl;
		//unsetType();
		setType(c.getType()); // WHY not ok?
	};


	// void setType(const std::type_info &t);

	inline
	void setType(const std::type_info &t){
		if (t == typeid(void))
			unsetType();
		else {
			Type::call<typesetter>(*this, t);
		}
	}

	// Use Type::typesetter<Caster> instead?
	class typesetter {
	public:
		template <class T>
		static
		inline
		void callback(Caster & c){
			c.setType<T>();
		}

	};

	///
	//   Implementation at the bottom.
	template <class F>
	void setType();

	/// VOID 
	void unsetType();

	/// Returns type_info of the current type.
	inline
	const std::type_info & getType() const { return *type; };

	inline
	bool typeIsSet() const { return type != &typeid(void); };

	/// Returns the size of the base type (size of an element, not of element array).
	inline
	size_t getByteSize() const { return byteSize; };


	/// Default conversion (for unconventional types). Uses std::stringstream for conversion.
	//  => See the base type implementations below.
	template <class T>
	void put(void *p, const T & x) const { 
		if (typeIsSet()){
			std::stringstream sstr;
			sstr << x;
			put(p, sstr.str());
		}
		else {
			throw std::runtime_error(std::string("Caster::put(void *, const T & ), type unset"));
		}
	}


	// Consider assigning default type (std::string?)
	/*
	 *  Notice that assigning to \c char is potentially ambiguous.
	 *  char c;
	 *  put(&c, "67");  // 6 or 67 or ascii('6') ?
	 *  put(&c, "C");   // 0 or 67=ascii('C') ?
	 */
	inline
	void put(void *p, const char * x) const {
		if (typeIsSet()){
			const std::type_info & t = getType();
			if (t == typeid(std::string)){
				//std::cout << "put(void *p, const char * x) => string\n";
				*(std::string *)p = x;
			}
			else {
				//std::cout << "put(void *p, const char * x=" << x <<") => double => "<< getType().name() << "\n";
				// Initialization important, because x maybe empty or other non-numeric std::string.
				std::stringstream sstr;
				sstr << x;

				//if (Type::call<drain::typeIsInteger>(t)){
				if ((t == typeid(float))|| (t == typeid(double))){
					double y = 0.0;
					sstr >> y;
					(this->putDouble)(p, y);
				}
				else {
					long y = 0;
					sstr >> y;
					(this->putLong)(p, y);
				}
				//std::cerr << "Caster::put(p, const char* x) d=" << d << std::endl;
				//(this->*putDouble)(p, d);

			}
		}
		else {
			throw std::runtime_error(std::string("Caster::put(void *, const char *), type unset"));
		}
	};

	/// Default implementation throws an error. See specialized implementation below.
	template <class T>
	T get(const void *p) const {
		throw std::runtime_error(std::string("Caster::get() unimplemented output type: ") + typeid(T).name());
		return T();
	}


	/// Convert from other pointer and Caster.
	inline 
	void cast(const Caster &c, const void *ptrC, void *ptr) const {
		(this->*castP)(c, ptrC, ptr);
	};


	/// New
	inline
	std::ostream & toOStream(std::ostream &ostr, const void *p) const {
		return (this->*toOStreamP)(ostr,p);
	}


protected:




	// New / experimental
	void (* putBool)(void *p, const bool &x);
	void (* putInt)(void *p, const int &x);
	void (* putChar)(void *p, const char &x);
	void (* putUChar)(void *p, const unsigned char &x);
	void (* putUInt)(void *p, const unsigned int &x);
	void (* putShort)(void *p, const short &x);
	void (* putUShort)(void *p, const unsigned short &x);
	void (* putLong)(void *p, const long &x);
	void (* putULong)(void *p, const unsigned long &x);
	void (* putFloat)(void *p, const float &x);
	void (* putDouble)(void *p, const double &x);


	char (* getChar)(const void *p);
	unsigned char (* getUChar)(const void *p);
	int  (* getInt)(const void *p);
	unsigned int (* getUInt)(const void *p);
	short (* getShort)(const void *p);
	unsigned short (* getUShort)(const void *p);
	long (* getLong)(const void *p);
	unsigned long (* getULong)(const void *p);
	float (* getFloat)(const void *p);
	double (* getDouble)(const void *p);

	bool (* getBool)(const void *p);


	/// Convert from other pointer and Caster.
	void (Caster::* castP)(const Caster &c, const void *ptrC, void *ptr) const;

	/// Write to stream.
	std::ostream & (Caster::* toOStreamP)(std::ostream &ostr, const void *p) const;


	/*
	 *  \tparam T - outer type
	 *  \tparam F - inner type
	 */
	template <class T,class F>
	static inline
	void putT(void *p, const T &x) {
		*(F*)p = static_cast<F>(x);
	}


	/// Void - does nothing, regardless of input type.
	template <class T>
	inline static
	void putToVoidT(void *p, const T &x) {
	}

	/// A handler for converting input to a std::string.
	/**
	 *  \tparam T - input type
	 *
	 *  The internal storage type is std::string.
	 */
	template <class T>
	inline static
	void putToStringT(void *p, const T &x) {
		std::stringstream sstr;
		sstr << x;
		*(std::string*)p = sstr.str();
	}

	/// The main handler converting input to output.
	/**
	 *  \tparam T - output type
	 *  \tparam F - internal storage type
	 */
	template <class T,class F>
	static inline
	T getT(const void *p) {
		return static_cast<T>(*(F*)p);
	}

	/// Void - returns an empty/default value.
	template <class T>
	static inline
	T getFromVoidT(const void *p){
		static const T t(0);
		return t;
	}

	/// The main handler converting input to output.
	/*
	 *  \tparam T - output type
	 *
	 *  The internal storage type is std::string.
	 *
	 */
	template <class T>
	static inline
	T getFromStringT(const void *p) {
		T x(0); // why (0) ?
		std::stringstream sstr;
		sstr << *(const std::string *)p;
		sstr >> x;
		return x;
	}


	/// Casts from ptrC to ptr
	template <class F>
	inline
	void _cast(const Caster &c, const void *ptrC, void *ptr) const {
		*(F *)ptr = c.get<F>(ptrC);
	}

	/// New
	template <class F>
	std::ostream & _toOStream(std::ostream & ostr, const void *p) const {
		if (p != NULL)
			ostr << *(F*)p;
		return ostr;
	}

	/// Current type.
	const std::type_info *type;

	/// Typically 1 or 2 (8 and 16 bits).
	size_t byteSize;

};





//template <>
//void Caster::setType<void>();
// VOID	unsetType();



template <>
void Caster::setType<std::string>();

template <>
void Caster::setType<void>();


template <class F>
void Caster::setType(){

	type = & typeid(F);
	byteSize = sizeof(F)/sizeof(char);

	/// NEW
	putBool   = & Caster::putT<bool,F>;
	putInt    = & Caster::putT<int,F>;
	putChar   = & Caster::putT<char,F>;
	putUChar  = & Caster::putT<unsigned char,F>;
	putUInt   = & Caster::putT<unsigned int,F>;
	putShort  = & Caster::putT<short,F>;
	putUShort = & Caster::putT<unsigned short,F>;
	putLong   = & Caster::putT<long int,F>;
	putULong  = & Caster::putT<unsigned long,F>;
	putFloat  = & Caster::putT<float,F>;
	putDouble = & Caster::putT<double,F>;

	getBool   = & Caster::getT<bool,F>;
	getInt    = & Caster::getT<int,F>;
	getChar   = & Caster::getT<char,F>;
	getUChar  = & Caster::getT<unsigned char,F>;
	getUInt   = & Caster::getT<unsigned int,F>;
	getShort  = & Caster::getT<short,F>;
	getUShort = & Caster::getT<unsigned short,F>;
	getLong   = & Caster::getT<long,F>;
	getULong  = & Caster::getT<unsigned long,F>;
	getFloat  = & Caster::getT<float,F>;
	getDouble = & Caster::getT<double,F>;



	toOStreamP = & Caster::_toOStream<F>;
	castP      = & Caster::_cast<F>;
}

// VOID
/// Append nothing to the stream.
template <>
inline
std::ostream & Caster::_toOStream<void>(std::ostream &ostr, const void *p) const {
	return ostr;
}

template <>
inline
void Caster::put<bool>(void *p, const bool & x) const {
	//(this->*putBool)(p,x);
	(this->putBool)(p,x);
}

template <>
inline
void Caster::put<char>(void *p, const char & x) const {
	(this->putChar)(p,x);
}

template <>
inline
void Caster::put<unsigned char>(void *p, const unsigned char & x) const {
	(this->putUChar)(p,x);
}

template <> inline
void Caster::put<int>(void *p, const int & x) const {
	// (this->putInt)(p,x);
	(this->putInt)(p,x);
}

template <> inline
void Caster::put<unsigned int>(void *p, const unsigned int & x) const {
	(this->putUInt)(p,x);
}

template <> inline
void Caster::put<long>(void *p, const long & x) const {
	(this->putLong)(p,x);
}

template <> inline
void Caster::put<unsigned long>(void *p, const unsigned long & x) const {
	(this->putULong)(p,x);
}

template <> inline
void Caster::put<float>(void *p, const float& x) const {
	(this->putFloat)(p,x);
}

template <> inline
void Caster::put<double>(void *p, const double & x) const {
	(this->putDouble)(p,x);
}

template <> inline
void Caster::put<std::string>(void *p, const std::string & x) const {
	//put(p, x.c_str());  Dangerous, if F = std::string

	//std::cerr << "Laita std::string x=" << x << std::endl;
	if (getType() == typeid(std::string))
		*(std::string *)p = x;
	else if (getType() == typeid(char)){
		// std::cout << "note: experimental str to char\n";
		if (x.empty())
			p = NULL; //'\0'; // NULL, 0 ?
		else {
			if (x.size() > 1)
				std::cerr << "Caster::put<std::string>() warning: single-char dst, assigned 1st of multi-char '"<< x << "'\n";
			*(char *)p = x.at(0);
		}
	}
	else
		put(p, x.c_str());

}

/*
template <> inline
void Caster::toOStr(std::ostream & ostr, void *p) const {
	(this->*toOStr)(p,x);
}
 */

template <> inline
bool Caster::get<bool>(const void *p) const {
	//return (this->*getBool)(p);
	return (this->getBool)(p);
}


template <> inline
char Caster::get<char>(const void *p) const {
	return (this->getChar)(p);
}

template <> inline
unsigned char Caster::get<unsigned char>(const void *p) const {
	return (this->getUChar)(p);
}

template <> inline
short Caster::get<short>(const void *p) const {
	return (this->getShort)(p);
}

template <> inline
unsigned short Caster::get<unsigned short>(const void *p) const {
	return (this->getUShort)(p);
}

template <> inline
int Caster::get<int>(const void *p) const {
	//return (this->*getInt)(p);
	return (this->getInt)(p);
}

template <> inline
unsigned int Caster::get<unsigned int>(const void *p) const {
	return (this->getUInt)(p);
}

template <> inline
long Caster::get<long>(const void *p) const {
	return (this->getLong)(p);
}

template <> inline
unsigned long Caster::get<unsigned long>(const void *p) const {
	return (this->getULong)(p);
}

template <> inline
float Caster::get<float>(const void *p) const {
	return (this->getFloat)(p);
}

template <> inline
double Caster::get<double>(const void *p) const {
	return (this->getDouble)(p);
}

/// Note: there is no conversion for const char * !

template <> inline
std::string Caster::get<std::string>(const void *p) const {

	// void is handled by toOStr, but this is faster.
	if (getType() == typeid(void)){
		static const std::string empty;
		return empty;
	}
	else if (getType() == typeid(char)){
		// std::cout << "note: experimental char to str\n";
		return std::string(1, *(char *)p); // NOTE: this does not handle char array of more elements!
	}
	else if (getType() == typeid(std::string)){
		return *(const std::string *)p;
	}
	else {
		std::stringstream sstr;
		toOStream(sstr, p);
		// sstr << (this->getDouble)(p);
		return sstr.str();
	}
}


// Does nothing, leaves ptr intact.
template <> inline
void Caster::_cast<void>(const Caster &c, const void *ptrC, void *ptr) const {
}






}  // namespace drain


#endif
