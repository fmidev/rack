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

	typedef unsigned char marker_t;

	virtual inline
	~ProberControl(){};

	/// NEW
	/**
	 */
	virtual inline
	bool isVisited(const Position & pos) const {
		//return (markerImage.get<int>(pos.i, pos.j) > 0); //== visitedMarker); // > 0
		return (markerImage.at(pos.i, pos.j) > 0); //== visitedMarker); // > 0
	}

	virtual inline
	void markVisited(const Position & pos){
		//markerImage.put(pos.i, pos.j, 255);
		markerImage.at(pos.i, pos.j) = visitedMarker;
	}

	/*
	virtual inline
	void mark(const Position & pos, marker_t m){
		markerImage.at(pos.i, pos.j) |= m;
		//markerImage.put(pos.i, pos.j, markerImage.get<int>(pos.i, pos.j) | m);
	}
	*/

	/// Disable \b exit \b from position \c pos towards direction \c dir.
	/**
	 *  Mark blocking for this position(i,j).
	 *
	 *  \param pos - value parameter forwarded to direction dir
	 *  \param dir - direction to be inverted for marking
	 *
	 */
	void markBlockedIn(const Position & pos, Direction::value_t dir){
		markerImage.at(pos.i, pos.j) |= dir;
	}

	/// Disable \b entry \b to position \c pos from direction \c dir \e inverted .
	/**
	 *  Mark inverse blocking for \c "pos+dir", with dir inverted 180 degrees.
	 *
	 *  \param pos - value parameter forwarded to direction dir
	 *  \param dir - direction to be inverted for marking
	 *  \return - \c true in success, \c false upon coordinate overflow
	 *
	 */
	bool markBlockedOut(Position pos, Direction::value_t dir){
		pos.add(Direction::offset.find(dir)->second);
		if (handler.handle(pos.i, pos.j)){
			return false;
		}
		else {
			markerImage.at(pos.i, pos.j) |= DIR_TURN_DEG(dir, 180);
			return true;
		}
	}

	///   Direction NONE is always valid,
	/**
	 *
	 */
	virtual inline
	bool isValidDir(const Position & pos, Direction::value_t dir) const {
		return (dir & markerImage.at(pos.i, pos.j)) == 0;
		//return (markerImage.get<int>(pos.i, pos.j) | dir) != 0;
	}

	virtual
	bool isValidPixel(const Channel & src, const Position & pos) const = 0;


	typedef enum {MOVE_ACCEPTED=0, COORD_ERROR=1, DIR_ERROR=2} move_status;


	virtual inline
	move_status move(Position & pos, Direction::value_t dir) const {

		Position pos2(pos);
		pos2.add(Direction::offset.find(dir)->second);

		if (!handler.handle(pos2)){ // Warning: in POLAR/WRAP coordinates, also dir should change.
			if (isValidDir(pos2, dir)){ // includes visited = invalid from all directions
				pos = pos2;
				return MOVE_ACCEPTED;
			}
			else {
				return DIR_ERROR;
			}
		}
		else {
			return COORD_ERROR;
		}

	}

	/*
	virtual inline
	bool move(Position & pos, Direction::value_t dir) const {

		Position posNext(pos);
		posNext.add(Direction::offset.find(dir)->second);

		if (!handler.handle(posNext)){
			if (isValidDir(posNext, dir)){
				pos = posNext;
				return true; //OK;
			}
			else {
				return false; // DIR_ERROR;
			}
		}
		else {
			return false; // COORD_ERROR;
		}

	}
	*/


	// This could be in proberCriteria, but is in (Super)Prober, inherited from SegmentProber
	CoordinateHandler2D handler;


	ImageT<marker_t> markerImage;
	//Image markerImage;

	marker_t visitedMarker = 0xff;

};

struct SimpleProberControl : public ProberControl {

	int threshold = 128;

	virtual
	bool isValidPixel(const Channel & src, const Position & pos) const {
		return src.get<int>(pos.i, pos.j) > threshold;
	};

	// int threshold = 128;

	/// Old, deprecating
	/*
	virtual inline
	bool isValidPixel(int i) const override {
		return i > threshold;
	};

	/// Old, deprecating
	virtual inline
	bool isVisited(int i) const override {
		return (i == visitedMarker);
	};
	*/

	/*
	virtual inline
	bool isVisited(const Position & pos) const {
		return controlImage.at(pos.i, pos.j) != visitedMarker;
	};
	*/


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



