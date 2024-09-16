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
#ifndef SUPER_PROBER_H
#define SUPER_PROBER_H


#include <drain/image/EdgeTracker.h>
#include <drain/PseudoTuple.h>

#include "Direction.h"
#include "FilePng.h"
#include "ImageFile.h"

#include "SegmentProber.h"
#include "TreeSVG.h"


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

struct SuperProberConf {

	Range<int> anchor;

	int visitedMarker = 255;

};



/// A DIRECTED recursive method for visiting pixels of a segment in an image.
/**
 *   A helper class applied by FloodFillOp and SegmentAreaOp.
 *
 *   \tparam S - storage type of the source image data (int by default, but should be floating-type, if src is).
 *   \tparam D - storage type of the destination image data
 *   \tparam C - configuration type,
 *
 *   \author Markus.Peura@fmi.fi
 */
class SuperProber : public SegmentProber<int,int,SuperProberConf> {
	// SegmentProber<int,int,SegmentProberConf<int, int> > {

public:

	size_t size = 0;


	//CoordinateHandler2D handler;

	// SuperProber(const Channel &s, Channel &d) : SegmentProber<int,int,SegmentProberConf<int, int> >(s, d), size(0) {};
	SuperProber(const Channel &s, Channel &d) : SegmentProber<int,int,SuperProberConf>(s, d), size(0) {};


	// Document the base?
	virtual inline
	void clear(){
		size = 0;
	};

	///  Application dependent operation performed in each segment location (i,j).
	virtual	inline
	void update(const Position & pos){
		++size;
	}

	virtual inline
	void visit(const Position & pos) {
		update(pos);
		dst->put(pos.i, pos.j, conf.visitedMarker);
	}


	/// Experimental
	/**
	 */
	virtual inline
	bool isVisited(const Position & pos) const {
		return (dst->get<dst_t>(pos.i, pos.j) > 0);
		//return (dst->get<dst_t>(pos.i, pos.j) == conf.visitedMarker);
	}

	/*
	virtual inline
	void markVisited(const Position & pos) const {
		return dst->put(pos.i, pos.j, conf.visitedMarker);
	}
	*/


	virtual // place holder
	bool isValidSegment(int i, int j) const {
		return true;
	};

	/// Application dependent. Assumes checked coordinates.
	/**
	 *	\param pos - new position (validated coordinates)
	 *  \param dir - direction of entry (from outside to inside [potential new segment])
	 *
	 *  \return - true, if this pixel belongs to a new segment, and false if probing can continue
	 */
	virtual inline
	bool checkNewSegment(const Position & pos, Direction::value_t dir) const {
		return this->conf.anchor.contains(src.get<src_t>(pos.i, pos.j));
		//return (dst->get<dst_t>(pos.i, pos.j) == conf.visitedMarker);
	}


	bool checkNext(Position & pos, Direction::value_t DIR, TreeSVG & tree){

		const Position & offset = Direction::offset.find(DIR)->second;

		pos.i += offset.i;
		pos.j += offset.j;

		if (!proberControl.handler.handle(pos.i, pos.j)){
			if (!isVisited(pos)){
				if (!checkNewSegment(pos,DIR)){ // <-- note: this may start new segment !
					visit(pos);
					// update(pos);
					return true;
				}
				else {
					TreeSVG & segment = tree[drain::StringBuilder<'-'>("seg",pos.i,pos.j)](NodeSVG::CIRCLE); // (NodeSVG::POLYGON);
					// TreeSVG & segment = tree["seg"](NodeSVG::CIRC); // (NodeSVG::POLYGON);
					segment->set("cx", pos.i);
					segment->set("cy", pos.j);
					segment->set("r", 5);
					//<polygon points="100,10 150,190 50,190" style="fill:lime;stroke:purple;stroke-width:3" />
				}
			}
		}

		return false;
	}


	void probe2(const Position & pos, Direction::value_t DIR, TreeSVG & tree){

		Position pos2(pos);

		Position posSide;

		while (checkNext(pos2, DIR, tree)){

			posSide = pos2;
			// if (checkNext(posSide, Direction::turn90(DIR), tree)){ /// TODO: Macro turn -> test -> complete with template
			if (checkNext(posSide, DIR_TURN_090(DIR), tree)){ /// TODO: Macro turn -> test -> complete with template
				probe2(posSide, DIR, tree); // recursion
				probe2(posSide, DIR_TURN_180(DIR), tree);
			}

			posSide = pos2;
			if (checkNext(posSide, DIR_TURN_270(DIR), tree)){
				probe2(posSide, DIR, tree); // recursion
				probe2(posSide, DIR_TURN_180(DIR), tree);
			}

		}

		//return false;
	};


};


} // image::

} // drain::

#endif /* SUPER_PROBER_H_ */



