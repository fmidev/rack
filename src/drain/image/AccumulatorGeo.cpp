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

#include "drain/util/Rectangle.h"
#include "AccumulatorGeo.h"



namespace drain
{

namespace image
{

std::ostream & AccumulatorGeo::toStream(std::ostream & ostr) const {
//std::ostream &operator<<(std::ostream &ostr, const drain::image::AccumulatorGeo & acc){
	ostr << "AccumulatorGeo ";
	GeoFrame::toStream(ostr);
	//AccumulationArray::toStream(ostr);
	// ostr << (const GeoFrame &)acc;
	// ostr << (const drain::image::AccumulationArray &)acc << " allocated (" << this->accArray.getGeometry() << ") \n";
	ostr << " allocated (" << this->accArray.getGeometry() << ") \n";
	return ostr;
}
/*
std::ostream &operator<<(std::ostream &ostr, const drain::image::AccumulatorGeo & acc){
	ostr << "AccumulatorGeo (frame " << acc.getFrameWidth() << 'x' << acc.getFrameHeight() << ") \n";
	ostr << (const drain::image::AccumulationArray &)acc << '\n';
	ostr << "   allocated (" << acc.getWidth() << 'x' << acc.getHeight() << ") \n";
	//ostr << "   resolution, metres: " << acc.getScaleX() << 'x' << acc.getScaleY() << "\n";
	ostr << "   coord system: " << acc.getCoordinateSystem() << '\n';
	ostr << "   proj:  " << acc.getProjection() << '\n';

	ostr << "   scope, metres:  [" << acc.getBoundingBoxNat() << "]\n";
	ostr << "   scope, radians: [" << acc.getBoundingBoxRad() << "]\n";
	ostr << "   scope, degrees: [" << acc.getBoundingBoxDeg() << "]\n";

	return ostr;
}
*/


} // namespace image
} // namespace drain



// Drain
