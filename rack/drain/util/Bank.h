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
#ifndef DRAIN_BANK_H_
#define DRAIN_BANK_H_

//
#include <iostream>
#include <map>

#include "Log.h"
#include "Registry.h"
#include "Cloner.h"


namespace drain
{

/// A registry that contains items that can be cloned with clone() or referenced directly with get().
template <class T>
class Bank : public Registry<ClonerBase<T> > {
};

//template <class T>
//std::ostream & operator<<(std::ostream & ostr, const Bank<T> & bank);

/*
template <class T>
std::ostream & operator<<(std::ostream & ostr, const Bank<T> & bank){
	for (typename Bank<T>::map_t::const_iterator it = bank.getMap().begin(); it != bank.getMap().end(); ++it){
		const T & obj = it->second.get();
		ostr << it->first << ' ' << '(' << obj.getName() << ')' << ':' << ' ' << obj.getDescription() << '\n';
		ostr << '\t' << obj.getParameters() << '\n';
	}
	return ostr;
}
*/

}

#endif

// Drain
