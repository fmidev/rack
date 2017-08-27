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
	putBool   = & Caster::putToVoidNEW<bool>;
	putInt    = & Caster::putToVoidNEW<int>;
	putChar   = & Caster::putToVoidNEW<char>;
	putUChar  = & Caster::putToVoidNEW<unsigned char>;
	putUInt   = & Caster::putToVoidNEW<unsigned int>;
	putShort  = & Caster::putToVoidNEW<short>;
	putUShort = & Caster::putToVoidNEW<unsigned short>;
	putLong   = & Caster::putToVoidNEW<long int>;
	putULong  = & Caster::putToVoidNEW<unsigned long>;
	putFloat  = & Caster::putToVoidNEW<float>;
	putDouble = & Caster::putToVoidNEW<double>;

	// NEW
	getBool   = & Caster::getFromVoidNEW<bool>;
	getInt    = & Caster::getFromVoidNEW<int>;
	getChar   = & Caster::getFromVoidNEW<char>;
	getUChar  = & Caster::getFromVoidNEW<unsigned char>;
	getUInt   = & Caster::getFromVoidNEW<unsigned int>;
	getShort  = & Caster::getFromVoidNEW<short>;
	getUShort = & Caster::getFromVoidNEW<unsigned short>;
	getLong   = & Caster::getFromVoidNEW<long>;
	getULong  = & Caster::getFromVoidNEW<unsigned long>;
	getFloat  = & Caster::getFromVoidNEW<float>;
	getDouble = & Caster::getFromVoidNEW<double>;

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
	putBool   = & Caster::putToStringNEW<bool>;
	putInt    = & Caster::putToStringNEW<int>;

	putChar   = & Caster::putToStringNEW<char>;
	putUChar  = & Caster::putToStringNEW<unsigned char>;
	// putInt    = & Caster::_putToString<int>;
	putUInt   = & Caster::putToStringNEW<unsigned int>;
	putShort  = & Caster::putToStringNEW<short>;
	putUShort = & Caster::putToStringNEW<unsigned short>;
	putLong   = & Caster::putToStringNEW<long int>;
	putULong  = & Caster::putToStringNEW<unsigned long>;
	putFloat  = & Caster::putToStringNEW<float>;
	putDouble = & Caster::putToStringNEW<double>;

	getBool   = & Caster::getFromStringNEW<bool>;
	getInt    = & Caster::getFromStringNEW<int>;

	getChar   = & Caster::getFromStringNEW<char>;
	getUChar  = & Caster::getFromStringNEW<unsigned char>;
	//getInt    = & Caster::_getFromString<int>;
	getUInt   = & Caster::getFromStringNEW<unsigned int>;
	getShort  = & Caster::getFromStringNEW<short>;
	getUShort = & Caster::getFromStringNEW<unsigned short>;
	getLong   = & Caster::getFromStringNEW<long>;
	getULong  = & Caster::getFromStringNEW<unsigned long>;
	getFloat  = & Caster::getFromStringNEW<float>;
	getDouble = & Caster::getFromStringNEW<double>;

	toOStreamP = & Caster::_toOStream<std::string>;
	castP      = & Caster::_cast<std::string>;

	return;

}

}  // namespace drain


// Drain
