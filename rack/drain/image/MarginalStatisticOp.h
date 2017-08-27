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
#ifndef MARGINALSTATISTICOP_H_
#define MARGINALSTATISTICOP_H_

#include "math.h"

#include "util/Histogram.h"


#include "ImageOp.h"


namespace drain
{

namespace image
{

/// Computes horizontal or vertical intensity statistics: average, sum, ...
/**

 \code
   drainage gray.png --marginStat horz,asmdvNX,0.50 -o marginStat.png
 \endcode

 \~exec
   convert marginStat.png -filter point -resize 500x360\! marginStatLarge.png
 \~
 */
class MarginalStatisticOp : public ImageOp
{
public:

	/// Default constructor.
	/**
	 *   \par mode - "horz" or "vert"
	 *   \par stat - a sequence of letters, referring to statistics through Histogram::get(const char &key) .
	 */
	MarginalStatisticOp(std::string mode="horz", std::string stat="asmdvNX", float medianPos=0.50) :
		ImageOp("Marginal","Computes statistics on <horz> or <vert> lines: average,sum,median,stdDev,variance,miN,maX") {
		parameters.reference("mode", this->mode = mode);
		parameters.reference("stat", this->stat = stat);
		parameters.reference("medianPos", this->medianPos = medianPos);
	};

	/// Unweighted computation
	inline
	void filter(const Image &src, Image &dst) const {
		filter(src, Image(), dst);
	}

	/// Unweighted computation
	void filter(const Image &src, const Image &weight, Image &dst) const;

	std::string mode;
	std::string stat;
	float medianPos;


};

}
}

#endif /*MARGINALSTATISTICOP_H_*/

// Drain
