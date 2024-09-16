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
#ifndef EDGE_TRACKER_H
#define EDGE_TRACKER_H

#include <list>

#include "CoordinateHandler.h"
#include "Direction.h"
#include "ImageChannel.h"
#include "ProbingCriteria.h"
// #include "FilePng.h"

namespace drain
{

namespace image
{

typedef std::list<drain::image::Position> Contour;

/// Container for parameters of SegmentProber
/**
 *
 *   \tparam S - storage type of the source image data (int by default, but should be floating-type, if src is).
 *   \tparam D - storage type of the destination image data
 *
 *   \author Markus.Peura@fmi.fi
 */

//typedef std::list<drain::Point2D<int> > Contour;


/// A recursive method for visiting pixels of a segment in an image.
/**
 *   A helper class applied by FloodFillOp and SegmentAreaOp.
 *
 *   \tparam S - storage type of the source image data (int by default, but should be floating-type, if src is).
 *   \tparam D - storage type of the destination image data
 *   \tparam C - configuration type,
 *
 *   \author Markus.Peura@fmi.fi
 */
template <class S, class D> //, class C=ProberCriteria! >
class EdgeTracker {

public:


	EdgeTracker(const Channel & src, ProberControl & cr) : proberCriteria(cr), src(src){
		init();
		proberCriteria.controlImage.setGeometry(src.getGeometry().area);
	}

	virtual
	~EdgeTracker(){};

	typedef std::vector<drain::Point2D<short int> > step_t;

	static
	const step_t steps;

	typedef S src_t;
	typedef D dst_t;



	bool ok(){
		return true;
	}

	/**
	 *  \param
	 *  \param
	 *  \param dir - Entry direction (from outside towards inside).
	 *  \param Contour - resulting edge coordinates
	 */
	void track(const Channel & anchor, Channel & mark, const PositionTuple & startPos, Direction::value_t dir, Contour & contour){

		CoordinateHandler2D handler(src.getGeometry(), src.getCoordinatePolicy());

		/*
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		const CoordinatePolicy & cp = handler.getPolicy();
		*/

		// Turn around (180deg)
		const Direction::value_t dirOrig = DIR_TURN_DEG(dir, 180);

		// Turn a bit right (45deg)
		dir = DIR_TURN_DEG(dirOrig, 45);

		PositionTuple pos(startPos);
		contour.push_front(startPos);


		// Idea: try to move, and if not possible, turn clockwise.
		// end when initial pos and dir (about to be) repeated.
		while ((dir != dirOrig) && (pos != startPos)){

			Direction::value_t d = dir;

			const Position & offset = Direction::offset.find(dir)->second;

			Position posNext(pos.i+offset.i, pos.j+offset.j);

			if (handler.handle(posNext)){
			}

			dir = DIR_TURN_DEG(d, 45);


		}


	}

	/*
	void setParams(const C & conf){
		this->conf = conf;
	}
	*/

	/// Called after src and dst have been set, but before processing. See clear().
	virtual
	void init(){

		/*
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		handler.set(src.getGeometry(), src.getCoordinatePolicy());
		// src.adjustCoordinateHandler(handler);
		mout.debug(handler );
		mout.debug2(src );
		mout.debug2(*dst );
		*/
	}


	/// A convenience function for traversing a whole image.
	/**
	 *  Applicable in cases where probing parameters are not spatically ("dynamically") changing.
	void scan(){
	}
	 *
	 */


	// consider protected:
	ProberControl & proberCriteria;

	// This could be in proberCriteria, but is in (Super)Prober, inherited from SegmentProber
	//CoordinateHandler2D handler;

	const Channel & src;




};


//template <class S, class D>
//const typename EdgeTracker<S,D>::step_t steps = {{0,0}, {1,0}};


template <class S, class D>
std::ostream & operator<<(std::ostream & ostr, const EdgeTracker<S,D> & prober){
	return ostr;
}

} // image::

} // drain::

#endif /* SEGMENT_PROBER_H_ */



