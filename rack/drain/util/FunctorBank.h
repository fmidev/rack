/**

    Copyright 2016 -   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef DRAIN_FUNCTOR_BANK_H_
#define DRAIN_FUNCTOR_BANK_H_

//

#include "Log.h"


//#include "Registry.h"
//#include "Cloner.h"
#include "Functor.h"
//#include "Fuzzy.h"
#include "Bank.h"

// using namespace std;

namespace drain
{

// Note: there is probably need for Bank<BinaryFunctor>, too.
typedef Bank<UnaryFunctor> FunctorBank;


template <class T, class T2>
class FunctorCloner : public Cloner<T,T2> {
public:

	static
	const std::string & getName(){
		static T2 ftor;
		return ftor.getName();
	}

};


extern
FunctorBank & getFunctorBank();

/// Returns functor the parameters of which have been set.
/**
 *   \param separator -
 */
extern
UnaryFunctor & getFunctor(const std::string & nameAndParams, char separator=',');

/*static inline
UnaryFunctor & getFunctor(const std::string & key) {

	FunctorBank & functorBank = getFunctorBank();
	return functorBank.get(key).clone();

}
*/


}


#endif
