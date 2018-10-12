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


namespace drain
{
namespace image
{



/// A helper class applied by FloodFillOp and SegmentAreaOp
/**
 *   \tparam S  - storage type of the source image (int by default, but should be floating-type, if src is).
 *   \tparam D - storage type of the destination image, the image to be filled.
 *
 *   \author Markus.Peura@fmi.fi
 */
template <class S, class D>
class SegmentProber {

public:

	typedef S src_t;
	typedef D dst_t;

	SegmentProber(const Channel &s) :
		src(s), dst(NULL),
		handler(s.getWidth(), s.getHeight(), s.getCoordinatePolicy()) { //, mout(getImgLog(), __FUNCTION__) {
		size = 0;
	};

	SegmentProber(const Channel &s, Channel &d) :
		src(s), dst(&d),
		handler(s.getWidth(), s.getHeight(), s.getCoordinatePolicy()) { //, mout(getImgLog(), __FUNCTION__) {
		size = 0;
	};

	virtual
	~SegmentProber(){};

	/// Set new target channel. Needed in multichannel operations.
	void setDst(Channel & d){
		dst = &d;
	}

	/// Called after src and dst have been set, but before processing. See clear().
	virtual
	void init(){

		drain::Logger mout("SegmentProber", __FUNCTION__);
		src.adjustCoordinateHandler(handler);
		mout.debug() << handler << mout.endl;
		mout.debug(1) <<  src << mout.endl;
		mout.debug(1) << *dst << mout.endl;

	}

	void probe(D fillValue = 1){
		init();
		for (size_t i=0; i<src.getWidth(); ++i)
			for (size_t j=0; j<src.getHeight(); ++j)
				probe(i,j, fillValue);
	}

	/// Fills the segment having constant intensity, that is, src.at(i0,j0).
	void probe(int i, int j, D fillValue = 1){
		const S anchor = src.get<S>(i,j);
		probe(i, j, fillValue, anchor, anchor);
	};

	/// Fills the segment having intensity between min and max.
	void probe(int i, int j, D fillValue, S min, S max){

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

		clear();

		// _stack = 0;
		// mout.debug(30) << *this << mout.endl;
		if (handler.validate(i, j))
			probe4(i,j);

	};

	S  anchorMin;
	S  anchorMax;

	mutable
	D value;  // "fill value", also dynamically changing visit marker?


	mutable size_t size; // Consider deriving SegmentProber => SegmentAreaProber


// consider protected:

	const Channel & src;
	Channel *dst;

	CoordinateHandler2D handler;

protected:

	/// Application dependent. Assumes checked coordinates.
	virtual inline
	bool isValidSegment(int i, int j){
		return (src.get<src_t>(i,j) >= anchorMin) && (src.get<src_t>(i,j) <= anchorMax);
	}

	/// Application dependent. Assumes checked coordinates.
	virtual inline
	bool isValidMove(int i0, int j0, int i, int j){
		return true;
	}


	///  Application dependent operation performed in each segment location (i,j).
	virtual	inline
	void update(int i, int j){

		++size;

		Logger mout("SegmentProber", __FUNCTION__);

		if (mout.isDebug(10)){
			static size_t counter = 0;
			if (size > counter){
				counter = size + 1000;
				mout.note() << size << mout.endl;
				if (mout.isDebug(20)){
					std::stringstream sstr;
					sstr << "SegmentProber-";
					sstr.width(3);
					sstr.fill('0');
					sstr << (counter/1000) << ".png";
					drain::image::FilePng::write(*dst, sstr.str());
				}
			}

		}



	}

	/// Visiting a single pixel when not coming from any direction.
	/**
	 *  Visiting means
	 *  - updating the status of this prober, for example by updating statistics like segment size
	 *  - marking the pixel visited in dst image
	 *
	 *  \param i0 - current i coordinate (always valid)
	 *  \param j0 - current j coordinate (always valid)
	 *
	 */
	virtual inline
	void visit(int i, int j) {

		update(i, j);

		/// Mark visited
		dst->put(i,j, value);

	}

protected:

	/// Called before processing each segment. Compare with init(), which is called once for each image.
	/**
	 *  Application dependent initialisation for statistics updated with update(int i, int j) function.
	 */
	virtual
	void clear(){
		size = 0;
	};


	/// Experimental
	/**
	 */
	virtual inline
	bool isVisited(int i, int j){
		//return (dst->get<D>(i,j) == value);
		return (dst->get<D>(i,j) != 0); // consider src_t::min()
	}



	bool moveHorz(int i0, int j0, int i, int j) {

		if ((i < 0) || (i > handler.getXMax()))
			return false;

		if (isVisited(i, j))
			return false;

		return isValidSegment(i, j) && isValidMove(i0,j0, i, j);

	}





    /// A semi-recursive approach that turns the 2D recursion to 1D traversal + 1D recursion.
	/**
	 *
	 *   the horizontal direction is handled sequentially whereas the vertical direction handled recursively.
	 *  Compared to fully recursive approach, this technique preserves the speed but offers smaller consumption of memory.
	 */
	void probe4(int i, int j) {

		if (isVisited(i,j))
			return; // false;

		if (!isValidSegment(i, j))
			return; // false;

		visit(i,j); // mark & update

		/// Scan right
		int iMax = i;
		while (moveHorz(iMax,j, iMax+1,j)){
			++iMax;
			visit(iMax, j);
		}
		// Now i2 is the maximum valid i coordinate.

		/// Scan left
		while (moveHorz(i,j, i-1,j)){
			--i;
			visit(i, j);
		}
		// Now i is the minimum valid i coordinate.

		/// Scan again, continuing one step above and below.
		int i2, j2;
		while (i <= iMax){
			i2 = i;
			j2 = j-1;
			if (handler.validate(i2, j2))
				if (isValidMove(i,j, i2,j2))
					probe4(i2,j2);
			i2 = i;
			j2 = j+1;
			if (handler.validate(i2, j2))
				if (isValidMove(i,j, i2,j2))
					probe4(i2,j2);
			// if (isValidMove(i,j, i,j+1))
			// probe4(i,j+1);
			++i;
		}

	}

};

//template <class T, class T2>



template <class S, class D>
std::ostream & operator<<(std::ostream & ostr, const SegmentProber<S,D> & prober){
	ostr << "value="     << (float)prober.value << ',';
	ostr << "anchorMin=" << (float)prober.anchorMin << ',';
	ostr << "anchorMax=" << (float)prober.anchorMax << ',';
	ostr << "size="      << (float)prober.size << ',';
	//ostr << "width="     << (float)prober.width << ',';
	//ostr << "height="    << (float)prober.height << ',';
	ostr << "handler="   << prober.handler << ',';
	//ostr << "p="         << prober.p;
	return ostr;
}


} // image::
} // drain::

#endif /* SEGMENT_PROBER_H_ */



