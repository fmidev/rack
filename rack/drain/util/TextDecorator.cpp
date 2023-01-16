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

//#include <sstream>
#include <ostream>
#include <stdexcept>

//#include "Log.h"

#include "TextDecorator.h"


namespace drain
{


template <>
const drain::FlaggerDict drain::EnumDict<TextDecorator::Style>::dict = {
		{"ITALIC", TextDecorator::ITALIC},
		{"BOLD", TextDecorator::BOLD},
		{"DIM", TextDecorator::DIM},
		{"INVERSE", TextDecorator::INVERSE},
//		{"", TextDecorator::},
};

template <>
const drain::FlaggerDict drain::EnumDict<TextDecorator::Colour>::dict = {
		{"BLACK", TextDecorator::BLACK},
		{"GRAY", TextDecorator::GRAY},
		{"WHITE", TextDecorator::WHITE},
		{"RED",   TextDecorator::RED},
		{"GREEN", TextDecorator::GREEN},
		{"YELLOW", TextDecorator::YELLOW},
		{"BLUE",  TextDecorator::BLUE},
		{"CYAN",  TextDecorator::CYAN},
		{"PURPLE", TextDecorator::PURPLE}
		//		{"", TextDecorator::}
};

template <>
const drain::FlaggerDict drain::EnumDict<TextDecorator::Line>::dict = {
		{"NO_LINE", TextDecorator::NO_LINE},
		{"UNDERLINE", TextDecorator::UNDERLINE},
		{"DOUBLE_UNDERLINE", TextDecorator::DOUBLE_UNDERLINE},
		{"OVERLINE", TextDecorator::OVERLINE},
//		{"", TextDecorator::},
};


void TextDecorator::debug(std::ostream & ostr) const {
	style.debug(ostr);
	ostr << '\n';
	color.debug(ostr);
	ostr << '\n';
	line.debug(ostr);
	ostr << '\n';
}

void TextDecorator::addKey(const std::string & key){
	if (style.getDict().hasKey(key)){
		style.add(key);
	}
	if (color.getDict().hasKey(key)){
		color.set(key);
	}
	if (line.getDict().hasKey(key)){
		line.set(key);
	}
}

/**
 *
 *
 *
 */


// void TextDecorator::




} // drain::
