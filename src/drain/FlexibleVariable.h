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

/*
#include <typeinfo>
#include <stdexcept>
#include <iostream>

#include "Convert.h"
#include "Referencer.h"
#include "SmartMap.h"
#include "Sprinter.h"

#include "VariableLike.h"
*/

#ifndef DRAIN_FLEXIBLE_VARIABLE
#define DRAIN_FLEXIBLE_VARIABLE

#include "ReferenceT.h"
#include "VariableT.h"
#include "Variable.h"

namespace drain {

/// Value container supporting dynamic type with own memory and optional linking (referencing) of external variables.
/**
 *   drain::Variable has memory of its own, and cannot be linked to external variables.
 *
 *   This class combines Reference and Variable, illustrated below
 *
 *   \copydoc drain::VariableT
 *
 *   \see Reference
 *   \see Variable
 *   \see FlexibleVariable
 */

typedef VariableT<ReferenceT<Variable> >  FlexibleVariable;

// Override for "ambivalent" FlexibleVariable
template <>
void FlexibleVariable::info(std::ostream & ostr) const;

}  // namespace drain

#define   VariableLike FlexibleVariable
#include "VariableLike.inc"
#undef    VariableLike


#endif

// Drain
