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
#include "Caster.h"

// // using namespace std;

namespace drain {

void Caster::unsetType(){

	type = &typeid(void);
	byteSize = 0;

	// NEW
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

	// NEW
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

	toOStreamP = & Caster::_toOStream<void>;
	castP      = & Caster::_cast<void>;

	return;

}

template <>
void Caster::setType<void>(){
	unsetType();
}


 
template <>
void Caster::setType<std::string>(){

	type = &typeid(std::string);
	byteSize = sizeof(std::string)/sizeof(char);  // TODO:0 ?

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

	toOStreamP = & Caster::_toOStream<std::string>;
	castP      = & Caster::_cast<std::string>;

	return;

}

}  // namespace drain


// Drain
