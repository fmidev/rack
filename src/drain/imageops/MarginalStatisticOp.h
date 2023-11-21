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

#include "drain/util/Histogram.h"


#include "ImageOp.h"


namespace drain
{

namespace image
{

/// Computes horizontal or vertical intensity statistics: iAverage, sum, ...
/**

 \code
   drainage image-gray.png --iMarginStat horz,asmdvNX,0.50 --iResize 100,270 -o marginStat.png
 \endcode

 */
class MarginalStatisticOp : public ImageOp
{
public:

	/// Default constructor.
	/**
	 *   \par mode - "horz" or "vert"
	 *   \par stat - a sequence of letters, referring to statistics through Histogram::get(const char &key) .
	 */
	MarginalStatisticOp(const std::string & mode = "horz", const std::string & stat="asmdvNX", float medianPos=0.50) :
		ImageOp("MarginStat","Computes statistics on <horz> or <vert> lines: iAverage,sum,median,stdDev,variance,miN,maX") {
		parameters.link("mode", this->mode = mode);
		parameters.link("stat", this->stat = stat);
		parameters.link("medianPos", this->medianPos = medianPos);
	};

	/// Unweighted computation
	inline
	void process(const ImageFrame &src, Image &dst) const {
		process(src, Image(), dst);
	}

	/// Unweighted computation
	void process(const ImageFrame &src, const ImageFrame &weight, Image &dst) const;

	std::string mode;
	std::string stat;
	float medianPos;


};

}
}

#endif /*MARGINALSTATISTICOP_H_*/

// Drain
