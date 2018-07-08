/**

    Copyright 2017 -   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
