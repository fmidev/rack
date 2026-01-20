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
#include <drain/TypeUtils.h>
#include "Caster.h"

// // using namespace std;

namespace drain {



void Caster::put(void *p, const char * x) const {

	const std::type_info & t = getType();

	if (t != typeid(void)){

		if (t == typeid(std::string)){
			//std::cout << "put(void *p, const char * x) => string\n";
			*(std::string *)p = x;
		}
		else {

			//  std::cout << "put(void *p, const char * x=" << x <<") => double => "<< getType().name() << "\n";
			//  Initialization important, because x maybe empty or str non-numeric std::string.
			std::stringstream sstr;
			sstr << x;

			/*
				if (t == typeid(Caster)){
					putToCasterT(p, sstr.str());
				}
				else */
			if ((t == typeid(float))|| (t == typeid(double))){
				double y = 0.0;
				sstr >> y;
				(this->putDouble)(p, y);
			}
			else if (t == typeid(bool)){
				// TODO: test against "true" and "TRUE"
				if (TypeUtils::trueRegExp.test(x)){
					(this->putBool)(p, true);
				}
				else if (TypeUtils::falseRegExp.test(x)){
					(this->putBool)(p, false);
				}
				else
					(this->putBool)(p, atof(x));
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
}


//void Caster::unupdateType(){
template <>
void Caster::updateType<void>(){

	type = &typeid(void);
	byteSize = 0;

	putBool   = & Caster::putToVoidT<bool>;
	putInt    = & Caster::putToVoidT<int>;
	putChar   = & Caster::putToVoidT<char>;
	putUChar  = & Caster::putToVoidT<unsigned char>;
	putUInt   = & Caster::putToVoidT<unsigned int>;
	putShort  = & Caster::putToVoidT<short>;
	putUShort = & Caster::putToVoidT<unsigned short>;
	putLong   = & Caster::putToVoidT<long int>;
	putULong  = & Caster::putToVoidT<unsigned long>;
	putFloat  = & Caster::putToVoidT<float>;
	putDouble = & Caster::putToVoidT<double>;

	getBool   = & Caster::getFromVoidT<bool>;
	getInt    = & Caster::getFromVoidT<int>;
	getChar   = & Caster::getFromVoidT<char>;
	getUChar  = & Caster::getFromVoidT<unsigned char>;
	getUInt   = & Caster::getFromVoidT<unsigned int>;
	getShort  = & Caster::getFromVoidT<short>;
	getUShort = & Caster::getFromVoidT<unsigned short>;
	getLong   = & Caster::getFromVoidT<long>;
	getULong  = & Caster::getFromVoidT<unsigned long>;
	getFloat  = & Caster::getFromVoidT<float>;
	getDouble = & Caster::getFromVoidT<double>;

	// FIX
	// ! fromStreamPtr = ...
	comparePtr   = & Caster::compareT<void>;
	toOStreamPtr = & Caster::toOStreamT<void>;
	translatePtr = & Caster::translateT<void>;

	return;

}

void Caster::unsetType(){
	Caster::updateType<void>();
}


template <>
void Caster::updateType<std::string>(){

	type = &typeid(std::string);
	byteSize = sizeof(std::string)/sizeof(char);  // Varies... getElementSize() could retrieve

	// NEW
	putBool   = & Caster::putToStringT<bool>;
	putInt    = & Caster::putToStringT<int>;

	putChar   = & Caster::putToStringT<char>;
	putUChar  = & Caster::putToStringT<unsigned char>;
	// putInt    = & Caster::_putToString<int>;
	putUInt   = & Caster::putToStringT<unsigned int>;
	putShort  = & Caster::putToStringT<short>;
	putUShort = & Caster::putToStringT<unsigned short>;
	putLong   = & Caster::putToStringT<long int>;
	putULong  = & Caster::putToStringT<unsigned long>;
	putFloat  = & Caster::putToStringT<float>;
	putDouble = & Caster::putToStringT<double>;

	getBool   = & Caster::getFromStringT<bool>;
	getInt    = & Caster::getFromStringT<int>;

	getChar   = & Caster::getFromStringT<char>;
	getUChar  = & Caster::getFromStringT<unsigned char>;
	//getInt    = & Caster::_getFromString<int>;
	getUInt   = & Caster::getFromStringT<unsigned int>;
	getShort  = & Caster::getFromStringT<short>;
	getUShort = & Caster::getFromStringT<unsigned short>;
	getLong   = & Caster::getFromStringT<long>;
	getULong  = & Caster::getFromStringT<unsigned long>;
	getFloat  = & Caster::getFromStringT<float>;
	getDouble = & Caster::getFromStringT<double>;

	comparePtr   = nullptr; //& Caster::compareT<std::string>;

	toOStreamPtr = & Caster::toOStreamT<std::string>;
	translatePtr = & Caster::translateT<std::string>;


	return;

}

/*
std::ostream & Caster::toOStreamFloats(std::ostream & ostr, const void *p){

	if (!p)
		throw std::runtime_error("Caster::toOStreamT<double>:: null ptr as argument");

	const double d = *(double *)p;

	if (d == rint(d)){
		const std::streamsize prec = ostr.precision();
		const std::ios_base::fmtflags flags = ostr.setf(std::ios::fixed, std::ios::floatfield );
		ostr.precision(1);
		ostr << d;
		ostr.setf(flags);
		ostr.precision(prec);
	}
	else
		ostr << d;

	return ostr;
}
*/


}  // drain::


