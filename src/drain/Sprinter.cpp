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



#include "Sprinter.h"


namespace drain
{


TypeLayout & TypeLayout::setLayout(const char *layout){

	switch (::strlen(layout)){
	case 3: // "(,)" = prefix, separator, suffix
		set(layout[0], layout[1], layout[2]); // notice order
		break;
	case 2: // "{}" =  prefix,suffix
		set(layout[0], ',', layout[1]);
		break;
	case 1: // "," = separator (resetting parentheses)
		set(0, layout[0], 0);
		break;
	case 0: // NOT A DEFAULT
		set(0, 0, 0);
		//set('{', ',', '}'); // ok?
		break;
	default:
		throw std::runtime_error("TypeLayout unsupported layout (over 3 chars)");
	}

	// std::cout << layout << '(' << ::strlen(layout) << ')' << *this << '\n';
	return *this;
}

const SprinterLayout Sprinter::UNSET_LAYOUT = {"","","",""};

const SprinterLayout Sprinter::defaultLayout  = {"[,]", "{,}", "(,)", ""};

const SprinterLayout Sprinter::plainLayout = {",", ",", ",", ""};

const SprinterLayout Sprinter::emptyLayout = {"","","",""};

const SprinterLayout Sprinter::lineLayout = {",", "\n", "=", ""};

const SprinterLayout Sprinter::cppLayout("{,}", "{,}", "{,}", "\"\"", "\"\""); // cppLayout("{,}", "{,}", "{,}", "\"\"");

const SprinterLayout Sprinter::pythonLayout("[,]", "{,}",  "(,)", "\"\"", "''", ":"); // last ':' means plain map entries (not tuples as in C++)

const SprinterLayout Sprinter::cmdLineLayout = {":", ",", "=", ""};

/// JSON layout uses std::pair<>(first,second) to display key:value pairs.
/**
 *   # arrays  as [...]
 *   # objects as {...}, starting from the arg
 *   # pairs   as key:value
 *   # strings as "..."
 */
const SprinterLayout Sprinter::jsonLayout("[,]", "{,}", ":", "\"\"", "\"\"");

/// Like JSON layout, but keys without hyphens.
const SprinterLayout Sprinter::jsLayout("[,]", "{,}", ":", "\"\"", "");

///
const SprinterLayout Sprinter::xmlAttributeLayout("[,]", " ", "=", "", "");



} // namespace drain


