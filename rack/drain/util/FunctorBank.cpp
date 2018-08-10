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


//
#include <cmath>

#include <vector>
#include <iostream>
#include <stdexcept>
#include <map>

#include "Log.h"
#include "FunctorBank.h"



#include "Fuzzy.h"


// using namespace std;

namespace drain
{


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

UnaryFunctor & getFunctor(const std::string & nameAndParams, char separator) {

	drain::Logger mout(getLog(), __FUNCTION__);


	size_t i = nameAndParams.find(':');
	if (i == std::string::npos)
		i = nameAndParams.size();

	const std::string name(nameAndParams.substr(0, i));
	const std::string params(nameAndParams.substr(i+1));

	mout.debug(2) << "functor name: " << name << ", params: " << params << mout.endl;

	FunctorBank & functorBank = getFunctorBank();
	if (!functorBank.has(name)){
		mout.error() << "functor not found: " << name << mout.endl;
	}

	UnaryFunctor & ftor = functorBank.get(name).get();  // or clone() ?
	mout.debug(2) << ftor.getName() << ',' << ftor.getDescription() << mout.endl;

	ftor.setParameters(params, '=', separator);
	mout.debug() << ftor.getParameters() << mout.endl;

	return ftor;
}

}


// Drain
