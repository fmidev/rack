/**

    Copyright 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

