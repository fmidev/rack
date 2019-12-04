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

//#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <cmath>  // for NaN
#include <vector>



#include "Type.h"
#include "TypeUtils.h" // for typesetter



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
		//std::cerr << "Caster::  warning: copy const;" << std::endl;
		setType(c.getType());
		ptr = c.ptr;
	};

	inline
	void link(void *p, const std::type_info &t){
		ptr = p;
		setType(t);
	}

	template <class T>
	inline
	void link(T *p){
		link(p, typeid(T));
	}

	template <class T>
	inline
	void link(T &p){
		link(&p, typeid(T));
	}


	/// Calls setType<T>() for which typeid(T) = t.
	/**
	 *  Caster implements setType<T>(), hence this utility can be applied.
	 */
	inline
	void setType(const std::type_info &t){
		Type::call<drain::typesetter>(*this, t);
	}

	/// Sets pointers to the type-dependent conversion functions.
	//
	// Note: updateType planned to be renamed "directly" to setType?
	template <class F>
	inline
	void setType(){
		updateType<F>();
	}

	/// Utility for derived classes, which may be able to adapt memory array type as well.
	virtual inline
	bool requestType(const std::type_info & t){
		return getType() == t;
	}

	/// calls void setType<void>().
	void unsetType(); //  inline not possible here due to template<> (?)


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
			//std::cerr << "Caster::put(p, T x): stringstream conversion: " << x << "=>" << sstr.str() << std::endl;
			put(p, sstr.str());
		}
		else {
			throw std::runtime_error(std::string("Caster::put(void *, const T & ), type unset"));
		}
	}

	/// Write to internal pointer using put(this->ptr, x).
	template <class T>
	inline
	void put(const T & x) const {  // NEW
		put(this->ptr, x);
	}

	// Consider assigning default type (std::string?)
	/*
	 *  Notice that assigning to \c char is potentially ambiguous.
	 *  char c;
	 *  put(&c, "67");  // 6 or 67 or ascii('6') ?
	 *  put(&c, "C");   // 0 or 67 == ascii('C') ?
	 */
	void put(void *p, const char * x) const;

	/// Write to internal pointer, calls put(this->ptr, x).
	inline
	void put(const char * x) const {  // NEW
		put(this->ptr, x);
	}


	/// Default implementation throws an error. See specialized implementation below.
	template <class T>
	T get(const void *p) const {
		throw std::runtime_error(std::string("Caster::get() unimplemented output type: ") + typeid(T).name());
		return T();
	}

	template <class T>
	inline
	T get() const {  // NEW
		return get<T>(this->ptr);
	}


	/// Convert from str pointer and Caster.
	inline 
	void translate(const Caster &c, const void *ptrC, void *ptr) const {
		//(this->*translatePtr)(c, ptrC, ptr);
		(*translatePtr)(c, ptrC, ptr);
	};


	/// Convert from str Caster.
	inline
	void translate(const Caster &c) const { // NEW
		//(this->*translatePtr)(c, c.ptr, this->ptr);
		(*translatePtr)(c, c.ptr, this->ptr);
	};


	/// Write data to output stream
	inline
	std::ostream & toOStream(std::ostream &ostr, const void *p) const {
		//return (this->*toOStreamPtr)(ostr,p);
		return (*toOStreamPtr)(ostr,p);
	}

	/// Write data to output stream
	inline
	std::ostream & toOStream(std::ostream &ostr) const {  // NEW
		//return (this->*toOStreamPtr)(ostr, this->ptr);
		return (*toOStreamPtr)(ostr, this->ptr);
	}

	/// Future member: enables setting Caster type.
	void *ptr; // NEW, visible for CastableIterator

