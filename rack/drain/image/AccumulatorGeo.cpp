/*

    Copyright 2001 - 2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include "util/Rectangle.h"
#include "AccumulatorGeo.h"



namespace drain
{

namespace image
{


std::ostream &operator<<(std::ostream &ostr, const drain::image::AccumulatorGeo & acc){
	ostr << "AccumulatorGeo (frame " << acc.getFrameWidth() << 'x' << acc.getFrameHeight() << ") \n";
	ostr << (const drain::image::AccumulationArray &)acc << '\n';
	ostr << "   allocated (" << acc.getWidth() << 'x' << acc.getHeight() << ") \n";
	//ostr << "   resolution, metres: " << acc.getScaleX() << 'x' << acc.getScaleY() << "\n";
	ostr << "   coord system: " << acc.getCoordinateSystem() << '\n';
	ostr << "   proj:  " << acc.getProjection() << '\n';

	ostr << "   scope, metres:  [" << acc.getBoundingBoxM() << "]\n";
	ostr << "   scope, radians: [" << acc.getBoundingBoxR() << "]\n";
	ostr << "   scope, degrees: [" << acc.getBoundingBoxD() << "]\n";

	return ostr;
}


} // namespace image
} // namespace drain


