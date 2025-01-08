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

	//int visitedMarker = 255;

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
	SuperProber(const Channel &s, Channel &d) : SegmentProber<int,int,SuperProberConf>(s, d), size(0) {
		//control.markerImage.set
	};


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
	void visit2(const Position & pos) {
		update(pos);
		control.markVisited(pos);
	}

	/*
	virtual inline
	void markVisited(const Position & pos) const {
		return dst->put(pos.i, pos.j, conf.visitedMarker);
	}
	 */

	// DEPRECATING
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
	bool isNewSegment(const Position & pos, Direction::value_t dir) const {
		return this->conf.anchor.contains(src.get<src_t>(pos.i, pos.j));
		//return (dst->get<dst_t>(pos.i, pos.j) == conf.visitedMarker);
	}


	bool checkNext(Position & pos, Direction::value_t dir, TreeSVG & tree){

		drain::Logger mout(__FILE__, __FUNCTION__, __LINE__);

		/* No use, offset + handler is not much more expensive
		if (DIR == Direction::NONE){
		}
		*/
		//mout.warn("check", pos, " -> ", dir);
		mout.warn(); // "check", pos, " -> ", dir);
		// , Direction::dict.getValue(dir), ' ', '<', dir , '>'
		mout << "check" << pos << ' ' << Direction::arrows.getValue(dir) << " <" << dir<< ">";

		// Position pos2(pos);
		switch (control.move(pos, dir)){
		case ProberControl::MOVE_ACCEPTED:
			if (!isNewSegment(pos,dir)){
				// Default = continue probing...
				mout << " mark" << pos;
				//visit2(pos);
				control.markVisited(pos);
				update(pos);
				// dst->put(pos.i, pos.j, dir);
				// update(pos);
				mout << mout.endl;
				return true;
			}
			else {

				if (dir != Direction::NONE){
					// New segment!
					mout << " NEW SEG! " << mout.endl;
					EdgeTracker<int,int> edgeTracker(src, control);
					edgeTracker.track(pos, dir);
					if (edgeTracker.contour.size() > 0){
						TreeSVG & contour = tree[drain::StringBuilder<'-'>("cont", pos.i, pos.j)](svg::POLYGON);
						FlexibleVariable & c = contour->get("points");
						c.setType(typeid(std::string));
						c.setSeparator(0);
						for (const Position & p: edgeTracker.contour){
							c << p.i << ',' << p.j << ' ';
						}
						// contour->set("points", drain::sprinter(edgeTracker.contour, drain::Sprinter::plainLayout));
						/*
						TreeSVG & segment = tree[drain::StringBuilder<'-'>("seg",pos.i,pos.j)](NodeSVG::CIRCLE); // (NodeSVG::POLYGON);
						segment->set("cx", pos.i);
						segment->set("cy", pos.j);
						segment->set("r", 5);
						 */
						// <polygon points="100,10 150,190 50,190" style="fill:lime;stroke:purple;stroke-width:3" />
					}
					dst->put(pos.i, pos.j, 192);
				}
				else {
					mout << " {none} ";
					dst->put(pos.i, pos.j, 85);
				}
				// Scan EDGE!
				//return true;
				// mout << mout.endl;
				//return false;
			}
			break;
		case ProberControl::COORD_ERROR:
			mout << " COORD_ERROR ";
			// return false;
			break;
		case ProberControl::DIR_ERROR:
			mout << " DIR_ERROR ";
			// return false;
			break;

		}

		mout << mout.endl;

		/*
		const Position & offset = Direction::offset.find(dir)->second;
		pos.add(offset);

		mout.debug() << "checkNext: " << pos;

		if (!control.handler.handle(pos)){
			mout << " (OK)";
			if (!control.isVisited(pos)){
			}
			else {
				mout << " (already visited)"  << pos.i << ',' << pos.j;
			}
		}
		else {
			mout << " (OUT)";
		}
		mout << mout.endl;
		 */

		return false;
	}


	// void probe2(const Position & pos, Direction::value_t DIR, TreeSVG & tree){
	void probe2(Position pos, Direction::value_t dir, TreeSVG & tree){

		// TODO: use rotating direction DIR  (old idea)

		// Position pos2(pos);
		Position p;

		while (checkNext(pos, dir, tree)){

			// tree->get("points") << " " << pos2.i << ','  << pos2.j;
			if (dir == Direction::NONE){
				dir = Direction::RIGHT;
				continue;
			}

			//Position
			p = pos;
			//dst->put(posSide.i, posSide.j, DIR);

			if (checkNext(p, DIR_TURN_DEG(dir, 90), tree)){ /// TODO: Macro turn -> test -> complete with template
				//dst->put(posSide.i, posSide.j, 64);
				std::cout << "090" << " = " << p.i << ',' << p.j << '\n';
				probe2(p, dir, tree); //["+90fwd"](NodeSVG::POLYGON)); // recursion
				probe2(p, DIR_TURN_DEG(dir, 180), tree); //["+90inv"](NodeSVG::POLYGON));
			}

			p = pos;
			if (checkNext(p, DIR_TURN_DEG(dir, 270), tree)){
				//dst->put(posSide.i, posSide.j, 192);
				std::cout << "270" << " = " << p.i << ',' << p.j << '\n';
				probe2(p, dir, tree); // ["-90pos"](NodeSVG::POLYGON)); // recursion
				probe2(p, DIR_TURN_DEG(dir, 180), tree); // ["-90inv"](NodeSVG::POLYGON));
			}

		}

		//return false;
	};


};


} // image::

} // drain::

#endif /* SUPER_PROBER_H_ */



