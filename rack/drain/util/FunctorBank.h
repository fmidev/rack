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
#ifndef DRAIN_FUNCTOR_BANK_H_
#define DRAIN_FUNCTOR_BANK_H_

//
#include <cmath>

#include <vector>
#include <iostream>
#include <stdexcept>
#include <map>

#include "Debug.h"


//#include "Registry.h"
//#include "Cloner.h"
#include "Functor.h"
#include "Fuzzy.h"
#include "Bank.h"

// using namespace std;

namespace drain
{


//typedef Registry<ClonerBase<UnaryFunctor> > UnaryFunctorCloner;
//typedef Registry<ClonerBase<UnaryFunctor> > FunctorBank;
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


static
inline
FunctorBank & getFunctorBank() {

	/// Shared functor bank
	static FunctorBank functorBank;

	if (functorBank.getMap().empty()){

		typedef UnaryFunctor UF;
		static FunctorCloner<UF, IdentityFunctor> identity;
		functorBank.add(identity, "");

		static FunctorCloner<UF, FuzzyStep<double> > step;
		functorBank.add(step, step.getName(), 's');

		static FunctorCloner<UF, FuzzyTriangle<double> > triangle;
		functorBank.add(triangle, triangle.getName());

		static FunctorCloner<UF, FuzzyBell<double> > bell;
		functorBank.add(bell, bell.getName(), 'b');

		static FunctorCloner<UF, FuzzyBell2<double> > bell2;
		functorBank.add(bell2, bell2.getName());

		static FunctorCloner<UF, FuzzyStepsoid<double> > stepsoid;
		functorBank.add(stepsoid, stepsoid.getName());

		static FunctorCloner<UF, FuzzySigmoid<double> > sigmoid;
		functorBank.add(sigmoid, sigmoid.getName());

	}

	return functorBank;

}

static
inline
UnaryFunctor & getFunctor(const std::string & key) {

	FunctorBank & functorBank = getFunctorBank();
	return functorBank.get(key).clone();

}


}


#endif

// Drain
