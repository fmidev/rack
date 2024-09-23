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

#include <drain/image/ProbingControl.h>
#include <list>

#include "CoordinateHandler.h"
#include "Direction.h"
#include "ImageChannel.h"
#include "ImageFile.h"

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

	Position pos;
	Direction::value_t dir = Direction::NONE;

	Position startPos;
	Direction::value_t startDir = Direction::NONE;

	// PositionTuple pos;
	// Direction::value_t dir = Direction::NONE;

	EdgeTracker(const Channel & src, ProberControl & control) : control(control), src(src){
		init();
		control.markerImage.setGeometry(src.getGeometry().area);
	}

	virtual
	~EdgeTracker(){};

	typedef S src_t;
	typedef D dst_t;

	// typedef std::vector<drain::Point2D<short int> > step_t;
	// static
	// const step_t steps;

	Contour contour;

	void setStart(const Position & startPos, Direction::value_t startDir){
		pos = startPos;
		dir = startDir;
		this->startPos = startPos;
		this->startDir = startDir;
	};

	//ProberControl::move_status next(){
	bool next(){

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		mout.accept<LOG_WARNING>("From ", pos, " to: ", Direction::arrows[dir], ' ', '<', dir , '>');

		Position p = pos;

		if (control.move(p, dir) == ProberControl::MOVE_ACCEPTED){ // p changes in this condition
			if (control.isValidPixel(src, p)){
				mout.accept<LOG_WARNING>(p);
				pos = p;
				contour.push_back(pos);
				dir = DIR_TURN_DEG(dir, 225);
				/* if (Direction::isDiagonal(dir)){
							Position p(startPos.i, pos.j);
							control.markBlocked(pos, dir+90deg)
						}
				 */
				//control.markBlockedOut(startPos,  DIR_TURN_DEG(dir, -90));
			}
			else {
				mout.pending<LOG_WARNING>(p, '>', dir, "->", pos, " outside segment ");
				// control.markBlockedIn(pos0, dir);
				dir = DIR_TURN_DEG(dir, 45);
			}
		}
		else {
			mout.reject<LOG_WARNING>(p, '>', dir, "->", pos, " illegal move ");
			dir = DIR_TURN_DEG(dir, 45);
		}
		// control.markBlocked(pos, dir); // NOT NEEDED...

		return (! (dir == startDir) && (pos == startPos) );

		//while ((dir != startDir) && (pos != startPos));

	}


	/**
	 *  \param
	 *  \param startPos - starting position (i,j)
	 *  \param startDir -
	 *  \param Contour - resulting edge coordinates
	 *
	 *  Note: \c dir is NOT Entry direction (from outside towards inside).
	 */
	void track(const Position & startPos, Direction::value_t startDir){

		/*
		*/
		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		//const Position startPos(pos);
		//const Direction::value_t startDir(dir);

		mout.accept<LOG_WARNING>("Start ", pos, ':', Direction::arrows[dir], ' ', '<', dir , '>');


		setStart(startPos, startDir);

		while (next()){
			mout.accept<LOG_WARNING>("New edge pos: ", pos);
		}
		/*
		Position p;

		// Idea: try to move, and if not possible, turn clockwise.
		do {

			p = pos;
			if (control.move(p, dir) == ProberControl::MOVE_ACCEPTED){ // Note: pos was changed in this condition
				if (control.isValidPixel(src, p)){
					mout.accept<LOG_WARNING>(p);
					pos = p;
					contour.push_back(pos);
					dir = DIR_TURN_DEG(dir, 225);
					//control.markBlockedOut(startPos,  DIR_TURN_DEG(dir, -90));
				}
				else {
					mout.pending<LOG_WARNING>(p, '>', dir, "->", pos, " outside segment ");
					// control.markBlockedIn(pos0, dir);
					dir = DIR_TURN_DEG(dir, 45);
				}
			}
			else {
				mout.reject<LOG_WARNING>(p, '>', dir, "->", pos, " illegal move ");
				dir = DIR_TURN_DEG(dir, 45);
			}
			// control.markBlocked(pos, dir); // NOT NEEDED...


		}
		while (! (dir == startDir) && (pos == startPos) );
		*/

		mout.note("Writing debug image");

		ImageFile::write(control.markerImage, "edge.png");
		exit(0);

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
	ProberControl & control;

	// This could be in control, but is in (Super)Prober, inherited from SegmentProber
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



