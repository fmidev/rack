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

#ifndef DRAIN_VARIABLE
#define DRAIN_VARIABLE VariableT<VariableInitializer<VariableBase> >

#include <drain/Sprinter.h>
#include "drain/VariableBase.h"
#include "drain/VariableT.h"


namespace drain {


/// Value container supporting dynamic type.
/**
 *   drain::Variable has memory of its own, and cannot be linked to external variables like drain::Reference and drain::FlexibleVariable .
 *
 *
\section ctors-var Variable constructors

\htmlinclude  VariableT-ctors-Variable.html

\section assignments-var Variable assignments

\htmlinclude  VariableT-assign-Variable.html
 *
 *   See the documentation of drain::VariableT template specialized by drain::Variable, drain::Reference and drain::FlexibleVariable .
 *
 *   \see VariableT
 *   \see Reference
 *   \see FlexibleVariable
 */
typedef VariableT<VariableInitializer<VariableBase> >  Variable;

DRAIN_TYPENAME(Variable);

/*
template <>
const std::string TypeName<Variable>::name;
*/

template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const Variable & v, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &) v, layout);
};


}


#define   VariableLike Variable
#include "VariableLike.inc"
#undef    VariableLike



#endif /* VARIABLE_H_ */

// Drain
