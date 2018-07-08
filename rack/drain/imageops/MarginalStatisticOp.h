/*

    Copyright 2001 - 2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
   drainage gray.png --marginStat horz,asmdvNX,0.50 --resize 100,270 -o marginStat.png
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
	MarginalStatisticOp(std::string mode="horz", std::string stat="asmdvNX", float medianPos=0.50) :
		ImageOp("Marginal","Computes statistics on <horz> or <vert> lines: average,sum,median,stdDev,variance,miN,maX") {
		parameters.reference("mode", this->mode = mode);
		parameters.reference("stat", this->stat = stat);
		parameters.reference("medianPos", this->medianPos = medianPos);
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
