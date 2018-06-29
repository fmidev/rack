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
#ifndef SEGMENT_PROBER_H
#define SEGMENT_PROBER_H

#include <sstream>
#include <ostream>
#include "Coordinates.h"
#include "FilePng.h"

//#include "ImageOp.h"


namespace drain
{
namespace image
{



/// A helper class applied by FloodFillOp and SegmentAreaOp
/**
 *   \tparam T  - storage type of the source image (int by default, but should be floating-type, if src is).
 *   \tparam T2 - storage type of the destination image, the image to be filled.
 *
 *   \author Markus.Peura@fmi.fi
 */
template <class T, class T2>
class SegmentProber {  // TODO: rename to SegmentProber

public:

	typedef T  src_t;
	typedef T2 dst_t;

	SegmentProber(const Channel &s) :
		// src(s), width(s.getWidth()), height(s.getHeight()), dst(NULL),
		// handler(width, height, s.getCoordinatePolicy()), mout(getImgLog(), "SegmentProber") {
		src(s), dst(NULL),
		handler(s.getWidth(), s.getHeight(), s.getCoordinatePolicy()), mout(getImgLog(), __FUNCTION__) {
		size = 0;
	};

	SegmentProber(const Channel &s, Channel &d) :
		src(s), dst(&d),
		handler(s.getWidth(), s.getHeight(), s.getCoordinatePolicy()), mout(getImgLog(), __FUNCTION__) {
		// src(s), width(s.getWidth()), height(s.getHeight()), dst(&d),
		// handler(width, height, s.getCoordinatePolicy()), mout(getImgLog(), __FUNCTION__) {
		size = 0;
	};

	virtual
	~SegmentProber(){};

	void setDst(Channel & d){
		dst = &d;
	}
	/*
	 FloodFillProber(const Image &s, Image &d, const CoordinateHandler &handler) :
		 src(s), width(src.getWidth()), height(src.getHeight()), dst(d), handler(handler) {
	 };
	 */

	/// Fills the segment having constant intensity, that is, src.at(i0,j0).
	void probe(size_t i, size_t j, T2 fillValue){
		const T anchor = src.get<T>(i,j);
		probe(i, j, fillValue, anchor, anchor);
	};

	/// Fills the segment having intensity between min and max.
	void probe(size_t i, size_t j, T2 fillValue, T min, T max){

		/*
		if (fillValue > min){
			Logger mout(getImgLog(), "SegmentProber", __FUNCTION__);
			mout.error() << "markerValue (" << fillValue << ") > min(" << min << ")" << mout.endl;
			return;
		}
		*/

		value = fillValue;
		anchorMin = min;
		anchorMax = max;

		//size = 0;
		clear();

		//_stack = 0;
		//mout.debug(30) << *this << mout.endl;

		probe8(i,j);

	};

	T  anchorMin;
	T  anchorMax;
	T2 value;  // fill

	// Skk
	mutable size_t size; // Consider deriving SegmentProber => SegmentAreaProber


// TODO protected:

	const Channel & src;
	//const int width;
	//const int height;
	Channel *dst;
	//const
	CoordinateHandler2D handler;

protected:

	drain::Logger mout;


	virtual
	void clear(){
		size = 0;
	};

	//mutable long int _stack;

	/*
	void test_8(unsigned int i,unsigned int j) const {
		if ((_stack&255)==0)
			std::cerr << _stack << '\t';
		_stack++;
		probe8(i,j);
		_stack--;
	}
	*/




	/// Operation performed in each segment location (i,j). A function to be redefined in derived classes.
	virtual
	inline
	void update(int i, int j){
		++size;
	}

	//
	virtual
	inline
	bool visit(int i0, int j0) {

		//static
		int i, j;
		i=i0;
		j=j0;

		// Outside image?  NOTE: may change (i.j).
		if ((handler.handle(i,j) & CoordinateHandler2D::IRREVERSIBLE) != 0 )
		//if (! handler.validate(i, j))
			return false;

		// Outside segment?
		if (src.get<T>(i,j) < anchorMin)
			return false;
		if (src.get<T>(i,j) > anchorMax)
			return false;

		// Already visited?
		if (dst->get<T2>(i,j) == value)
			return false;

		/// NEW, it's here.
		dst->put(i,j, value);
		update(i, j);

		return true;
		/*
		if (!_check(i2,j2))
			return false;
		else {
			++size;
			return true;
		}*/

	}


    /// A semi-recursive approach that turns the 2D recursion to 1D traversal + 1D recursion.
	/**
	 *
	 *   the horizontal direction is handled sequentially whereas the vertical direction handled recursively.
	 *  Compared to fully recursive approach, this technique preserves the speed but offers smaller consumption of memory.
	 */
	void probe8(int i, int j) {

		if (!visit(i,j))
			return;

		/// NOTE: visits can overflow, depends on coordPolicy!

		/// Scan right
		int i2 = i+1;
		while (visit(i2,j))
			++i2;

		/// Scan left
		--i;
		while (visit(i,j))
			--i;

		/// Scan again, continuing one step above and below.
		++i;
		while (i < i2){
			probe8(i,j+1);
			probe8(i,j-1);
			++i;
		}

		// _stack--;
	}

};

//template <class T, class T2>



template <class T, class T2>
std::ostream & operator<<(std::ostream & ostr, const SegmentProber<T,T2> &floodFill){
	ostr << "value="     << (float)floodFill.value << ',';
	ostr << "anchorMin=" << (float)floodFill.anchorMin << ',';
	ostr << "anchorMax=" << (float)floodFill.anchorMax << ',';
	ostr << "size="      << (float)floodFill.size << ',';
	//ostr << "width="     << (float)floodFill.width << ',';
	//ostr << "height="    << (float)floodFill.height << ',';
	ostr << "handler="   << floodFill.handler << ',';
	//ostr << "p="         << floodFill.p;
	return ostr;
}



}
}
#endif /* FLOODFILL_H_ */


// Drain
