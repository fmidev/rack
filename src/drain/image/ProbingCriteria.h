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
#ifndef PROBING_CRITERIA_H
#define PROBING_CRITERIA_H

#include <list>

#include "Direction.h"
#include "ImageChannel.h"
#include "ImageT.h"
#include "CoordinateHandler.h"
// #include "FilePng.h"

namespace drain
{

namespace image
{


/// Container for parameters of SegmentProber
/**
 *
 *   \tparam S - storage type of the source image data (int by default, but should be floating-type, if src is).
 *   \tparam D - storage type of the destination image data
 *
 *   \author Markus.Peura@fmi.fi
 */

//typedef std::list<drain::Point2D<int> > Contour;

struct ProberControl {

	virtual inline
	bool isValidPixel(int i) const = 0;

	virtual inline
	bool isVisited(const Position & pos) const {
		return controlImage.at(pos.i, pos.j) != visitedMarker;
	};

	virtual inline
	~ProberControl(){};

	// This could be in proberCriteria, but is in (Super)Prober, inherited from SegmentProber
	CoordinateHandler2D handler;

	ImageT<unsigned char> controlImage;

	unsigned char visitedMarker = 0xff;

};

struct SimpleProrolberControl : public ProberControl {

	int threshold = 128;

	virtual inline
	bool isValidPixel(int i) const {
		return i > threshold;
	};

	virtual inline
	bool isVisited(int i) const {
		return (i == visitedMarker);
	};


};


/*
template <class S, class D>
std::ostream & operator<<(std::ostream & ostr, const EdgeTracker<S,D> & prober){
	return ostr;
}
*/


} // image::

} // drain::

#endif /* PROBER_CRITERIA_H_ */



