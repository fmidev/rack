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



/// Reads and writes scalar values in memory using desired base type.
/**  Caster assumes that the user provides a pointer to a memory resource
 *  that stores base type objects (char, int, ...). The type may change dynamically.
 *  The implementation is based on function pointers to predefined template functions performing casts between
 *  base types.
 *
 *  See also:
 *  - Castable
 *  - CastableIterator
 *
 *  \example Caster-example.cpp
 */
// Consider: "typedef unsigned char byte" to handle numeric values, leaving characters to char?
class Caster {


//protected:
public:

	/// Leaves the type undefined.
        /** Initially, type is void, which can be indentified
         *  with an unset type or a type requiring storage of undetectValue length. 
         */
	inline
	Caster (){
	  unsetType();
	};

	inline
	Caster (const Caster &c){
		std::cerr << "Caster::  warning: copy const;" << std::endl;
		//unsetType();
		setType(c.getType()); // WHY not ok?
	};


	// void setType(const std::type_info &t);

	inline
	void setType(const std::type_info &t){
	//void setType(const Type & t){ // allows std::type_info, char, std::string...
	//	if (t.getType() == typeid(void))
		if (t == typeid(void))
			unsetType();
		else {
			Type::callFunc<typesetter>(*this, t);
		}
	}

	class typesetter {
	public:
		template <class T>
		static
		inline
		void call(Caster & c){
			c.setType<T>();
		}

	};

	///
	//   Implementation at the bottom.
	template <class F>
	void setType();

	/// VOID 
	void unsetType();
       

	inline
	const std::type_info & getType() const { return *type; };

	inline
	bool typeIsSet() const { return type != &typeid(void); };

	/// Returns the size of the base type (size of an element, not of element array).
	inline
	const size_t & getByteSize() const { return byteSize; };