protected:


	/// Sets pointers to the type-dependent conversion functions.
	template <class F>
	void updateType();


	//void updateCasterType();

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


	/// Convert from str pointer and Caster.
	//void (Caster::* translatePtr)(const Caster &c, const void *ptrC, void *ptr) const;
	void (* translatePtr)(const Caster &c, const void *ptrC, void *ptr);

	/// Write to stream.
	// std::ostream & (Caster::* toOStreamPtr)(std::ostream &ostr, const void *p) const;
	std::ostream & (* toOStreamPtr)(std::ostream &ostr, const void *p);


	/// Convert input of base type to internally applied base type
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
	static inline
	void putToVoidT(void *p, const T &x) {
	}

	/// A handler for converting input to a std::string.
	/**
	 *  \tparam T - input type
	 *
	 *  The internal storage type is std::string.
	 */
	template <class T>
	static inline
	void putToStringT(void *p, const T &x) {
		std::stringstream sstr;
		sstr << x;
		*(std::string*)p = sstr.str();
	}


	// Future member: enables forwarding value x to a Caster.
	template <class T>
	static inline
	void putToCasterT(void *p, const T &x) {
		Caster &c = *(Caster *)p;
		// std::cerr << __FUNCTION__ << "(p, T x): x="   << x << std::endl;
		// std::cerr << __FUNCTION__ << "(p, T x): dst=" << c.getType().name() << std::endl;
		c.put(c.ptr, x);
		// std::cerr << __FUNCTION__ << "(p, T x): (string)" << c.get<std::string>(c.ptr) << std::endl;
		// std::cerr << __FUNCTION__ << "(p, T x): (double)" << c.get<double>(c.ptr) << std::endl;
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

	/// This default implementation is for integral numeric types, returns zero. Specialisation for floats returns NAN.
	/**
	 *
	 *
	 *
	 */
	template <class T>
	static inline
	T getFromVoidT(const void *p){
		static const T t(0);
		//static const T t(NAN);
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

	// Future member: enables forwarding from a Caster.
	/*
	template <class T>
	static inline
	T getFromCasterT(const void *p) {
		const Caster &c = *(const Caster *)p;
		//std::cerr << c.getType().name() << " <- " << x << std::endl;
		return c.get<T>(c.ptr); // or even c.get<T>()
	}
	*/


	/// Casts from ptrC to ptr
	/**
	 *   Given data of type c.getType() behind pointer ptrC, cast it to type F and store it behind ptr.
	 */
	template <class F> // STATIC!
	//inline
	static
	void translateT(const Caster &c, const void *ptrC, void *ptr) { // const {
		//std::cerr << "Caster::" << __FUNCTION__ << ": " << c.getType().name() << std::endl;
		*(F *)ptr = c.get<F>(ptrC);
	}

	/// New
	template <class F>
	static
	std::ostream & toOStreamT(std::ostream & ostr, const void *p) { //const {
		if (p != NULL)
			ostr << *(F*)p;
		return ostr;
	}

	template <class F>
	static
	std::ostream & toOStreamFloats(std::ostream & ostr, const void *p){

		if (!p)
			throw std::runtime_error("Caster::toOStreamT<double>:: null ptr as argument");

		const F d = *(F *)p;

		if (std::isnan(d)){
			ostr << "NaN";  // JSON std literal
		}
		else if (d == rint(d)){
			//ostr << std::ios::fixed << std::setprecision(2) << d;

			const std::streamsize prec = ostr.precision();
			const std::ios_base::fmtflags flags = ostr.setf(std::ios::fixed, std::ios::floatfield );
			ostr.precision(1);
			ostr << d;
			ostr.setf(flags);
			ostr.precision(prec);

		}
		else {
			ostr << d;
		}

		return ostr;

	}


	/// Current type.
	const std::type_info *type;

	/// Size of target memory (target object) in bytes.
	size_t byteSize;

};

template <>
inline
void Caster::link(Caster &c){
	link(c.ptr, c.getType());
}


template <>
void Caster::updateType<void>();

template <>
void Caster::updateType<std::string>();

/// Forward pointer
template <>
void Caster::updateType<Caster>();


template <class F>
void Caster::updateType(){

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



	toOStreamPtr = & Caster::toOStreamT<F>;
	translatePtr = & Caster::translateT<F>;
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
	else if (getType() == typeid(bool)){
		return *(const bool *)p ? "true" : "false";
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
template <>
inline
void Caster::translateT<void>(const Caster &c, const void *ptrC, void *ptr){ // const {
}

// Writes to buffer of Caster pointed by ptr.
template <>
inline
void Caster::translateT<Caster>(const Caster &c, const void *ptrC, void *ptr){ // const {
	Caster & c2 = *(Caster *)ptr;
	c2.translate(c, ptrC, c2.ptr);
	//*(F *)ptr = c.get<F>(ptrC);
}

/// Specialisation for floats returns NAN.
template <>
inline
double Caster::getFromVoidT<double>(const void *p){
	return NAN;
}


// Specialisation for floats returns NAN.
template <>
inline
float Caster::getFromVoidT<float>(const void *p){
	return NAN;
}


// VOID
/// Append nothing to the stream.
template <>
inline
std::ostream & Caster::toOStreamT<void>(std::ostream &ostr, const void *p){ // const {
	return ostr;
}

// BOOL
/// Append 'true' or false to the stream.
template <>
inline
std::ostream & Caster::toOStreamT<bool>(std::ostream &ostr, const void *p){
	ostr << (*(const bool *)p ? "true" : "false");
	return ostr;
}

/// Append  to the stream.
template <>
inline
std::ostream & Caster::toOStreamT<Caster>(std::ostream &ostr, const void *p){ // const {
	const Caster &c = *(const Caster *)p;
	c.toOStream(ostr);
	return ostr;
}

// Yes, to get trailing period and zero: .0
template <>
inline
std::ostream & Caster::toOStreamT<float>(std::ostream & ostr, const void *p){
	return toOStreamFloats<float>(ostr, p);
}

// Yes, to get trailing period and zero: .0
template <>
inline
std::ostream & Caster::toOStreamT<double>(std::ostream & ostr, const void *p){
	return toOStreamFloats<double>(ostr, p);
}



}  // namespace drain


#endif

// Drain
