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