	/// General conversion (slow, because uses std::stringstream for conversion).
	// See the base type implementations below.
	template <class T>
	void put(void *p, const T & x) const { 
		if (typeIsSet()){
			std::stringstream sstr;
			sstr << x;
			put(p,sstr.str());
		}
		// TODO: alert if type unset?
	}
		//throw std::runtime_error(std::string("Caster::put() unimplemented input type: ") + typeid(T).name());

	

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
			if (getType() == typeid(std::string))
				*(std::string *)p = x;
			else {
				// Initialization important, because x maybe empty or other non-numeric std::string.
				double d = 0;
				std::stringstream sstr;
				sstr << x;
				sstr >> d;
				//std::cerr << "Caster::put(p, const char* x) d=" << d << std::endl;
				//(this->*putDouble)(p, d);
				(this->putDouble)(p, d);
			}
		}
		// throw?
	};

	template <class T>
	T get(const void *p) const {
		throw std::runtime_error(std::string("Caster::get() unimplemented output type: ") + typeid(T).name());
		return T();
	}

	
	inline 
	void cast(const Caster &c, const void *ptrC, void *ptr){
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
	// void (* putInt)(void *p,; int &x);
	void (* putUInt)(void *p, const unsigned int &x);
	void (* putShort)(void *p, const short &x);
	void (* putUShort)(void *p, const unsigned short &x);
	void (* putLong)(void *p, const long &x);
	void (* putULong)(void *p, const unsigned long &x);
	void (* putFloat)(void *p, const float &x);
	void (* putDouble)(void *p, const double &x);


	char (* getChar)(const void *p);
	unsigned char (* getUChar)(const void *p);
	//int (* getInt)(const void *p);
	unsigned int (* getUInt)(const void *p);
	short (* getShort)(const void *p);
	unsigned short (* getUShort)(const void *p);
	long (* getLong)(const void *p);
	unsigned long (* getULong)(const void *p);
	float (* getFloat)(const void *p);
	double (* getDouble)(const void *p);

	bool (* getBool)(const void *p);
	int  (* getInt)(const void *p);


	/// New?
	void (Caster::* castP)(const Caster &c, const void *ptrC, void *ptr) const;

	/// New=
	std::ostream & (Caster::* toOStreamP)(std::ostream &ostr, const void *p) const;

	


	/*
	 *  T - outer type
	 *  F - inner type
	 */
	template <class T,class F>
	static
	inline
	void putNEW(void *p, const T &x) {
		*(F*)p = static_cast<F>(x);
	}


	/// VOID - does nothing.
	template <class T>
	inline
	static
	void putToVoidNEW(void *p, const T &x) {
	}

	/// A handler for converting input to a std::string.
	/**
	 *  T - outer type
	 *  (inner type is std::string)
	 *  TODO: specialize for std::string
	 */
	template <class T>
	inline
	static
	void putToStringNEW(void *p, const T &x) {
		//*(std::string*)p = Data(x);
		std::stringstream sstr;
		sstr << x;
		*(std::string*)p = sstr.str();   // TODO: enough space?
	}

	

	template <class T,class F>
	static
	inline
	T getNEW(const void *p) {
		return static_cast<T>(*(F*)p);
	}

	/// VOID
	template <class T>
	inline
	static
	T getFromVoidNEW(const void *p){
	  static const T t(0);
	  return t;
	}

	/*
	 *  T - outer type
	 *  (inner type is std::string)
	 *
	 *  TODO: specialize for std::string
	 */
	template <class T>
	inline
	static
	T getFromStringNEW(const void *p) {
		T x(0); // why (0) ?
		std::stringstream sstr;
		sstr << *(const std::string *)p;
		sstr >> x;
		return x;
	}


	// New
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

	
	const std::type_info *type;

	/// Typically 1 or 2 (8 and 16 bits).  (??)
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
   putBool   = & Caster::putNEW<bool,F>;
   putInt    = & Caster::putNEW<int,F>;
   putChar   = & Caster::putNEW<char,F>;
   putUChar  = & Caster::putNEW<unsigned char,F>;
   putUInt   = & Caster::putNEW<unsigned int,F>;
   putShort  = & Caster::putNEW<short,F>;
   putUShort = & Caster::putNEW<unsigned short,F>;
   putLong   = & Caster::putNEW<long int,F>;
   putULong  = & Caster::putNEW<unsigned long,F>;
   putFloat  = & Caster::putNEW<float,F>;
   putDouble = & Caster::putNEW<double,F>;

   getBool   = & Caster::getNEW<bool,F>;
   getInt    = & Caster::getNEW<int,F>;
   getChar   = & Caster::getNEW<char,F>;
   getUChar  = & Caster::getNEW<unsigned char,F>;
   getUInt   = & Caster::getNEW<unsigned int,F>;
   getShort  = & Caster::getNEW<short,F>;
   getUShort = & Caster::getNEW<unsigned short,F>;
   getLong   = & Caster::getNEW<long,F>;
   getULong  = & Caster::getNEW<unsigned long,F>;
   getFloat  = & Caster::getNEW<float,F>;
   getDouble = & Caster::getNEW<double,F>;



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
	else
		put(p, x.c_str());

}

/*
template <> inline
void Caster::toOStream(std::ostream & ostr, void *p) const {
	(this->*toOStream)(p,x);
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
  // VOID
	if (getType() == typeid(void)){
	    static std::string empty;
	    return empty;
	}
	else if (getType() == typeid(std::string)){
		return *(const std::string *)p;
	}
	else {
		std::stringstream sstr;
		sstr << (this->getDouble)(p);
		return sstr.str();
		//return Data((this->*getDouble)(p));
		//(this->*putDouble)(p, Data(std::string));
	}
}


// Does nothing, leaves ptr intact.
template <> inline
void Caster::_cast<void>(const Caster &c, const void *ptrC, void *ptr) const {
}


/*
class Caster : public CasterConst {
public:
};
*/




}  // namespace drain


#endif

// Drain
