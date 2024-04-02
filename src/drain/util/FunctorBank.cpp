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
#include <drain/Log.h>
#include <cmath>

#include <vector>
#include <iostream>
#include <stdexcept>
#include <map>

#include "FunctorBank.h"

#include "Fuzzy.h"
#include "FunctorPack.h"


namespace drain
{


FunctorBank & getFunctorBank() { // who uses this (limited) set?

	/// Shared functor bank
	static FunctorBank functorBank;

	if (functorBank.getMap().empty()){

		// Double: perhaps first retrieved with "", but then cloned with key "IdentityFunctor".
		functorBank.add<IdentityFunctor>();
		functorBank.add<IdentityFunctor>("");

		functorBank.add<ScalingFunctor>("Scale");
		functorBank.add<RemappingFunctor>("Remap");
		functorBank.add<ThresholdFunctor>();
		functorBank.add<BinaryThresholdFunctor>();

		functorBank.add<FuzzyStep<double> >('s');
		functorBank.add<FuzzyTriangle<double> >();
		functorBank.add<FuzzyBell<double> >('b');
		functorBank.add<FuzzyBell2<double> >();
		functorBank.add<FuzzyStepsoid<double> >();
		functorBank.add<FuzzySigmoid<double> >();
		functorBank.add<FuzzyTwinPeaks<double> >();

	}

	return functorBank;

}

UnaryFunctor & getFunctor(const std::string & nameAndParams, char separator) {

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL getLog(), __FUNCTION__);

	std::string name;
	std::string params;

	size_t i = nameAndParams.find(':');
	if (i == std::string::npos){
		name = nameAndParams;
	}
	else {
		name   = nameAndParams.substr(0, i);
		params = nameAndParams.substr(i+1);
		//i = nameAndParams.size();
	}

	//const std::string name(nameAndParams.substr(0, i));
	//const std::string params(nameAndParams.substr(i+1));

	mout.debug3("functor name: " , name , ", params: " , params );

	FunctorBank & functorBank = getFunctorBank();
	if (!functorBank.has(name)){
		mout.error("functor not found: " , name );
	}

	UnaryFunctor & ftor = functorBank.get(name);  // or clone() ?
	mout.debug3(ftor.getName() , ',' , ftor.getDescription() );

	ftor.setParameters(params, '=', separator);
	mout.debug(ftor.getParameters() );

	return ftor;
}

}


// Drain
