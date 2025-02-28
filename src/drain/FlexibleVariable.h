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


#ifndef DRAIN_FLEXIBLE_VARIABLE
#define DRAIN_FLEXIBLE_VARIABLE

#include "ReferenceT.h"
#include "VariableT.h"
#include "Variable.h"

namespace drain {

/// Value container supporting dynamic type with own memory and optional linking (referencing) of external variables.
/**
 *   drain::FlexibleVariable combines properties of drain::Variable and drain::Reference : it has memory of its own but
 *   can also be linked to external variables.
 *
 *
\section ctors-flexvar FlexibleVariable constructors

\htmlinclude  VariableT-ctors-FlexibleVariable.html

\section assignments-flexvar FlexibleVariable assignments

\htmlinclude  VariableT-assign-FlexibleVariable.html


\subsection Variable Variable
 *
 *
 *   See the combined review of drain::Variable, drain::Reference and drain::FlexibleVariable in the documentation of template drain::VariableT .
 *
 *   \see VariableT
 *   \see Reference
 *   \see Variable
 */

typedef VariableT<VariableInitializer<ReferenceT<VariableBase> > >  FlexibleVariable;



/// Redefine for "ambivalent" FlexibleVariable: indicate if local or ref
template <>
void FlexibleVariable::info(std::ostream & ostr) const;

template <>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const FlexibleVariable & v, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &) v, layout);
};


DRAIN_TYPENAME(FlexibleVariable);


}  // namespace drain



#define   VariableLike FlexibleVariable
#include "VariableLike.inc"
#undef    VariableLike


#endif

// Drain
