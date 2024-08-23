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
#ifndef DRAIN_SEGMENT_OP_H
#define DRAIN_SEGMENT_OP_H

#include <math.h>

//#include "drain/util/Cloner.h"

#include "drain/util/FunctorBank.h"
#include "drain/image/SegmentProber.h"

#include "ImageOp.h"
//#include "FloodFillOp.h"

namespace drain
{
namespace image
{

/// A base class for computing statistics of segments. As segment is an area of connected pixels.
/**
 *
 */
class SegmentOp: public ImageOp {

public:

	Range<double> intensity;
	// double min;  // TODO: instead consider criterion method inside(i,j)
	// double max;

	bool clearDst;

	std::string functorStr;  // consider internal split util


protected:

	inline
	SegmentOp(const std::string & name, const std::string & description) :
		ImageOp(name, description),
		intensity(0,0),
		clearDst(true),
		unicloner(getFunctorBank()),
		functor(identityFtor)
	{
	}

	SegmentOp(const std::string & name, const std::string & description, const drain::UnaryFunctor & ftor) :
		ImageOp(name, description + " ftor=" + ftor.getName()),
		intensity(0,0),
		clearDst(true),
		unicloner(getFunctorBank()),
		functor(ftor) {
	}

	mutable
	UniCloner<UnaryFunctor> unicloner;

	/// Retrieve functor to process the result: either the functor defined as functorStr or member functor (identity by default).
	/**
	 *  \param scale - default scaling, possibly overridden by functor parameters
	 *  \param bias  - default bias, possibly overridden by functor parameters
	 */
	inline
	const UnaryFunctor & getFunctor(double scale = 0.0, double bias = 0.0) const {

		// Consider: this could be generalized? Or unicloner needs scope?

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		if (functorStr.empty()){
			// Use my functor.
			return this->functor;
		}
		else {
			// Override with a cloned functor instance.
			std::string functorName;
			std::string functorParams;
			drain::StringTools::split2(this->functorStr, functorName, functorParams, ":");
			if (unicloner.bank.has(functorName)){
				UnaryFunctor & f =  unicloner.getCloned(functorName); // drain::getFunctor(functorStr, ':');
				if (scale > 0.0){
					f.setScale(scale, bias);
				}
				f.setParameters(functorParams,'=',':');
				mout.debug3(" ftor: " , f );
				// mout.warn("cftor: " , (const UnaryFunctor &)f );
				return f;
			}
			else {
				mout.fail("Functor '" , functorName , "' not found. Available functors:" );
				//std::cerr << unicloner.bank
				mout.note("Available functors:\n" , unicloner.bank );
				//static const SprinterLayout layout("\n", "<?>", "\0=;", "''");
				//Sprinter::sequenceToStream(std::cerr, unicloner.bank, layout);
				mout.warn("using default ftor: " , this->functor );
				return this->functor;
			}
		}
	}

protected:

	const drain::IdentityFunctor identityFtor;
	const drain::UnaryFunctor & functor;




};


} // namespace image

} // namespace drain

#endif

// Drain
